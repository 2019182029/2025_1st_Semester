#include <iostream>
#include <chrono>

#include <WS2tcpip.h>
#include <MSWSock.h>

#include <mutex>
#include <thread>

#include <array>
#include <queue>
#include <vector>
#include <unordered_set>
#include <concurrent_unordered_map.h>
#include <concurrent_unordered_set.h>

#include <sqlext.h>  
#include <locale.h>

#include "protocol.h"

#include "include/lua.hpp"

#pragma comment(lib, "WS2_32.lib")
#pragma comment(lib, "MSWSock.lib")
#pragma comment(lib, "lua54.lib")

using namespace std;

HANDLE h_iocp;

constexpr int MOVE_COUNT = 3;
constexpr int VIEW_RANGE = 5;

#define NAME_LEN 50  
#define PHONE_LEN 60

enum COMP_TYPE { OP_ACCEPT, OP_RECV, OP_SEND, OP_PLAYER_LOGIN, OP_PLAYER_MOVE, OP_NPC_MOVE };
class OVER_EXP {
public:
	WSAOVERLAPPED _over;
	WSABUF _wsabuf;
	char _send_buf[BUF_SIZE];
	COMP_TYPE _comp_type;
	int _ai_target_obj;
	OVER_EXP()
	{
		_wsabuf.len = BUF_SIZE;
		_wsabuf.buf = _send_buf;
		_comp_type = OP_RECV;
		ZeroMemory(&_over, sizeof(_over));
	}
	OVER_EXP(char* packet)
	{
		_wsabuf.len = packet[0];
		_wsabuf.buf = _send_buf;
		ZeroMemory(&_over, sizeof(_over));
		_comp_type = OP_SEND;
		memcpy(_send_buf, packet, packet[0]);
	}
};

enum EVENT_TYPE { EV_MOVE, EV_HEAL, EV_ATTACK };
struct event_type {
	int obj_id;
	chrono::high_resolution_clock::time_point wakeup_time;
	EVENT_TYPE event_id;
	int target_id;

	constexpr bool operator < (const event_type& _Left) const
	{
		return (wakeup_time > _Left.wakeup_time);
	}
};

mutex timer_lock;
priority_queue<event_type> timer_queue;

enum QUERY_TYPE { QU_LOGIN, QU_UPDATE_COORDINATE };
struct query_type {
	int obj_id;
	chrono::high_resolution_clock::time_point wakeup_time;
	QUERY_TYPE query_id;
	int login_id;

	constexpr bool operator < (const query_type& _Left) const
	{
		return (wakeup_time > _Left.wakeup_time);
	}
};

mutex query_lock;
queue<query_type> query_queue;

class SESSION;

atomic<int> g_new_id = 0;
concurrency::concurrent_unordered_map<int, std::atomic<std::shared_ptr<SESSION>>> clients;
concurrency::concurrent_unordered_set<int> client_login_ids;

enum S_STATE { ST_FREE, ST_ALLOC, ST_INGAME, ST_CLOSE };
class SESSION {
	OVER_EXP _recv_over;

public:
	mutex _s_lock;
	S_STATE _state;
	int _id;
	int _login_id;
	SOCKET _socket;
	short	x, y;
	char	_name[NAME_SIZE];
	int		_prev_remain;
	unordered_set <int> _view_list;
	mutex	_vl;
	long long last_move_time;
	atomic<bool> _is_active;
	lua_State* _L;
	mutex	_ll;

public:
	SESSION()
	{
		_id = -1;
		_socket = 0;
		x = y = 0;
		_name[0] = 0;
		_state = ST_FREE;
		_prev_remain = 0;
	}

	~SESSION() {}

	void do_recv()
	{
		DWORD recv_flag = 0;
		memset(&_recv_over._over, 0, sizeof(_recv_over._over));
		_recv_over._wsabuf.len = BUF_SIZE - _prev_remain;
		_recv_over._wsabuf.buf = _recv_over._send_buf + _prev_remain;
		WSARecv(_socket, &_recv_over._wsabuf, 1, 0, &recv_flag,
			&_recv_over._over, 0);
	}

