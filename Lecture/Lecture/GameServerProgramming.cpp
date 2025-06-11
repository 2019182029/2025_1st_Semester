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
                                            좀비 최소화 : 두 번째 이동에서는 무조건 좀비 제거, 일정 시야 내에서는 좀비가 절대 존재하지 않는다.


 5. 시야 리스트
    → 클라이언트에 표시되고 있는 객체의 리스트
       → 최적화 : 시야 리스트의 업데이트를 직접 하는 것이 아니라, 복사하여 사용한 후 새 시야 리스트로 교체
                   → 시야 리스트를 복사할 때만 Lock을 수행하므로 Lock으로 인한 성능 저하 최소화
                      나와 상대의 시야 리스트를 동시에 Lock하는 과정에서 발생할 수 있는 데드락 회피

                   이동 시 상대방의 시야 리스트를 조작하지 않고, 상대방에게 접근, 이동, 이탈 메세지를 보낸다.
                   → Mutex가 필요 없어진다.
                      Non-Blocking Concurrent Queue가 필요하다.

                   NPC는 시야 리스트를 갖지 않고, 필요할 때마다 Sector 검색 후 Near List 생성
                   → NPC 이동 시 발생하는 NPC의 시야 리스트 업데이트 오버헤드 제거

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
       → 메모리 용량 : 지형의 정밀도 결정
          
          검색 속도 : 지형 데이터의 크기  // 면적 * 복잡도
                      → 서버 부하 = O(동접 수 * 검색 속도)
                         
          → 메모리 용량과 검색 속도는 Trade Off 관계
             → 성능 향상을 위해 더 큰 메모리를 사용하는 것은 Cache Hit Ratio를 떨어뜨리므로 바람직하지 않다.


 2. 2D 지형
    → 2D 배열 : 서버 내 모든 객체의 좌표는 정수
                 2D 이미지를 토대로 사람이 작성
                 이동 가능 여부 Flag가 Cell마다 존재
                 → bit can_move[WORLD_WIDTH][WORLD_HEIGHT]
                    → WORLD_WIDTH, WORLD_HEIGHT는 전체 맵의 크기와 장애물 표현 정밀도에 의해 결정 : 맵의 크기 / 장애물 최소 크기

                 → 장점 : 매우 빠른 속도, 메모리 절약
                    단점 : 3D 지형 표현 불가능


 3. 3D 지형
    → 다층 지형을 위해 필요하며, 이동 시 높이 검사가 필요하다.
       사람이 작성할 수 없으므로, 충돌 검사용 데이터 자동 생성 필수

    → 확장 타일 : 3D 배열
                   → 서버 내 모든 객체의 좌표는 실수
                      여러 개의 2D 배열로 다층 구조 표현  
                      메모리 낭비가 심해 2D를 기본으로 일부분만 3D로 표현하는 Sparse Matrix 사용
                   
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

    → 기본적인 길 찾기 방식 : 다음 Step 정하기  // 단위 시간에 갈 수 있는 직선 상의 위치
                               → Timer를 통한 이동으로 패킷 개수 절약, 계산 시간 절약 
                                  매 Step마다 길 찾기를 다시 수행할 필요 有


 5. 길 찾기 알고리즘
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

 1. NPC
    → 인공지능 
       → 복잡한 인공지능은 Script로 구현 : 게임 제작 파이프라인 단축
                                            → 서버 프로그래머의 개입 없이 기획자가 작성  // 서버 프로그래머는 Script 연동 시스템을 구현하고 샘플 Script를 작성할 필요 有
                                               서버 리부팅 필요 無

       → 능동적 인공지능 : 플레이어의 이동을 근처 NPC에게 Broadcast
                            → 플레이어가 움직이지 않을 시 인식하지 못하는 경우가 발생할 수 있으므로, 플레이어 또는 NPC 생성 시에도 Broadcast

    → NPC 서버
       → 장점 : 안정성  // NPC 서버가 종료되어도 메인 서버는 정상 작동
                 부하 분산

          단점 : 메인 서버와의 통신 오버헤드
                 → 공유 메모리 참조가 패킷 통신으로 악화
                    서버 입장에서는 NPC와 플레이어가 동일한 부하


 2. 어떠한 컨테이너에 담아야 하는가?
    → 같은 컨테이너 : NPC가 SESSION 정보를 갖는 낭비 발생
                       → PC와 NPC를 구분하는 방법 : PC와 NPC를 구분하는 변수 추가
                                                     → 컨테이너 검색 오버헤드 발생
                                                        PC와 NPC의 ID 구간을 따로 설정함으로써 해결

       다른 컨테이너 : ID를 별도로 관리하는 오버헤드 발생


 3. 구현
    → while (true) {
           if (A_done == false) {
               DO(A);
               A_done = true;
           }
       }
       → 문제점 : 매 루프마다 여러 개의 if문 검사 
                   Busy Waiting
                   → 캐시 문제, Pipeline Stall

          해결책 : heart_beat 함수를 두고 일정 시간 간격마다 호출하여 Busy Waiting이 없도록 한다.

    → Cobj::heart_beat() {
           if (m_hp < m_max_hp) {
               m_hp += HEAL_AMOUNT;
           }
       }

       while (true) {
           for (int i = 0; i < MAX_NPC; ++i) {
               NPC[i].heart_beat();
           }

           Sleep(delay);
       }
       → 문제점 : 계속해서 호출되는 heart_beat()가 아무것도 수행하지 않는다.

          해결책 : 필요한 경우만 heart_beat()가 호출되도록 한다.
                   → 복잡한 인공지능의 경우 프로그래밍이 어려워진다.
                      불리지 않는 경우를 판단하는 것 자체가 오버헤드

                   각 모듈에서 timer를 직접 사용한다.

    → struct event_type {
           int obj_id;
           int target_id;
           int event_id;
           high_resolution_clock::time_point wakeup_time;

           constexpr bool operator<(const event_type& _Left) const {
               return (wakeup_time > _Left.wakeup_time);
           }
       };
    
       priority_queue<event_type> timer_queue;
       mutex timer_lock;

       get_damage(int damage) {
           m_hp -= damage;
           add_timer(m_heal_event, 1000);
       }

       m_heal_event() {
           m_hp += HEAL_AMOUNT;

           if (m_hp < m_max_hp) {
               add_timer(m_heal_event, 1000);
           }
       }

       do_timer() {
           do {
               do {
                   timer_lock.lock();
                   event_type k = top(timer_queue);
                   timer_lock.unlock();

                   if (k.start_time > current_time()) {
                        break;
                   }

                   timer_lock.lock();
                   pop(timer_queue);
                   timer_lock.unlock();

                   EXP_OVER eo = new EXP_OVER(MOVE);
                   PostQueuedCompletionStatue(h_iocp, 0, id, eo);  // timer thread의 과부하를 방지하기 위해 실제 작업은 worker thread에서 수행해야 한다.
               } 
               while (true);
               Sleep(1);
           }
           while (true);
       }
       → 문제점 : 플레이어의 시야 리스트에 있는 NPC만 행동해야 한다.
          
          해결책 : 플레이어가 근처에 있으면 Cobj::wake_up() 호출
                   플레이어가 근처에 없으면 Cobj::sleep() 호출
                   → m_is_active 변수를 통해 중복 Cobj::wake_up() 호출 방지  // Mutex 대신 CAS를 사용하는 것이 부하 ↓

    → Cobj::wake_up() {
           if (!m_is_active) {
               bool expected = false;

               if (std::atomic_compare_exchange_strong(&m_is_active, &expected, true)) {
                   add_timer(m_event, 1000);
               }
           }
       }

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

       C# : 멀티쓰레드 지원
            C에서 호출 오버헤드 존재

       XML : 언어가 아니다.
             최근에는 JSON 사용


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

              lua_State* L = luaL_newstate();                  // Open LUA
              luaL_openlibs(L);                                // Open LUA Standard Library

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
                → LUA 함수 호출 시 매개변수 Push : lua_pushnumber(L, arg);
                   Stack에 글로벌 변수 값 저장하기 : lua_getglobal(L, "rows");
                   Stack에 저장된 값 읽기 : (int)lua_tonumber(L, -1);

       실습 3
       → // exl.lua
          function add_num_lua(a, b)
              return c_addnum(a, b)
          end

          // lua.cpp
          int c_addnum(lua_State* L) {
              int a = (int)lua_tonumber(L, -2);
              int b = (int)lua_tonumber(L, -1);

              int result = a + b;

              lua_pop(L, 2);

              lua_pushnumber(L, result);

              return 1;
          }

          int main() {
              ...

              lua_register(L, "c_addnum", c_addnum);

              lua_getglobal(L, "add_num_lua");
              lua_pushnumber(L, 100);
              lua_pushnumber(L, 200);

              lua_pcall(L, 2, 1, 0);

              result = (int)lua_tonumber(L, -1);

              lua_pop(L, 1);

              ...
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
                                         → LUA라서 가능
                                            Event에 따라 적절한 LUA 함수 호출
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
        
 ====================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================

 ■ DB

 1. DB
    → 구조화된 데이터의 모임
       컴퓨터에 저장, 쿼리를 사용하여 접근

    → 요구 사항
       → 저장소 : 인덱스를 통한 고속 검색
          트랜잭션, 동시성
          복제 : 안정성, 부하 분산
          보안

    → 사용 이유
       → 데이터 크기 : 서버 프로그램이 모든 데이터를 메모리에 가지고 있기 어렵다.
          데이터 보존 : 서버 프로그램이 종료되어도 데이터가 남아 있어야 한다.
          안정성, 효율성 : 상용 DB보다 데이터 관리를 잘하는 프로그램을 만들기 어렵다.

       → 장점 : 각종 툴을 사용할 수 있다.
                 제 3자의 데이터 관리 프로그램 작성이 가능하다.

          단점 : 잘 모르는 상태에서 사용하면 해결하기 어려운 문제를 맞닥뜨릴 수 있다.


 2. 데이터
    → DB에 저장되는 데이터 : 과금 정보 등
                              → 게임 로그는 저장만 수행하고 읽지 않는다.

       DB에 저장되지 않는 데이터 : 서버 재부팅 시 초기화되는 정보
                                   게임 플레이 중 변경되지 않는 정보
                                   → 파일로 저장되어 서버 부팅 시 메모리에 읽는다.

    → 서버는 얼마나 자주 DB에 접근해야 하는가?
       → 필수 : 캐릭터 LOGIN, 캐릭터 LOGOUT, 서버 BOOTING, 서버 SHUTDOWN
          추가적 : 아이템 거래       // DB 반영 이후 거래 성사
                   주기적 Auto Save  // 중요도와 주기는 반비례
                   중요한 데이터 변경
                   → 서버 다운에도 안전하게!


 3. 게임 서버
    → DB 접근은 Blocking API : DB 접근 쓰레드와 Worker 쓰레드를 분리
                                → DB 관련 프로토콜 정의 필요
                                   Multiple Write / Single Read Concurrent Queue 필요
                                   프로세스 간의 통신 오버헤드 존재
                                
                                다른 컴퓨터에서 수행 : 부하와 메모리 분산
                                                       Cache 서버 혹은 Query 서버라고 불리기도 한다.

       DB 접근마다 SQLConnect를 사용하는 것은 오버헤드 大 : SQLConnect로 연결한 핸들을 계속 저장해두고 사용
                                                            → 멀티쓰레드인 경우
                                                               → 방법 1 : Lock 사용 
                                                                  방법 2 : 쓰레드마다 핸들 생성


 4. SQL 
    → DB의 데이터를 조작하는 언어
       → CREATE : CREATE TABLE items (id INT, type INT);
          UPDATE : UPDATE items SET type = 3 WHERE id = 42;
          SELECT : SELECT * FROM items WHERE id = 42;
          INSERT : INSERT INTO items (id, type) VALUES (1043, 21);
          DELETE : DELETE FROM items WHERE id = 1043;


 5. ODBC
    → 프로그래밍 언어에서 SQL을 사용하기 위한 표준
       → #include <sqlext.h>
          
          HENV henv;    // ODBC 환경 핸들
          HDBC hdbc;    // DB 연결 핸들
          HSTMT hstmt;  // SQL 실행 핸들

          SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);                           // ODBC 환경 핸들 생성
          → 핸들의 종류, 상위 핸들, 새로 생성된 핸들을 받을 포인터

          SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER*)SQL_OV_ODBC3, 0);         // ODBC 버전 지정
          → 환경 핸들, 설정할 속성, 속성 값, 문자열일 경우 문자열의 길이

          SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);                                      // DB 연결 핸들 생성

          SQLSetConnectAttr(hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0);                     // 로그인 타임아웃 설정
          → 연결 핸들, 설정할 속성, 속성 값, 문자열일 경우 문자열의 길이

          SQLConnect(hdbc, (SQLWCHAR*)L"2025_game", SQL_NTS, NULL, 0, NULL, 0);             // DB 연결
          → 연결 핸들, DSN, 문자열일 경우 SQL_NTS, ID, 0 또는 SQL_NTS, PW, 0 또는 SQL_NTS

          SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);                                    // SQL 실행 핸들 생성

          SQLExecDirect(hstmt, (SQLWCHAR*)L"EXEC stored_procedured param", SQL_NTS);        // Stored Procedure 호출
          → SQL 실행 핸들, Stored Procedure, 문자열일 경우 SQL_NTS
             → Stored Procedure :  일련의 동작을 SQL로 프로그래밍하여 DB에 저장해놓은 것
                                    → 장점 : 성능 향상
                                              Transaction 구현
                                              네트워크 트래픽 감소
                                              보안

          SQLBindCol(hstmt, 1, SQL_C_LONG, &user_id, sizeof(user_id), &cb_user_id);         // 결과 컬럼을 C++ 변수에 바인딩
          → SQL 실행 핸들, 결과셋의 몇 번째 컬럼인지, C 타입, 데이터를 저장할 변수의 주소, 버퍼 크기, 실제 읽힌 값을 저장할 변수의 주소

          SQLFetch(hstmt);                                                                  // 다음 결과 행 가져오기
          → SQL 실행 핸들

          SQLCancel(hstmt);                       
          SQLFreeHandle(SQL_HANDLE_STMT, hstmt); 
          SQLDisconnect(hdbc);                    
          SQLFreeHandle(SQL_HANDLE_DBC, hdbc);    
          SQLFreeHandle(SQL_HANDLE_ENV, henv);

    → 오류 발생 시 대처
       → 변수의 크기를 나타내는 매개변수 확인  // 문자열의 경우 SQL_NTS를 사용해야 한다.
          SQLCHAR를 SQLWCHAR로 바꾸고, 모든 문자열을 "..."에서 L"..."으로 바꾼다.
          핸들은 타임아웃 시간이 설정되어 있다.  // 기본 8시간

 ====================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================

 ■ 보안

 1. 보안
    → 컴퓨터 보안 : 해커에 의한 프로그램 오동작 방지
                     → 클라이언트 컴퓨터 보안 : 불가능, 따라서 모든 연산을 서버에서 수행할 필요 有  
                                                 → 게임 가드 설치를 통한 유해 소프트웨어 차단
                                                    → 대안 : OTP
                        
                        클라이언트 프로그램 보안 : 난독화, 클라이언트 변조 검사

                        서버 컴퓨터 보안 : 운영팀의 몫                                      
                                           → 보안 패치, 방화벽
                        
                        서버 프로그램 보안 : 서버 프로그램 해킹 방지, 클라이언트 해킹 감지    
                                             → 클라이언트의 게임 가드 실행 여부 확인

       네트워크 보안 : 네트워크로 오고 가는 데이터 유출과 변조 방지
                       → 망 분리

                          패킷 암호화 : 인터넷은 도청에 무방비  // 모바일 게임의 보안이 허술한 이유
                                        → SSL : 표준, 강력하지만 서버에 큰 부하
                                           간단한 XOR 암호화 : 허술하지만 서버에 작은 부하
                                           고정 키 방식 : 취약, 실시간 키 생성으로 보완
       

 2. 서버 프로그램 해킹
    → Stack Overflow : 문자열 길이 관리 시 strcpy 대신 strncpy, strcpy_s 사용할 필요 有
                        → 클라이언트 프로그램 해킹 후 BUFF 크기보다 큰 문자열을 전송하면, 서버 프로그램의 Stack을 문자열이 덮어씌운다.
                           → strncpy, strcpy_s는 strcpy 크기를 지정하므로 Stack Overflow 불가능
                              → Windows는 strcpy 사용 시 경고문을 출력한다.
                                 Linus는 strcpy 사용 시 경고문을 출력하지 않는다.  // Linux가 해킹에 취약한 원인

       SQL Injection : DB에 저장되는 문자열에 SQL 명령어를 심는 것
                       → 클라이언트는 특수 문자를 변환하여 전송, 서버는 특수 문자를 해석하여 사용
               
       디버깅용 백도어 삭제

       Auto 프로그램 : 게임의 재미 저하 → 사용자 수 하락 → Auto 프로그램 사용자 수 하락 → 게임 회사 수입 감소
                       → 해결책 : 클라이언트 검증, 사용자 검증
                                   → 패킷 암호화, 특수 패킷, 내부 메모리 검사  // Delayed 처벌로 강화

                                   → 해결 불가능? : 지속적인 업데이트로 Auto 프로그램 제작 억제
                                                     정책, 기획 수립  // 게임성 희생, Auto 프로그램을 게임에 포함

 ====================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================

 ■ Linux

 1. UNIX
    → 텍스트 기반의 UI
       멀티프로세스 시분할 운영체제
       

 2. LINUX
    → 장점 : 가격이 싸다.
              가볍다.
              오픈 소스

       단점 : 익숙해지기 힘들다.

    
 3. Windows와의 비교
    → 언어 : Visual Studio의 long은 32bit, GCC의 long은 64bit

       UI : 텍스트 기반의 UI

       멀티쓰레드 : POSIX 쓰레드 라이브러리 사용 → C++11 사용

       네트워크 : epoll 또는 kqueue 사용 → boost/asio 사용

       DB : MySQL을 많이 사용하며, ODBC 사용이 가능하다.
            직접 소켓 프로그래밍으로 접속하기도 한다.
            REDIS 등의 NoSQL은 LINUX가 대세이다.


 4. 개발 환경
    → 컴파일러 : GCC, G++(ANSI C++ 표준), CLANG
       디버거 : GDB
       프로파일러 : prof, grof
       통합개발환경 : QT Creater, VS Code, Eclipse
       메뉴얼 : man
       DB : MySQL, NoSQL


 5. 용어
    → 디렉토리 : Windows의 폴더를 의미
       . : 현재 디렉토리
       .. : 부모 디렉토리

    → CPU 정보 : lscpu
       메모리 크기 : meminfo
       실행 중인 프로세스 : top
       리소스 사용량 : nmon

       디렉토리 탐색 : ls
       디렉토리 : cd, mkdir, rmdir  // 디렉토리 구분은 '/'로 한다.
       파일 조작 : mv, cp
       파일 내용 보기 : cat

       여러 개의 소스 코드로 구성된 프로젝트 컴파일 : make  // 수정된 소스 코드만 골라서 컴파일하는 기능 제공
       디버깅 : gdb
                → 명령어 : list, next, step, start(run), cont, break
                   변수 내용 확인 : print  


 6. 에디터
    → VIM, Emacs : 유명하지만 진입 장벽이 높다.
                    확장성이 뛰어나다.

       Gedit : 표준 에디터
               GUI 필수

       nano : 메모장
              → nano hellp.cpp
                 g++ -o hello -g hello.cpp
                 ./hello

 ====================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================

 ■ ASIO

 1. Boost
    → C++ 프로그래밍에 사용되는 라이브러리 집합  // 라이브러리를 헤더 파일로 구현
       → 템플릿 프로그래밍
          → 장점 : 호환성, 실행 속도
             단점 : 가독성, 컴파일 속도, 일부는 바이너리 라이브러리 필요


 2. ASIO
    → 소켓 API를 C++ API로 재작성 : 직관적이어서 사용하기 편하다.
                                     람다 함수에 매우 의존적이다.
                                     소켓 객체의 관리에 주의해야 한다.

       운영체제 별로 최신 API를 사용하여 구현
       → Linux : io_uring
          Mac : kqueue
          Windows : IOCP

    → 동작 : io_context와 socket 필요
              → boost::asio::io_context io_context;
                 boost::asio::ip::tcp::socket socket(io_context);

              → 동기식 동작 : 소켓 API 호출
                               소켓이 요청을 io_context에 전달
                               io_context가 운영체제 호출
                               운영체제가 결과를 io_context에 전달
                               io_context는 에러 처리 후 소켓에 결과 전달
                               소켓은 결과를 프로그램에 전달
                                  
                 비동기식 동작 : 소켓 API 호출
                                 소켓이 요청을 io_context에 전달
                                 io_context가 운영체제 호출
                                 운영체제가 결과를 큐에 넣고 io_context에 완료를 알려준다.
                                 프로그램에서 io_context::run() 호출  // run()은 완료를 발견할 때까지 Block
                                 run()이 완료를 발견할 시 결과를 큐에서 뺀 후 my_handler에 전달

              → 멀티쓰레드 연동 : io_context에 대한 모든 접근은 멀티쓰레드 safety를 보장한다.
                                   → 여러 개의 쓰레드에서 io_context::run()을 호출함으로써 병렬성을 얻을 수 있다.

    → 주소는 endpoint로 관리한다.
       → boost::asio::ip::tcp::endpoint server_addr(boost::asio::ip::address::from_string("127.0.0.1"), 3000);
          boost::asio::ip::tcp::v4()가 소켓 API의 INADDR_ANY에 해당


 3. API
    → 서버 : boost::asio::ip::tcp::accept가 소켓 API의 Listen Socket에 해당
              → boost::asio::ip::tcp::accept acceptor(io_context, endpoint);

    → 동기식 송수신
       → size_t boost::asio::ip::tcp::socket::write(
              const ConstBufferSequence& buffers, 
              boost::system::error_code& ec
          );
          → write : 버퍼의 내용이 다 전송될 때까지 대기  // WSASend
             read : 버퍼가 다 찰 때까지 대기

          size_t boost::asio::ip::tcp::socket::write_some(
              const ConstBufferSequence& buffers, 
              boost::system::error_code& ec
          );
          → 버퍼가 다 전송되거나 차지 않아도 완료  // WSARecv, Linux의 recv와 send

       비동기식 송수신
       → void boost::asio::ip::tcp::socket::async_write(
              AsyncWriteStream& s,
              const ConstbufferSequence& buffers,
              BOOST_ASIO_MOVE_ARG handler)
          );

          void boost::asio::ip::tcp::socket::async_write_some(
              const ConstbufferSequence& buffers,
              BOOST_ASIO_MOVE_ARG handler)
          );
          
          void handler(
              const boost::system::error_code& error,
              size_t bytes_transferred
          );
          → buffers는 WSABUF와 마찬가지로 여러 개의 버퍼의 모임
             handler가 필요로 하는 추가 정보는 람다 함수로 전달

    → 그 외
       → Worker Thread : boost::asio::post()
          Timer Thread : boost::asio::steady_timer t(io_context, boost::chrono::seconds(1));
                         t.async_wait(&func);


 4. 실습
    → #include <SDKDDKVER.h>
       #include <boost/asio.hpp>

    → 클라이언트 : int main() {
                        try {
                            boost::asio::io_context io_context;
                            boost::asio::ip::tcp::endpoint server_addr(boost::asio::ip::address::from_string("127.0.0.1"), 3500);
                            boost::asio::ip::tcp::socket socket(io_context);

                            boost::asio::connect(socket, &server_addr);

                            for (;;) {
                                std::string buf;
                                boost::system::error_code error;

                                std::getline(std::cin, buf);
                                if (0 == buf.size()) break;

                                socket.write_some(boost::asio::buffer(buf), error);
                                if (error == boost::asio::error::eof) break;
                                else if (error) throw boost::system::system_error(error);

                                char reply[1024 + 1];
                                size_t len = socket.read_some(boost::asio::buffer(reply, 1024), error);
                                if (error == boost::asio::error::eof) break;
                                else if (error) throw boost::system::system_error(error);
                                reply[len] = 0;
                            }
                        } catch (std::exception& e) {
                            std::cerr << e.what() << std::endl;
                        }
                    }

    → 서버 : void accept_callback(boost::system::error_code ec, tcp::socket& socket, tcp::acceptor& my_acceptor) {
                  g_clients.try_emplace(g_client_id, move(socket), g_client_id++);
                  
                  my_acceptor.async_accept(
                      [&my_acceptor](boost::system::error_code ec, tcp::socket socket) {
                          accept_callback(ec, socket, my_acceptor);
                      }
                  );
              }

              // SESSION
              void do_read() {
                  socket_.async_read_some( 
                      boost::asio::buffer(data_, max_length), 
                      [this](boost::system::error_code ec, size_t length) {
                          if (ec) { g_clients.erase(my_id); }
                          else { g_clients[my_id].do_write(length); }
                      }
                  );
              }

              void do_write(size_t length) {
                  boost::asio::async_write(
                      socket_, 
                      boost::asio::buffer(data_, length), 
                      [this](boost::system::error_code ec, size_t length) {
                          if { (!ec) g_clients[my_id].do_read(); }
                          else { g_clients.erase(my_id); }
                      }
                  );
              }

              int main(int argc, char* argv[]) {
                  try {
                      boost::asio::io_context io_context;
                      tcp::acceptor my_acceptor{ io_context, tcp::endpoint(tcp::v4(), PORT) };
                 
                      my_acceptor.async_accept(
                          [&my_acceptor](boost::system::error_code ec, tcp::socket socket) {
                              accept_callback(ec, socket, my_acceptor); 
                          }
                      );
                 
                      io_context.run();
                  } catch (std::exception& e) {
                     std::cerr << "Exception: " << e.what() << "\n";
                  }
              }

    → 문제점 : SESSION 소멸자 호출 시 socket 객체 또한 소멸자가 호출된다.
                → atomic<shared_ptr<T>>를 사용해야 한다.

                mutex 사용은 엄청난 병렬성 감소를 유발한다.
                → array 또는 병렬 컨테이너를 사용해야 한다.
                   → concurrency::concurrent_unordered_map<int, atomic<shared_ptr<SESSION>>> : 소켓 객체가 copyable하지 않으므로 atomic에서 오류 발생

 ====================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================

 ■ 모바일 서버

 1. 모바일 게임
    → 구조 : 유저 정보를 게임 서버 DB에 저장
              
              게임 서버는 스테이지 정산만을 수행
              → 해킹에 매우 취약
                 웹 서비스로 구현 가능  // 클라이언트는 3D 그래픽 웹 브라우저

              서버 분리 X
              → DB 부하가 크므로 NoSQL 사용

              기능 별로 복수의 서버가 분산 처리
              → SPOF : 특정 시스템 고장 시 전체 시스템이 멈출 때, 특정 시스템을 이르는 말
                        → MMORPG에서는 Sharding으로 인해 문제가 되지 않았다.
                        → 해결책 : 중복을 통해 일부분만 DOWN
                                    마이그레이션을 통한 대체

    → 발전 추세 : 실시간 온라인 플레이의 추가
                   → MMORPG 이외는 인스턴트 던전 형식의 플레이
                      → Hotspot과 시야의 개념이 없는 서버
                         멀티쓰레드 SESSION이 필요 없어 구현 난이도 低


 2. PC와의 차이
    → Hand Over : 모바일은 소켓 연결이 자주 끊긴다.
                   → 소켓 연결이 끊어진 경우 서버와 클라이언트에서 Transparent하게 재접속

    → 클라우드 서버 : 서버를 실제 컴퓨터가 아닌 가상 머신에서 실행
                       → 가상 머신은 여러 대의 컴퓨터 중 아무데서나 실행 가능
                          → 유연성 : 컴퓨터 고장 시 옆 컴퓨터에서 실행
                                      → 가상머신은 CPU, 메모리, 디스크 데이터의 묶음인 파일이므로 가능

                             확장성 : 가상 머신을 복사하여 빈 컴퓨터에서 실행

                       → 장점 : 관리가 쉽다.
                                 경제적이다.
                                 → 서버의 부하가 없을 경우 하나의 컴퓨터에서 여러 가상 머신을 실행하면 된다.
                          
                          단점 : 느리다.

                       → 주의점 : 오픈 소스에 친숙해야 한다.
                                   서비스 게임이 많아지면 자체 클라우드를 운영하는 것이 저렴하다.
                                   기존 게임을 클라우드로 이전할 때 성능 측정을 미리 해보아야 한다.

*/