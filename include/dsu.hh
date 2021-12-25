/** Disjoint Set Union (DSU)
 *  ------------------------
 *  More info on implementation (ru and eng):
 *  -  https://e-maxx.ru/algo/dsu
 *  -  https://cp-algorithms.com/data_structures/disjoint_set_union.html
 */

#ifndef __DSU_HH
#define __DSU_HH

#include <unordered_map>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/unordered_map.hpp>


class Node {
    uint data;
    uint rank;
    Node * parent;

  public:
    Node();
    Node(uint);

  private:
    friend class DSU;
    friend class Flood;
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive & ar, unsigned) {
      ar & data;
      ar & rank;
      ar & parent;
  }
};


class DSU {
  std::unordered_map<uint, Node*> map;

  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive & ar, unsigned) { ar & map; }

public:
  ~DSU();
  void makeSet(uint);
  Node * findSet(uint);
  Node * findSet(Node *);
  void unionSets(uint, uint);
  void changeParent(uint, uint);
};

#endif