	void do_send(void* packet)
	{
		OVER_EXP* sdata = new OVER_EXP{ reinterpret_cast<char*>(packet) };
		WSASend(_socket, &sdata->_wsabuf, 1, 0, 0, &sdata->_over, 0);
	}
	void send_login_info_packet()
	{
		SC_LOGIN_INFO_PACKET p;
		p.id = _id;
		p.size = sizeof(SC_LOGIN_INFO_PACKET);
		p.type = SC_LOGIN_INFO;
		p.x = x;
		p.y = y;
		strcpy_s(p.name, _name);
		do_send(&p);
	}
	void send_login_fail_packet()
	{
		SC_LOGIN_FAIL_PACKET p;
		p.size = sizeof(SC_LOGIN_FAIL_PACKET);
		p.type = SC_LOGIN_FAIL;
		do_send(&p);

		closesocket(_socket);

		{
			lock_guard<mutex> ll(_s_lock);
			_state = ST_CLOSE;
		}

		clients.at(_id) = nullptr;
		client_login_ids.unsafe_erase(_login_id);
	}
	void send_move_packet(int c_id);
	void send_add_player_packet(int c_id);
	void send_chat_packet(int c_id, const char* mess);
	void send_remove_player_packet(int c_id)
	{
		_vl.lock();
		if (_view_list.count(c_id))
			_view_list.erase(c_id);
		else {
			_vl.unlock();
			return;
		}
		_vl.unlock();
		SC_REMOVE_OBJECT_PACKET p;
		p.id = c_id;
		p.size = sizeof(p);
		p.type = SC_REMOVE_OBJECT;
		do_send(&p);
	}

	void wake_up(int player_id) {
		bool expected = false;

		if (std::atomic_compare_exchange_strong(&_is_active, &expected, true)) {
			OVER_EXP* eo = new OVER_EXP;

			eo->_comp_type = OP_PLAYER_MOVE;
			eo->_ai_target_obj = player_id;

			PostQueuedCompletionStatus(h_iocp, 1, _id, &eo->_over);
		}
	}

	void sleep() {
		_is_active = false;
	}
};

mutex g_mutex[SECTOR_ROWS][SECTOR_COLS];
vector<int> g_sector[SECTOR_ROWS][SECTOR_COLS];

OVER_EXP g_a_over;
SOCKET g_s_socket, g_c_socket;

bool is_pc(int object_id)
{
	return object_id < MAX_USER;
}

bool is_npc(int object_id)
{
	return !is_pc(object_id);
}

bool can_see(int from, int to)
{
	std::shared_ptr<SESSION> client_from = clients.at(from);
	std::shared_ptr<SESSION> client_to = clients.at(to);
	if (!client_from || !client_to) return false;

	if (abs(client_from->x - client_to->x) > VIEW_RANGE) return false;
	return abs(client_from->y - client_to->y) <= VIEW_RANGE;
}

void SESSION::send_move_packet(int c_id)
{
	std::shared_ptr<SESSION> client = clients.at(c_id);
	if (nullptr == client) return;

	SC_MOVE_OBJECT_PACKET p;
	p.id = c_id;
	p.size = sizeof(SC_MOVE_OBJECT_PACKET);
	p.type = SC_MOVE_OBJECT;
	p.x = client->x;
	p.y = client->y;
	p.move_time = static_cast<unsigned int>(client->last_move_time);
	do_send(&p);
}

void SESSION::send_add_player_packet(int c_id)
{
	std::shared_ptr<SESSION> client = clients.at(c_id);
	if (nullptr == client) return;

	SC_ADD_OBJECT_PACKET add_packet;
	add_packet.id = c_id;
	strcpy_s(add_packet.name, client->_name);
	add_packet.size = sizeof(add_packet);
	add_packet.type = SC_ADD_OBJECT;
	add_packet.x = client->x;
	add_packet.y = client->y;
	_vl.lock();
	_view_list.insert(c_id);
	_vl.unlock();
	do_send(&add_packet);
}

void SESSION::send_chat_packet(int p_id, const char* mess)
{
	SC_CHAT_PACKET packet;
	packet.id = p_id;
	packet.size = sizeof(packet);
	packet.type = SC_CHAT;
	strcpy_s(packet.mess, mess);
	do_send(&packet);
}

