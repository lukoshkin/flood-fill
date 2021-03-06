#include <iostream>
#include "../include/dsu.hh"


Node::Node(uint _data) {
  data = _data;
  parent = this;
  rank = 0;
}

DSU::~DSU() {
  for (auto it: map) delete it.second;
}

void DSU::makeSet(uint data) {
  Node * node = new Node(data);
  map.emplace(data, node);
}

uint DSU::findSet(uint data) {
  return findSet(map[data])->data;
}

Node * DSU::findSet(Node * node) {
  if (node->parent == node) return node;
  return node->parent = findSet(node->parent);
}

void DSU::unionSets(uint data1, uint data2) {
  Node * node1 = map[data1];
  Node * node2 = map[data2];

  node1 = findSet(node1);
  node2 = findSet(node2);

  if (node1 != node2) {
    if (node1->rank < node2->rank) std::swap(node1, node2);
    node2->parent = node1->parent;
    if (node1->rank == node2->rank) ++node1->rank;
  }
}
