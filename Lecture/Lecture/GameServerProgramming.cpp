/*

 ■ 스크립트

 1. 스크립트
    → 사용하기 편해 생산성이 높은 언어
       → 장점 : 빠른 개발 속도
                 서버 재 컴파일 불필요

          단점 : 느린 실행 속도
                 가비지 컬렉션 랙

    → 요구 : 배우기 쉬울 것
              객체지향 프로그래밍 지원
              멀티쓰레드 기능 제공
              → Reentrant : 하나의 객체를 여러 쓰레드에서 동시에 호출해도 문제없이 실행
                             → 쓰레드별로 별도의 인터프리터 객체를 실행할 수도 있다.
                                개별 객체 구현이 아닌, 시스템을 구현하는 경우 필요하다.
    
    → 특징 : 직관적인 문법
              관용적인 문법
              성능보다는 표현에 중심
              인터프리터

    → 게임에서의 활용 : 데이터 정의
                         NPC AI
                         Non-Programmer


 2. 스크립트 언어
    → LUA : 사용하기 쉽다.
             성능이 뛰어나다.
    
       Python : 생산성이 높다.
                덩치가 크며 멀티쓰레드 성능에 문제가 있다.

       Java, JavaScript : 높은 인지도
                          가비지 컬렉션 랙

       C# : C에서 호출 오버헤드 존재
            멀티쓰레드 지원

       XML : 언어가 아니며, 최근에는 JSON 사용


 3. LUA
    → 특징 : 가벼운 명령형, 절차식 언어
              확장 가능한 문법을 가진 스크립트 언어를 주 목적으로 설계
              간단한 자료구조  // boolean, number, string, table

    → 장점 : 핵심 모듈의 크기가 120KB 이하
              게임 개발에 자주 사용되는 Python에 비해 빠른 실행 속도
              오픈소스

    → Visual Studio C++와의 연동 : include 파일을 프로젝트에 추가 
                                    lua54.lib를 라이브러리에 추가   

    → 문법 1
       → function factorial(n)
              if n == 0 then
                  return 1
              end
              return n * factorial(n - 1)
          end

          print (factorial(5))

          print ([multiple
                lines])

       문법 2
       → a_table = { x = 10 }
          print(a_table["x"])

          b_table = a_table
          a_table["x"] = 20
          print(a_table["x"])
          print(b_table["x"])

       문법 3
       → point = { x = 10, y = 20 }
          print(point["x"])
          print(point.x)


    → 실습 1
       → // lua.cpp
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
          → C++에서 LUA 함수 호출
             → int lua_pcall(
                    lua_State* L,  
                    int nargs,     // 매개변수 개수
                    int nresults,  // 리턴값 개수
                    int msgh       // 0
                );
                → Stack에 매개변수를 넣어놓아야 한다.
                   실행이 끝나면 Stack에는 리턴값만 남는다.
                   에러 없이 종료되면 0을 리턴한다.

       실습 2
       → // exl.lua
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
          → LUA의 Virtual Machine은 Stack Machine이다.
             → C++와 LUA 프로그램 간의 자료 교환은 Stack을 통해 이뤄진다.
                → LUA 함수 호출 시 매개변수 Push : lua_pushnumber(L, 1);
                   Stack에 저장된 값 읽기 : (int)lua_tonumber(L, -1);
                   Stack에 글로벌 변수 값 저장하기 : lua_getglobal(L, "rows");

       실습 3
       → // exl.lua
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


 4. NPC SCRIPT 연동
    → NPC의 동작을 스크립트 언어로 정의
       → Finite State Machine : Event Driven
                                 스크립트로 관리되는 객체 정보를 Virtual Machine에 저장
                                 스크립트 언어 인터프리터는 Virtual Machine의 내용을 업데이트

    → 성능 : 간단한 AI는 하드 코딩
              멀티쓰레드 활용
              → 1 Thread & 1 VM : 하나의 VM이 모든 객체를 컨트롤
                                   성능 문제 발생

                 N Thread & N VM : 하나의 VM이 여러 개의 객체를 컨트롤
                                   Load Balacing 문제 발생
                                   Worker Thread와의 충돌 발생

                 Worker Thread & 1 VM : 멀티쓰레드에서 동시 호출 가능한 스크립트 언어 필요

                 Worker Thread & MM VM : 객체 하나당 하나의 VM
                                         메모리 낭비 발생  // 대부분의 VM이 대기 상태
                                         → LUA라서 가능, Event에 따라 적절한 LUA 함수 호출
                                            스크립트 언어가 실행하기 곤란한 기능을 게임 서버에서 API로 제공해 주어야 한다.

    → 구현 : SESSION에 lua_State* 할당
              → 플레이어 이동 시 주위 NPC의 EVENT 생성
                 PQCS로 Worker Thread에게 전달
                 LUA 스크립트 실행

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