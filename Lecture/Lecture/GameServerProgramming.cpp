/*

 �� ��ũ��Ʈ

 1. ��ũ��Ʈ
    �� ����ϱ� ���� ���꼺�� ���� ���
       �� ���� : ���� ���� �ӵ�
                 ���� �� ������ ���ʿ�

          ���� : ���� ���� �ӵ�
                 ������ �÷��� ��

    �� �䱸 : ���� ���� ��
              ��ü���� ���α׷��� ����
              ��Ƽ������ ��� ����
              �� Reentrant : �ϳ��� ��ü�� ���� �����忡�� ���ÿ� ȣ���ص� �������� ����
                             �� �����庰�� ������ ���������� ��ü�� ������ ���� �ִ�.
                                ���� ��ü ������ �ƴ�, �ý����� �����ϴ� ��� �ʿ��ϴ�.
    
    �� Ư¡ : �������� ����
              �������� ����
              ���ɺ��ٴ� ǥ���� �߽�
              ����������

    �� ���ӿ����� Ȱ�� : ������ ����
                         NPC AI
                         Non-Programmer


 2. ��ũ��Ʈ ���
    �� LUA : ����ϱ� ����.
             ������ �پ��.
    
       Python : ���꼺�� ����.
                ��ġ�� ũ�� ��Ƽ������ ���ɿ� ������ �ִ�.

       Java, JavaScript : ���� ������
                          ������ �÷��� ��

       C# : C���� ȣ�� ������� ����
            ��Ƽ������ ����

       XML : �� �ƴϸ�, �ֱٿ��� JSON ���


 3. LUA
    �� Ư¡ : ������ �����, ������ ���
              Ȯ�� ������ ������ ���� ��ũ��Ʈ �� �� �������� ����
              ������ �ڷᱸ��  // boolean, number, string, table

    �� ���� : �ٽ� ����� ũ�Ⱑ 120KB ����
              ���� ���߿� ���� ���Ǵ� Python�� ���� ���� ���� �ӵ�
              ���¼ҽ�

    �� Visual Studio C++���� ���� : include ������ ������Ʈ�� �߰� 
                                    lua54.lib�� ���̺귯���� �߰�   

    �� ���� 1
       �� function factorial(n)
              if n == 0 then
                  return 1
              end
              return n * factorial(n - 1)
          end

          print (factorial(5))

          print ([multiple
                lines])

       ���� 2
       �� a_table = { x = 10 }
          print(a_table["x"])

          b_table = a_table
          a_table["x"] = 20
          print(a_table["x"])
          print(b_table["x"])

       ���� 3
       �� point = { x = 10, y = 20 }
          print(point["x"])
          print(point.x)


    �� �ǽ� 1
       �� // lua.cpp
          int main() {
              const char* buff = "print \"Hello from Lua.\"\n";

              lua_State* L = luaL_newstate();  // Open LUA
              luaL_openlibs(L);  // Open LUA Standard Library
              luaL_loadbuffer(L, buff, strlen(buff), "line");

              int error = lua_pcall(L, 0, 0, 0);
              if (error) {
                  std::cout << "Error : " << lua_tostring(L, -1);
                  lua_pop(L, 1);
              }

              lua_close(L);
          }
          �� C++���� LUA �Լ� ȣ��
             �� int lua_pcall(
                    lua_State* L,  
                    int nargs,     // �Ű����� ����
                    int nresults,  // ���ϰ� ����
                    int msgh       // 0
                );
                �� Stack�� �Ű������� �־���ƾ� �Ѵ�.
                   ������ ������ Stack���� ���ϰ��� ���´�.
                   ���� ���� ����Ǹ� 0�� �����Ѵ�.

       �ǽ� 2
       �� // exl.lua
          function plustwo(x)
          local a
          a = 2
          return x + a
          end

          pos_x = 6
          pos_y = plustwo(pos_x)

          // lua.cpp
          int main() {
              int rows, cols;

              lua_State* L = luaL_newstate();
              luaL_openlibs(L);
              luaL_loadfile(L, "exl.lua");

              lua_pcall(L, 0, 0, 0);

              lua_getglobal(L, "pos_x");
              lua_getglobal(L, "pos_y");
              rows = (int)lua_tonumber(L, -2);
              cols = (int)lua_tonumber(L, -1);

              lua_pop(L, 2);
              lua_close(L);
          }
          �� LUA�� Virtual Machine�� Stack Machine�̴�.
             �� C++�� LUA ���α׷� ���� �ڷ� ��ȯ�� Stack�� ���� �̷�����.
                �� LUA �Լ� ȣ�� �� �Ű����� Push : lua_pushnumber(L, 1);
                   Stack�� ����� �� �б� : (int)lua_tonumber(L, -1);
                   Stack�� �۷ι� ���� �� �����ϱ� : lua_getglobal(L, "rows");

       �ǽ� 3
       �� // exl.lua
          function add_num_lua(a, b)
          return c_addnum(a, b)
          end

          // lua.cpp
          int addnum_c(lua_State* L) {
              int a = (int)lua_tonumber(L, -2);
              int b = (int)lua_tonumber(L, -1);

              int result = a + b;

              lua_pop(L, 2);
              lua_pushnumber(L, result);
              return 1;
          }

          int main() {
              ...

              lua_register(L, "c_addnum", addnum_c);

              lua_getglobal(L, "addnum_lua");
              lua_pushnumber(L, 100);
              lua_pushnumber(L, 200);

              lua_pcall(L, 2, 1, 0);

              result = (int)lua_tonumber(L, -1);

              lua_pop(L, 1);
          }


 4. NPC SCRIPT ����
    �� NPC�� ������ ��ũ��Ʈ ���� ����
       �� Finite State Machine : Event Driven
                                 ��ũ��Ʈ�� �����Ǵ� ��ü ������ Virtual Machine�� ����
                                 ��ũ��Ʈ ��� ���������ʹ� Virtual Machine�� ������ ������Ʈ

    �� ���� : ������ AI�� �ϵ� �ڵ�
              ��Ƽ������ Ȱ��
              �� 1 Thread & 1 VM : �ϳ��� VM�� ��� ��ü�� ��Ʈ��
                                   ���� ���� �߻�

                 N Thread & N VM : �ϳ��� VM�� ���� ���� ��ü�� ��Ʈ��
                                   Load Balacing ���� �߻�
                                   Worker Thread���� �浹 �߻�

                 Worker Thread & 1 VM : ��Ƽ�����忡�� ���� ȣ�� ������ ��ũ��Ʈ ��� �ʿ�

                 Worker Thread & MM VM : ��ü �ϳ��� �ϳ��� VM
                                         �޸� ���� �߻�  // ��κ��� VM�� ��� ����
                                         �� LUA�� ����, Event�� ���� ������ LUA �Լ� ȣ��
                                            ��ũ��Ʈ �� �����ϱ� ����� ����� ���� �������� API�� ������ �־�� �Ѵ�.

    �� ���� : SESSION�� lua_State* �Ҵ�
              �� �÷��̾� �̵� �� ���� NPC�� EVENT ����
                 PQCS�� Worker Thread���� ����
                 LUA ��ũ��Ʈ ����

                 // monster.lua
                 myid = 99999;

                 function set_uid(x)
                     myid = x;
                 end

                 function event_player_move(player)
                     player_x = API_get_x(player)
                     player_y = API_get_y(player)
                     my_x = API_get_x(myid)
                     my_y = API_get_y(myid)

                     if (player_x == my_x) then
                         if (player_y == my_y) then
                             API_send_message(myid, player, "HELLO");
                         end
                     end
                 end

                 // lua.cpp
                 void Initialize_VM() {
                     lua_State* L = players[my_id].L = luaL_newstate();
                     luaL_openlibs(L);

                     luaL_loadfile(L, "monster.lua");
                     lua_pcall(L, 0, 0, 0);

                     lua_getglobal(L, "set_uid");
                     lua_pushnumber(L, my_id);
                     lua_pcall(L, 1, 0, 0);

                     lua_register(L, "API_send_message", API_send_message);
                     lua_register(L, "API_get_x", API_get_x);
                     lua_register(L, "API_get_y", API_get_y);
                 }

                 int API_get_x(lua_State* L) {
                     int user_id = (int)lua_tointerger(L, -1);

                     lua_pop(L, 1);

                     int x = players[user_id].x;

                     lua_pushnumber(L, x);
                     return 1;
                 }

                 int API_send_message(lua_State* L) {
                     int my_id = (int)lua_tointerger(L, -3);
                     int user_id = (int)lua_tointeger(L, -2);
                     char* mess = (char*)lua_tostring(L, -1);

                     lua_pop(L, 3);

                     send_chat_packet(user_id, my_id, mess);
                     return 0;
                 }

                 void process_packet(int c_id, char* packet) {
                     ...

                     case IO_PLAYER_MOVE:
                         objects[id].lua_lock.lock();

                         lua_getglobal(objects[id].L, "event_player_move");
                         lua_pushnumber(objects[id].L, caller_id);
                         lua_pcall(objects[id].L, 1, 0, 0);

                         objects[id].lua_lock.unlock();

                         delete eo;
                         break;
                 }

*/