void update_sector(int c_id, short old_x, short old_y, short new_x, short new_y) {
	short old_sx = old_x / SECTOR_WIDTH;
	short old_sy = old_y / SECTOR_HEIGHT;

	short new_sx = new_x / SECTOR_WIDTH;
	short new_sy = new_y / SECTOR_HEIGHT;

	if ((old_sx != new_sx) || (old_sy != new_sy)) {
		{
			std::lock_guard<std::mutex> lock(g_mutex[old_sy][old_sx]);
			auto& old_list = g_sector[old_sy][old_sx];
			old_list.erase(std::remove(old_list.begin(), old_list.end(), c_id), old_list.end());
		}

		std::lock_guard<std::mutex> lock(g_mutex[new_sy][new_sx]);
		g_sector[new_sy][new_sx].emplace_back(c_id);
	}
}

void process_packet(int c_id, char* packet)
{
	std::shared_ptr<SESSION> client = clients.at(c_id);
	if (nullptr == client) return;

	switch (packet[1]) {
	case CS_LOGIN: {
		CS_LOGIN_PACKET* p = reinterpret_cast<CS_LOGIN_PACKET*>(packet);

		if (!client_login_ids.insert(p->id).second) {
			client->send_login_fail_packet();
		}

		query_lock.lock();
		query_queue.emplace(query_type{ c_id, chrono::high_resolution_clock::now() + chrono::milliseconds(10), QU_LOGIN, p->id });
		query_lock.unlock();
		break;
	}

	case CS_MOVE: {
		CS_MOVE_PACKET* p = reinterpret_cast<CS_MOVE_PACKET*>(packet);
		client->last_move_time = p->move_time;
		short old_x = client->x;
		short old_y = client->y;
		short new_x = old_x;
		short new_y = old_y;
		switch (p->direction) {
		case 0: if (old_y > 0) --new_y; break;
		case 1: if (old_y < W_HEIGHT - 1) ++new_y; break;
		case 2: if (old_x > 0) --new_x; break;
		case 3: if (old_x < W_WIDTH - 1) ++new_x; break;
		}
		client->x = new_x;
		client->y = new_y;

		// Update Sector
		update_sector(c_id, old_x, old_y, new_x, new_y);

		int sx = client->x / SECTOR_WIDTH;
		int sy = client->y / SECTOR_HEIGHT;

		// Create View List by Sector
		unordered_set<int> near_list;
		client->_vl.lock();
		unordered_set<int> old_vlist = client->_view_list;
		client->_vl.unlock();
		for (int dy = -1; dy <= 1; ++dy) {
			for (int dx = -1; dx <= 1; ++dx) {
				short nx = sx + dx;
				short ny = sy + dy;

				if (nx < 0 || ny < 0 || nx >= SECTOR_COLS || ny >= SECTOR_ROWS) {
					continue;
				}

				for (auto cl : g_sector[ny][nx]) {
					std::shared_ptr<SESSION> other = clients.at(cl);
					if (nullptr == other) continue;

					if (ST_INGAME != other->_state) continue;
					if (other->_id == c_id) continue;
					if (can_see(c_id, other->_id))
						near_list.insert(other->_id);
				}
			}
		}

		client->send_move_packet(c_id);

		for (auto& pl : near_list) {
			std::shared_ptr<SESSION> other = clients.at(pl);
			if (nullptr == other) continue;

			if (is_pc(pl)) {
				other->_vl.lock();
				if (other->_view_list.count(c_id)) {
					other->_vl.unlock();
					other->send_move_packet(c_id);
				}
				else {
					other->_vl.unlock();
					other->send_add_player_packet(c_id);
				}
			}
			else {
				other->wake_up(client->_id);
			}

			if (old_vlist.count(pl) == 0)
				client->send_add_player_packet(pl);
		}

		for (auto& pl : old_vlist) {
			std::shared_ptr<SESSION> other = clients.at(pl);
			if (nullptr == other) continue;

			if (0 == near_list.count(pl)) {
				client->send_remove_player_packet(pl);
				if (is_pc(pl)) { other->send_remove_player_packet(c_id); }
			}
		}
	}
	break;
	}
}

