/*

 ■ 합의 객체

 1. 합의 객체
    → 새로운 동기화 연산을 제공하는 가상의 객체
       → Type_t decide(Type_t value) : n개의 쓰레드가 decide를 각각 한 번 이하로 호출한다.
                                        decide는 모든 호출에 대해 같은 값을 반환한다.
                                        decide가 반환하는 값은 전달된 value 중 하나이다.  // 높은 확률로 제일 처음 decide를 호출한 쓰레드가 선택된다.
                                        Atomic하고 Wait-Free하게 동작한다.

    → 합의 : 모든 쓰레드가 같은 결론을 얻는 방법
              → class Consensus {
                 private:
                     Type_t value;

                 public:
                     Consensus() { value = INIT; }  // INIT은 절대로 사용되지 않는 값

                     Type_t decide(Type_t v) {
                         CAS(&value, INIT, v);

                         return value;
                     }
                 };


 2. 합의수
    → 동기화 연산을 제공하는 클래스 C가 있을 때, 클래스 C와 Atomic 메모리를 사용해 N개 쓰레드에 대한 합의 객체를 구현할 수 있다.
       → 클래스 C가 N 쓰레드 합의 문제를 해결한다고 한다.
          최대의 N을 합의 수라고 하며, 최대 N이 존재하지 않는다면 그 클래스의 합의 수를 무한하다고 한다.

    → 문제 : Atomic 메모리만으로 N 쓰레드 합의 문제를 해결할 수 있는가?  // atomic_load(), atomic_store() 연산만을 사용해 N 쓰레드 합의 문제를 해결할 수 있는가?
              → 쓰레드 A와 B가 각각 0과 1로 합의 시도 : 쓰레드 A와 B는 임의의 개수의 공유 메모리에 대해 read(atomic_load())와 write(atomic_store()) 연산을 수행
                                                         → return값에 영향을 미치는 연산은 공유 메모리에 대한 read, write밖에 없다.
                                                            return값은 공유 메모리 연산이 어떠한 순서대로 실행되었느냐에 의해 결정된다.
                                                            → 같은 순서로 공유 메모리 연산이 실행되면 항상 같은 return값이 나온다.
                                                               return값은 모두 0이던가, 모두 1이어야 한다.
                                                               
                                                         알고리즘의 실행 과정 중 공유 메모리에 대한 read, write를 따로 분리하여 이동이라고 부른다.
                                                         → 쓰레드는 합의를 이룰 때까지 계산하며 이동한다.
                                                            → Wait-Free이므로 언젠가는 이동이 끝난다.
                                                               이동할 때에만 실행 상태(return값에 영향을 미치는 메모리, 레지스터)가 변경될 수 있다.
                                                               
                                                         알고리즘의 모든 실행 가능한 경로를 이진 트리로 나타낼 수 있고, 이를 프로토콜이라 부른다.
                                                         → 프로토콜의 생김새는 알고리즘과 input 값에 의해 결정된다.
                                                            실제 실행 시 어느 방향으로 진행할 지는 정해져 있지 않다.
                                                            → 어느 방향으로 진행하더라도 올바른 결과가 나와야 한다.


 3. 용어
    → 초기 상태 : 아무런 이동이 수행되지 않은 상태
       최종 상태 : 모든 쓰레드가 이동을 마친 상태  // 프로토콜의 Leaf

       일가 상태 : 앞으로 어떠한 이동을 하더라도 결과값의 변화가 없는 경우
       이가 상태 : 결과값이 결정되지 않은 상태
       임계 상태 : 다음의 이동으로 일가 상태가 되는 이가 상태
       → 모든 2 쓰레드 합의 프로토콜의 초기 상태는 이가 상태이다.
          → A만 실행하면 0, B만 실행하면 1을 결정해야 한다.
             즉, 실행 순서에 따라 결과가 바뀐다.

          모든 Wait-Free 합의 프로토콜은 임계 상태가 반드시 존재한다.
          → 트리의 높이는 유한하다.
             마지막 층의 노드는 모두 일가 상태이다.
             마지막 이가 상태의 노드가 존재하는 높이가 있다.
             그 높이에 존재하는 모든 이가 상태의 노드는 임계 상태이다.


 4. 증명
    → Atomic 메모리로 구현했다면 임계 상태가 반드시 존재하고, 그 때 이동 가능한 시나리오는 다음과 같다.
       → A는 Read, B는 Any : A의 읽기 연산은 A의 내부 상태만 변경
                              → B는 A가 read를 수행했는지 구분할 수 없으므로 결과값을 결정할 수 없다.

          A는 wrtie x, B는 write y : s'와 s''는 같은 상태
                                     → A, B 둘 중 누가 먼저 실행되었는지 구분할 수 없으므로 결과값을 결정할 수 없다.

          A는 write x, B는 write x : A의 쓰기 연산은 B의 쓰기 연산을 덮어씌운다.
                                     → A는 B가 write를 수행했는지 구분할 수 없으므로 결과값을 결정할 수 없다.

          → 어떠한 경우에도 임계 상태가 존재할 수 없으므로 read, write 이동만으로는 2 쓰레드 합의를 Non-Blocking하게 구현하는 것이 불가능하다.
             → Non Blocking 프로그램은 언젠가는 결과를 정해야 하고, 결과를 정한 쓰레드가 공유 메모리 연산으로 결과를 알려주지 않아도 다른 쓰레드는 결과를 알아야 한다.
                하지만, Atomic Memory의 read, write만으로는 모든 쓰레드가 정해진 결과를 확실히 알아내는 것이 불가능하다.
                   
 ====================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================
 
 ■ 다중 대입 객체

 1. 다중 대입 객체
    → 멤버 변수로 크기가 n인 배열을 갖고, 복수의 원소를 Atomic하고 Wait-Free하게 변경할 수 있는 객체
       → Assign : 인자로 m개의 값과 m개의 인덱스를 받아 대입한다.  
          Read : 인자로 인덱스를 i를 받아 n[i]를 반환한다.

       → Atomic 메모리로 (m, n) 대입 객체를 Wait-Free하게 구현할 수 있을까?
          → 증명 : class MultiConsensus {
                    private:
                        Assign23 assign23;
                        int proposed[2] = { INIT, INIT };

                    public:
                        MultiConsensus() { assign23.init(INIT); }

                        int decide(int value) {
                            int i = thread_id;
                            proposed[i] = value;
                            int j = 1 - i;
                            assign23.assign(i, i, i, i + 1);
                            int other = assign23.read((i + 1) % 3);
                            if ((other == INIT) ||
                                (other == assign23.read(1))} {
                                return proposed[i];
                            } else {
                                return proposed[j];
                            }
                        }
                    }
                    → (2, 3) 대입 객체로 2 쓰레드 합의를 구현할 수 있다.
                       → Atomic 메모리로 (m, n) 대입 객체를 Wait-Free하게 구현하는 것은 불가능하다.

    → (n, n * (n + 1) / 2)) 대입 객체는 최소 n의 합의 수를 갖는다.
       → 따라서, 대입 객체를 HW 구현 시 합의 객체를 Wait-Free로 구현할 수 있다.
          → HW 구현 비용이 너무 크므로 RMW를 대안으로 사용한다.


 2. RMW
    → HW가 지원하는 동기화 연산의 한 종류
       Wait-Free가 되기 위해서는 특수 명령어가 반드시 필요하다.
       → f(x) : Atomic하게 현재 메모리 값을 v에서 x로 바꾸고, 원래 값 v를 리턴한다.
                 → GetAndSet
                    GetAndIncrement
                    GetAndAdd(k)
                    CompareAndSet 
                    Get  // 항등 함수
                    → RMW 연산이 항등 함수가 아닐 때, 그 RMW 연산을 명확하지 않다고 한다.

    → 임의의 명확하지 않은 RMW 연산은 합의 수가 최소 2이다.
       → 증명 : class RMWConsensus {
                 private:
                     RMWobject r;
                     int proposed[2] = { INIT, INIT };

                 public:
                     RMWConsensus() { r.init(INIT); }

                     int decide(int value) {
                         int i = thread_id;
                         proposed[i] = value;
                         int j = 1 - i;
                         if (r.rmw() == INIT) {
                             return proposed[i];
                         } else {
                             return proposed[j];
                         }
                     } 
                 }
                 → Atomic 메모리로 2개 이상의 쓰레드에 대한 명확하지 않은 RMW 연산을 구현할 수 없다.


 3. Common2 RMW
    → 모든 값 v와 함수 집합 F에 속하는 모든 함수 fi, fj에 대해 둘 중 하나가 성립하는 경우
       → fi와 fj는 교환이 가능 : fi(fj(v)) = fj(fi(v))                               // GetAndIncrement
          한 함수가 다른 함수를 덮어쓰는 경우 : fi(fj(v)) = fi(v), fj(fi(v)) = fj(v)  // GetAndSet

       → 특징 : 오늘날의 프로세서에서 제거되는 추세이다.
                 합의 수가 2이다.
                 → 3 쓰레드 : A는 항상 자신이 승자라는 것을 알 수 있어야 하고, B와 C는 자신이 패자라는 것을 알 수 있어야 한다.
                               → Common2 RMW 연산은 교환이나 덮어쓰기가 가능하여 C는 A와 B 중 어느 것이 먼저 실행되었는지 판단할 수 없다.


 4. CAS
    → 무한대의 합의 수를 갖는다.
       → 증명 : class CASConsensus {
                 private:
                     int FIRST = -1;
                     AtomicInt r = FIRST;

                 public:
                     int decide(int value) {
                         propose(value);
                         int i = thread_id;
                         if (r.CAS(FIRST, i)) {
                             return proposed[i];
                         } else {
                             return proposed[r];
                         }
                     }
                 }
                 → 모든 자료구조를 n 쓰레드에서 Wait-Free하게 구현할 수 있다.

 ====================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================

 ■ 무잠금 만능

 1. 합의의 의미
    → 적은 합의 수를 갖는 객체로 큰 합의 수 객체를 구현할 수 없다.
       → 합의 수 1 : Atomic 메모리
          합의 수 2 : GetAndSet, GetAndAdd, 큐, 스택
          합의 수 n : (n, n * (n + 1) / 2) 대입 객체
          합의 수 X : 메모리 이동, CompareAndSet(), LL-SC

       → 의의 : 불가능한 시도를 미연에 방지할 수 있다.
                 구현 방법과 원리를 알면, 최적화에 도움이 된다.


 2. 만능 
    → 클래스 C와 Atomic 메모리로 모든 객체를 Wait-Free하게 변환하는 것이 가능하다면, 클래스 C는 만능이다.
       → 클래스 C를 사용하여 직접적으로 변환하지 않고, Atomic 메모리를 사용하여 대상 객체를 약간 변형한 후 변환한다.  // 메소드, 파라미터, 리턴값 통합

       → 만능 객체 : 어떠한 객체든 Wait-Free하게 변환하는 것이 가능한 객체
                      → n 쓰레드에서 동작하는 만능 객체는 합의 수 n 이상의 객체로 구현 가능하다.
                         → 무한대의 합의 수를 갖는 CAS 객체를 통해 쓰레드 개수에 상관없이 만능 객체를 구현할 수 있다.


 3. 순차 객체
    → 조건 : 순차 객체는 결정적이다.
              → 모든 객체의 초기 상태는 동일하다.
                 같은 상태에서 같은 입력을 주면 항상 같은 결과가 나온다.

    → 순차 객체 : 변환하고자 하는 객체를 감싼 객체
                   메소드를 apply 하나로 통일
                   → class Invocation {
                          MethodType type;
                          int v;
                      };
                      → Invocation : 변환하고자 하는 객체의 메소드와 입력값을 갖는 객체

                      class SeqObject {
                      private:
                          std::set<int> m_set;

                      public:
                          Response apply(Invocation invoc) {
                              Response r{ true };

                              switch (invoc.type) {
                              case ADD:
                                  r.m_bool = m_set.insert(invoc.v).second;
                                  break;

                              case REMOVE:
                                  r.m_bool = (0 != m_set.count(invoc.v));

                                  if (true == r.m_bool) {
                                      m_set.erase(invoc.v);
                                  }
                                  break;

                              case CONTAINS:
                                  r.m_bool = (0 != m_set.count(invoc.v));
                                  break;
                              }

                              return r;
                          }
                      };
                      → Response : 여러 메소드의 결과 값을 압축한 객체


 4. Lock-Free 만능 객체
    → class NODE {
       public:
           Invocation m_inv;
           int m_seq;
           NODE* volatile next;
       };
       → Log는 tail부터 시작하는 NODE의 리스트이다.
          → 지금까지 순차 객체에 가해진 모든 메소드 호출을 보관
             순차 객체와 Log를 조합하여 순차 객체의 모든 상태를 알 수 있다.

       class LFUniversal {
       private:
           NODE* volatile m_head[MAX_THREADS];
           NODE* tail;

           NODE* get_max_head() {
               NODE* h = m_head[0];

               for (int i = 1; i < MAX_THREADS; ++i) {
                   if (h->m_seq < m_head[i]->m_seq) {
                       h = m_head[i];
                   }
               }

               return h;
           }

       public:
           LFUniversal() {
               tail.m_seq = 0;
               tail.next = nullptr;
               
               for (auto& h : m_head) {
                   h = &tail;
               }
           }

           RESPONSE apply(Invocation invoc) {                                       
               NODE* prefer = new NODE{invoc, 0, nullptr};                          

               while (0 == prefer->m_seq) {                                         // prefer가 성공적으로 추가되었는지 검사
                   NODE* head = get_max_head();
                   long long temp = 0;

                   std::atomic_compare_exchange_strong(                             // 합의 객체를 사용하여 새로운 NODE를 Log의 head에 덧붙인다.
                       reinterpret_cast<volatile std::atomic_llong*>(&head->next),  
                       &temp,
                       reinterpret_cast<long long>(prefer));

                   NODE* after = head->next;
                   after->m_seq = head->m_seq + 1;                                  // 여러 쓰레드가 같은 작업을 반복할 수 있지만, 상관 없다.
                   m_head[thread_id] = after;                                       // 동일한 합의 객체를 두 번 이상 호출하지 않도록 한다.
               }

               SeqObject std_set;                                                   // 순차 객체를 생성한 후, 
               NODE* p = tail.next;

               while (p != prefer) {                                                
                   std_set.apply(p->m_inv);                                         // Log에 있는 Invocation을 새로운 호출까지 적용시키고,
                   p = p->next;
               }

               return std_set.apply(invoc);                                         // 그 결과를 반환한다.
           }
       }
       → 합의 객체가 노드마다 존재하며, 각 쓰레드는 head 배열을 통해 한 번 호출한 합의 객체를 다시 호출하지 않는다.
          합의 객체로 인해 한 NODE의 next는 어떤 쓰레드에서도 유일하다.
          순차 객체는 매 호출마다 생성되며, 다른 쓰레드는 절대로 해당 순차 객체를 호출하지 않는다.
          
    → Lock-Free인 이유? : 다른 노드에서 계속 head 배열을 갱신하는 경우 무한히 실행될 수 있기 때문
                         
 ====================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================
 
 ■ 무대기 만능

 1. Wait-Free 만능 객체
    → class WFUniversal {
       private:
           Node* announce[MAX_THREADS];
           Node* head[MAX_THREADS];
           Node* tail;

       public:
           WFUniversal() {
               tail.seq = 1;

               for (int i = 0; i < MAX_THREADS; ++i) {
                   announce[i] = &tail;
                   head[i] = &tail;
               }
           }

           Response apply(Invocation invoc) {
               announce[i] = new Node(invoc);
               head[i] = tail.max(head);

               while (announce[i]->seq == 0) {
                   Node* before = head[i];
                   long long temp = 0;

                   Node* help = announce[(before->seq + 1) % MAX_THREADS)];
                   Node* prefer;

                   if (help->seq == 0) {
                       prefer = help;
                   } else {
                       prefer = announce[i];
                   }

                   std::atomic_compare_exchange_strong(                            
                       reinterpret_cast<volatile std::atomic_llong*>(&before->next),
                       &temp,
                       reinterpret_cast<long long>(prefer));

                   Node* after = before->next;
                   after->seq = before->seq + 1;
                   head[i] = after;
               }

               SeqObject std_set;
               Node* p = tail.next;

               while (p != announce[i]) {
                   std_set.apply(p->invoc);
                   p = p->next;
               }

               head[i] = announce[i];

               return std_set.apply(p->invoc);
           }
       }
       → 합의할 때 우선순위가 높은 다른 쓰레드의 합의를 수행한다.  // 물론, 자기 자신이 합의 될 때까지 계속 합의를 시도한다.
          다른 쓰레드를 유일하고 공평하게 결정할 수 있으면 기아 상태에 빠지는 것을 방지할 수 있다. 
          → Node의 seq를 MAX_THREADS로 나눈 나머지 사용
             → 모든 쓰레드가 같은 결정을 한다. 
                자기 자신이 기아 상태에 빠지는 것을 방지할 수 있다.


 2. 옳음 증명
    → Log에 노드가 누락되는가?
       → 모든 apply는 자신의 Node가 추가될 때까지 실행된다.

       Log에 한 노드가 여러 번 추가될 수 있는가?
       → 같은 자리에 추가 : 합의 객체에 의해 해결된다.
          다른 자리에 추가 : 먼저 추가된 노드 a가 어디선가 한 번은 before가 된 적이 있다.
                             → before는 head[]에서 꺼내는 수밖에 없다.
                                a는 적어도 한 번 head[]에 들어간 적이 있다.
                                a가 head[]에 들어간 시점에서 a.seq는 0이 될 수 없으므로, prefer는 a가 될 수 없다.

       head[i] = announce[i]에서 head[i]가 후퇴하는 경우가 있는가?
       → announce[i] = new Node(invoc); 이후 announce[i]와 다른 노드가 다른 쓰레드에 의해 추가된 경우
          → 합의 객체에 의해 해결된다.
             반복문을 몇 번 더 돌아 비효율적일 뿐이다.

 ====================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================

 ■ 큐

 1. 풀
    → 특징 : 같은 아이템의 복수 존재 허용
              Get(), Set() 메소드를 제공한다.
              Contains() 메소드를 항상 제공하지 않는다.
              보통 생산자 - 소비자 문제의 버퍼로 사용한다.

    → 종류
       → 무제한 큐
          제한 큐 : 구현하기 쉽다.
                    생산자와 소비자의 간격을 제한한다.
                    → ConditionVariable() 메소드 필요
                       → 스케줄 될 때 Lock 획득
                          Lock을 가진 채로 Block 시 Lock 해제
                          운영체제 호출 필요  // 쓰레드 스케줄링과의 연동 필수

    → 성질
       → 완전 : 특정 조건을 기다릴 필요가 없다.  // 비어있는 풀에 Get() 메소드 호출 시 실패 코드 반환
          부분적 : 특정 조건의 만족을 기다려야 한다.  // 비어있는 풀에 Get() 메소드 호출 시 다른 누군가의 Set() 메소드 호출을 기다린다.
          동기적 : 다른 쓰레드의 메소드 호출과의 중첩이 필요하다.


 2. 큐
    → Enq
       → 단순한 구현 : tail이 가리키는 Node에 CAS로 새 노드 추가
                        성공 시 tail 이동
                        실패 시 재시도
                        → void Enq(int x) {
                               Node* n = new Node(x);

                               while (true) {
                                   if (CAS(&tail->next, NULL, n) {
                                       tail = e;
                                       return;
                                   }
                               }
                           }
                           → 문제점 : CAS 성공 후 tail의 전진이 이뤄지지 않을 시에도 다른 모든 쓰레드 Non-Blocking으로 동작해야 한다.
                                       → 해결책 : tail의 전진을 보조한다.

       → 1차 수정 : void Enq(int x) {
                         Node* n = new Node(x);

                         while (true) {
                             if (CAS(&tail->next, NULL, n) {
                                 tail = e;           // A
                                 return;
                             }

                             if (nullptr != tail->next) {
                                 tail = tail->next;  // B
                             }
                         }
                     }
                     → 문제점 : A, 이제는 안전하지 않다.
                                 B, 다른 쓰레드의 변경을 덮어쓸 수 있다.
                                 → 해결책 : CAS 사용

       → 2차 수정 : void Enq(int x) {
                         Node* n = new Node(x);

                         while (true) {
                             Node* last = tail;
                             Node* next = last->next;

                             if (last != tail) { continue; }

                             if (nullptr == next) {
                                 if (CAS(&last->next, nullptr, n)) {
                                     CAS(&tail, &last, n);
                                     return;
                                 } 
                             } else {
                                 CAS(&tail, &last, next);
                             }
                         }
                     }

    → Deq
       → 단순한 구현 : 큐가 비어 있는지 검사
                        head 전진
                        → int deq() {
                               while (true) {
                                   Node* first = head;

                                   if (first != head) { continue; }

                                   if (nullptr == first->next) { empty_error(); }

                                   if (CAS(&head, &first, first->next) {
                                       int value = first->next->item;
                                       delete first;
                                       return value;
                                   }
                               }
                           }
                           → 문제점 : 다른 쓰레드에서 first->next를 이미 delete 했을 가능성 有
                                       
       → 1차 수정 : int deq() {
                         while (true) {
                             Node* first = head;
                             Node* next = first->next;

                             if (first != head) { continue; }

                             if (nullptr == next) { empty_error(); }

                             int value = next->item;

                             if (CAS(&head, &first, next)) {
                                 delete first;
                                 return value;
                             }
                         }
                     }
                     → 문제점 : Enq에서 tail의 전진이 이뤄지지 않았을 시에도 Non-Blocking으로 동작해야 한다.
                                 → 해결책 : tail의 전진을 보조한다.

       → 2차 수정 : int deq() {
                         while (true) {
                             Node* first = head;
                             Node* last = tail;
                             Node* next = first->next;

                             if (first != head) { continue; }

                             if (nullptr == next) { empty_error(); }

                             if (first == last) {
                                 CAS(&tail, &last, next);
                                 continue;
                             }

                             int value = next->item;

                             if (CAS(&head, &first, next)) {
                                 delete first;
                                 return value;
                             }
                         }
                     }


 3. ABA
    → 문제점 : new(), free()는 메모리를 재사용하므로, CAS 수행 시 동일한 주소에 동일한 값을 갖는 다른 객체가 들어오는 경우 문제가 발생한다.  // Java는 Garbage Collector를 사용하므로 이러한 문제가 발생하지 않는다.
                → 해결책 1 : 포인터와 스탬프의 확장 구조체를 포인터로 사용한다.
                              → LL-SC 명령 : 값을 검사하는 것이 아닌, 변경 여부를 검사
                                              → CAS보다 우월하지만, Wait-Free가 아니다.

                   해결책 2 : 참조 카운터를 사용한다.
                              → std::atomic<std::shared_ptr<>> : 특정 노드를 참조 중인 쓰레드가 있을 경우, 해당 노드는 재사용될 수 없다.
                                                                  → std::atmoic<std::shared_ptr>>는 Lock-Free가 아니다.

                   해결책 3 : 별도의 메모리 관리 기법을 사용한다.
                              → EBR, Hazard Pointer, etc...


    → 이론
       → 32bit : ST_Ptr는 64bit 자료구조이므로, 캐시 경계선에 놓일 수 있다.
                  → std::atomic_llong으로 선언

          64bit : ST_Ptr는 128bit 자료구조이므로, 128bit CAS 필요
                  → BOOLEAN InterlockedCompareExchange128(
                         LONG64 volatile* Dest,
                         LONG64           ExchangeHigh,
                         LONG64           ExchangedLow,
                         LONG64*          ComparandResult
                     );
                     → 자료구조를 128bit 단위로 정렬해야 한다.  // 캐시 라인 문제
                        → class alignas(16) ST_Ptr {
                           public:
                               ST_Node* volatile ptr;
                               long long volatile stamp;
                               
                               ST_Ptr(const ST_Ptr& rhs) {
                                   ST_Ptr temp{ this->ptr, this->st };

                                   while (InterlockedCompareExchanged128(
                                       reinterpret_cast<LONG64 volatile*>(this),
                                       rhs.stamp,
                                       reinterpret_cast<LONG64>(rhs.ptr),
                                       reinterpret_cast<LONG64*>(&temp)
                                   ));
                               }

                               void set(ST_NODE* p) {
                                   ptr = p;
                                   stamp = stamp + 1;
                               }

                               ST_NODE* get_ptr() {
                                   return ptr;
                               }
                           }

                           class ST_Node {
                           public:
                               int v;
                               ST_Ptr next;
                           }

                     → bool CAS(ST_Ptr* next_ptr, ST_NODE* old_ptr, ST_NODE* new_ptr, long long old_st, long long new_st) {
                            ST_Ptr old_st { old_ptr, old_st };

                            return InterlockedCompareExchange128(
                                reinterpret_cast<LONG64 volatile*>(next_ptr),
                                new_stamp,
                                reinterpret_cast<LONG64>(new_ptr),
                                reinterpret_cast<LONG64*>(&old_st)
                            );
                        }

                     → 문제점 : 모든 ST_Ptr load와 store가 atomic하게 동작해야 하지만, atomic_128int는 존재하지 않는다.
                                 → 모든 load와 store에 InterlockedCompareExchange128을 사용해야 하므로 성능이 저하된다.
                                 
                                 메모리 재사용 시 head->next가 오염될 수 있다.
                                 → 해결책 : free_list를 사용하여 오염을 막는다.
                                             Lock-Free 혹은 thread_local로 구현해야 한다.

    → 구현 : class ST_Queue {
              private:
                  ST_Ptr head{ nullptr };
                  ST_Ptr tail{ nullptr };

              public:
                  void Enq(int x) {
                      ST_Node* n = nullptr;

                      if (free_node == nullptr) {
                          n = new ST_Node{ x };
                      } else {
                          n = free_node;
                          free_node = free_node->next.get_ptr();
                          n->next.set(nullptr);
                          n->next.stamp = 0;
                          n->v = x;
                      }

                      while (true) {
                          ST_Ptr last{ tail };
                          ST_Ptr next{ last.get_ptr()->next };

                          if (last.stamp != tail.stamp) { continue; }

                          if (nullptr == next.get_ptr()) {
                              if (CAS(&last.get_ptr()->next, nullptr, n, next.stamp, next.stamp + 1) {
                                  CAS(&tail, last.get_ptr(), n, last.stamp, last.stamp + 1);
                                  return;
                              }
                          } else {
                              CAS(&tail, last.get_ptr(), next.get_ptr(), last.stamp, last.stamp + 1);
                          }
                      }
                  }

              int Deq() {
                  while (true) {
                      ST_Ptr first{ head };
                      ST_Ptr last{ tail };
                      ST_Ptr next{ first.get_ptr()->next };

                      if (first.stamp != head.stamp) { continue; }

                      if (nullptr == next.get_ptr()) { empty_error(); }

                      if (first.get_ptr() == last.get_ptr()) {
                          CAS(&tail, last.get_ptr(), next.get_ptr(), last.stamp, last.stamp + 1);
                          continue;
                      }

                      int value = next.get_ptr()->v;

                      if (CAS(&head, first.get_ptr(), next.get_ptr(), first.stamp, first.stamp + 1) {
                          first.get_ptr()->next.stamp = 0;
                          first.get_ptr()->next.set(free_node);
                          free_node = first.get_ptr();

                          return value;
                      }
                  }
              }
          }

 ====================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================

 ■ 스택

 1. 스택
    → 연결 리스트로 구성
       top은 첫 번째 노드를 가리킨다.
       → -1을 스택에 추가하는 것은 고려하지 않는다.
          스택이 비어있을 경우 top은 nullptr
          스택이 비어있을 경우 Pop() 메소드는 -2를 리턴한다.


 2. 무잠금 스택
    → CAS를 이용하여 top의 변환을 Non-Blocking으로 구현한다.
       ABA 문제를 방지하기 위해 delete하지 않는다.

    → 구현 : class LF_STACK {
              private:
                  NODE* volatile top;

                  bool CAS(NODE* old, NODE* new) {
                      return std::atomic_compare_exchange_strong(
                          reinterpret_cast<volatile std::atomic_llong*>(&top),
                          reinterpret_cast<long long*>(&old),
                          reinterpret_cast<long long>(new);
                  }

              public:
                  void Push(int x) {
                      Node* n = new Node{ x };

                      while (true) {
                          Node* last = top;

                          n->next = last;

                          if (CAS(last, n)) { return; }
                      }
                  }


                  int Pop() {
                      while (true) {
                          NODE* last = top;

                          if (nullptr == last) { return -2; }

                          Node* next last->next;

                          if (last != top) { continue; }

                          int v = last->key;

                          if (CAS(last, next)) { return v; }
                      }
                  }
              }
              → 문제점 : 메소드 호출은 스택의 top에 대해 성공한 CAS 순서로 하나씩 진행되므로, 순차병목현상이 나타날 수 있다.
                          new, delete 사용 시 ABA 문제가 발생할 확률이 큐보다 높다.
                          경쟁이 심해질 경우 CAS가 실패할 확률이 높아지고, 모든 코어의 메모리 접근이 중단된다.


 3. BACK OFF 스택
    → CAS가 실패했을 경우 적절한 기간 동안 실행을 멈춘다.
       → 처음에는 짧게
          계속 실패하면 점점 길게
          첫 번째 시도에 성공하게 짧게
          쓰레드마다 기간을 범위 안에서 랜덤하게 설정

    → BACK OFF 객체 : CAS가 실패했을 경우 다음 CAS를 시도하기 전에 사용
                       → class BackOff {
                          private:
                              int minDelay, maxDelay,
                              int limit;

                          public:
                              void InterruptedException() {
                                  int delay = 0;
                                  if (0 != limit) delay = rand() % limit;
                                  if (0 == delay) return;
                                  limit = limit + limit;
                                  if (limit > maxDelay) limit = maxDelay;

                                  delay = delay * delay;
                                  for (int i = 0; i < delay; ++i) {
                                      mm_pause();
                                  }
                              }

                              void decrement() {
                                  limit = limit / 2;
                                  if (limit < minDelay) limit = minDelay;
                              }
                          }

    → 구현 : class LF_BO_STACK {
              private:
                  ...

              public:
                  void Push(int x) {
                      Node* n = new Node{ x };
                      bool first_time = true;

                      while (true) {
                          Node* last = top;
                          n->next = last;

                          if (CAS(last, n)) {
                              if (first_time) { bo.decrement(); }
                              return;
                          }

                          first_time = false;
                          bo.interruptedException();
                      }
                  }

                  int Pop() {
                      bool first_time = true;
                        
                      while (true) {
                          NODE* volatile last = m_top;
                           
                          if (nullptr == last) { return -2; }

                          NODE* volatile next = last->next;

                          if (last != m_top) { continue; }

                          int v = last->key;

                          if (true == CAS(last, next)) {
                              if (first_time) { bo.decrement(); }
                              return v;
                          }

                          first_time = false;
                          bo.InterruptedException();
                      }
                  }
              }
              → 절대 시간이 중요하지 않다.
                 루프에서 메모리 접근을 수행하지 않는 것이 바람직하다.
                 → _asm mov eax, delay;
                    myloop:
                    _asm dec eax
                    _asm jnz myloop;


 4. 소거
    → 큐와 스택은 리스트의 말단 부분에서 잦은 충돌이 발생하므로 fine-grained synchronization이 불가능하다.
       → 쓰레드가 서로 충돌할 경우 별도의 객체를 통해 Lock-Free로 데이터를 주고 받도록 한다.
          높은 경쟁에 대비하여 별도의 객체를 복수로 준비한다.
          → Push() 메소드와 Pop() 메소드가 거의 동시에 호출된다면 각각의 쓰레드는 스택 변화 없이 Lock-Free로 값을 교환할 수 있다.  // 서로를 소거한다고 표현한다.

    → EliminationArray : 여러 개의 원소를 갖고 부하를 분산
                          → 경쟁이 심할 경우 원소의 개수가 많아야 하고, 경쟁이 적을 경우 원소의 개수가 적어야 한다.
                             → Visit() 메소드 호출 시 RET_BUSY_TIMEOUT이 리턴되었을 경우 늘린다.
                                Visit() 메소드 호출 시 RET_TIMEOUT이 리턴되었을 경우 줄인다.

                          쓰레드는 임의의 원소를 골라 소거 시도
                          → 임의 X : 정보 전달 자체가 오버헤드이고, Lock-Free 구현이 어렵다. 

                          class EliminationArray {
                          private:
                              Exchanger exchanger[MAX_EXCHANGER];

                          public:
                              int Visit(int x) {
                                  if (range == 0) range = 1;

                                  int slot = rand() % range;

                                  int ret = exchanger[slot].exchange(x);

                                  int old_range = range;

                                  if (RET_BUSY_TIMEOUT == ret) {
                                      if (old_range > 1) {
                                           std::atomic_compare_exchange_strong(
                                               reinterpret_cast<std::atomic_int*>(&range),
                                               &old_range,
                                               old_range - 1);
                                      }
                                  } else if (RET_TIMEOUT == ret) {
                                        if (old_range < MAX_EXCHANGER - 1) {
                                           std::atomic_compare_exchange_strong(
                                               reinterpret_cast<std::atomic_int*>(&range),
                                               &old_range,
                                               old_range + 1);
                                  }

                                  return ret;
                              }
                          }

    → Exchanger : 두 개의 쓰레드가 Lock-Free로 값을 교환할 수 있게 해주는 객체
                   두 개의 쓰레드가 Exchange() 메소드를 호출하면 서로의 입력 값을 리턴한다.
                   리턴 서로의 입력 값과 상태(성공, 실패, 타임아웃(-2))이다.

                   → Exchanger는 3개의 상태를 갖는다.
                      → EMPTY : CAS를 이용하여 slot에 자신의 입력 값을 넣고 상태를 WAITING으로 바꾼다.
                                 → 성공 : 스핀을 하며 다른 쓰레드의 교환 시도를 기다린다.
                                           교환이 완료된 후 대기 중이던 쓰레드는 상태를 EMPTY로 바꾼다.  // 한 쓰레드만 작업하므로 CAS 사용 X

                                    실패 : 처음부터 다시 시도

                                 다른 쓰레드가 나타나지 않는다면, CAS를 이용하여 상태를 EMPTY로 바꾼다.
                                 → 성공 : RET_TIME_OUT 리턴
                                  
                                    실패 : 다른 쓰레드가 나타났다는 의미이므로, 대기 중이던 쓰레드는 교환을 완료한다.

                         WAITING : CAS를 이용하여 slot에 자신의 입력 값을 넣고 대기 중인 쓰레드의 입력 값을 얻는다.
                                   → 성공 : 대기 중인 쓰레드의 입력 값 리턴

                                      실패 : 처음부터 다시 시도

                         BUSY : 처음부터 다시 시도
                         → 값의 교환과 상태 변경을 Atomic하게 구현할 필요 有 : int의 상위 2bit를 상태를 나타내는 데 사용
                   
                   → class Exchanger {
                      private:
                          volatile unsigned int slot;

                          unsigned int get_slot(unsigned int* st) {
                              unsigned int t = slot;
                              *st = t >> 30;
                              unsigned int ret = t & 0x3FFFFFFF;
                              if (ret == 0x3FFFFFFF) { return 0xFFFFFFFF; } 
                              else { return ret; }
                          }

                          unsigned int get_state() {
                              return slot >> 30;
                          }

                          bool CAS(unsigned int old_v, unsigned int new_v, unsigned int old_st, unsigned int new_st) {
                              unsigned int old_slot = (old_v & 0x3FFFFFFF) + (old_st << 30);
                              unsigned int new_slot = (new_v & 0x3FFFFFFF) + (new_st << 30);

                              return std::atomic_compare_exchange_strong(
                                  reinterpret_cast<volatile std::atomic_uint*>(&slot),
                                  &old_slot,
                                  new_slot);
                          }

                      public:
                          int exchange(int v) {
                              unsigned int st = 0;

                              for (int i = 0; i < MAX_LOOP; ++i) {
                                  unsigned int old_v = get_slot(&st);

                                  switch (st) {
                                  case EMPTY:
                                      if (CAS(old_v, v, EMPTY, WAIT) {
                                          bool time_out = true;

                                          for (int j = 0; j < MAX_LOOP; ++j) {
                                              if (WAIT != get_state()) {
                                                  time_out = false;
                                                  break;
                                              }
                                          }

                                          if (false == time_out) {
                                              int ret = get_slot(&st);
                                              slot = 0;
                                              return ret;
                                          } else {
                                              if (CAS(v, 0, WAIT, EMPTY) {
                                                  return RET_TIMEOUT;
                                              } else {
                                                  int ret = get_slot(&st);
                                                  slot = 0;
                                                  return ret;
                                              }
                                          }
                                      }
                                      break;

                                  case WAIT:
                                      if (CAS(old_v, v, WAIT, BUSY) { return old_v; }
                                      break;

                                  case BUSY:
                                      break;
                                  }
                              }
                              
                              return RET_BUSY_TIMEOUT;
                          }
                      }

    → EliminationBackoffStack : class LF_BO_STACK {
                                 private:
                                     ...

                                 public:
                                     void Push(int x) {
                                         Node* n = new Node{ x };

                                         while (true) {
                                             Node* last = top;

                                             n->next = last;

                                             if (CAS(last, n)) {
                                                 return;
                                             }

                                             int ret = m_earr.Visit(x));

                                             if ((RET_TIMEOUT == ret) || 
                                                 (RET_BUSY_TIMEOUT == ret)) {
                                                 continue;    
                                             } else {
                                                 if (-1 == ret) {
                                                     delete n;
                                                     return;
                                                 } else {
                                                     continue;
                                                 }
                                             }
                                         }
                                     }

                                     int Pop() {
                                         while (true) {
                                             NODE* volatile last = m_top;

                                             if (nullptr == last) { return -2; }

                                             NODE* volatile next = last->next;

                                             if (last != m_top) { continue; }

                                             int v = last->key;

                                             if (true == CAS(last, next)) {
                                                 return v;
                                             }

                                             int ret = m_earr.Visit(x));

                                             if ((RET_TIMEOUT == ret) ||
                                                 (RET_BUSY_TIMEOUT == ret)) {
                                                 continue;
                                             } else {
                                                 if (-1 != ret) {
                                                     return ret;
                                                 } else {
                                                     continue;
                                                 }
                                             }
                                         }
                                     }
                                }
                                → LF_STACK과 달리 확장의 여지 존재
                                   부하가 높아지면 성공적인 소거가 증가하고, 병렬적으로 여러 개의 연산이 완료된다.
                                   소거된 연산은 스택에 접근하기 않기 때문에 경쟁이 줄어든다.

    → 문제점?
       → ABA : ABA 현상이 발생해도, 교환 대상이 바뀐 것 뿐이므로 문제가 발생하지 않는다.
       → 타임아웃 : 너무 짧은 교환 시간은 항상 실패하므로, 타임아웃 설정에 주의해야 한다.

 ====================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================

 ■ 스킵 리스트

 1. Lazy SkipList
    → Non-Blocking SkipList의 전 단계
       Marked 필드 사용

    → 문제점 : 중간 단계의 증가
                → Add : 0 레벨은 연결되었으나 윗 레벨이 연결되지 않은 경우
                   Remove : Marking은 되었으나 링크에서 완전히 제거되지 않은 상태

                → 중간 단계의 노드에 대한 명확한 정의 필요
                   → Add : 중간 단계의 노드는 아직 Add되지 않은 노드
                      Remove : Marking 되었으면 Remove된 노드
                   
                   중간 단계의 노드가 발견되면 완전히 처리될 때까지 기다린 뒤 리턴
                   Add : 아래에서부터 위로 링크 연결
                   Remove : 아래에서부터 위로 Locking하고, 위에서부터 아래로 링크 제거
                   → 위의 링크가 연결되지 않아도 pred, curr에 잘못된 값이 들어갈 수 있을 뿐, 검색에는 문제 없다.
                      아래의 링크가 연결되지 않았다면 nullptr 참조 오류가 발생한다.


 2. 구현
    → Add : 모든 링크가 연결되었는지 나타내는 Flag 필요

       Remove : prev[]와 curr[]를 완전히 Locking하고 Marking하는 것은 비효율적  // head, tail이 Locking되면 SkipList 전체가 멈춘다.
                Marking 후 위의 링크에서 Invalidate 발생 시 Find를 다시 수행할 필요 有

    → 노드
       → 개별적인 잠금 : std::recursive_mutex 사용
          Marked 필드 : Remove 시 논리적으로 제거하고 있는 중이라면 true
          next[n] : 각 층에 해당하는 포인터의 배열
          fullylinked : 모든 층에서 추가된 노드에 제대로 참조를 설정할 때까지 논리적으로 SkipList에 없다고 판단
                        → false일 경우 접근이 허용되지 않으며 true가 될 때까지 스핀

       보초 노드
       → 초기에는 head의 모든 층이 tail을 가리킨다.


 3. 메소드
    → Find : int Find(int key, NODE* prev[], NODE* curr[]) {
                  int found_level = -1;

                  for (int i = MAX_LEVEL; i >= 0; --i) {
                      if (i == MAX_LEVEL) {
                          prev[i] = head;
                      } else {
                          prev[i] = prev[i + 1];
                      }

                      curr[i] = prev[i]->next[i];

                      while (curr[i]->key < key) {
                          prev[i] = curr[i];
                          curr[i] = curr[i]->next[i];
                      }

                      if ((curr[i]->key == key) &&
                          (found_level == -1) {
                          found_level = i;    
                      }
                  }

                  return found_level;  // 찾지 못했을 경우 -1을 리턴, 찾았을 경우 최고 레벨을 리턴
              }
              → 모든 링크가 연결되지 않았을 경우가 있으므로, 리턴 값과 최고 레벨을 비교하여 prev[]와 curr[]가 모든 레벨의 링크를 담고 있는가 확인할 필요 有

       Add : bool Add(int key) {
                 NODE* prev[MAX_LEVEL + 1];
                 NODE* curr[MAX_LEVEL + 1];

                 while (true) {
                     int found_level = Find(key, prev, curr);

                     if (-1 != found_level) {                               // 존재할 경우
                         while (curr[found_level]->fullylinked == false);   // fullylinked == true일 때까지 기다린 후
                         return false;                                      // false 리턴
                     }

                     int top_level = 0;
                     bool valid = false;

                     for (int i = 0; i < MAX_LEVEL; ++i) {
                         if (rand() % 2 == 0) {
                             break;
                         }

                         ++top_level;
                     }

                     for (int level = 0; level <= top_level; ++level) {     // 0레벨부터 Valid 검사하며 Locking
                         prev[level]->nl.lock();                            // 노드가 추가되는 동안 앞 노드가 변경되는 것을 방지하기 위해 앞 노드를 잠근다.

                         if ((prev[level]->removed == true) ||
                             (curr[level]->removed == true) ||
                             (prev[level]->next != curr[level]) {  
                             valid = false;

                             for (int i = 0; i <= level; ++i) {
                                 pred[i]->nl.unlock();
                             }
                             
                             break;
                         }
                     }

                     if (valid == false) {                                  // Invalid할 경우
                         continue;                                          // 처음부터 다시 시작
                     }

                     NODE* n = new NODE(key, top_level);

                     for (int level = 0; level <= top_level; ++level) {     // 0레벨부터 SkipList 연결
                         n->next[level] = curr[level];
                         prev[level]->next[level] = n;
                     }

                     n->fullylinked = true;

			         for (int level = 0; level <= top_level; ++level) {     // 0레벨부터 Locking 해제
				         pred[level]->nl.unlock();
			         }

 			         return true;
                 }
             }

       Remove : bool Remove(int key) {
                    NODE* prev[MAX_LEVEL + 1];
                    NODE* curr[MAX_LEVEL + 1];

                    NODE* victim = nullptr;
                    bool is_marked = false;
                    int top_level = 0;

                    while (true) {
                        int found_level = Find(key, prev, curr);

                        if (found_level != -1) {
                            victim = curr[found_level];
                        }

                        if (is_marked ||                                            // Marking을 했거나 제거 조건이 만족되면 제거 시도
                            (found_level != -1 && 
                             (victim.fullylinked &&
                              victim.top_level == found_level &&  
                              victim.removed == false)) {
				            if (is_marked == false) {                               // Marking이 되지 않았으면 Marking 수행
					            top_level = victim->top_level;
					            victim->nl.lock();

                                if (victim->removed) {                              // 다른 스레드에서 먼저 Marking했다면
						            victim->nl.unlock();    
                                    return false;                                   // false 리턴
                                }

                                victim->removed = true;
                                is_marked = true;
                            }

				            bool valid = true;

				            for (int level = 0; level <= top_level; ++level) {
					            pred[level]->nl.lock();

					            if ((pred[level]->removed == true) ||               // 0레벨부터 Valid 검사하며 Locking
						            (pred[level]->next[level] != victim)) {  
						            valid = false;

						            for (int i = 0; i <= level; ++i) {       
							            pred[i]->nl.unlock();                
						            }
						            break;
					            }
				            }

				            if (valid == false) {                                   // Invalid할 경우
					            continue;                                           // 처음부터 다시 시작
				            }

                            for (int level = top_level; level >= 0; --level) {      // top_level부터 SkipList에서 제거한다.
					            pred[level]->next[level] = victim->next[level]; 
				            }

				            victim->nl.unlock();

				            for (int level = 0; level <= top_level; ++level) {
					            pred[level]->nl.unlock();
				            }

				            return true;
			            } else {                                                    // 제거 조건이 만족되지 않았다면
				            return false;                                           // false 리턴
		                }
		            }
                }

       Contains : bool Contains(int key) {
                    NODE* prev[MAX_LEVEL + 1];
                    NODE* curr[MAX_LEVEL + 1];

                    int found_level = Find(key, prev, curr);

                    return (found_level != -1 &&
                            curr[found_level]->fullylinked &&
                            curr[found_level]->removed == false);
                  }

 ====================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================

 ■ TBB



*/

