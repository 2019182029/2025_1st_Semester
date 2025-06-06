/*

 �� �̷�

 1. ���� ��ü
    �� ���ο� ����ȭ ������ �����ϴ� ������ ��ü
       �� Type_t decide(Type_t value) : n���� �����尡 decide�� ���� �� �� ���Ϸ� ȣ���Ѵ�.
                                        decide�� ��� ȣ�⿡ ���� ���� ���� ��ȯ�Ѵ�.
                                        decide�� ��ȯ�ϴ� ���� ���޵� value �� �ϳ��̴�.
                                        �� ���� Ȯ���� ���� ó�� decide�� ȣ���� �����尡 ���õȴ�.
                                        Atomic�ϰ� Wait-Free�ϰ� �����Ѵ�.

    �� ���� : ��� �����尡 ���� ����� ��� ���
              �� class Consensus {
                 private:
                     Type_t value;

                 public:
                     Consensus() { value = INIT; }  // INIT�� ����� ������ �ʴ� ��

                     Type_t decide(Type_t v) {
                         CAS(&value, INIT, v);
                         return value;
                     }
                 };


 2. ���Ǽ�
    �� ����ȭ ������ �����ϴ� Ŭ���� C�� ���� ��, Ŭ���� C�� Atomic �޸𸮸� ����� N�� �����忡 ���� ���� ��ü�� ������ �� �ִ�.
       �� Ŭ���� C�� N ������ ���� ������ �ذ��Ѵٰ� �Ѵ�.
          �ִ��� N�� ���� ����� �ϸ�, �ִ� N�� �������� �ʴ´ٸ� �� Ŭ������ ���� ���� �����ϴٰ� �Ѵ�.

    �� ���� : Atomic �޸𸮸����� N ������ ���� ������ �ذ��� �� �ִ°�?
              atomic_load(), atomic_store() ���길�� ����� N ������ ���� ��ü�� ���� �� �ִ°�?
              �� ������ A�� B�� ���� 0�� 1�� ���� �õ� : ������ A�� B�� ������ ������ ���� �޸𸮿� ���� read(atomic_load())�� write(atomic_store()) ������ ����
                                                         �� return���� ������ ��ġ�� ������ ���� �޸𸮿� ���� read, write�ۿ� ����.
                                                            return���� ���� �޸� ������ ��� ������� ����Ǿ����Ŀ� ���� �����ȴ�.
                                                            �� ���� ������ ���� �޸� ������ ����Ǹ� �׻� ���� return���� ���´�.
                                                            return���� ��� 0�̴���, ��� 1�̾�� �Ѵ�.

                                                         �˰����� ���� ���� �� ���� �޸𸮿� ���� read, write�� ���� �и��Ͽ� �̵��̶�� �θ���.
                                                         �� ������� ���Ǹ� �̷� ������ ����ϸ� �̵��Ѵ�.
                                                            �� Wait-Free�̹Ƿ� �������� �̵��� ������.
                                                               �� A�� B�� decide ȣ���� �Ϸ�Ǿ� return���� 0 �Ǵ� 1�� �����Ǿ� �ִ�.
                                                               �̵��� ������ ���� ����(return���� ������ ��ġ�� �޸�, ��������)�� ����� �� �ִ�.

                                                         �˰����� ��� ���� ������ ��θ� ���� Ʈ���� ��Ÿ�� �� �ְ�, �̸� ���������̶� �θ���.
                                                         �� ���������� ������� �˰���� input ���� ���� �����ȴ�.
                                                            ���� ���� �� ��� �������� ������ ���� ������ ���� �ʴ�.
                                                            �� ��� �������� �����ϴ��� �ùٸ� ����� ���;� �Ѵ�.


 3. ���
    �� �ʱ� ���� : �ƹ��� �̵��� ���� ���
       ���� ���� : ��� �����尡 �̵��� ��ģ ����
                   �� ���������� Leaf
                      decide �޼ҵ尡 ������ ���� �����Ѵ�.

       �ϰ� ���� : ������ ��� �̵��� �ϴ��� ������� ��ȭ�� ���� ���
       �̰� ���� : ������� �������� ���� ����
       �Ӱ� ���� : ������ �̵����� �ϰ� ���°� �Ǵ� �̰� ����
       �� ��� 2 ������ ���� ���������� �ʱ� ���´� �̰� �����̴�.
          �� A�� �����ϸ� 0, B�� �����ϸ� 1�� �����ؾ� �Ѵ�.
             ���� ������ ���� ����� �ٲ��.

          ��� Wait-Free ���� ���������� �Ӱ� ���°� �ݵ�� �����Ѵ�.
          �� Ʈ���� ���̴� �����ϴ�.
             ������ ���� ���� ��� �ϰ� �����̴�.
             ������ �̰� ������ ��尡 �����ϴ� ���̰� �ִ�.
             �� ���̿� �����ϴ� ��� �̰� ������ ���� �Ӱ� �����̴�.


 4. ����
    �� Atomic �޸𸮷� �����ߴٸ� �Ӱ� ���°� �ݵ�� �����ϰ�, �� �� �̵� ������ �ó������� ������ ����.
       �� A�� Read, B�� Any : A�� �б� ������ A�� ���� ���¸� ����
                              �� B�� A�� read�� �����ߴ��� ������ �� �����Ƿ� ������� ������ �� ����.

          A�� wrtie x, B�� write y : s'�� s''�� ���� ����
                                     �� A, B �� �� ���� ���� ����Ǿ����� ������ �� �����Ƿ� ������� ������ �� ����.

          A�� write x, B�� write x : A�� ���� ������ B�� ���� ������ ������.
                                     �� A�� B�� write�� �����ߴ��� ������ �� �����Ƿ� ������� ������ �� ����.

          �� ��� ��쿡�� �Ӱ� ���°� ������ �� �����Ƿ� read, write �̵������δ� 2 ������ ���Ǹ� Non-Blocking�ϰ� �����ϴ� ���� �Ұ����ϴ�.
             �� Non Blocking ���α׷��� �������� ����� ���ؾ� ������, Atomic Memory�� read, write�����δ� ������ ����� ��� �����忡�� �˸��� ���� �Ұ����ϴ�.
                Wait-free�̹Ƿ� ����� ���� �����尡 ������ ���� �޸� �������� ����� �˷����� �ʾƵ� �ٸ� ������� ����� �˾ƾ� �Ѵ�.
                
 ====================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================
 
 �� Lazy SkipList

 1. Lazy SkipList
    �� Non-Blocking SkipList�� �� �ܰ�
       Marked �ʵ� ���

    �� ������ : �߰� �ܰ��� ����
                �� Add : 0 ������ ����Ǿ����� �� ������ ������� ���� ���
                   Remove : Marking�� �Ǿ����� ��ũ���� ������ ���ŵ��� ���� ����

                �� �߰� �ܰ��� ��忡 ���� ��Ȯ�� ���� �ʿ�
                   �� Add : �߰� �ܰ��� ���� ���� Add���� ���� ���
                      Remove : Marking �Ǿ����� Remove�� ���
                   
                   �߰� �ܰ��� ��尡 �߰ߵǸ� ������ ó���� ������ ��ٸ� �� ����
                   Add : �Ʒ��������� ���� ��ũ ����
                   Remove : �Ʒ��������� ���� Locking�ϰ�, ���������� �Ʒ��� ��ũ ����
                   �� ���� ��ũ�� ������� �ʾƵ� pred, curr�� �߸��� ���� �� �� ���� ��, �˻����� ���� ����.
                      �Ʒ��� ��ũ�� ������� �ʾҴٸ� nullptr ���� ������ �߻��Ѵ�.


 2. ����
    �� Add : ��� ��ũ�� ����Ǿ����� ��Ÿ���� Flag �ʿ�

       Remove : prev[]�� curr[]�� ������ Locking�ϰ� Marking�ϴ� ���� ��ȿ����  // head, tail�� Locking�Ǹ� SkipList ��ü�� �����.
                Marking �� ���� ��ũ���� Invalidate �߻� �� Find�� �ٽ� ������ �ʿ� ��

    �� ���
       �� �������� ��� : std::recursive_mutex ���
          Marked �ʵ� : Remove �� �������� �����ϰ� �ִ� ���̶�� true
          next[n] : �� ���� �ش��ϴ� �������� �迭
          fullylinked : ��� ������ �߰��� ��忡 ����� ������ ������ ������ �������� SkipList�� ���ٰ� �Ǵ�
                        �� false�� ��� ������ ������ ������ true�� �� ������ ����

       ���� ���
       �� �ʱ⿡�� head�� ��� ���� tail�� ����Ų��.


 3. �޼ҵ�
    �� Find : int Find(int key, NODE* prev[], NODE* curr[]) {
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

                  return found_level;  // ã�� ������ ��� -1�� ����, ã���� ��� �ְ� ������ ����
              }
              �� ��� ��ũ�� ������� �ʾ��� ��찡 �����Ƿ�, ���� ���� �ְ� ������ ���Ͽ� prev[]�� curr[]�� ��� ������ ��ũ�� ��� �ִ°� Ȯ���� �ʿ� ��

       Add : bool Add(int key) {
                 NODE* prev[MAX_LEVEL + 1];
                 NODE* curr[MAX_LEVEL + 1];

                 while (true) {
                     int found_level = Find(key, prev, curr);

                     if (-1 != found_level) {                               // ������ ���
                         while (curr[found_level]->fullylinked == false);   // fullylinked == true�� ������ ��ٸ� ��
                         return false;                                      // false ����
                     }

                     int top_level = 0;
                     bool valid = false;

                     for (int i = 0; i < MAX_LEVEL; ++i) {
                         if (rand() % 2 == 0) {
                             break;
                         }

                         ++top_level;
                     }

                     for (int level = 0; level <= top_level; ++level) {     // 0�������� Valid �˻��ϸ� Locking
                         prev[level]->nl.lock();                            // ��尡 �߰��Ǵ� ���� �� ��尡 ����Ǵ� ���� �����ϱ� ���� �� ��带 ��ٴ�.

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

                     if (valid == false) {                                  // Invalid�� ���
                         continue;                                          // ó������ �ٽ� ����
                     }

                     NODE* n = new NODE(key, top_level);

                     for (int level = 0; level <= top_level; ++level) {     // 0�������� SkipList ����
                         n->next[level] = curr[level];
                         prev[level]->next[level] = n;
                     }

                     n->fullylinked = true;

			         for (int level = 0; level <= top_level; ++level) {     // 0�������� Locking ����
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

                        if (is_marked ||                                            // Marking�� �߰ų� ���� ������ �����Ǹ� ���� �õ�
                            (found_level != -1 && 
                             (victim.fullylinked &&
                              victim.top_level == found_level &&  
                              victim.removed == false)) {
				            if (is_marked == false) {                               // Marking�� ���� �ʾ����� Marking ����
					            top_level = victim->top_level;
					            victim->nl.lock();

                                if (victim->removed) {                              // �ٸ� �����忡�� ���� Marking�ߴٸ�
						            victim->nl.unlock();    
                                    return false;                                   // false ����
                                }

                                victim->removed = true;
                                is_marked = true;
                            }

				            bool valid = true;

				            for (int level = 0; level <= top_level; ++level) {
					            pred[level]->nl.lock();

					            if ((pred[level]->removed == true) ||               // 0�������� Valid �˻��ϸ� Locking
						            (pred[level]->next[level] != victim)) {  
						            valid = false;

						            for (int i = 0; i <= level; ++i) {       
							            pred[i]->nl.unlock();                
						            }
						            break;
					            }
				            }

				            if (valid == false) {                                   // Invalid�� ���
					            continue;                                           // ó������ �ٽ� ����
				            }

                            for (int level = top_level; level >= 0; --level) {      // top_level���� SkipList���� �����Ѵ�.
					            pred[level]->next[level] = victim->next[level]; 
				            }

				            victim->nl.unlock();

				            for (int level = 0; level <= top_level; ++level) {
					            pred[level]->nl.unlock();
				            }

				            return true;
			            } else {                                                    // ���� ������ �������� �ʾҴٸ�
				            return false;                                           // false ����
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