void disconnect(int c_id)
{
	std::shared_ptr<SESSION> client = clients.at(c_id);
	if (nullptr == client) return;

	client->_vl.lock();
	unordered_set <int> vl = client->_view_list;
	client->_vl.unlock();
	for (auto& p_id : vl) {
		std::shared_ptr<SESSION> other = clients.at(p_id);
		if (nullptr == other) continue;

		if (is_npc(p_id)) continue;
		{
			lock_guard<mutex> ll(other->_s_lock);
			if (ST_INGAME != other->_state) continue;
		}
		if (other->_id == c_id) continue;
		other->send_remove_player_packet(c_id);
	}
	closesocket(client->_socket);

	int sx = client->x / SECTOR_WIDTH;
	int sy = client->y / SECTOR_HEIGHT;

	// Delete Client from Sector
	{
		std::lock_guard<std::mutex> lock(g_mutex[sy][sx]);
		auto& v_list = g_sector[sy][sx];
		v_list.erase(std::remove(v_list.begin(), v_list.end(), c_id), v_list.end());
	}

	{
		lock_guard<mutex> ll(client->_s_lock);
		client->_state = ST_CLOSE;
	}

	clients.at(c_id) = nullptr;
	client_login_ids.unsafe_erase(client->_login_id);
}

void worker_thread(HANDLE h_iocp)
{
	while (true) {
		DWORD num_bytes;
		ULONG_PTR key;
		WSAOVERLAPPED* over = nullptr;
		BOOL ret = GetQueuedCompletionStatus(h_iocp, &num_bytes, &key, &over, INFINITE);
		OVER_EXP* ex_over = reinterpret_cast<OVER_EXP*>(over);
		if (FALSE == ret) {
			if (ex_over->_comp_type == OP_ACCEPT) cout << "Accept Error";
			else {
				cout << "GQCS Error on client[" << key << "]\n";
				disconnect(static_cast<int>(key));
				if (ex_over->_comp_type == OP_SEND) delete ex_over;
				continue;
			}
		}

		if ((0 == num_bytes) && ((ex_over->_comp_type == OP_RECV) || (ex_over->_comp_type == OP_SEND))) {
			disconnect(static_cast<int>(key));
			if (ex_over->_comp_type == OP_SEND) delete ex_over;
			continue;
		}

		switch (ex_over->_comp_type) {
		case OP_ACCEPT: {
			int client_id = g_new_id++;
			if (client_id != -1) {
				std::shared_ptr<SESSION> p = std::make_shared<SESSION>();
				p->x = 0;
				p->y = 0;
				p->_id = client_id;
				p->_name[0] = 0;
				p->_prev_remain = 0;
				p->_state = ST_ALLOC;
				CreateIoCompletionPort(reinterpret_cast<HANDLE>(g_c_socket), h_iocp, client_id, 0);
				p->_socket = g_c_socket;
				clients.insert(std::make_pair(client_id, p));
				p->do_recv();
				g_c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
			}
			else {
				cout << "Max user exceeded.\n";
			}
			ZeroMemory(&g_a_over._over, sizeof(g_a_over._over));
			int addr_size = sizeof(SOCKADDR_IN);
			AcceptEx(g_s_socket, g_c_socket, g_a_over._send_buf, 0, addr_size + 16, addr_size + 16, 0, &g_a_over._over);
			break;
		}

		case OP_RECV: {
			int client_id = static_cast<int>(key);

			std::shared_ptr<SESSION> client = clients.at(client_id);
			if (nullptr == client) return;

			int remain_data = num_bytes + client->_prev_remain;
			char* p = ex_over->_send_buf;
			while (remain_data > 0) {
				int packet_size = p[0];
				if (packet_size <= remain_data) {
					process_packet(client_id, p);
					p = p + packet_size;
					remain_data = remain_data - packet_size;
				}
				else break;
			}
			client->_prev_remain = remain_data;
			if (remain_data > 0) {
				memcpy(ex_over->_send_buf, p, remain_data);
			}
			client->do_recv();
			break;
		}

		case OP_SEND:
			delete ex_over;
			break;

		case OP_PLAYER_LOGIN: {
			int client_id = static_cast<int>(key);

			std::shared_ptr<SESSION> client = clients.at(client_id);
			if (nullptr == client) { break; }

			client->send_login_info_packet();

			// Add Client into Sector
			short sx = client->x / SECTOR_WIDTH;
			short sy = client->y / SECTOR_HEIGHT;

			{
				lock_guard<mutex> sl(g_mutex[sy][sx]);
				g_sector[sy][sx].emplace_back(client_id);
			}

			// Search Nearby Objects by Sector
			for (short dy = -1; dy <= 1; ++dy) {
				for (short dx = -1; dx <= 1; ++dx) {
					short nx = sx + dx;
					short ny = sy + dy;

					if (nx < 0 || ny < 0 || nx >= SECTOR_COLS || ny >= SECTOR_ROWS) {
						continue;
					}

					std::lock_guard<std::mutex> lock(g_mutex[ny][nx]);
					for (auto pl : g_sector[ny][nx]) {
						std::shared_ptr<SESSION> other = clients.at(pl);
						if (nullptr == other) continue;

						{
							lock_guard<mutex> ll(other->_s_lock);
							if (ST_INGAME != other->_state) continue;
						}

						if (other->_id == client_id) continue;
						if (false == can_see(client_id, other->_id)) continue;

						if (is_pc(other->_id)) { other->send_add_player_packet(client_id); }
						else { other->wake_up(client->_id); }

						client->send_add_player_packet(other->_id);

					}
				}
			}
			break;
		}

		case OP_PLAYER_MOVE: {
			int npc_id = static_cast<int>(key);

			std::shared_ptr<SESSION> npc = clients.at(npc_id);
			if (nullptr == npc) return;

			npc->_ll.lock();

			auto L = npc->_L;
			lua_getglobal(L, "event_player_move");
			lua_pushnumber(L, ex_over->_ai_target_obj);

			lua_pcall(L, 1, 1, 0);

			int result = (int)lua_tointeger(L, -1);

			npc->_ll.unlock();

			if (result) {
				for (int i = 0; i < MOVE_COUNT; ++i) {
					timer_lock.lock();
					timer_queue.emplace(event_type{ npc_id, chrono::high_resolution_clock::now() + chrono::seconds(i + 1), EV_MOVE, ex_over->_ai_target_obj });
					timer_lock.unlock();
				}
			}
			else {
				npc->sleep();
			}

			delete ex_over;
			break;
		}

		case OP_NPC_MOVE: {
			int npc_id = static_cast<int>(key);

			std::shared_ptr<SESSION> npc = clients.at(npc_id);
			if (nullptr == npc) return;

			npc->_ll.lock();

			auto L = npc->_L;
			lua_getglobal(L, "event_npc_move");
			lua_pushnumber(L, ex_over->_ai_target_obj);

			lua_pcall(L, 1, 1, 0);

			int result = (int)lua_tointeger(L, -1);

			npc->_ll.unlock();

			if (result) {
				npc->sleep();
			}

			delete ex_over;
			break;
		}
		}
	}
}

