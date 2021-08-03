#ifndef RTREE_H
#define RTREE_H

#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <stdlib.h>

#include <algorithm>
#include <functional>
#include <vector>
#include <limits>
#include <iostream>

#include "utils.h"

class RTree {
 public:
  RTree();
  RTree(const RTree& other);
  ~RTree();

  void insert(const double a_min[2], const double a_max[2], Points& a_dataId);
  void remove(const double a_min[2], const double a_max[2], const Points& a_dataId);
  int search(const Point a_min, const Point a_max, std::vector<Points>& objs) const;
  void removeAll();

  int count();

  bool getMBRs(std::vector<std::vector<Points>>& mbrs_n);
  bool nearest(int k, Point point, std::vector<Points>& objs);
  bool inRadius(Point point, double distance, std::vector<Points>& objs);
  Rect mbr(Points pol);
  double minDist(Point Point, Rect a_rect);
  double minMaxDist(Point Point, Rect a_rect);

  bool onSegment(Point p, Point q, Point r);
  int orientation(Point p, Point q, Point r);
  bool doIntersect(Point p1, Point q1, Point p2, Point q2);
  bool isInside(Points& polygon, Point p);
  double pointToLine(const Point& p, const Point& l1, const Point& l2);
  double pointToPoly(const Point& p, Points& poly);

  std::vector<Points> mObjs;

 protected:
  Node* allocateNode();
  void deleteNode(Node* a_node);
  void initNode(Node* a_node);
  void initRect(Rect* a_rect);
  bool insertRectHelper(const Branch& a_branch, Node* a_node, Node** a_newNode, int a_level);
  bool insertRect(const Branch& a_branch, Node** a_root, int a_level);
  Rect nodeCover(Node* a_node);
  bool addBranch(const Branch* a_branch, Node* a_node, Node** a_newNode);
  void disconnectBranch(Node* a_node, int a_index);
  int pickBranch(const Rect* a_rect, Node* a_node);
  Rect mergeRects(const Rect* a_rectA, const Rect* a_rectB);
  void splitNode(Node* a_node, const Branch* a_branch, Node** a_newNode);
  double calcRectVolume(Rect* a_rect);
  void getBranches(Node* a_node, const Branch* a_branch, PartitionVars* a_parVars);
  void choosePartition(PartitionVars* a_parVars, int a_minFill);
  void loadNodes(Node* a_nodeA, Node* a_nodeB, PartitionVars* a_parVars);
  void initParVars(PartitionVars* a_parVars, int a_maxRects, int a_minFill);
  void pickSeeds(PartitionVars* a_parVars);
  void classify(int a_index, int a_group, PartitionVars* a_parVars);
  bool removeRect(Rect* a_rect, const Points& a_id, Node** a_root);
  bool removeRectHelper(Rect* a_rect, const Points& a_id, Node* a_node, ListNode** a_listNode);
  bool overlapSpecial(Rect* a_rectA, Rect* a_rectB) const;
  bool overlap(Rect* a_rectA, Rect* a_rectB) const;
  void reInsert(Node* a_node, ListNode** a_listNode);
  bool search(Node* a_node, Rect* a_rect, int& a_foundCount, std::vector<Points>& objs) const;
  void removeAllHelper(Node* a_node);
  void reset();
  void countHelper(Node* a_node, int& a_count);

  void copy(Node* current, Node* other);

  Node* m_root;
  double m_unitSphereVolume;
};

#endif
