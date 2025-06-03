/*

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