int API_get_x(lua_State* L)
{
	int user_id = (int)lua_tointeger(L, -1);

	std::shared_ptr<SESSION> npc = clients.at(user_id);
	if (nullptr == npc) return -1;

	lua_pop(L, 2);
	int x = npc->x;
	lua_pushnumber(L, x);

	return 1;
}

int API_get_y(lua_State* L)
{
	int user_id = (int)lua_tointeger(L, -1);

	std::shared_ptr<SESSION> npc = clients.at(user_id);
	if (nullptr == npc) return -1;

	lua_pop(L, 2);
	int y = npc->y;
	lua_pushnumber(L, y);

	return 1;
}

int API_do_npc_random_move(lua_State* L)
{
	int npc_id = (int)lua_tointeger(L, -1);
	
	std::shared_ptr<SESSION> npc = clients.at(npc_id);
	if (nullptr == npc) return 0;

	short old_x = npc->x;
	short old_y = npc->y;
	short new_x = old_x;
	short new_y = old_y;
	switch (rand() % 4) {
	case 0: if (old_y > 0) --new_y; break;
	case 1: if (old_y < W_HEIGHT - 1) ++new_y; break;
	case 2: if (old_x > 0) --new_x; break;
	case 3: if (old_x < W_WIDTH - 1) ++new_x; break;
	}
	npc->x = new_x;
	npc->y = new_y;

	// Update Sector
	update_sector(npc_id, old_x, old_y, new_x, new_y);

	int sx = npc->x / SECTOR_WIDTH;
	int sy = npc->y / SECTOR_HEIGHT;

	// Create Old View List by Sector
	unordered_set<int> old_vl;
	for (int dy = -1; dy <= 1; ++dy) {
		for (int dx = -1; dx <= 1; ++dx) {
			short nx = sx + dx;
			short ny = sy + dy;

			if (nx < 0 || ny < 0 || nx >= SECTOR_COLS || ny >= SECTOR_ROWS) { continue; }

			std::lock_guard<std::mutex> lock(g_mutex[ny][nx]);
			for (auto obj : g_sector[ny][nx]) {
				std::shared_ptr<SESSION> other = clients.at(obj);
				if (nullptr == other) continue;

				if (ST_INGAME != other->_state) continue;
				if (true == is_npc(other->_id)) continue;
				if (true == can_see(npc->_id, other->_id))
					old_vl.insert(other->_id);
			}
		}
	}

	// Create New View List by Sector
	unordered_set<int> new_vl;
	for (int dy = -1; dy <= 1; ++dy) {
		for (int dx = -1; dx <= 1; ++dx) {
			short nx = sx + dx;
			short ny = sy + dy;

			if (nx < 0 || ny < 0 || nx >= SECTOR_COLS || ny >= SECTOR_ROWS) { continue; }

			std::lock_guard<std::mutex> lock(g_mutex[ny][nx]);
			for (auto obj : g_sector[ny][nx]) {
				std::shared_ptr<SESSION> other = clients.at(obj);
				if (nullptr == other) continue;

				if (ST_INGAME != other->_state) continue;
				if (true == is_npc(other->_id)) continue;
				if (true == can_see(npc->_id, other->_id))
					new_vl.insert(other->_id);
			}
		}
	}

	for (auto pl : new_vl) {
		std::shared_ptr<SESSION> other = clients.at(pl);
		if (nullptr == other) return 0;

		if (0 == old_vl.count(pl)) {
			// 플레이어의 시야에 등장
			other->send_add_player_packet(npc->_id);
		}
		else {
			// 플레이어가 계속 보고 있음.
			other->send_move_packet(npc->_id);
		}
	}

	for (auto pl : old_vl) {
		std::shared_ptr<SESSION> other = clients.at(pl);
		if (nullptr == other) return 0;

		if (0 == new_vl.count(pl)) {
			other->_vl.lock();
			if (0 != other->_view_list.count(npc->_id)) {
				other->_vl.unlock();
				other->send_remove_player_packet(npc->_id);
			}
			else {
				other->_vl.unlock();
			}
		}
	}

	return 0;
}

