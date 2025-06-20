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
                                       tail = n;
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
       → 64bit : ST_Ptr는 128bit 자료구조이므로, 128bit CAS 필요
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

                     → 문제점 : 모든 ST_Ptr의 load와 store가 atomic하게 동작해야 하지만, atomic_128int는 존재하지 않는다.
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
              → 절대 시간이 중요한 것이 아니다.
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
                                      if (old_range < MAX_EXCHANGER - 1) {
                                           std::atomic_compare_exchange_strong(
                                               reinterpret_cast<std::atomic_int*>(&range),
                                               &old_range,
                                               old_range + 1);
                                      }
                                  } else if (RET_TIMEOUT == ret) {
                                      if (old_range > 1) {
                                          std::atomic_compare_exchange_strong(
                                              reinterpret_cast<std::atomic_int*>(&range),
                                              &old_range,
                                              old_range - 1);
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

 ■ 순차 스킵 리스트

 1. 스킵 리스트
    → O(logn)의 검색 시간을 갖는다.  // Worst Case : O(n)
       재균형 작업이 필요 없는 랜덤 자료구조다.


 2. 순차 스킵 리스트
    → Find() : 높은 레벨의 포인터부터 검색, 한 레벨의 검색이 끝나면 다음 레벨의 검색 시작
                → 레벨 별 검색 결과 pred[], curr[]에 저장
                   맨 아래 레벨에 도달할 경우 종료

       Add() : 랜덤한 높이의 노드 생성  // 높이가 높을수록 노드의 개수 감소
               → 검색한 위치에 추가

    → 구현 : struct SKNODE {
                  int key;
                  int top_level;
                  SKNODE* next[MAX_LEVEL + 1];
              };

              class CSKLIST {
              private:
                  SKNODE* head;
                  SKNODE* tail;
                  std::mutex sm;

              public:
                  void Find(int key, SKNODE* pred[], SKNODE* curr[]) {
                      for (int i = MAX_LEVEL; i >= 0; --i) {
                          if (i == MAX_LEVEL) {
                              pred[i] = head;
                          } else {
                              pred[i] = pred[i + 1];
                          }

                          curr[i] = pred[i]->next[i];

                          while (curr[i]->key < key) {
                              pred[i] = curr[i];
                              curr[i] = curr[i]->next[i];
                          }
                      }
                  }

                  bool Add(int key) {
                      SKNODE* pred[MAX_LEVEL + 1];
                      SKNODE* curr[MAX_LEVEL + 1];

                      sm.lock();

                      Find(key, pred, curr);

                      if (curr[0]->key == key) {
                          sm.unlock();
                          return false;
                      } else {
                          int top_level = 0;

                          for (int i = 0; i < MAX_LEVEL; ++i) {
                              if (rande() % 2 == 0) { break; }
                              ++top_level;
                          }

                          auto n = new SKNODE{ key, top_level };

                          for (int i = 0; i <= top_level; ++i) {
                              pred[i]->next[i] = n;
                              n->next[i] = curr[i];
                          }

                          sm.unlock();
                          return true;
                      }
                  }

                  bool Remove(int key) {
                      SKNODE* pred[MAX_LEVEL + 1];
                      SKNODE* curr[MAX_LEVEL + 1];

                      sm.lock();

                      Find(key, pred, curr);

                      if (curr[0]->key == key) {
                          auto n = curr[0];

                          for (int i = 0; i <= curr[0]->top_level; ++i) {
                              pred[i]->next[i] = curr[i]->next[i];
                          }

                          sm.unlock();

                          delete n;
                          return true;
                      } else {
                          sm.unlock();
                          return false;
                      }
                  }

                  bool Contains(int key) {
                      SKNODE* pred[MAX_LEVEL + 1];
                      SKNODE* curr[MAX_LEVEL + 1];

                      sm.lock();

                      Find(key, pred, curr);

                      if (curr[0]->key == key) {
                          sm.unlock();
                          return true;
                      } else {
                          sm.unlock();
                          return false;
                      }
                  }
              };

 ====================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================

 ■ 게으른 스킵 리스트

 1. 게으른 스킵 리스트
    → Non-Blocking 스킵 리스트의 전 단계
       Marked 필드 사용

    → 문제점 : 중간 단계의 증가
                → Add : 0 레벨은 연결되었으나 윗 레벨이 연결되지 않은 경우
                   Remove : Marking은 되었으나 링크에서 완전히 제거되지 않은 상태

                → 중간 단계의 노드에 대한 명확한 정의 필요
                   → Add : 중간 단계의 노드는 아직 Add되지 않은 노드
                      Remove : Marking 되었으면 Remove된 노드
                   
                → 해결책 : 중간 단계의 노드가 발견되면 완전히 처리될 때까지 기다린다.
                            Add : 아래에서부터 위로 링크 연결
                            Remove : 아래에서부터 위로 Locking하고, 위에서부터 아래로 링크 제거
                            → 위의 링크가 연결되지 않아도 pred, curr에 잘못된 값이 들어갈 수 있을 뿐, 검색에는 문제 없다.
                               아래의 링크가 연결되지 않았다면 nullptr 참조 오류가 발생한다.


 2. 자료구조
    → 노드
       → 개별적인 잠금 : std::recursive_mutex 사용
          Marked 필드 : Remove 시 논리적으로 제거하고 있는 중이라면 true
          fullylinked : 모든 층에서 추가된 노드에 제대로 참조를 설정할 때까지 논리적으로 스킵 리스트에 없다고 판단


 3. 구현
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
                         while (curr[0]->fullylinked == false);             // fullylinked == true일 때까지 기다린 후
                         return false;                                      // false 리턴
                     }

                     int top_level = 0;
                     bool valid = false;

                     for (int i = 0; i < MAX_LEVEL; ++i) {
                         if (rand() % 2 == 0) { break; }
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

                     if (valid == false) { continue; }                      // Invalid할 경우 처음부터 다시 시작

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

       Remove : prev[]와 curr[]를 완전히 Locking하고 Marking하는 것은 비효율적  // head, tail이 Locking되면 스킵 리스트 전체가 멈춘다.
                → bool Remove(int key) {
                       NODE* prev[MAX_LEVEL + 1];
                       NODE* curr[MAX_LEVEL + 1];

                       NODE* victim = nullptr;
                       bool is_marked = false;
                       int top_level = 0;

                       while (true) {
                           int found_level = Find(key, prev, curr);

                           if (found_level != -1) { victim = curr[0]; }

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

                               for (int level = 0; level <= top_level; ++level) {      // 0레벨부터 Valid 검사하며 Locking
					               pred[level]->nl.lock();

                                   if ((pred[level]->removed == true) ||               
						               (pred[level]->next[level] != victim)) {  
						               valid = false;

                                       for (int i = 0; i <= level; ++i) {
							               pred[i]->nl.unlock();                
                                       }
						               break;
                                   }
				               }

                               if (valid == false) { continue; }                       // Invalid할 경우 처음부터 다시 시작

                               for (int level = top_level; level >= 0; --level) {      // top_level부터 스킵 리스트에서 제거한다.
					               pred[level]->next[level] = victim->next[level]; 
				               }

                               delete victim;

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

 ■ 무잠금 스킵 리스트

 1. 무잠금 스킵 리스트
    → CAS를 사용한 삽입과 삭제
       → 논리적 제거는 제거할 노드의 마킹을 함으로써 수행된다.
          물리적 제거는 Find() 메소드에서 마킹된 노드를 제거함으로써 수행된다.


 2. 메소드
    → Find() : 리스트를 순회하며 마킹된 노드를 만나면 링크를 잘라낸다.
                → 모든 층의 링크를 잘라내지는 않으므로 정확한 스킵리스트가 유지되지 않는다.
                   하지만 검색의 정확성에는 문제가 없다.

                대상 키를 갖는 노드가 레벨 0에 존재하고, 마킹이 되지 않았다면 true를 반환한다.

       Add() : 새 노드를 레벨 0에 연결하여 논리적으로 추가한다.
               순서대로 최상층까지 연결한다.

       Remove() : 레벨 0을 제외한 레벨에 마킹을 함으로써 논리적으로 제거한다.
                  최하층에 마킹을 한다.
                  → 성공 : Find() 메소드를 통해 물리적으로 제거한다.
                     실패 : 재시도가 필요하지 않으므로 false를 반환한다.


 3. 구현
    → class SKAMR {
       private: 
           std::atomic_llong data;

       public:
           LFSKNODE* get_ptr() {
               long long temp = data & 0xFFFFFFFFFFFFFFFE;
               return reinterpret_cast<LFSKNODE*>(temp);
           }

	       LFSKNODE* get_ptr(bool* is_removed) {
		       long long temp = data;
		       *is_removed = (temp & 1) == 1;
		       return reinterpret_cast<LFSKNODE*>(temp & 0xFFFFFFFFFFFFFFFE);
	       }

           void set_ptr(LFSKNODE* p) {
               long long temp = reinterpret_cast<long long>(p);
               temp = temp & 0xFFFFFFFFFFFFFFFE;
               data = temp;
           }

           bool get_mark() {
               return (data & 1) == 1;
           }

           bool CAS(LFSKNODE* old_p, LFSKNODE* new_p, bool old_m, bool new_m) {
               long long old_value = reinterpret_cast<long long>(old_p);
               long long new_value = reinterpret_cast<long long>(new_p);

               if (true == old_m) {
                   old_value = old_value | 1;
               } else {
                   old_value = old_value & 0xFFFFFFFFFFFFFFFE;
               }

               ...

               return std::atomic_compare_exchange_strong(
                   &data,
                   &old_value,
                   new_value);
           }
       }

       class LFSKLIST {
       public:
           bool Find(int key, LFSKNODE* pred[], LFSKNODE* curr[]) {
           retry:
               for (int i = MAX_LEVEL, i >= 0; --i) {
                   if (i == MAX_LEVEL) {
                       pred[i] = head;
                   } else {
                       pred[i] = pred[i + 1];
                   }

                   curr[i] = pred[i]->next[i].get_ptr();

                   while (true) {
                       bool removed = false;
                       LFSKNODE* succ = curr[i]->next[i].get_ptr(&removed);

                       while (true == removed) {
                           if (false == pred[i]->next[i].CAS(curr[i], succ, false, false)) {
                               goto retry;
                           }

                           curr[i] = succ;
                           succ = curr[i]->next[i].get_ptr(&removed);
                       }

                       if (curr[i]->key >= key) { break; }

                       pred[i] = curr[i];
                       curr[i] = succ;
                   }
               }

               return curr[0]->key == key;
           }

           bool Add(int key) {
               LFSKNODE* pred[MAX_LEVEL + 1];
               LFSKNODE* curr[MAX_LEVEL + 1];

               while (true) {
                   if (true == Find(key, pred, curr) { return false; }

                   int top_level = 0;

                   for (int i = 0; i < MAX_LEVEL; ++i) {
                       if (rande() % 2 == 0) { break; }
                       ++top_level;
                   }

                   auto n = new LFSKNODE{ key, top_level };

                   for (int i = 0; i <= top_level; ++i) {
                       n->next[i].set_ptr(curr[i]);
                   }

                   if (false == pred[0]->next[0].CAS(curr[0], n, false, false)) { 
                       delete n;
                       continue; 
                   }

                   for (int i = 1; i <= top_level; ++i) {
                       while (false == pred[i]->next[i].CAS(curr[i], n, false, false)) {
                           Find(key, pred, curr);
                       }
                   }

                   return true;
               }
           }

           bool Remove(int key) {
               LFSKNODE* pred[MAX_LEVEL + 1];
               LFSKNODE* curr[MAX_LEVEL + 1];

               if (false == Find(key, pred, curr)) { return false; }

               LFSKNODE* victim = curr[0];
               int top_level = victim->top_level;

               LFSKNODE* succ;

               for (int i = 1; i <= top_level; ++i) {
                   succ = victim->next[i].get_ptr();

                   while (false == victim->next[i].CAS(succ, succ, false, true) {
                       if (true == victim->next[i].get_mark()) { break; }
                       if (false == Find(key, pred, curr)) { return false; }
                   }
               }

               succ = victim->next[0].get_ptr();

               while (true) {
                   if (true == victim->next[0].CAS(succ, succ, false, true)) {
                       Find(key, pred, curr);
                       return true;
                   } else {
                       bool removed = false;

                       succ = victim->next[0].get_ptr(&removed);

                       if (removed) {
                           return false;
                       }
                   }
               }
           }

           bool Contains(int key) {
               LFSKNODE* pred;
               LFSKNODE* curr;

               pred = head;

               for (int i = MAX_LEVEL; i >= 0; --i) {
                   curr = pred->next[i].get_ptr();

                   while (true) {
                       bool removed = false;
                       LFSKNODE* succ = curr->next[i].get_ptr(&removed);

                       while (true == removed) {
                           curr = succ;
                           succ = curr->next[i].get_ptr(&removed);
                       }

                       if (curr->key == key) { break; }

                       pred = curr;
                       curr = succ;
                   }
               }

               return curr->key == key;
           }
       }
       → 메모리 재사용 : EBR 사용

 ====================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================

 ■ 라이브러리

 1. OpenMP
    → C, C++, FORTRAN에서 병렬 프로그래밍을 가능토록 해주는 API
       → 내부적으로 멀티쓰레드, 공유 메모리를 사용  
          컴파일러 디렉티브와 함수, 변수로 구성
          표준으로 지정되어 대부분의 컴파일러에서 구현되어 있다.

    → 특징 : 분산 메모리에서는 사용할 수 없다.
              최상의 공유 메모리 사용 패턴을 보장하지 않는다.
              프로그래머는 Data Dependency, Data Race, Deadlock 검사를 수행해야 한다.
              프로그래머는 프로그램의 어느 부분을 병렬화 할 지 지정해주어야 한다.

    → 프로그래밍 모델 : Fork-Join          
                         → 컴파일러 디렉티브에 의존
                            Nesting 가능
                            고정식 쓰레드 생성, 동적 작업 할당
                            메모리 일관성 필요 시 FLUSH 명령어를 사용할 필요 有

                         → #pragma omp 
                            directive_name 
                            [clause, ...] : 옵션  // 순서에 상관 없이 여러 개가 올 수 있다.
                            → directive 뒤에는 반드시 Block이 와야 한다.
                               이미 생성되어 있는 Worker Thread가 블록을 실행한다.
                               블록의 끝에서 모든 쓰레드의 종료를 확인한 후 진행한다.

    → 작업 분배
       → Do/For : 루프를 여러 쓰레드가 나누어 수행  
          SECTIONS : 블록으로 나누어진 작업을 여러 쓰레드가 나누어 수행
          SINGLE : 한 개의 쓰레드가 전담하여 수행


 2. TBB
    → Loop Parallelizer : 사용자가 문제를 멀티쓰레드 적용 가능한 형태로 변환해야 한다.  
                           → 루프 본체를 함수로 변환한다.
                              함수를 TBB용 클래스로 변환한다.
                              TBB Parallelizer를 호출한다.
                              → parallel_for(range<int>(0, x), function());

                           → 람다를 사용하여 간단하게 변환할 수 있다.
                              → parallel_for(0, n, [&](int i) { sum += 2; });

    → Container : Lock-Free 혹은 Fine-Grained Synchronization으로 구현되어 고성능이다.
                   → concurrent_unordered_map : erase()가 Thread Safe하지 않다.
                                                 → 데이터를 넣었다 뺐다 하는 용도로 사용할 수 없다.

                      concurrent_hash_map : remove()가 Thread Safe하다.
                                            → 모든 자료의 접근은 accessor라는 스마트 포인터를 통해 이뤄진다.
                                               읽기만 하고 수정하지 않는 경우 const_accessor를 사용하는 것이 좋다.

                      concurrent_vector : clear(), swap(), resize(), reserve()가 Thread Safe하지 않다.
                                          shrink가 불가능하다.
                                          원소가 연속된 주소에 있지 않아 일반적인 포인터 연산이 불가능하다.
                                          원소를 읽을 때 원소가 생성 중일 수 있으므로 읽기 전 생성 완료를 확인해야 한다.

                      concurrent_queue : unsafe_size(), empty(), clear(), swap()이 Thread Safe하지 않다.
                                         → empty() 호출이 pop()의 성공을 보장하지 않으므로 try_pop()을 제공한다.

    → Mutual Exclusion : scoped_lock, 선언된 블록을 빠져 나올 때 자동으로 unlock을 한다.
                          → 예외 상황 프로그래밍 편리
                             실수로 unlock을 하지 않는 경우 제거

                          다양한 locking을 지원한다.
                          → Scalable : busy waiting을 없애 CPU 낭비를 막는다.
                                        오버헤드가 크다.

                             Fair : Critical Section에 도착한 순서대로 lock을 얻는다.

                             Recursive : 같은 쓰레드는 lock을 다중으로 얻을 수 있다.
                                         → recursive 알고리즘에서 편리

                             Long Wait : 오래 기다리고 있을 경우
                                         → yield : 다른 쓰레드 실행
                                            block : 깨워줄 때까지 멈춤

                             RW : 공유 메모리를 여러 쓰레드에서 동시에 읽는 것이 문제가 되지 않을 경우 사용
                                  → 읽기의 비중이 커질수록 효율적

    → 메모리 할당자 : 메모리 할당자 템플릿 제공
                       → 직렬 프로그램에서 고안된 메모리 할당자는 single share pool에 동시에 하나의 쓰레드만 접근 가능
                          → scalable_allocator로 해결

                          두 개의 쓰레드가 같은 cache line을 사용할 경우 문제 발생 가능
                          → cached_aligned_allocator는 잘못된 cache line 공유를 막아주는 것을 보장

                       → Windows와 Linux에서 기본 동적 메모리 할당 함수는 자동적으로 TBB의 메모리 할당 함수로 대체된다.

    → 태스크 스케줄링 : 작업을 여러 개의 task로 나눠 병렬로 처리하는 방식
                         → parrallel_for와 달리 작업끼리 연관관계가 있거나 작업이 동적으로 추가되는 경우도 다룰 수 있다.
                            task 관리 객체를 로컬 객체로 생성하고 소수의 task를 넣는 방식이 효율적이다.

                         → 메소드
                            → tbb::task_group 객체명
                               run() : 별도 task로 실행
                               run_and_wait() : run(); wait() 연속 호출과 동일하나, run()을 현재 쓰레드에서 실행하여 좀 더 효율적이다.
                               wait() : 모든 task의 종료를 기다린다.
                               cancel() : 모든 task를 취소한다.
                               is_canceling() : cancel 중이라면 true

                            → 예제 : tbb_task_group g;
                                      g.run([&] { x = function(n - 1); });
                                      g.run([&] { y = function(n - 2); });
                                      g.wait();


 3. CUDA
    → 병렬 처리를 GPU에서 수행하는 GPGPU의 일종
       
    → 장점 : 계산량이 많을 경우 CPU의 몇 십 배의 속도
    
       단점 : Nvidia 하드웨어만 지원한다.
              → DirectCompute, OpenCL을 사용하여 해결

              낮은 직렬 계산 및 I/O 속도  
              → CPU와 GPU 사이의 병목 현상

              적은 메모리
              → CPU 내장 GPU인 HAS를 사용하여 해결


 4. 병렬 하드웨어
    → Hyper Thread : Out of Order의 비는 파이프라인을 메꾸기 위하여 도입
                      → 2개의 코어가 캐시와 ALU 공유
                         벤치마크 프로그램에 따른 성능차이가 커 오늘날에는 사용 X

       Big-Little : Performance, Economic 두 종류의 코어
                    → Economic 코어에 Bottle Neck Thread가 스케줄링 되는 경우 

    → Transactional Memory : 하나의 쓰레드가 실행하는 프로그램 구간을 트랜잭션으로 정의
                              → 각각의 트랜잭션은 Atomic하여 한 번에 하나씩 실행되는 것처럼 보이며, 데드락이 발생하지 않는다.
                                 → 모든 메모리 연산을 임시적으로 실행, 실행이 끝난 후 충돌 검사
                                    충돌이 없으면 임시적 실행을 영구화
                                    충돌이 있으면 모든 실행을 무효화  
                                    → Undo 및 Redo 리스트를 관리한다.
                                       ACTIVE, COMMIT, ABORTED의 세 가지 상태 존재

                              → 하드웨어 트랜잭션 : 캐시 일관성 프로토콜을 수정하여 구현
                                                     → 캐시의 태그에 transaction bit 추가
                                                        transaction 메모리 연산을 transaction cache line에 수행한다.
                                                        transaction cache line이 invalidate되면 cache line과 cpu의 실행을 abort

                                                     → 단점
                                                        → 범용성 : 일부 CPU에서만 지원
                                                           
                                                           제한성 : HW 용량의 한계로 알고리즘 제한
                                                                    게으른 동기화 정도의 구현 난이도
                                                                    High Contention 상황에서 Lock-Free보다 성능 저하

                                                           한계 : 코어의 개수가 많아질수록 성능 향상의 한계가 찾아온다.

                                                     → Haswell HTM : 복수의 메모리에 대한 transaction을 L1 캐시의 용량 한도까지 허용한다.
                                                                      cpu에서 transaction 실패 시 메모리와 레지스터의 변경을 롤백한다.
                                                                      → 한계 : 모든 알고리즘에 적용할 수 없다.
                                                                                → HW 용량 한계로 인한 알고리즘 수정 필요
                                                                                   Nested Transaction 불가능  // 가능하지만 모두 롤백

                                                                                모든 레지스터 내용 저장 및 롤백으로 인한 오버헤드

                                 소프트웨어 트랜잭션 : 공유 메모리 접근을 Transational Memory 모듈을 통해 수행
                                                       → 단점 : Zombie 트랜잭션
                                                                 → 충돌 이후에도 트랜잭션이 계속 실행될 수 있다.
                                                                    충돌 결과 원래는 불가능한 값을 갖게될 수 있다.
                                                                    틀린 값을 읽지 않도록 보장해야 한다.

                                                                 모든 read, write가 api를 통해 이뤄지므로 성능 저하 발생

                             → 장점
                                → 생산성 : 싱글쓰레드 알고리즘을 그대로 사용하지 못한다.
                                            → 성능 향상을 위해 접근하는 공유 메모리 수와 트랜잭션 실행 시간을 줄이고, 시스템 호출을 제거해야 한다.

                                   확장성 : 공유 객체 연결에 제한이 없다.

                                   정확성 : 알고리즘의 검증이 쉽다.

                                   성능 : Lock-Free에 근접
                                          → 충돌이 심한 경우 성능 저하


 5. 새로운 언어
    → 함수형 언어 : 모든 변수가 const이므로 Data Race가 발생하지 않는다.
                     → 하스켈 : 순수 함수형 언어
                                 개념은 뛰어나나 난이도로 인해 사용률 저조
                                 → 순수 병렬성 : 언어에 내재된 병렬성 이용
                                                  → 항상 같은 결과
                                                     Data Race와 Deadlock 없음
                                                     I/O 처리 안됨

                                    동시 실행성 : I/O 처리를 위해 사용
                                                  → 실행 순서를 프로그래머가 제어
                                                     Data Race와 Deadlock 발생 가능
                        
                        Erlang : 프로세스 단위의 병렬성
                                 → 프로세스 사이의 동기화 : shared-nothing asynchronous message passing으로 구현
                                                             → 큐를 통한 message passing으로 동기화
                                                                분산 처리 가능

                        Elixir : Erlang 기반
                                 → Process 지원
                                    Send와 Receive를 사용한 동기화

                     → 문제점
                        → 생산성 : 높은 진입장벽
                                    익숙해지면 오히려 생산성 향상

                           성능 : 구조체 전달 시 포인터가 아닌 내용을 전달하는 것과 비슷한 오버헤드
                                  너무 자잘한 병렬화

                           I/O : I/O는 순서대로 행해져야 하지만 함수형 언어에서 I/O의 순서를 정해주는 것은 비효율적

    → Go : 언어에서 멀티쓰레드 지원
            → Gorountine : SW 쓰레드지만 커널 쓰레드 풀에서 병렬로 동작
               공유 메모리 모델 : Data Race 존재, mutex 필요
               → Channel : 멀티쓰레드 문제에 대한 대책
                            → Goroutine 사이의 고속 통신
                               공유 메모리 대신 사용


 6. 지금까지...
    → Blocking
       → 장점 : 직관적이다.
                 의도대로 잘 작동한다.

          단점 : 병렬성이 없다.
                 의도하지 않는 현상(우선순위 역전, 호위)을 야기한다.
                 데드락이 발생할 수 있다.

       Non-Blocking
       → 장점 : HW 도움으로 Wait-Free하게 수행되는 CAS 연산을 활용하여 락으로 인한 멈춤 현상을 회피할 수 있다.

          단점 : 설계가 매우 어렵다.
                 프로세서가 많아질수록 CAS 연산의 부하가 커진다.
                 CAS의 연산 단위가 Word이다.
                 
    → Lock-Free 
       → 한계 : 확장성이 떨어진다.
                 복수 메소드 호출, 다른 자료구조의 연속적인 메소드 호출의 atomic화가 매우 어렵다.
                 정확성을 증명하는 것이 매우 어렵다.

*/

