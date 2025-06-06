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
    → 멤버 변수로 크기가 n인 배열을 갖고, 복수의 원소를  Atomic하고 Wait-Free하게 변경할 수 있는 객체
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
                 → 모든 자료구조를 n 쓰레드에서 Non-Blocking하게 구현할 수 있다.

 ====================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================
 
 ■ Lazy SkipList

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

*/