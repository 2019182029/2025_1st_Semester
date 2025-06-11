/*

 �� �þ� ó��

 1. ��� ����
    �� �� ������Ʈ�� ���°� ����Ǹ� ������ �����ϴ� ��� �÷��̾�� ���� ������ Broadcasting �ؾ� �Ѵ�.
       �� ���� N�� �� N * N�� ��Ŷ : 1�ʿ� 1�� �̵�
                                     ���� 5000 �� 25M packet/sec
                                     1000 CPU cycle per packet �� 25GHz
                                     20byte per packet �� 4Gbps bandwidth


 2. �ذ�å
    �� WSASend ����ȭ : Ŀ�� ����ȭ�� �ʿ��ϹǷ� �Ұ���
    
       Broadcasting ����ȭ : RIO �Ǵ� IO-uring�� ���

       WSASend Ƚ�� ���� : ���� ���� WSASend�� ��Ƽ� ������.
                           ���� ���� �ִ� �÷��̾�Ը� ���� �������� �˸���.
                           �� ���� N * N���� N * K�� ����
                              ȿ������ �˻� �ʿ�


 3. ȿ������ �˻�
    �� Zone : ��ü ���带 ū ������ ������ �ɰ� ��
              �� ������ �ٸ��ų�, �ٴٰ� ���θ��� �ְų�, etc...
                 Seamless���� �ʾ� Zone �̵� �� �ε� �ʿ�

       Sector(Cluster) : ������ �˻� ȿ������ ���� ��������, Ŭ���̾�Ʈ�ʹ� �ƹ��� ����� ���� �����̴�.
                         �� �ڽŰ� ���� Sector�� �˻��Ͽ� �˻� ��� ������Ʈ�� ������ ���δ�.
                            �� Sector���� Sector�� �����ϴ� ������Ʈ�� ����� �����ϴ� ��Ƽ������ �ڷᱸ�� �ʿ�
                               ��� ������Ʈ�� �̵�, ����, �Ҹ� �� �ڽ��� ���� Sector�� ������Ʈ ����� ������Ʈ

                         �� Sector�� ũ��� �����ؾ� �Ѵ�.
                            �� �ʹ� ũ�� : �þ� ���� ���� ��ü�� ���� �˻��Ǿ� �˻� ��� ����
                                           ���� �����尡 �� ���͸� ���������ν� ���ļ� ����

                               �ʹ� ������ : ���� Sector�� �˻��� �ʿ� ��
                                             �̵� �� ���� ���� ���� ������� �߻�


 4. �þ� ó��
    �� �⺻ �˰��� : �̵� ���� ���� �þ� ���� �����ϴ� ��ü ��
                       �� �þ߿� ���´ٸ� ���ο��� ���־� ���� ����
                          �þ߿��� ������ٸ� ���ο��� �Ҹ� ��ȣ ����
                          ��� �����ϴ� �÷��̾�� �� ��ǥ ����
                          �� MOVE ��Ŷ�� ��� ������ ��Ƴ��ٸ� ���� ���ϰ� �����ϹǷ� ���� ��Ŷ�� �����Ͽ� ����� �ʿ� ��

                       �� ��Ƽ�����忡�� �����ϹǷ� �̵� ��ü���� Data Race �߻�
                          �� ���� : �þ߿��� ������µ� Ŭ���̾�Ʈ���� ��� ����
                             ���� : ���־� ������ ���� ���� ��ü�� �̵�, ������ �ʴ� ��ü�� �ٸ� ��ü��� ��ȣ�ۿ�

                          �� �ذ�å : ���� �̵� ����
                                      �� Mutex ������� ���� ���� ����
                                         �� Sector ���� �÷��̾�� ���̿����� Lock�� �����ϴ� ������ ���� ���� ����

                                      ����
                                      �� Ŭ���̾�Ʈ�� ǥ�õǴ� ��ü���� ����Ʈ�� ������ ����, �̵� �� �̸� ������� �þ� ó��
                                         �� ���� ���� : ����Ʈ�� ������ ������ ���־� ���� ����
                                            ���� �ּ�ȭ : �� ��° �̵������� ������ ���� ����, ���� �þ� �������� ���� ���� �������� �ʴ´�.


 5. �þ� ����Ʈ
    �� Ŭ���̾�Ʈ�� ǥ�õǰ� �ִ� ��ü�� ����Ʈ
       �� ����ȭ : �þ� ����Ʈ�� ������Ʈ�� ���� �ϴ� ���� �ƴ϶�, �����Ͽ� ����� �� �� �þ� ����Ʈ�� ��ü
                   �� �þ� ����Ʈ�� ������ ���� Lock�� �����ϹǷ� Lock���� ���� ���� ���� �ּ�ȭ
                      ���� ����� �þ� ����Ʈ�� ���ÿ� Lock�ϴ� �������� �߻��� �� �ִ� ����� ȸ��

                   �̵� �� ������ �þ� ����Ʈ�� �������� �ʰ�, ���濡�� ����, �̵�, ��Ż �޼����� ������.
                   �� Mutex�� �ʿ� ��������.
                      Non-Blocking Concurrent Queue�� �ʿ��ϴ�.

                   NPC�� �þ� ����Ʈ�� ���� �ʰ�, �ʿ��� ������ Sector �˻� �� Near List ����
                   �� NPC �̵� �� �߻��ϴ� NPC�� �þ� ����Ʈ ������Ʈ ������� ����

       �� �̵� �� �þ� ó�� ���� : Sector �˻� �� Near List ����
                                   �� Near List�� ��� ��ü�� ���� : �þ� ����Ʈ�� ������
                                                                     �� �þ� ����Ʈ�� �߰�
                                                                        ������ ����� ���־� ���� ����
                                                                        ��� �þ� ����Ʈ�� ������ ���� �� ��ǥ ����
                                                                        ��� �þ� ����Ʈ�� ������ ��� �þ� ����Ʈ�� ���� �߰��� �� ��뿡�� ���� ���־� ���� ����

                                                                     �þ� ����Ʈ�� ������
                                                                     �� ��� �þ� ����Ʈ�� ������ ���� �� ��ǥ ����
                                                                        ��� �þ� ����Ʈ�� ������ ��� �þ� ����Ʈ�� ���� �߰��� �� ��뿡�� ���� ���־� ���� ����

                                      �þ� ����Ʈ�� ��� ��ü�� ���� : Near List�� ������
                                                                       �� �þ� ����Ʈ���� ����
                                                                          ������ ����� �Ҹ� ��ȣ ����
                                                                          ��� �þ� ����Ʈ�� ������ ��� �þ� ����Ʈ���� ���� ������ �� ��뿡�� ���� �Ҹ� ��ȣ ����

 ====================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================

 �� ����

 1. ���� ����
    �� ������ ������ �ν��ؾ� �Ѵ�.
       �� Ŭ���̾�Ʈ�� ������ �ִ� �����͸� ������ �����ϴ� ���� ������ : �̵��� �ʿ��� ������ ���
                                                                          �� Ŭ���̾�Ʈ�� �ƹ�Ÿ �̵��� ������ ���� ���

       �� Ŭ���̾�Ʈ������ ���� �浹 �˻縦 ����
          �� UI ���� �ӵ� ��� : �ƹ�Ÿ�� �̵�, ������ ����, ȭ�� �ܳ� ���� ��� ����
          
          ��ŷ �õ��� �������� ���� ������ �����Ͽ� �����Ѵ�.
          �� ������ ���� ���� ��쿡�� �̵����� �� ���� �� �� �ִ�.
             �������� �ź��ϱ� �� ���������� ��Ŷ ���� ���� �������� �����Ѵ�.
             ������ ��Ŷ ������ ������ �� �ִ�.


    �� �޸� �뷮�� �˻� �ӵ��� �߿�  
       �� �޸� �뷮 : ������ ���е� ����
          
          �˻� �ӵ� : ���� �������� ũ��  // ���� * ���⵵
                      �� ���� ���� = O(���� �� * �˻� �ӵ�)
                         
          �� �޸� �뷮�� �˻� �ӵ��� Trade Off ����
             �� ���� ����� ���� �� ū �޸𸮸� ����ϴ� ���� Cache Hit Ratio�� ����߸��Ƿ� �ٶ������� �ʴ�.


 2. 2D ����
    �� 2D �迭 : ���� �� ��� ��ü�� ��ǥ�� ����
                 2D �̹����� ���� ����� �ۼ�
                 �̵� ���� ���� Flag�� Cell���� ����
                 �� bit can_move[WORLD_WIDTH][WORLD_HEIGHT]
                    �� WORLD_WIDTH, WORLD_HEIGHT�� ��ü ���� ũ��� ��ֹ� ǥ�� ���е��� ���� ���� : ���� ũ�� / ��ֹ� �ּ� ũ��

                 �� ���� : �ſ� ���� �ӵ�, �޸� ����
                    ���� : 3D ���� ǥ�� �Ұ���


 3. 3D ����
    �� ���� ������ ���� �ʿ��ϸ�, �̵� �� ���� �˻簡 �ʿ��ϴ�.
       ����� �ۼ��� �� �����Ƿ�, �浹 �˻�� ������ �ڵ� ���� �ʼ�

    �� Ȯ�� Ÿ�� : 3D �迭
                   �� ���� �� ��� ��ü�� ��ǥ�� �Ǽ�
                      ���� ���� 2D �迭�� ���� ���� ǥ��  
                      �޸� ���� ���� 2D�� �⺻���� �Ϻκи� 3D�� ǥ���ϴ� Sparse Matrix ���
                   
                   �� ���� : ���� �ӵ�
                      ���� : ������ ��ü ���� ǥ�� �Ұ���

       Polygon : Ŭ���̾�Ʈ�� ������ �ִ� �����͸� �����Ͽ� ���
                 �� ���� �� ��� ��ü�� ��ǥ�� �Ǽ�
                    �޸� ��뷮�� Ȯ�� Ÿ�ϰ� ū ���̰� ����.
                    Ŭ���̾�Ʈ�� ������ ������� �̵� ���� ���θ� �˻��Ѵ�.  
                    �� ���� ���� ��� �ʿ�
                       3D ���� ������ Dedicated Server ��� ��� ����  // ��, ���� ���� ���� ����
                    
                 �� ���� : ���� ��Ȯ��
                    ���� : ���� �ӵ�


 4. �� ã��
    �� �� ã�� �ڷᱸ�� �ʿ�
       �� Path Node : ��ü�� ��ġ�� ����� ��ġ�� �Ϻ��� ��ġ�� Ȯ���� ���� �������� �ʴ´�.
       
          Path Mesh : ��ü�� �ö� �� �ִ� ������ ���͸� �� ����� ������ �ռ�
                      �� �ִ��� ū ���� �ٰ������� �ռ�
                         ����ȭ�� ���� �ణ�� ��� ���� ����
                         �� �ٰ����� Node�� �ϰ�, ���� ���θ� Edge�� �� �׷��� ����
                            �� ���� ���� ��縦 ����� �Ϲ����� �׷���
                               ���� ���� ���� �̵� �����ϸ� ����� ���

    �� �⺻���� �� ã�� ��� : ���� Step ���ϱ�  // ���� �ð��� �� �� �ִ� ���� ���� ��ġ
                               �� Timer�� ���� �̵����� ��Ŷ ���� ����, ��� �ð� ���� 
                                  �� Step���� �� ã�⸦ �ٽ� ������ �ʿ� ��


 5. �� ã�� �˰���
    �� ���鼭 ã�� : ���� ��ü�� �� �� ���� ���  
                     �� ����
                        ��ֹ� ���� ����
                        ���� ã�� ����

       �̸� ã�� : Depth First Search
                   �� IDDF : Ž�� ���� ���� �ʿ�
                    
                   Breadth First Search
                   �� Bidirectional Breath First Search : ���� ���� ��ǥ ��� ���ʿ��� Breadth First Search ����

                      Dijkstra's Algorithm : ��忡 Cost�� �� �� �ִ�.
                                             Tile ������ �ƴ� �׷��� ������ ����
                                             �׻� ������ ���� �����Ѵ�.

                      Best First Search : Dijkstra's Algorithm�� Cost�� �޸���ƽ���� ��ü
                                          �׻� ������ ���� ���������� �ʴ´�.

                      A* Search : Dijkstra's Algorithm + Best First Search
                                  F(n) = G(n) + H(n)
                                  �� F(n) : ��� n�� ���
                                     G(n) : ���� ��忡������ ��� n������ �ּ� ���
                                     H(n) : ��ǥ �������� �ٻ� ���
       
 ====================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================

 �� NPC

 1. NPC
    �� �ΰ����� 
       �� ������ �ΰ������� Script�� ���� : ���� ���� ���������� ����
                                            �� ���� ���α׷����� ���� ���� ��ȹ�ڰ� �ۼ�  // ���� ���α׷��Ӵ� Script ���� �ý����� �����ϰ� ���� Script�� �ۼ��� �ʿ� ��
                                               ���� ������ �ʿ� ��

       �� �ɵ��� �ΰ����� : �÷��̾��� �̵��� ��ó NPC���� Broadcast
                            �� �÷��̾ �������� ���� �� �ν����� ���ϴ� ��찡 �߻��� �� �����Ƿ�, �÷��̾� �Ǵ� NPC ���� �ÿ��� Broadcast

    �� NPC ����
       �� ���� : ������  // NPC ������ ����Ǿ ���� ������ ���� �۵�
                 ���� �л�

          ���� : ���� �������� ��� �������
                 �� ���� �޸� ������ ��Ŷ ������� ��ȭ
                    ���� ���忡���� NPC�� �÷��̾ ������ ����


 2. ��� �����̳ʿ� ��ƾ� �ϴ°�?
    �� ���� �����̳� : NPC�� SESSION ������ ���� ���� �߻�
                       �� PC�� NPC�� �����ϴ� ��� : PC�� NPC�� �����ϴ� ���� �߰�
                                                     �� �����̳� �˻� ������� �߻�
                                                        PC�� NPC�� ID ������ ���� ���������ν� �ذ�

       �ٸ� �����̳� : ID�� ������ �����ϴ� ������� �߻�


 3. ����
    �� while (true) {
           if (A_done == false) {
               DO(A);
               A_done = true;
           }
       }
       �� ������ : �� �������� ���� ���� if�� �˻� 
                   Busy Waiting
                   �� ĳ�� ����, Pipeline Stall

          �ذ�å : heart_beat �Լ��� �ΰ� ���� �ð� ���ݸ��� ȣ���Ͽ� Busy Waiting�� ������ �Ѵ�.

    �� Cobj::heart_beat() {
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
       �� ������ : ����ؼ� ȣ��Ǵ� heart_beat()�� �ƹ��͵� �������� �ʴ´�.

          �ذ�å : �ʿ��� ��츸 heart_beat()�� ȣ��ǵ��� �Ѵ�.
                   �� ������ �ΰ������� ��� ���α׷����� ���������.
                      �Ҹ��� �ʴ� ��츦 �Ǵ��ϴ� �� ��ü�� �������

                   �� ��⿡�� timer�� ���� ����Ѵ�.

    �� struct event_type {
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
                   PostQueuedCompletionStatue(h_iocp, 0, id, eo);  // timer thread�� �����ϸ� �����ϱ� ���� ���� �۾��� worker thread���� �����ؾ� �Ѵ�.
               } 
               while (true);
               Sleep(1);
           }
           while (true);
       }
       �� ������ : �÷��̾��� �þ� ����Ʈ�� �ִ� NPC�� �ൿ�ؾ� �Ѵ�.
          
          �ذ�å : �÷��̾ ��ó�� ������ Cobj::wake_up() ȣ��
                   �÷��̾ ��ó�� ������ Cobj::sleep() ȣ��
                   �� m_is_active ������ ���� �ߺ� Cobj::wake_up() ȣ�� ����  // Mutex ��� CAS�� ����ϴ� ���� ���� ��

    �� Cobj::wake_up() {
           if (!m_is_active) {
               bool expected = false;

               if (std::atomic_compare_exchange_strong(&m_is_active, &expected, true)) {
                   add_timer(m_event, 1000);
               }
           }
       }

 ====================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================

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

       C# : ��Ƽ������ ����
            C���� ȣ�� ������� ����

       XML : �� �ƴϴ�.
             �ֱٿ��� JSON ���


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
                �� LUA �Լ� ȣ�� �� �Ű����� Push : lua_pushnumber(L, arg);
                   Stack�� �۷ι� ���� �� �����ϱ� : lua_getglobal(L, "rows");
                   Stack�� ����� �� �б� : (int)lua_tonumber(L, -1);

       �ǽ� 3
       �� // exl.lua
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
                                         �� LUA�� ����
                                            Event�� ���� ������ LUA �Լ� ȣ��
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
        
 ====================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================

 �� DB

 1. DB
    �� ����ȭ�� �������� ����
       ��ǻ�Ϳ� ����, ������ ����Ͽ� ����

    �� �䱸 ����
       �� ����� : �ε����� ���� ��� �˻�
          Ʈ�����, ���ü�
          ���� : ������, ���� �л�
          ����

    �� ��� ����
       �� ������ ũ�� : ���� ���α׷��� ��� �����͸� �޸𸮿� ������ �ֱ� ��ƴ�.
          ������ ���� : ���� ���α׷��� ����Ǿ �����Ͱ� ���� �־�� �Ѵ�.
          ������, ȿ���� : ��� DB���� ������ ������ ���ϴ� ���α׷��� ����� ��ƴ�.

       �� ���� : ���� ���� ����� �� �ִ�.
                 �� 3���� ������ ���� ���α׷� �ۼ��� �����ϴ�.

          ���� : �� �𸣴� ���¿��� ����ϸ� �ذ��ϱ� ����� ������ �´ڶ߸� �� �ִ�.


 2. ������
    �� DB�� ����Ǵ� ������ : ���� ���� ��
                              �� ���� �α״� ���常 �����ϰ� ���� �ʴ´�.

       DB�� ������� �ʴ� ������ : ���� ����� �� �ʱ�ȭ�Ǵ� ����
                                   ���� �÷��� �� ������� �ʴ� ����
                                   �� ���Ϸ� ����Ǿ� ���� ���� �� �޸𸮿� �д´�.

    �� ������ �󸶳� ���� DB�� �����ؾ� �ϴ°�?
       �� �ʼ� : ĳ���� LOGIN, ĳ���� LOGOUT, ���� BOOTING, ���� SHUTDOWN
          �߰��� : ������ �ŷ�       // DB �ݿ� ���� �ŷ� ����
                   �ֱ��� Auto Save  // �߿䵵�� �ֱ�� �ݺ��
                   �߿��� ������ ����
                   �� ���� �ٿ�� �����ϰ�!


 3. ���� ����
    �� DB ������ Blocking API : DB ���� ������� Worker �����带 �и�
                                �� DB ���� �������� ���� �ʿ�
                                   Multiple Write / Single Read Concurrent Queue �ʿ�
                                   ���μ��� ���� ��� ������� ����
                                
                                �ٸ� ��ǻ�Ϳ��� ���� : ���Ͽ� �޸� �л�
                                                       Cache ���� Ȥ�� Query ������� �Ҹ��⵵ �Ѵ�.

       DB ���ٸ��� SQLConnect�� ����ϴ� ���� ������� �� : SQLConnect�� ������ �ڵ��� ��� �����صΰ� ���
                                                            �� ��Ƽ�������� ���
                                                               �� ��� 1 : Lock ��� 
                                                                  ��� 2 : �����帶�� �ڵ� ����


 4. SQL 
    �� DB�� �����͸� �����ϴ� ���
       �� CREATE : CREATE TABLE items (id INT, type INT);
          UPDATE : UPDATE items SET type = 3 WHERE id = 42;
          SELECT : SELECT * FROM items WHERE id = 42;
          INSERT : INSERT INTO items (id, type) VALUES (1043, 21);
          DELETE : DELETE FROM items WHERE id = 1043;


 5. ODBC
    �� ���α׷��� ���� SQL�� ����ϱ� ���� ǥ��
       �� #include <sqlext.h>
          
          HENV henv;    // ODBC ȯ�� �ڵ�
          HDBC hdbc;    // DB ���� �ڵ�
          HSTMT hstmt;  // SQL ���� �ڵ�

          SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);                           // ODBC ȯ�� �ڵ� ����
          �� �ڵ��� ����, ���� �ڵ�, ���� ������ �ڵ��� ���� ������

          SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER*)SQL_OV_ODBC3, 0);         // ODBC ���� ����
          �� ȯ�� �ڵ�, ������ �Ӽ�, �Ӽ� ��, ���ڿ��� ��� ���ڿ��� ����

          SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);                                      // DB ���� �ڵ� ����

          SQLSetConnectAttr(hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0);                     // �α��� Ÿ�Ӿƿ� ����
          �� ���� �ڵ�, ������ �Ӽ�, �Ӽ� ��, ���ڿ��� ��� ���ڿ��� ����

          SQLConnect(hdbc, (SQLWCHAR*)L"2025_game", SQL_NTS, NULL, 0, NULL, 0);             // DB ����
          �� ���� �ڵ�, DSN, ���ڿ��� ��� SQL_NTS, ID, 0 �Ǵ� SQL_NTS, PW, 0 �Ǵ� SQL_NTS

          SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);                                    // SQL ���� �ڵ� ����

          SQLExecDirect(hstmt, (SQLWCHAR*)L"EXEC stored_procedured param", SQL_NTS);        // Stored Procedure ȣ��
          �� SQL ���� �ڵ�, Stored Procedure, ���ڿ��� ��� SQL_NTS
             �� Stored Procedure :  �Ϸ��� ������ SQL�� ���α׷����Ͽ� DB�� �����س��� ��
                                    �� ���� : ���� ���
                                              Transaction ����
                                              ��Ʈ��ũ Ʈ���� ����
                                              ����

          SQLBindCol(hstmt, 1, SQL_C_LONG, &user_id, sizeof(user_id), &cb_user_id);         // ��� �÷��� C++ ������ ���ε�
          �� SQL ���� �ڵ�, ������� �� ��° �÷�����, C Ÿ��, �����͸� ������ ������ �ּ�, ���� ũ��, ���� ���� ���� ������ ������ �ּ�

          SQLFetch(hstmt);                                                                  // ���� ��� �� ��������
          �� SQL ���� �ڵ�

          SQLCancel(hstmt);                       
          SQLFreeHandle(SQL_HANDLE_STMT, hstmt); 
          SQLDisconnect(hdbc);                    
          SQLFreeHandle(SQL_HANDLE_DBC, hdbc);    
          SQLFreeHandle(SQL_HANDLE_ENV, henv);

    �� ���� �߻� �� ��ó
       �� ������ ũ�⸦ ��Ÿ���� �Ű����� Ȯ��  // ���ڿ��� ��� SQL_NTS�� ����ؾ� �Ѵ�.
          SQLCHAR�� SQLWCHAR�� �ٲٰ�, ��� ���ڿ��� "..."���� L"..."���� �ٲ۴�.
          �ڵ��� Ÿ�Ӿƿ� �ð��� �����Ǿ� �ִ�.  // �⺻ 8�ð�

 ====================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================

 �� ����

 1. ����
    �� ��ǻ�� ���� : ��Ŀ�� ���� ���α׷� ������ ����
                     �� Ŭ���̾�Ʈ ��ǻ�� ���� : �Ұ���, ���� ��� ������ �������� ������ �ʿ� ��  
                                                 �� ���� ���� ��ġ�� ���� ���� ����Ʈ���� ����
                                                    �� ��� : OTP
                        
                        Ŭ���̾�Ʈ ���α׷� ���� : ����ȭ, Ŭ���̾�Ʈ ���� �˻�

                        ���� ��ǻ�� ���� : ����� ��                                      
                                           �� ���� ��ġ, ��ȭ��
                        
                        ���� ���α׷� ���� : ���� ���α׷� ��ŷ ����, Ŭ���̾�Ʈ ��ŷ ����    
                                             �� Ŭ���̾�Ʈ�� ���� ���� ���� ���� Ȯ��

       ��Ʈ��ũ ���� : ��Ʈ��ũ�� ���� ���� ������ ����� ���� ����
                       �� �� �и�

                          ��Ŷ ��ȣȭ : ���ͳ��� ��û�� �����  // ����� ������ ������ ����� ����
                                        �� SSL : ǥ��, ���������� ������ ū ����
                                           ������ XOR ��ȣȭ : ��������� ������ ���� ����
                                           ���� Ű ��� : ���, �ǽð� Ű �������� ����
       

 2. ���� ���α׷� ��ŷ
    �� Stack Overflow : ���ڿ� ���� ���� �� strcpy ��� strncpy, strcpy_s ����� �ʿ� ��
                        �� Ŭ���̾�Ʈ ���α׷� ��ŷ �� BUFF ũ�⺸�� ū ���ڿ��� �����ϸ�, ���� ���α׷��� Stack�� ���ڿ��� ������.
                           �� strncpy, strcpy_s�� strcpy ũ�⸦ �����ϹǷ� Stack Overflow �Ұ���
                              �� Windows�� strcpy ��� �� ����� ����Ѵ�.
                                 Linus�� strcpy ��� �� ����� ������� �ʴ´�.  // Linux�� ��ŷ�� ����� ����

       SQL Injection : DB�� ����Ǵ� ���ڿ��� SQL ��ɾ �ɴ� ��
                       �� Ŭ���̾�Ʈ�� Ư�� ���ڸ� ��ȯ�Ͽ� ����, ������ Ư�� ���ڸ� �ؼ��Ͽ� ���
               
       ������ �鵵�� ����

       Auto ���α׷� : ������ ��� ���� �� ����� �� �϶� �� Auto ���α׷� ����� �� �϶� �� ���� ȸ�� ���� ����
                       �� �ذ�å : Ŭ���̾�Ʈ ����, ����� ����
                                   �� ��Ŷ ��ȣȭ, Ư�� ��Ŷ, ���� �޸� �˻�  // Delayed ó���� ��ȭ

                                   �� �ذ� �Ұ���? : �������� ������Ʈ�� Auto ���α׷� ���� ����
                                                     ��å, ��ȹ ����  // ���Ӽ� ���, Auto ���α׷��� ���ӿ� ����

 ====================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================

 �� Linux

 1. UNIX
    �� �ؽ�Ʈ ����� UI
       ��Ƽ���μ��� �ú��� �ü��
       

 2. LINUX
    �� ���� : ������ �δ�.
              ������.
              ���� �ҽ�

       ���� : �ͼ������� �����.

    
 3. Windows���� ��
    �� ��� : Visual Studio�� long�� 32bit, GCC�� long�� 64bit

       UI : �ؽ�Ʈ ����� UI

       ��Ƽ������ : POSIX ������ ���̺귯�� ��� �� C++11 ���

       ��Ʈ��ũ : epoll �Ǵ� kqueue ��� �� boost/asio ���

       DB : MySQL�� ���� ����ϸ�, ODBC ����� �����ϴ�.
            ���� ���� ���α׷������� �����ϱ⵵ �Ѵ�.
            REDIS ���� NoSQL�� LINUX�� �뼼�̴�.


 4. ���� ȯ��
    �� �����Ϸ� : GCC, G++(ANSI C++ ǥ��), CLANG
       ����� : GDB
       �������Ϸ� : prof, grof
       ���հ���ȯ�� : QT Creater, VS Code, Eclipse
       �޴��� : man
       DB : MySQL, NoSQL


 5. ���
    �� ���丮 : Windows�� ������ �ǹ�
       . : ���� ���丮
       .. : �θ� ���丮

    �� CPU ���� : lscpu
       �޸� ũ�� : meminfo
       ���� ���� ���μ��� : top
       ���ҽ� ��뷮 : nmon

       ���丮 Ž�� : ls
       ���丮 : cd, mkdir, rmdir  // ���丮 ������ '/'�� �Ѵ�.
       ���� ���� : mv, cp
       ���� ���� ���� : cat

       ���� ���� �ҽ� �ڵ�� ������ ������Ʈ ������ : make  // ������ �ҽ� �ڵ常 ��� �������ϴ� ��� ����
       ����� : gdb
                �� ��ɾ� : list, next, step, start(run), cont, break
                   ���� ���� Ȯ�� : print  


 6. ������
    �� VIM, Emacs : ���������� ���� �庮�� ����.
                    Ȯ�强�� �پ��.

       Gedit : ǥ�� ������
               GUI �ʼ�

       nano : �޸���
              �� nano hellp.cpp
                 g++ -o hello -g hello.cpp
                 ./hello

 ====================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================

 �� ASIO

 1. Boost
    �� C++ ���α׷��ֿ� ���Ǵ� ���̺귯�� ����  // ���̺귯���� ��� ���Ϸ� ����
       �� ���ø� ���α׷���
          �� ���� : ȣȯ��, ���� �ӵ�
             ���� : ������, ������ �ӵ�, �Ϻδ� ���̳ʸ� ���̺귯�� �ʿ�


 2. ASIO
    �� ���� API�� C++ API�� ���ۼ� : �������̾ ����ϱ� ���ϴ�.
                                     ���� �Լ��� �ſ� �������̴�.
                                     ���� ��ü�� ������ �����ؾ� �Ѵ�.

       �ü�� ���� �ֽ� API�� ����Ͽ� ����
       �� Linux : io_uring
          Mac : kqueue
          Windows : IOCP

    �� ���� : io_context�� socket �ʿ�
              �� boost::asio::io_context io_context;
                 boost::asio::ip::tcp::socket socket(io_context);

              �� ����� ���� : ���� API ȣ��
                               ������ ��û�� io_context�� ����
                               io_context�� �ü�� ȣ��
                               �ü���� ����� io_context�� ����
                               io_context�� ���� ó�� �� ���Ͽ� ��� ����
                               ������ ����� ���α׷��� ����
                                  
                 �񵿱�� ���� : ���� API ȣ��
                                 ������ ��û�� io_context�� ����
                                 io_context�� �ü�� ȣ��
                                 �ü���� ����� ť�� �ְ� io_context�� �ϷḦ �˷��ش�.
                                 ���α׷����� io_context::run() ȣ��  // run()�� �ϷḦ �߰��� ������ Block
                                 run()�� �ϷḦ �߰��� �� ����� ť���� �� �� my_handler�� ����

              �� ��Ƽ������ ���� : io_context�� ���� ��� ������ ��Ƽ������ safety�� �����Ѵ�.
                                   �� ���� ���� �����忡�� io_context::run()�� ȣ�������ν� ���ļ��� ���� �� �ִ�.

    �� �ּҴ� endpoint�� �����Ѵ�.
       �� boost::asio::ip::tcp::endpoint server_addr(boost::asio::ip::address::from_string("127.0.0.1"), 3000);
          boost::asio::ip::tcp::v4()�� ���� API�� INADDR_ANY�� �ش�


 3. API
    �� ���� : boost::asio::ip::tcp::accept�� ���� API�� Listen Socket�� �ش�
              �� boost::asio::ip::tcp::accept acceptor(io_context, endpoint);

    �� ����� �ۼ���
       �� size_t boost::asio::ip::tcp::socket::write(
              const ConstBufferSequence& buffers, 
              boost::system::error_code& ec
          );
          �� write : ������ ������ �� ���۵� ������ ���  // WSASend
             read : ���۰� �� �� ������ ���

          size_t boost::asio::ip::tcp::socket::write_some(
              const ConstBufferSequence& buffers, 
              boost::system::error_code& ec
          );
          �� ���۰� �� ���۵ǰų� ���� �ʾƵ� �Ϸ�  // WSARecv, Linux�� recv�� send

       �񵿱�� �ۼ���
       �� void boost::asio::ip::tcp::socket::async_write(
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
          �� buffers�� WSABUF�� ���������� ���� ���� ������ ����
             handler�� �ʿ�� �ϴ� �߰� ������ ���� �Լ��� ����

    �� �� ��
       �� Worker Thread : boost::asio::post()
          Timer Thread : boost::asio::steady_timer t(io_context, boost::chrono::seconds(1));
                         t.async_wait(&func);


 4. �ǽ�
    �� #include <SDKDDKVER.h>
       #include <boost/asio.hpp>

    �� Ŭ���̾�Ʈ : int main() {
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

    �� ���� : void accept_callback(boost::system::error_code ec, tcp::socket& socket, tcp::acceptor& my_acceptor) {
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

    �� ������ : SESSION �Ҹ��� ȣ�� �� socket ��ü ���� �Ҹ��ڰ� ȣ��ȴ�.
                �� atomic<shared_ptr<T>>�� ����ؾ� �Ѵ�.

                mutex ����� ��û�� ���ļ� ���Ҹ� �����Ѵ�.
                �� array �Ǵ� ���� �����̳ʸ� ����ؾ� �Ѵ�.
                   �� concurrency::concurrent_unordered_map<int, atomic<shared_ptr<SESSION>>> : ���� ��ü�� copyable���� �����Ƿ� atomic���� ���� �߻�

 ====================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================

 �� ����� ����

 1. ����� ����
    �� ���� : ���� ������ ���� ���� DB�� ����
              
              ���� ������ �������� ���길�� ����
              �� ��ŷ�� �ſ� ���
                 �� ���񽺷� ���� ����  // Ŭ���̾�Ʈ�� 3D �׷��� �� ������

              ���� �и� X
              �� DB ���ϰ� ũ�Ƿ� NoSQL ���

              ��� ���� ������ ������ �л� ó��
              �� SPOF : Ư�� �ý��� ���� �� ��ü �ý����� ���� ��, Ư�� �ý����� �̸��� ��
                        �� MMORPG������ Sharding���� ���� ������ ���� �ʾҴ�.
                        �� �ذ�å : �ߺ��� ���� �Ϻκи� DOWN
                                    ���̱׷��̼��� ���� ��ü

    �� ���� �߼� : �ǽð� �¶��� �÷����� �߰�
                   �� MMORPG �ܴ̿� �ν���Ʈ ���� ������ �÷���
                      �� Hotspot�� �þ��� ������ ���� ����
                         ��Ƽ������ SESSION�� �ʿ� ���� ���� ���̵� �


 2. PC���� ����
    �� Hand Over : ������� ���� ������ ���� �����.
                   �� ���� ������ ������ ��� ������ Ŭ���̾�Ʈ���� Transparent�ϰ� ������

    �� Ŭ���� ���� : ������ ���� ��ǻ�Ͱ� �ƴ� ���� �ӽſ��� ����
                       �� ���� �ӽ��� ���� ���� ��ǻ�� �� �ƹ������� ���� ����
                          �� ������ : ��ǻ�� ���� �� �� ��ǻ�Ϳ��� ����
                                      �� ����ӽ��� CPU, �޸�, ��ũ �������� ������ �����̹Ƿ� ����

                             Ȯ�强 : ���� �ӽ��� �����Ͽ� �� ��ǻ�Ϳ��� ����

                       �� ���� : ������ ����.
                                 �������̴�.
                                 �� ������ ���ϰ� ���� ��� �ϳ��� ��ǻ�Ϳ��� ���� ���� �ӽ��� �����ϸ� �ȴ�.
                          
                          ���� : ������.

                       �� ������ : ���� �ҽ��� ģ���ؾ� �Ѵ�.
                                   ���� ������ �������� ��ü Ŭ���带 ��ϴ� ���� �����ϴ�.
                                   ���� ������ Ŭ����� ������ �� ���� ������ �̸� �غ��ƾ� �Ѵ�.

*/