int API_SendMessage(lua_State* L)
{
	int my_id = (int)lua_tointeger(L, -3);
	int user_id = (int)lua_tointeger(L, -2);
	char* mess = (char*)lua_tostring(L, -1);

	lua_pop(L, 4);

	std::shared_ptr<SESSION> client = clients.at(user_id);
	if (nullptr == client) return 0;

	client->send_chat_packet(my_id, mess);

	return 0;
}

void InitializeNPC()
{
	cout << "NPC intialize begin.\n";
	for (int i = MAX_USER; i < MAX_USER + MAX_NPC; ++i) {
		if ((i % 10'000) == 0) { 
			cout << "NPC initializing : " << (i - MAX_USER) << " initialized" << endl; 
		}

		std::shared_ptr<SESSION> p = std::make_shared<SESSION>(); 
		p->x = rand() % W_WIDTH;
		p->y = rand() % W_HEIGHT;
		p->_id = i;
		sprintf_s(p->_name, "NPC%d", i);
		p->_state = ST_INGAME;
		p->_is_active = false;

		p->_L = luaL_newstate();
		luaL_openlibs(p->_L);
		luaL_loadfile(p->_L, "npc.lua");
		lua_pcall(p->_L, 0, 0, 0);

		lua_getglobal(p->_L, "set_uid");
		lua_pushnumber(p->_L, i);
		lua_pcall(p->_L, 1, 0, 0);

		lua_register(p->_L, "API_SendMessage", API_SendMessage);
		lua_register(p->_L, "API_get_x", API_get_x);
		lua_register(p->_L, "API_get_y", API_get_y);
		lua_register(p->_L, "API_do_npc_random_move", API_do_npc_random_move);

		clients.insert(std::make_pair(p->_id, p));

		// Add Npc into Sector
		short sx = p->x / SECTOR_WIDTH;
		short sy = p->y / SECTOR_HEIGHT;
		{
			lock_guard<mutex> sl(g_mutex[sy][sx]);
			g_sector[sy][sx].emplace_back(i);
		}
	}
	cout << "NPC initialize end.\n";
}

// STEP 5 : Wake Up
void do_timer()
{
	while (true) {
		do {
			do {
				timer_lock.lock();
				if (timer_queue.empty()) {
					timer_lock.unlock();
					break;
				}

				auto& k = timer_queue.top();

				if (k.wakeup_time > chrono::high_resolution_clock::now()) {
					timer_lock.unlock();
					break;
				}
				timer_lock.unlock();

				switch (k.event_id) {
				case EV_MOVE:
					OVER_EXP* o = new OVER_EXP;
					o->_comp_type = OP_NPC_MOVE;
					o->_ai_target_obj = k.target_id;
					PostQueuedCompletionStatus(h_iocp, 0, k.obj_id, &o->_over);
					break;
				}

				timer_lock.lock();
				timer_queue.pop();
				timer_lock.unlock();
			} while (true);
			this_thread::sleep_for(chrono::milliseconds(10));
		} while (true);

	}
}

void show_error() 
{
	printf("error\n");
}

void HandleDiagnosticRecord(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode)
{
	SQLSMALLINT iRec = 0;
	SQLINTEGER  iError;
	WCHAR       wszMessage[1000];
	WCHAR       wszState[SQL_SQLSTATE_SIZE + 1];

	if (RetCode == SQL_INVALID_HANDLE) {
		fwprintf(stderr, L"Invalid handle!\n");
		return;
	}

	while (SQLGetDiagRec(hType, hHandle, ++iRec, wszState, &iError, wszMessage,
		(SQLSMALLINT)(sizeof(wszMessage) / sizeof(WCHAR)), (SQLSMALLINT*)NULL) == SQL_SUCCESS) {
		// Hide data truncated..
		if (wcsncmp(wszState, L"01004", 5)) {
			fwprintf(stderr, L"[%5.5s] %s (%d)\n", wszState, wszMessage, iError);
		}
	}
}

void do_query()
{
	SQLHENV henv = nullptr;
	SQLHDBC hdbc = nullptr;
	SQLHSTMT hstmt = nullptr;

	SQLINTEGER user_x, user_y;
	SQLLEN cb_user_x = 0, cb_user_y = 0;

	SQLRETURN retcode;

	retcode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);
	if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) goto clean_up;

	retcode = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER*)SQL_OV_ODBC3, 0);
	if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) goto clean_up;

	retcode = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);
	if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) goto clean_up;

	retcode = SQLSetConnectAttr(hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0);
	if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) goto clean_up;

	retcode = SQLConnect(hdbc, (SQLWCHAR*)L"2019182029_OBDC", SQL_NTS, (SQLWCHAR*)NULL, 0, NULL, 0);
	if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) goto clean_up;

	retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
	if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO) goto clean_up;

	// Query Loop
	while (true) {
		query_lock.lock();
		
		if (!query_queue.empty()) {
			query_type q = query_queue.front();

			if (q.wakeup_time > chrono::high_resolution_clock::now()) {
				query_lock.unlock();
				this_thread::sleep_for(chrono::milliseconds(10));
				continue;
			}

			query_queue.pop();
			query_lock.unlock();

			switch (q.query_id) {
			case QU_LOGIN: {
				std::shared_ptr<SESSION> client = clients.at(q.obj_id);
				if (nullptr == client) { break; }

				std::wstring query = L"EXEC user_login " + std::to_wstring(q.login_id);
				retcode = SQLExecDirect(hstmt, (SQLWCHAR*)query.c_str(), SQL_NTS);

				if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
					retcode = SQLBindCol(hstmt, 1, SQL_C_LONG, &user_x, PHONE_LEN, &cb_user_x);
					retcode = SQLBindCol(hstmt, 2, SQL_C_LONG, &user_y, PHONE_LEN, &cb_user_y);

					retcode = SQLFetch(hstmt);

					if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
						if ((user_x != -1) && (user_y != -1)) {
							char buf[NAME_LEN];
							sprintf_s(buf, "%d", q.login_id);
							strcpy_s(client->_name, buf);

							{
								lock_guard<mutex> ll{ client->_s_lock };
								client->_login_id = q.login_id;
								client->x = static_cast<int>(user_x);
								client->y = static_cast<int>(user_y);
								client->_state = ST_INGAME;
							}

							OVER_EXP* o = new OVER_EXP;
							o->_comp_type = OP_PLAYER_LOGIN;
							PostQueuedCompletionStatus(h_iocp, 0, q.obj_id, &o->_over);

							SQLCloseCursor(hstmt);
						} else {
							client->send_login_fail_packet();
							SQLCloseCursor(hstmt);
							break;
						}
					} else {
						HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, retcode);
						client->send_login_fail_packet();
						SQLCloseCursor(hstmt);
						break;
					}
				} else {
					HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, retcode);
					client->send_login_fail_packet();
					SQLCloseCursor(hstmt); 
					break;
				}
				break;
			}

			case QU_UPDATE_COORDINATE: {
				for (const auto& p : clients) {
					if (true == is_npc(p.first)) { continue; }

					std::shared_ptr<SESSION> client = p.second.load();
					if (nullptr == client) { continue; }

					if (ST_INGAME != client->_state) { continue; }

					std::wstring query = L"EXEC save_coor " + std::to_wstring(client->_login_id) + L", " + std::to_wstring(client->x) + L", " + std::to_wstring(client->y);
					retcode = SQLExecDirect(hstmt, (SQLWCHAR*)query.c_str(), SQL_NTS);

					if (!(retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)) {
						HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, retcode);
					}
				}

				query_lock.lock();
				query_queue.emplace(query_type{ -1, chrono::high_resolution_clock::now() + chrono::seconds(5), QU_UPDATE_COORDINATE, -1 });
				query_lock.unlock();
				break;
			}
			}
		} else {
			query_lock.unlock();
		}
		this_thread::sleep_for(chrono::milliseconds(10));
	}

