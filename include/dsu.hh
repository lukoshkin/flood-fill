/** Disjoint Set Union (DSU)
 *  ------------------------
 *  More info on implementation (ru and eng):
 *  -  https://e-maxx.ru/algo/dsu
 *  -  https://cp-algorithms.com/data_structures/disjoint_set_union.html
 */

#ifndef __DSU_HH
#define __DSU_HH

#include <unordered_map>


struct Node {
  uint data;
  Node * parent;
  uint rank;

  Node(uint);
};


class DSU {
  std::unordered_map<uint, Node*> map;

public:
  ~DSU();
  void makeSet(uint);
  uint findSet(uint);
  Node * findSet(Node *);
  void unionSets(uint, uint);
};

#endif
