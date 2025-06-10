/*

 �� ���� ��ü

 1. ���� ��ü
    �� ���ο� ����ȭ ������ �����ϴ� ������ ��ü
       �� Type_t decide(Type_t value) : n���� �����尡 decide�� ���� �� �� ���Ϸ� ȣ���Ѵ�.
                                        decide�� ��� ȣ�⿡ ���� ���� ���� ��ȯ�Ѵ�.
                                        decide�� ��ȯ�ϴ� ���� ���޵� value �� �ϳ��̴�.  // ���� Ȯ���� ���� ó�� decide�� ȣ���� �����尡 ���õȴ�.
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

    �� ���� : Atomic �޸𸮸����� N ������ ���� ������ �ذ��� �� �ִ°�?  // atomic_load(), atomic_store() ���길�� ����� N ������ ���� ������ �ذ��� �� �ִ°�?
              �� ������ A�� B�� ���� 0�� 1�� ���� �õ� : ������ A�� B�� ������ ������ ���� �޸𸮿� ���� read(atomic_load())�� write(atomic_store()) ������ ����
                                                         �� return���� ������ ��ġ�� ������ ���� �޸𸮿� ���� read, write�ۿ� ����.
                                                            return���� ���� �޸� ������ ��� ������� ����Ǿ����Ŀ� ���� �����ȴ�.
                                                            �� ���� ������ ���� �޸� ������ ����Ǹ� �׻� ���� return���� ���´�.
                                                               return���� ��� 0�̴���, ��� 1�̾�� �Ѵ�.
                                                               
                                                         �˰����� ���� ���� �� ���� �޸𸮿� ���� read, write�� ���� �и��Ͽ� �̵��̶�� �θ���.
                                                         �� ������� ���Ǹ� �̷� ������ ����ϸ� �̵��Ѵ�.
                                                            �� Wait-Free�̹Ƿ� �������� �̵��� ������.
                                                               �̵��� ������ ���� ����(return���� ������ ��ġ�� �޸�, ��������)�� ����� �� �ִ�.
                                                               
                                                         �˰����� ��� ���� ������ ��θ� ���� Ʈ���� ��Ÿ�� �� �ְ�, �̸� ���������̶� �θ���.
                                                         �� ���������� ������� �˰���� input ���� ���� �����ȴ�.
                                                            ���� ���� �� ��� �������� ������ ���� ������ ���� �ʴ�.
                                                            �� ��� �������� �����ϴ��� �ùٸ� ����� ���;� �Ѵ�.


 3. ���
    �� �ʱ� ���� : �ƹ��� �̵��� ������� ���� ����
       ���� ���� : ��� �����尡 �̵��� ��ģ ����  // ���������� Leaf

       �ϰ� ���� : ������ ��� �̵��� �ϴ��� ������� ��ȭ�� ���� ���
       �̰� ���� : ������� �������� ���� ����
       �Ӱ� ���� : ������ �̵����� �ϰ� ���°� �Ǵ� �̰� ����
       �� ��� 2 ������ ���� ���������� �ʱ� ���´� �̰� �����̴�.
          �� A�� �����ϸ� 0, B�� �����ϸ� 1�� �����ؾ� �Ѵ�.
             ��, ���� ������ ���� ����� �ٲ��.

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
             �� Non Blocking ���α׷��� �������� ����� ���ؾ� �ϰ�, ����� ���� �����尡 ���� �޸� �������� ����� �˷����� �ʾƵ� �ٸ� ������� ����� �˾ƾ� �Ѵ�.
                ������, Atomic Memory�� read, write�����δ� ��� �����尡 ������ ����� Ȯ���� �˾Ƴ��� ���� �Ұ����ϴ�.
                   
 ====================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================
 
 �� ���� ���� ��ü

 1. ���� ���� ��ü
    �� ��� ������ ũ�Ⱑ n�� �迭�� ����, ������ ���Ҹ� Atomic�ϰ� Wait-Free�ϰ� ������ �� �ִ� ��ü
       �� Assign : ���ڷ� m���� ���� m���� �ε����� �޾� �����Ѵ�.  
          Read : ���ڷ� �ε����� i�� �޾� n[i]�� ��ȯ�Ѵ�.

       �� Atomic �޸𸮷� (m, n) ���� ��ü�� Wait-Free�ϰ� ������ �� ������?
          �� ���� : class MultiConsensus {
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
                    �� (2, 3) ���� ��ü�� 2 ������ ���Ǹ� ������ �� �ִ�.
                       �� Atomic �޸𸮷� (m, n) ���� ��ü�� Wait-Free�ϰ� �����ϴ� ���� �Ұ����ϴ�.

    �� (n, n * (n + 1) / 2)) ���� ��ü�� �ּ� n�� ���� ���� ���´�.
       �� ����, ���� ��ü�� HW ���� �� ���� ��ü�� Wait-Free�� ������ �� �ִ�.
          �� HW ���� ����� �ʹ� ũ�Ƿ� RMW�� ������� ����Ѵ�.


 2. RMW
    �� HW�� �����ϴ� ����ȭ ������ �� ����
       Wait-Free�� �Ǳ� ���ؼ��� Ư�� ��ɾ �ݵ�� �ʿ��ϴ�.
       �� f(x) : Atomic�ϰ� ���� �޸� ���� v���� x�� �ٲٰ�, ���� �� v�� �����Ѵ�.
                 �� GetAndSet
                    GetAndIncrement
                    GetAndAdd(k)
                    CompareAndSet 
                    Get  // �׵� �Լ�
                    �� RMW ������ �׵� �Լ��� �ƴ� ��, �� RMW ������ ��Ȯ���� �ʴٰ� �Ѵ�.

    �� ������ ��Ȯ���� ���� RMW ������ ���� ���� �ּ� 2�̴�.
       �� ���� : class RMWConsensus {
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
                 �� Atomic �޸𸮷� 2�� �̻��� �����忡 ���� ��Ȯ���� ���� RMW ������ ������ �� ����.


 3. Common2 RMW
    �� ��� �� v�� �Լ� ���� F�� ���ϴ� ��� �Լ� fi, fj�� ���� �� �� �ϳ��� �����ϴ� ���
       �� fi�� fj�� ��ȯ�� ���� : fi(fj(v)) = fj(fi(v))                               // GetAndIncrement
          �� �Լ��� �ٸ� �Լ��� ����� ��� : fi(fj(v)) = fi(v), fj(fi(v)) = fj(v)  // GetAndSet

       �� Ư¡ : ���ó��� ���μ������� ���ŵǴ� �߼��̴�.
                 ���� ���� 2�̴�.
                 �� 3 ������ : A�� �׻� �ڽ��� ���ڶ�� ���� �� �� �־�� �ϰ�, B�� C�� �ڽ��� ���ڶ�� ���� �� �� �־�� �Ѵ�.
                               �� Common2 RMW ������ ��ȯ�̳� ����Ⱑ �����Ͽ� C�� A�� B �� ��� ���� ���� ����Ǿ����� �Ǵ��� �� ����.


 4. CAS
    �� ���Ѵ��� ���� ���� ���´�.
       �� ���� : class CASConsensus {
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
                 �� ��� �ڷᱸ���� n �����忡�� Wait-Free�ϰ� ������ �� �ִ�.

 ====================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================

 �� ����� ����

 1. ������ �ǹ�
    �� ���� ���� ���� ���� ��ü�� ū ���� �� ��ü�� ������ �� ����.
       �� ���� �� 1 : Atomic �޸�
          ���� �� 2 : GetAndSet, GetAndAdd, ť, ����
          ���� �� n : (n, n * (n + 1) / 2) ���� ��ü
          ���� �� X : �޸� �̵�, CompareAndSet(), LL-SC

       �� ���� : �Ұ����� �õ��� �̿��� ������ �� �ִ�.
                 ���� ����� ������ �˸�, ����ȭ�� ������ �ȴ�.


 2. ���� 
    �� Ŭ���� C�� Atomic �޸𸮷� ��� ��ü�� Wait-Free�ϰ� ��ȯ�ϴ� ���� �����ϴٸ�, Ŭ���� C�� �����̴�.
       �� Ŭ���� C�� ����Ͽ� ���������� ��ȯ���� �ʰ�, Atomic �޸𸮸� ����Ͽ� ��� ��ü�� �ణ ������ �� ��ȯ�Ѵ�.  // �޼ҵ�, �Ķ����, ���ϰ� ����

       �� ���� ��ü : ��� ��ü�� Wait-Free�ϰ� ��ȯ�ϴ� ���� ������ ��ü
                      �� n �����忡�� �����ϴ� ���� ��ü�� ���� �� n �̻��� ��ü�� ���� �����ϴ�.
                         �� ���Ѵ��� ���� ���� ���� CAS ��ü�� ���� ������ ������ ������� ���� ��ü�� ������ �� �ִ�.


 3. ���� ��ü
    �� ���� : ���� ��ü�� �������̴�.
              �� ��� ��ü�� �ʱ� ���´� �����ϴ�.
                 ���� ���¿��� ���� �Է��� �ָ� �׻� ���� ����� ���´�.

    �� ���� ��ü : ��ȯ�ϰ��� �ϴ� ��ü�� ���� ��ü
                   �޼ҵ带 apply �ϳ��� ����
                   �� class Invocation {
                          MethodType type;
                          int v;
                      };
                      �� Invocation : ��ȯ�ϰ��� �ϴ� ��ü�� �޼ҵ�� �Է°��� ���� ��ü

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
                      �� Response : ���� �޼ҵ��� ��� ���� ������ ��ü


 4. Lock-Free ���� ��ü
    �� class NODE {
       public:
           Invocation m_inv;
           int m_seq;
           NODE* volatile next;
       };
       �� Log�� tail���� �����ϴ� NODE�� ����Ʈ�̴�.
          �� ���ݱ��� ���� ��ü�� ������ ��� �޼ҵ� ȣ���� ����
             ���� ��ü�� Log�� �����Ͽ� ���� ��ü�� ��� ���¸� �� �� �ִ�.

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

               while (0 == prefer->m_seq) {                                         // prefer�� ���������� �߰��Ǿ����� �˻�
                   NODE* head = get_max_head();
                   long long temp = 0;

                   std::atomic_compare_exchange_strong(                             // ���� ��ü�� ����Ͽ� ���ο� NODE�� Log�� head�� �����δ�.
                       reinterpret_cast<volatile std::atomic_llong*>(&head->next),  
                       &temp,
                       reinterpret_cast<long long>(prefer));

                   NODE* after = head->next;
                   after->m_seq = head->m_seq + 1;                                  // ���� �����尡 ���� �۾��� �ݺ��� �� ������, ��� ����.
                   m_head[thread_id] = after;                                       // ������ ���� ��ü�� �� �� �̻� ȣ������ �ʵ��� �Ѵ�.
               }

               SeqObject std_set;                                                   // ���� ��ü�� ������ ��, 
               NODE* p = tail.next;

               while (p != prefer) {                                                
                   std_set.apply(p->m_inv);                                         // Log�� �ִ� Invocation�� ���ο� ȣ����� �����Ű��,
                   p = p->next;
               }

               return std_set.apply(invoc);                                         // �� ����� ��ȯ�Ѵ�.
           }
       }
       �� ���� ��ü�� ��帶�� �����ϸ�, �� ������� head �迭�� ���� �� �� ȣ���� ���� ��ü�� �ٽ� ȣ������ �ʴ´�.
          ���� ��ü�� ���� �� NODE�� next�� � �����忡���� �����ϴ�.
          ���� ��ü�� �� ȣ�⸶�� �����Ǹ�, �ٸ� ������� ����� �ش� ���� ��ü�� ȣ������ �ʴ´�.
          
    �� Lock-Free�� ����? : �ٸ� ��忡�� ��� head �迭�� �����ϴ� ��� ������ ����� �� �ֱ� ����
                         
 ====================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================
 
 �� ����� ����

 1. Wait-Free ���� ��ü
    �� class WFUniversal {
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
       �� ������ �� �켱������ ���� �ٸ� �������� ���Ǹ� �����Ѵ�.  // ����, �ڱ� �ڽ��� ���� �� ������ ��� ���Ǹ� �õ��Ѵ�.
          �ٸ� �����带 �����ϰ� �����ϰ� ������ �� ������ ��� ���¿� ������ ���� ������ �� �ִ�. 
          �� Node�� seq�� MAX_THREADS�� ���� ������ ���
             �� ��� �����尡 ���� ������ �Ѵ�. 
                �ڱ� �ڽ��� ��� ���¿� ������ ���� ������ �� �ִ�.


 2. ���� ����
    �� Log�� ��尡 �����Ǵ°�?
       �� ��� apply�� �ڽ��� Node�� �߰��� ������ ����ȴ�.

       Log�� �� ��尡 ���� �� �߰��� �� �ִ°�?
       �� ���� �ڸ��� �߰� : ���� ��ü�� ���� �ذ�ȴ�.
          �ٸ� �ڸ��� �߰� : ���� �߰��� ��� a�� ��𼱰� �� ���� before�� �� ���� �ִ�.
                             �� before�� head[]���� ������ ���ۿ� ����.
                                a�� ��� �� �� head[]�� �� ���� �ִ�.
                                a�� head[]�� �� �������� a.seq�� 0�� �� �� �����Ƿ�, prefer�� a�� �� �� ����.

       head[i] = announce[i]���� head[i]�� �����ϴ� ��찡 �ִ°�?
       �� announce[i] = new Node(invoc); ���� announce[i]�� �ٸ� ��尡 �ٸ� �����忡 ���� �߰��� ���
          �� ���� ��ü�� ���� �ذ�ȴ�.
             �ݺ����� �� �� �� ���� ��ȿ������ ���̴�.

 ====================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================

 �� ť

 1. Ǯ
    �� Ư¡ : ���� �������� ���� ���� ���
              Get(), Set() �޼ҵ带 �����Ѵ�.
              Contains() �޼ҵ带 �׻� �������� �ʴ´�.
              ���� ������ - �Һ��� ������ ���۷� ����Ѵ�.

    �� ����
       �� ������ ť
          ���� ť : �����ϱ� ����.
                    �����ڿ� �Һ����� ������ �����Ѵ�.
                    �� ConditionVariable() �޼ҵ� �ʿ�
                       �� ������ �� �� Lock ȹ��
                          Lock�� ���� ä�� Block �� Lock ����
                          �ü�� ȣ�� �ʿ�  // ������ �����ٸ����� ���� �ʼ�

    �� ����
       �� ���� : Ư�� ������ ��ٸ� �ʿ䰡 ����.  // ����ִ� Ǯ�� Get() �޼ҵ� ȣ�� �� ���� �ڵ� ��ȯ
          �κ��� : Ư�� ������ ������ ��ٷ��� �Ѵ�.  // ����ִ� Ǯ�� Get() �޼ҵ� ȣ�� �� �ٸ� �������� Set() �޼ҵ� ȣ���� ��ٸ���.
          ������ : �ٸ� �������� �޼ҵ� ȣ����� ��ø�� �ʿ��ϴ�.


 2. ť
    �� Enq
       �� �ܼ��� ���� : tail�� ����Ű�� Node�� CAS�� �� ��� �߰�
                        ���� �� tail �̵�
                        ���� �� ��õ�
                        �� void Enq(int x) {
                               Node* n = new Node(x);

                               while (true) {
                                   if (CAS(&tail->next, NULL, n) {
                                       tail = e;
                                       return;
                                   }
                               }
                           }
                           �� ������ : CAS ���� �� tail�� ������ �̷����� ���� �ÿ��� �ٸ� ��� ������ Non-Blocking���� �����ؾ� �Ѵ�.
                                       �� �ذ�å : tail�� ������ �����Ѵ�.

       �� 1�� ���� : void Enq(int x) {
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
                     �� ������ : A, ������ �������� �ʴ�.
                                 B, �ٸ� �������� ������ ��� �� �ִ�.
                                 �� �ذ�å : CAS ���

       �� 2�� ���� : void Enq(int x) {
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

    �� Deq
       �� �ܼ��� ���� : ť�� ��� �ִ��� �˻�
                        head ����
                        �� int deq() {
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
                           �� ������ : �ٸ� �����忡�� first->next�� �̹� delete ���� ���ɼ� ��
                                       
       �� 1�� ���� : int deq() {
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
                     �� ������ : Enq���� tail�� ������ �̷����� �ʾ��� �ÿ��� Non-Blocking���� �����ؾ� �Ѵ�.
                                 �� �ذ�å : tail�� ������ �����Ѵ�.

       �� 2�� ���� : int deq() {
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
    �� ������ : new(), free()�� �޸𸮸� �����ϹǷ�, CAS ���� �� ������ �ּҿ� ������ ���� ���� �ٸ� ��ü�� ������ ��� ������ �߻��Ѵ�.  // Java�� Garbage Collector�� ����ϹǷ� �̷��� ������ �߻����� �ʴ´�.
                �� �ذ�å 1 : �����Ϳ� �������� Ȯ�� ����ü�� �����ͷ� ����Ѵ�.
                              �� LL-SC ��� : ���� �˻��ϴ� ���� �ƴ�, ���� ���θ� �˻�
                                              �� CAS���� ���������, Wait-Free�� �ƴϴ�.

                   �ذ�å 2 : ���� ī���͸� ����Ѵ�.
                              �� std::atomic<std::shared_ptr<>> : Ư�� ��带 ���� ���� �����尡 ���� ���, �ش� ���� ����� �� ����.
                                                                  �� std::atmoic<std::shared_ptr>>�� Lock-Free�� �ƴϴ�.

                   �ذ�å 3 : ������ �޸� ���� ����� ����Ѵ�.
                              �� EBR, Hazard Pointer, etc...


    �� �̷�
       �� 32bit : ST_Ptr�� 64bit �ڷᱸ���̹Ƿ�, ĳ�� ��輱�� ���� �� �ִ�.
                  �� std::atomic_llong���� ����

          64bit : ST_Ptr�� 128bit �ڷᱸ���̹Ƿ�, 128bit CAS �ʿ�
                  �� BOOLEAN InterlockedCompareExchange128(
                         LONG64 volatile* Dest,
                         LONG64           ExchangeHigh,
                         LONG64           ExchangedLow,
                         LONG64*          ComparandResult
                     );
                     �� �ڷᱸ���� 128bit ������ �����ؾ� �Ѵ�.  // ĳ�� ���� ����
                        �� class alignas(16) ST_Ptr {
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

                     �� bool CAS(ST_Ptr* next_ptr, ST_NODE* old_ptr, ST_NODE* new_ptr, long long old_st, long long new_st) {
                            ST_Ptr old_st { old_ptr, old_st };

                            return InterlockedCompareExchange128(
                                reinterpret_cast<LONG64 volatile*>(next_ptr),
                                new_stamp,
                                reinterpret_cast<LONG64>(new_ptr),
                                reinterpret_cast<LONG64*>(&old_st)
                            );
                        }

                     �� ������ : ��� ST_Ptr load�� store�� atomic�ϰ� �����ؾ� ������, atomic_128int�� �������� �ʴ´�.
                                 �� ��� load�� store�� InterlockedCompareExchange128�� ����ؾ� �ϹǷ� ������ ���ϵȴ�.
                                 
                                 �޸� ���� �� head->next�� ������ �� �ִ�.
                                 �� �ذ�å : free_list�� ����Ͽ� ������ ���´�.
                                             Lock-Free Ȥ�� thread_local�� �����ؾ� �Ѵ�.

    �� ���� : class ST_Queue {
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

 �� ����

 1. ����
    �� ���� ����Ʈ�� ����
       top�� ù ��° ��带 ����Ų��.
       �� -1�� ���ÿ� �߰��ϴ� ���� ������� �ʴ´�.
          ������ ������� ��� top�� nullptr
          ������ ������� ��� Pop() �޼ҵ�� -2�� �����Ѵ�.


 2. ����� ����
    �� CAS�� �̿��Ͽ� top�� ��ȯ�� Non-Blocking���� �����Ѵ�.
       ABA ������ �����ϱ� ���� delete���� �ʴ´�.

    �� ���� : class LF_STACK {
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
              �� ������ : �޼ҵ� ȣ���� ������ top�� ���� ������ CAS ������ �ϳ��� ����ǹǷ�, �������������� ��Ÿ�� �� �ִ�.
                          new, delete ��� �� ABA ������ �߻��� Ȯ���� ť���� ����.
                          ������ ������ ��� CAS�� ������ Ȯ���� ��������, ��� �ھ��� �޸� ������ �ߴܵȴ�.


 3. BACK OFF ����
    �� CAS�� �������� ��� ������ �Ⱓ ���� ������ �����.
       �� ó������ ª��
          ��� �����ϸ� ���� ���
          ù ��° �õ��� �����ϰ� ª��
          �����帶�� �Ⱓ�� ���� �ȿ��� �����ϰ� ����

    �� BACK OFF ��ü : CAS�� �������� ��� ���� CAS�� �õ��ϱ� ���� ���
                       �� class BackOff {
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

    �� ���� : class LF_BO_STACK {
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
              �� ���� �ð��� �߿����� �ʴ�.
                 �������� �޸� ������ �������� �ʴ� ���� �ٶ����ϴ�.
                 �� _asm mov eax, delay;
                    myloop:
                    _asm dec eax
                    _asm jnz myloop;


 4. �Ұ�
    �� ť�� ������ ����Ʈ�� ���� �κп��� ���� �浹�� �߻��ϹǷ� fine-grained synchronization�� �Ұ����ϴ�.
       �� �����尡 ���� �浹�� ��� ������ ��ü�� ���� Lock-Free�� �����͸� �ְ� �޵��� �Ѵ�.
          ���� ���￡ ����Ͽ� ������ ��ü�� ������ �غ��Ѵ�.
          �� Push() �޼ҵ�� Pop() �޼ҵ尡 ���� ���ÿ� ȣ��ȴٸ� ������ ������� ���� ��ȭ ���� Lock-Free�� ���� ��ȯ�� �� �ִ�.  // ���θ� �Ұ��Ѵٰ� ǥ���Ѵ�.

    �� EliminationArray : ���� ���� ���Ҹ� ���� ���ϸ� �л�
                          �� ������ ���� ��� ������ ������ ���ƾ� �ϰ�, ������ ���� ��� ������ ������ ����� �Ѵ�.
                             �� Visit() �޼ҵ� ȣ�� �� RET_BUSY_TIMEOUT�� ���ϵǾ��� ��� �ø���.
                                Visit() �޼ҵ� ȣ�� �� RET_TIMEOUT�� ���ϵǾ��� ��� ���δ�.

                          ������� ������ ���Ҹ� ��� �Ұ� �õ�
                          �� ���� X : ���� ���� ��ü�� ��������̰�, Lock-Free ������ ��ƴ�. 

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

    �� Exchanger : �� ���� �����尡 Lock-Free�� ���� ��ȯ�� �� �ְ� ���ִ� ��ü
                   �� ���� �����尡 Exchange() �޼ҵ带 ȣ���ϸ� ������ �Է� ���� �����Ѵ�.
                   ���� ������ �Է� ���� ����(����, ����, Ÿ�Ӿƿ�(-2))�̴�.

                   �� Exchanger�� 3���� ���¸� ���´�.
                      �� EMPTY : CAS�� �̿��Ͽ� slot�� �ڽ��� �Է� ���� �ְ� ���¸� WAITING���� �ٲ۴�.
                                 �� ���� : ������ �ϸ� �ٸ� �������� ��ȯ �õ��� ��ٸ���.
                                           ��ȯ�� �Ϸ�� �� ��� ���̴� ������� ���¸� EMPTY�� �ٲ۴�.  // �� �����常 �۾��ϹǷ� CAS ��� X

                                    ���� : ó������ �ٽ� �õ�

                                 �ٸ� �����尡 ��Ÿ���� �ʴ´ٸ�, CAS�� �̿��Ͽ� ���¸� EMPTY�� �ٲ۴�.
                                 �� ���� : RET_TIME_OUT ����
                                  
                                    ���� : �ٸ� �����尡 ��Ÿ���ٴ� �ǹ��̹Ƿ�, ��� ���̴� ������� ��ȯ�� �Ϸ��Ѵ�.

                         WAITING : CAS�� �̿��Ͽ� slot�� �ڽ��� �Է� ���� �ְ� ��� ���� �������� �Է� ���� ��´�.
                                   �� ���� : ��� ���� �������� �Է� �� ����

                                      ���� : ó������ �ٽ� �õ�

                         BUSY : ó������ �ٽ� �õ�
                         �� ���� ��ȯ�� ���� ������ Atomic�ϰ� ������ �ʿ� �� : int�� ���� 2bit�� ���¸� ��Ÿ���� �� ���
                   
                   �� class Exchanger {
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

    �� EliminationBackoffStack : class LF_BO_STACK {
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
                                �� LF_STACK�� �޸� Ȯ���� ���� ����
                                   ���ϰ� �������� �������� �ҰŰ� �����ϰ�, ���������� ���� ���� ������ �Ϸ�ȴ�.
                                   �Ұŵ� ������ ���ÿ� �����ϱ� �ʱ� ������ ������ �پ���.

    �� ������?
       �� ABA : ABA ������ �߻��ص�, ��ȯ ����� �ٲ� �� ���̹Ƿ� ������ �߻����� �ʴ´�.
       �� Ÿ�Ӿƿ� : �ʹ� ª�� ��ȯ �ð��� �׻� �����ϹǷ�, Ÿ�Ӿƿ� ������ �����ؾ� �Ѵ�.

 ====================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================

 �� ��ŵ ����Ʈ

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

 ====================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================

 �� TBB



*/