clean_up:
	if (hstmt) {
		SQLCancel(hstmt);
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	}

	if (hdbc) {
		SQLDisconnect(hdbc);
		SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
	}

	if (henv) {
		SQLFreeHandle(SQL_HANDLE_ENV, henv);
	}
}

int main()
{
	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);
	g_s_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	SOCKADDR_IN server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT_NUM);
	server_addr.sin_addr.S_un.S_addr = INADDR_ANY;
	bind(g_s_socket, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
	listen(g_s_socket, SOMAXCONN);
	SOCKADDR_IN cl_addr;
	int addr_size = sizeof(cl_addr);

	InitializeNPC();

	h_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(g_s_socket), h_iocp, 9999, 0);
	g_c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	g_a_over._comp_type = OP_ACCEPT;
	AcceptEx(g_s_socket, g_c_socket, g_a_over._send_buf, 0, addr_size + 16, addr_size + 16, 0, &g_a_over._over);

	vector <thread> worker_threads;
	int num_threads = std::thread::hardware_concurrency();

	for (int i = 0; i < num_threads; ++i)
		worker_threads.emplace_back(worker_thread, h_iocp);

	thread timer_thread(do_timer);

	query_lock.lock();
	query_queue.emplace(query_type{ -1, chrono::high_resolution_clock::now() + chrono::seconds(5), QU_UPDATE_COORDINATE, -1 });
	query_lock.unlock();

	thread query_thread(do_query);

	setlocale(LC_ALL, "korean");

	query_thread.join();
	timer_thread.join();

	for (auto& th : worker_threads)
		th.join();

	closesocket(g_s_socket);
	WSACleanup();
}
