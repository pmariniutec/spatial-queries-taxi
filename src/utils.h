#ifndef UTILS_H
#define UTILS_H

#include <limits>
#include <string>
#include <vector>

enum PointType {
  DEFAULT,
  PICKUP,
  DROPOFF,
  NH_BOUND
};

constexpr int MAXNODES = 4;
constexpr int MINNODES = MAXNODES / 2;


struct Point {
  Point() = default;
  Point(double f, double s, PointType t = DEFAULT) : first{ f }, second{ s }, type{ t } {}

  double first, second;
  PointType type = DEFAULT;

  bool operator<(Point& rhs) {
	return first < rhs.first && second < rhs.second;
  }
  bool operator>(Point& rhs) {
	return first > rhs.first && second > rhs.second;
  }
};

inline bool operator==(const Point& lhs, const Point& rhs) {
  auto eps = std::numeric_limits<double>::epsilon();
  return std::abs(lhs.first - rhs.first) < eps && std::abs(lhs.second - rhs.second) < eps;
}

using Points = std::vector<Point>;

inline bool operator<(std::pair<double, Points>& lhs, std::pair<double, Points>& rhs) {
  return lhs.first < rhs.first;
}

struct Neighborhood {
  std::string name;
  Points polygon;
};

struct Rect {
  Rect() = default;

  Rect(double a_minX, double a_minY, double a_maxX, double a_maxY) {
	m_min[0] = a_minX;
	m_min[1] = a_minY;

	m_max[0] = a_maxX;
	m_max[1] = a_maxY;
  }

  double m_min[2];
  double m_max[2];
};

struct Node;

struct Branch {
  Rect m_rect;
  Node* m_child;
  Points m_data;
};

struct Node {
  bool isInternal() { return (m_level > 0); }
  bool isLeaf() { return (m_level == 0); }

  int m_count;
  int m_level;
  Branch m_branch[MAXNODES];
};

struct ListNode {
  ListNode* m_next;
  Node* m_node;
};

struct PartitionVars {
  enum { NOT_TAKEN = -1 };

  int m_partition[MAXNODES + 1];
  int m_total;
  int m_minFill;
  int m_count[2];
  Rect m_cover[2];
  double m_area[2];

  Branch m_branchBuf[MAXNODES + 1];
  int m_branchCount;
  Rect m_coverSplit;
  double m_coverSplitArea;
};

#endif
