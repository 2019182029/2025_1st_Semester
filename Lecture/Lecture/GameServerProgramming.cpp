/*

 ■ 시야 처리

 1. 비용 문제
    → 한 오브젝트의 상태가 변경되면 서버에 존재하는 모든 플레이어에게 변경 내용을 Broadcasting 해야 한다.
       → 동접 N일 때 N * N의 패킷 : 1초에 1번 이동
                                     동접 5000 → 25M packet/sec
                                     1000 CPU cycle per packet → 25GHz
                                     20byte per packet → 4Gbps bandwidth


 2. 해결책
    → WSASend 최적화 : 커널 최적화가 필요하므로 불가능
    
       Broadcasting 최적화 : RIO 또는 IO-uring을 사용

       WSASend 횟수 감소 : 여러 개의 WSASend를 모아서 보낸다.
                           나를 보고 있는 플레이어에게만 나의 움직임을 알린다.
                           → 부하 N * N에서 N * K로 감소
                              효율적인 검색 필요


 3. 효율적인 검색
    → Zone : 전체 월드를 큰 논리적인 단위로 쪼갠 것
              → 차원이 다르거나, 바다가 가로막고 있거나, etc...
                 Seamless하지 않아 Zone 이동 시 로딩 필요

       Sector(Cluster) : 서버의 검색 효율성을 위한 개념으로, 클라이언트와는 아무런 상관이 없는 개념이다.
                         → 자신과 인접 Sector만 검색하여 검색 대상 오브젝트의 개수를 줄인다.
                            → Sector마다 Sector에 존재하는 오브젝트의 목록을 관리하는 멀티쓰레드 자료구조 필요
                               모든 오브젝트의 이동, 생성, 소멸 시 자신이 속한 Sector의 오브젝트 목록을 업데이트

                         → Sector의 크기는 적절해야 한다.
                            → 너무 크면 : 시야 범위 밖의 개체가 많이 검색되어 검색 대상 증가
                                           여러 쓰레드가 한 섹터를 공유함으로써 병렬성 저하

                               너무 작으면 : 많은 Sector를 검색할 필요 有
                                             이동 시 잦은 섹터 변경 오버헤드 발생


 4. 시야 처리
    → 기본 알고리즘 : 이동 전과 후의 시야 내에 존재하는 객체 비교
                       → 시야에 들어온다면 서로에게 비주얼 정보 전송
                          시야에서 사라진다면 서로에게 소멸 신호 전송
                          계속 존재하는 플레이어에게 새 좌표 전송
                          → MOVE 패킷에 모든 정보를 담아낸다면 서버 부하가 증가하므로 별도 패킷을 정의하여 사용할 필요 有

                       → 멀티쓰레드에서 동작하므로 이동 자체에서 Data Race 발생
                          → 좀비 : 시야에서 사라졌는데 클라이언트에서 계속 존재
                             유령 : 비주얼 정보를 받지 않은 객체가 이동, 보이지 않는 객체가 다른 객체들과 상호작용

                          → 해결책 : 동시 이동 불허
                                      → Mutex 사용으로 인한 성능 저하
                                         → Sector 내의 플레이어들 사이에서만 Lock을 수행하는 것으로 성능 개선 가능

                                      보정
                                      → 클라이언트에 표시되는 객체들의 리스트를 서버에 저장, 이동 시 이를 기반으로 시야 처리
                                         → 유령 배제 : 리스트에 없으면 무조건 비주얼 정보 전송
                                            좀비 최소화 : 두 번째 이동에서는 무조건 좀비 삭제되어, 일정 시야 내에서는 좀비가 절대 존재하지 않는다.


 5. 시야 리스트
    → 클라이언트에 표시되고 있는 객체의 리스트
       → 최적화 : 시야 리스트의 업데이트를 직접 하는 것이 아니라, 복사하여 사용한 후 새 시야 리스트로 교체
                   → 시야 리스트를 복사할 때만 Lock을 수행하므로 Lock으로 인한 성능 저하 최소화
                      나와 상대의 시야 리스트를 동시에 Lock하는 과정에서 발생할 수 있는 데드락 회피

                   이동 시 상대방의 시야 리스트를 조작하지 않고, 상대방에게 접근, 이동, 이탈 메세지를 보낸다.
                   → Mutex가 필요 없어진다.
                      Non-Blocking Concurrent Queue가 필요하다.

       → 이동 시 시야 처리 순서 : Sector 검색 후 Near List 생성
                                   → Near List의 모든 객체에 대해 : 시야 리스트에 없으면
                                                                     → 시야 리스트에 추가
                                                                        나에게 상대의 비주얼 정보 전송
                                                                        상대 시야 리스트에 있으면 나의 새 좌표 전송
                                                                        상대 시야 리스트에 없으면 상대 시야 리스트에 나를 추가한 뒤 상대에게 나의 비주얼 정보 전송

                                                                     시야 리스트에 있으면
                                                                     → 상대 시야 리스트에 있으면 나의 새 좌표 전송
                                                                        상대 시야 리스트에 없으면 상대 시야 리스트에 나를 추가한 뒤 상대에게 나의 비주얼 정보 전송

                                      시야 리스트의 모든 객체에 대해 : Near List에 없으면
                                                                       → 시야 리스트에서 제거
                                                                          나에게 상대의 소멸 신호 전송
                                                                          상대 시야 리스트에 있으면 상대 시야 리스트에서 나를 제거한 뒤 상대에게 나의 소멸 신호 전송

 ====================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================

 ■ 지형

 1. 지형 구현
    → 서버가 지형을 인식해야 한다.
       → 클라이언트가 가지고 있는 데이터를 서버에 복사하는 것은 과부하 : 이동에 필요한 정보만 사용
                                                                          → 클라이언트도 아바타 이동에 동일한 정보 사용

       → 클라이언트에서도 지형 충돌 검사를 수행
          → UI 반응 속도 향상 : 아바타의 이동, 아이템 집기, 화살 겨냥 등이 즉시 수행
          
          해킹 시도는 서버에서 사후 검증을 수행하여 차단한다.
          → 서버의 랙이 심한 경우에도 이동만은 랙 없이 할 수 있다.
             서버에서 거부하기 전 일차적으로 패킷 낭비 없이 오동작을 차단한다.
             서버의 패킷 전송을 생략할 수 있다.


    → 메모리 용량과 검색 속도가 중요
       메모리 용량과 검색 속도는 서로 Trade Off 관계
       → 메모리 용량 : 지형의 정밀도 결정
          
          검색 속도 : 지형 데이터의 크기(= 면적 * 복잡도)
                      서버 부하 = O(동접 수 * 검색 속도)
                      → 그 다음으로 Cache 적중률에 가장 많은 영향을 받으므로, 성능 향상을 위해 더 큰 메모리를 사용하는 것은 바람직하지 않다.


 2. 2D 지형
    → 2D 배열 : 서버 내 모든 객체의 좌표는 정수
                 2D 이미지를 토대로 사람이 작성
                 이동 가능 여부 Flag가 Cell마다 존재
                 → bool can_move[WORLD_WIDTH][WORLD_HEIGHT]
                    → WORLD_WIDTH, WORLD_HEIGHT는 전체 맵의 크기와 장애물 표현 정밀도에 의해 결정 : 맵의 크기 / 장애물 최소 크기
                       bool 대신 bit를 사용할 필요 有

                 → 장점 : 매우 빠른 속도, 메모리 절약
                    단점 : 3D 지형 표현 불가능


 3. 3D 지형
    → 다층 지형을 위해 필요하며, 이동 시 높이 검사가 필요하다.
       사람이 작성할 수 없으므로, 충돌 검사용 데이터 자동 생성 필수

    → 확장 타일 : 3D 배열
                   → 서버 내 모든 객체의 좌표는 실수
                      메모리 낭비가 심해 2D를 기본으로 일부분만 3D로 표현하는 Sparse Matrix 사용
                      여러 개의 2D 배열로 다층 구조 표현  
                   
                   → 장점 : 빠른 속도
                      단점 : 복잡한 입체 구조 표현 불가능

       Polygon : 클라이언트가 가지고 있는 데이터를 가공하여 사용
                 → 서버 내 모든 객체의 좌표는 실수
                    메모리 사용량은 확장 타일과 큰 차이가 없다.
                    클라이언트와 동일한 방법으로 이동 가능 여부를 검사한다.  
                    → 물리 엔진 사용 필요
                       3D 게임 엔진의 Dedicated Server 모드 사용 가능  // 단, 서버 부하 대폭 증가
                    
                 → 장점 : 높은 정확성
                    단점 : 느린 속도


 4. 길 찾기
    → 길 찾기 자료구조 필요
       → Path Node : 객체의 위치와 노드의 위치가 완벽히 일치할 확률이 거의 존재하지 않는다.
       
          Path Mesh : 객체가 올라설 수 있는 폴리곤 필터링 후 연결된 폴리곤 합성
                      → 최대한 큰 볼록 다각형으로 합성
                         최적화를 위해 약간의 경사 차이 무시
                         → 다각형을 Node로 하고, 연결 여부를 Edge로 한 그래프 생성
                            → 높이 차와 경사를 고려한 일방통행 그래프
                               점프 등을 통해 이동 가능하면 연결로 취급

    → 기본적인 길 찾기 방식 : 다음 Step(단위 시간에 갈 수 있는 직선 상의 위치) 정하기
                               → Timer를 통한 이동 : 패킷 개수 절약, 계산 시간 절약 
                                  매 Step마다 길 찾기를 다시 수행할 필요 有


 5. 길 찾기 알고리즘
    → 그래프 최단 경로 탐색 : 그래프의 노드 정의 필요
                               Weight를 부여할 수도 있다.

    → 가면서 찾기 : 지형 전체를 알 수 없는 경우  
                     → 랜덤
                        장애물 따라 돌기
                        직선 찾아 돌기

       미리 찾기 : Depth First Search
                   → IDDF : 탐색 길이 제한 필요
                    
                   Breadth First Search
                   → Bidirectional Breath First Search : 시작 노드와 목표 노드 양쪽에서 Breadth First Search 수행

                      Dijkstra's Algorithm : 노드에 Cost를 줄 수 있다.
                                             Tile 구조가 아닌 그래프 구조에 유용
                                             항상 최적의 답을 도출한다.

                      Best First Search : Dijkstra's Algorithm의 Cost를 휴리스틱으로 대체
                                          항상 최적의 답을 도출하지는 않는다.

                      A* Search : Dijkstra's Algorithm + Best First Search
                                  F(n) = G(n) + H(n)
                                  → F(n) : 노드 n의 비용
                                     G(n) : 시작 노드에서부터 노드 n까지의 최소 비용
                                     H(n) : 목표 노드까지의 근사 비용
       
 ====================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================

 ■ NPC

  2. NPC-AI
                             NPC 서버를 둘 경우 NPC 서버의 연산 결과를 메인 서버로 전송해야 하므로 통신 오버헤드 ↑
    → my_hp += HEAL_AMOUNT;
       → my_hp = MIN(my_hp, my_max_hp);를 추가하는 것이 낫다.
          → 그렇지만 서순이 이상 → 실제 예로 바꿔야 하는데, busy waiting

    → 해결책 2 : 필요한 NPC의 heart_beat()만을 호출
                  → 해결책 1과의 차이점 : heart_beat() 함수에서 다음 heart_beat이 필요한지 판단
                                           → 필요 시 자신을 등록
                                           → 등록된 객체들에 대해서만 heart_beat() 호출
                                              → 문제점 : heart_beat() 함수 내부의 수많은 조건문
                                                          → heart_beat() 함수를 없애서 각 모듈에서 timer를 직접 사용
                                                          → timerthread는 가장 오래된 것부터 처리
                                                             → 또한, Priority_queue를 사용하여 우선순위가 높은 것부터 처리
                                                                → AI는 process_event 함수를 사용

    → PostQueuedCompletionStatus의 num_bytes, key, LPWSAOVERLAPPED는 iocp 객체에 등록하지 않아도 그대로 넘어간다.
       → 즉, 실제 IO와는 관련이 없고 worker_thread를 깨울 때 사용하는 것
       → concurrent_priority_queue 쓰면 된다.

    → 같은 컨테이너
       → 장점 :
          단점 : NPC마다 사용하지 않는 SESSION 정보 추가

       다른 컨테이너
       → 장점 :
          단점 : ID를 별도로 관리하는 오버헤드 발생
                 → can_see 함수를 예로 들면, user - user, user - npc, user - skill 등의 여러 함수를 정의할 필요 有

       → 꼴리는 거 써라


 4. NPC인지 PC인지 구분하기 위해 멤버 변수 추가?
    → 컨테이너 검색 오버헤드 발생
       → PC와 NPC의 ID 구간을 따로 설정함으로써 해결
          → array라면 ID 구간을 신중히 결정해야 한다.  // by) 메모리 낭비
          → unordered_map이라면 ID 구간이 자유롭다!

    → npc는 vl를 두지 않고, 이동할 때마다 새로 생성
       → 유지한다면 pc 이동 시에도 update 필요...

    → pc 이동 시 npc에게는 당연히! 패킷 전송 X

    → 타이머 큐를 concurrent_queue로 하면 락할 필요 X,
       → pop() 대신 try_pop()이 존재 : 인자로 넘겨준 애한테 pop한 애를 복사한다.
          TIMER_EVENT ev;
          true = timer_queue.try_pop(ev);

       → OP_NPC_MOVE에서 awake할 필요가 없으면 다시 sleep하는 코드 추가!

 ====================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================

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