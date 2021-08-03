#include "rtree.h"
#include <limits>

RTree::RTree() {
  m_root = allocateNode();
  m_root->m_level = 0;
  m_unitSphereVolume = 3.141593;
}


RTree::RTree(const RTree& other) : RTree() {
  copy(m_root, other.m_root);
}


RTree::~RTree() {
  reset();
}


void RTree::insert(const double a_min[2], const double a_max[2], Points& a_dataId) {
  mObjs.push_back(a_dataId);

  Branch branch;
  branch.m_data = a_dataId;
  branch.m_child = nullptr;

  for (int axis = 0; axis < 2; ++axis) {
	branch.m_rect.m_min[axis] = a_min[axis];
	branch.m_rect.m_max[axis] = a_max[axis];
  }

  insertRect(branch, &m_root, 0);
}


void RTree::remove(const double a_min[2], const double a_max[2], const Points& a_dataId) {
  Rect rect;

  for (int axis = 0; axis < 2; ++axis) {
	rect.m_min[axis] = a_min[axis];
	rect.m_max[axis] = a_max[axis];
  }

  removeRect(&rect, a_dataId, &m_root);
}


int RTree::search(const Point a_min, const Point a_max, std::vector<Points>& objs) const {

  objs.clear();
  Rect rect;

  rect.m_min[0] = a_min.first;
  rect.m_min[1] = a_min.second;
  rect.m_max[0] = a_max.first;
  rect.m_max[1] = a_max.second;


  int foundCount = 0;
  search(m_root, &rect, foundCount, objs);

  return foundCount;
}


int RTree::count() {
  int count = 0;
  countHelper(m_root, count);

  return count;
}


void RTree::countHelper(Node* a_node, int& a_count) {
  if (a_node->isInternal()) {
	for (int index = 0; index < a_node->m_count; ++index) {
	  countHelper(a_node->m_branch[index].m_child, a_count);
	}
  } else {
	a_count += a_node->m_count;
  }
}


void RTree::copy(Node* current, Node* other) {
  current->m_level = other->m_level;
  current->m_count = other->m_count;

  if (current->isInternal()) {
	for (int index = 0; index < current->m_count; ++index) {
	  Branch* currentBranch = &current->m_branch[index];
	  Branch* otherBranch = &other->m_branch[index];

	  std::copy(otherBranch->m_rect.m_min,
		otherBranch->m_rect.m_min + 2,
		currentBranch->m_rect.m_min);

	  std::copy(otherBranch->m_rect.m_max,
		otherBranch->m_rect.m_max + 2,
		currentBranch->m_rect.m_max);

	  currentBranch->m_child = allocateNode();
	  copy(currentBranch->m_child, otherBranch->m_child);
	}
  } else {
	for (int index = 0; index < current->m_count; ++index) {
	  Branch* currentBranch = &current->m_branch[index];
	  Branch* otherBranch = &other->m_branch[index];

	  std::copy(otherBranch->m_rect.m_min,
		otherBranch->m_rect.m_min + 2,
		currentBranch->m_rect.m_min);

	  std::copy(otherBranch->m_rect.m_max,
		otherBranch->m_rect.m_max + 2,
		currentBranch->m_rect.m_max);

	  currentBranch->m_data = otherBranch->m_data;
	}
  }
}


void RTree::removeAll() {
  mObjs.clear();

  reset();

  m_root = allocateNode();
  m_root->m_level = 0;
}


void RTree::reset() {
  removeAllHelper(m_root);
}


void RTree::removeAllHelper(Node* a_node) {
  if (a_node->isInternal()) {
	for (int index = 0; index < a_node->m_count; ++index) {
	  removeAllHelper(a_node->m_branch[index].m_child);
	}
  }
  deleteNode(a_node);
}


Node* RTree::allocateNode() {
  Node* newNode;
  newNode = new Node;
  initNode(newNode);
  return newNode;
}


void RTree::deleteNode(Node* a_node) {
  delete a_node;
}


void RTree::initNode(Node* a_node) {
  a_node->m_count = 0;
  a_node->m_level = -1;
}


void RTree::initRect(Rect* a_rect) {
  for (int index = 0; index < 2; ++index) {
	a_rect->m_min[index] = 0;
	a_rect->m_max[index] = 0;
  }
}

bool RTree::insertRectHelper(const Branch& a_branch, Node* a_node, Node** a_newNode, int a_level) {
  if (a_node->m_level > a_level) {
	Node* otherNode;

	int index = pickBranch(&a_branch.m_rect, a_node);

	bool childWasSplit = insertRectHelper(a_branch, a_node->m_branch[index].m_child, &otherNode, a_level);

	if (!childWasSplit) {
	  a_node->m_branch[index].m_rect = mergeRects(&a_branch.m_rect, &(a_node->m_branch[index].m_rect));
	  return false;
	} else {
	  a_node->m_branch[index].m_rect = nodeCover(a_node->m_branch[index].m_child);
	  Branch branch;
	  branch.m_child = otherNode;
	  branch.m_rect = nodeCover(otherNode);

	  return addBranch(&branch, a_node, a_newNode);
	}
  } else if (a_node->m_level == a_level) {
	return addBranch(&a_branch, a_node, a_newNode);
  }

  return false;
}

bool RTree::insertRect(const Branch& a_branch, Node** a_root, int a_level) {
  Node* newNode;

  if (insertRectHelper(a_branch, *a_root, &newNode, a_level)) {

	Node* newRoot = allocateNode();
	newRoot->m_level = (*a_root)->m_level + 1;

	Branch branch;

	branch.m_rect = nodeCover(*a_root);
	branch.m_child = *a_root;
	addBranch(&branch, newRoot, nullptr);

	branch.m_rect = nodeCover(newNode);
	branch.m_child = newNode;
	addBranch(&branch, newRoot, nullptr);

	*a_root = newRoot;

	return true;
  }

  return false;
}


Rect RTree::nodeCover(Node* a_node) {
  auto rect = a_node->m_branch[0].m_rect;
  for (int index = 1; index < a_node->m_count; ++index) {
	rect = mergeRects(&rect, &(a_node->m_branch[index].m_rect));
  }

  return rect;
}


bool RTree::addBranch(const Branch* a_branch, Node* a_node, Node** a_newNode) {
  if (a_node->m_count < MAXNODES) {
	a_node->m_branch[a_node->m_count] = *a_branch;
	++a_node->m_count;
	return false;
  }

  splitNode(a_node, a_branch, a_newNode);
  return true;
}

void RTree::disconnectBranch(Node* a_node, int a_index) {
  a_node->m_branch[a_index] = a_node->m_branch[a_node->m_count - 1];

  --a_node->m_count;
}

int RTree::pickBranch(const Rect* a_rect, Node* a_node) {

  bool firstTime = true;
  double increase;
  double bestIncr = -1.0;
  double area;
  double bestArea;
  int best = 0;
  Rect tempRect;

  for (int index = 0; index < a_node->m_count; ++index) {
	auto curRect = &a_node->m_branch[index].m_rect;
	area = calcRectVolume(curRect);
	tempRect = mergeRects(a_rect, curRect);
	increase = calcRectVolume(&tempRect) - area;
	if ((increase < bestIncr) || firstTime) {
	  best = index;
	  bestArea = area;
	  bestIncr = increase;
	  firstTime = false;
	} else if ((increase == bestIncr) && (area < bestArea)) {
	  best = index;
	  bestArea = area;
	  bestIncr = increase;
	}
  }
  return best;
}

Rect RTree::mergeRects(const Rect* a_rectA, const Rect* a_rectB) {
  Rect newRect;

  for (int index = 0; index < 2; ++index) {
	newRect.m_min[index] = std::min(a_rectA->m_min[index], a_rectB->m_min[index]);
	newRect.m_max[index] = std::max(a_rectA->m_max[index], a_rectB->m_max[index]);
  }

  return newRect;
}

void RTree::splitNode(Node* a_node, const Branch* a_branch, Node** a_newNode) {

  PartitionVars localVars;
  PartitionVars* parVars = &localVars;

  getBranches(a_node, a_branch, parVars);

  choosePartition(parVars, MINNODES);

  *a_newNode = allocateNode();
  (*a_newNode)->m_level = a_node->m_level;

  a_node->m_count = 0;
  loadNodes(a_node, *a_newNode, parVars);
}


double RTree::calcRectVolume(Rect* a_rect) {

  double sumOfSquares = 0.0;
  double radius;

  for (int index = 0; index < 2; ++index) {
	double halfExtent = (a_rect->m_max[index] - a_rect->m_min[index]) * 0.5;
	sumOfSquares += halfExtent * halfExtent;
  }

  radius = std::sqrt(sumOfSquares);


  return (radius * radius * m_unitSphereVolume);
}

void RTree::getBranches(Node* a_node, const Branch* a_branch, PartitionVars* a_parVars) {

  for (int index = 0; index < MAXNODES; ++index) {
	a_parVars->m_branchBuf[index] = a_node->m_branch[index];
  }
  a_parVars->m_branchBuf[MAXNODES] = *a_branch;
  a_parVars->m_branchCount = MAXNODES + 1;

  a_parVars->m_coverSplit = a_parVars->m_branchBuf[0].m_rect;
  for (int index = 1; index < MAXNODES + 1; ++index) {
	a_parVars->m_coverSplit = mergeRects(&a_parVars->m_coverSplit, &a_parVars->m_branchBuf[index].m_rect);
  }
  a_parVars->m_coverSplitArea = calcRectVolume(&a_parVars->m_coverSplit);
}


void RTree::choosePartition(PartitionVars* a_parVars, int a_minFill) {

  double biggestDiff;
  int group, chosen = 0, betterGroup = 0;

  initParVars(a_parVars, a_parVars->m_branchCount, a_minFill);
  pickSeeds(a_parVars);

  while (((a_parVars->m_count[0] + a_parVars->m_count[1]) < a_parVars->m_total)
		 && (a_parVars->m_count[0] < (a_parVars->m_total - a_parVars->m_minFill))
		 && (a_parVars->m_count[1] < (a_parVars->m_total - a_parVars->m_minFill))) {
	biggestDiff = -1.0;
	for (int index = 0; index < a_parVars->m_total; ++index) {
	  if (PartitionVars::NOT_TAKEN == a_parVars->m_partition[index]) {
		Rect* curRect = &a_parVars->m_branchBuf[index].m_rect;
		Rect rect0 = mergeRects(curRect, &a_parVars->m_cover[0]);
		Rect rect1 = mergeRects(curRect, &a_parVars->m_cover[1]);
		double growth0 = calcRectVolume(&rect0) - a_parVars->m_area[0];
		double growth1 = calcRectVolume(&rect1) - a_parVars->m_area[1];
		double diff = growth1 - growth0;
		if (diff >= 0) {
		  group = 0;
		} else {
		  group = 1;
		  diff = -diff;
		}

		if (diff > biggestDiff) {
		  biggestDiff = diff;
		  chosen = index;
		  betterGroup = group;
		} else if ((diff == biggestDiff) && (a_parVars->m_count[group] < a_parVars->m_count[betterGroup])) {
		  chosen = index;
		  betterGroup = group;
		}
	  }
	}
	classify(chosen, betterGroup, a_parVars);
  }

  if ((a_parVars->m_count[0] + a_parVars->m_count[1]) < a_parVars->m_total) {
	if (a_parVars->m_count[0] >= a_parVars->m_total - a_parVars->m_minFill) {
	  group = 1;
	} else {
	  group = 0;
	}
	for (int index = 0; index < a_parVars->m_total; ++index) {
	  if (PartitionVars::NOT_TAKEN == a_parVars->m_partition[index]) {
		classify(index, group, a_parVars);
	  }
	}
  }
}


void RTree::loadNodes(Node* a_nodeA, Node* a_nodeB, PartitionVars* a_parVars) {
  for (int index = 0; index < a_parVars->m_total; ++index) {

	int targetNodeIndex = a_parVars->m_partition[index];
	Node* targetNodes[] = { a_nodeA, a_nodeB };

	addBranch(&a_parVars->m_branchBuf[index], targetNodes[targetNodeIndex], nullptr);
  }
}

void RTree::initParVars(PartitionVars* a_parVars, int a_maxRects, int a_minFill) {
  a_parVars->m_count[0] = a_parVars->m_count[1] = 0;
  a_parVars->m_area[0] = a_parVars->m_area[1] = 0.0;
  a_parVars->m_total = a_maxRects;
  a_parVars->m_minFill = a_minFill;
  for (int index = 0; index < a_maxRects; ++index) {
	a_parVars->m_partition[index] = PartitionVars::NOT_TAKEN;
  }
}


void RTree::pickSeeds(PartitionVars* a_parVars) {
  int seed0 = 0, seed1 = 0;
  double worst, waste;
  double area[MAXNODES + 1];

  for (int index = 0; index < a_parVars->m_total; ++index) {
	area[index] = calcRectVolume(&a_parVars->m_branchBuf[index].m_rect);
  }

  worst = -a_parVars->m_coverSplitArea - 1;
  for (int indexA = 0; indexA < a_parVars->m_total - 1; ++indexA) {
	for (int indexB = indexA + 1; indexB < a_parVars->m_total; ++indexB) {
	  auto oneRect = mergeRects(&a_parVars->m_branchBuf[indexA].m_rect, &a_parVars->m_branchBuf[indexB].m_rect);
	  waste = calcRectVolume(&oneRect) - area[indexA] - area[indexB];
	  if (waste > worst) {
		worst = waste;
		seed0 = indexA;
		seed1 = indexB;
	  }
	}
  }

  classify(seed0, 0, a_parVars);
  classify(seed1, 1, a_parVars);
}


void RTree::classify(int a_index, int a_group, PartitionVars* a_partitionVars) {
  a_partitionVars->m_partition[a_index] = a_group;

  if (a_partitionVars->m_count[a_group] == 0) {
	a_partitionVars->m_cover[a_group] = a_partitionVars->m_branchBuf[a_index].m_rect;
  } else {
	a_partitionVars->m_cover[a_group] = mergeRects(&a_partitionVars->m_branchBuf[a_index].m_rect, &a_partitionVars->m_cover[a_group]);
  }

  a_partitionVars->m_area[a_group] = calcRectVolume(&a_partitionVars->m_cover[a_group]);

  ++a_partitionVars->m_count[a_group];
}

bool RTree::removeRect(Rect* a_rect, const Points& a_id, Node** a_root) {
  ListNode* reInsertList = nullptr;

  if (!removeRectHelper(a_rect, a_id, *a_root, &reInsertList)) {
	while (reInsertList) {
	  Node* tempNode = reInsertList->m_node;

	  for (int index = 0; index < tempNode->m_count; ++index) {
		insertRect(tempNode->m_branch[index],
		  a_root,
		  tempNode->m_level);
	  }

	  ListNode* remLNode = reInsertList;
	  reInsertList = reInsertList->m_next;

	  deleteNode(remLNode->m_node);
	  delete remLNode;
	}

	if ((*a_root)->m_count == 1 && (*a_root)->isInternal()) {
	  Node* tempNode = (*a_root)->m_branch[0].m_child;

	  deleteNode(*a_root);
	  *a_root = tempNode;
	}
	return false;
  } else {
	return true;
  }
}

bool RTree::removeRectHelper(Rect* a_rect, const Points& a_id, Node* a_node, ListNode** a_listNode) {

  if (a_node->isInternal()) {
	for (int index = 0; index < a_node->m_count; ++index) {
	  if (overlapSpecial(a_rect, &(a_node->m_branch[index].m_rect))) {
		if (!removeRectHelper(a_rect, a_id, a_node->m_branch[index].m_child, a_listNode)) {
		  if (a_node->m_branch[index].m_child->m_count >= MINNODES) {
			a_node->m_branch[index].m_rect = nodeCover(a_node->m_branch[index].m_child);
		  } else {
			reInsert(a_node->m_branch[index].m_child, a_listNode);
			disconnectBranch(a_node, index);
		  }
		  return false;
		}
	  }
	}
	return true;
  } else {
	for (int index = 0; index < a_node->m_count; ++index) {
	  if (a_node->m_branch[index].m_data == a_id) {
		disconnectBranch(a_node, index);
		return false;
	  }
	}
	return true;
  }
}


bool RTree::overlapSpecial(Rect* a_rectA, Rect* a_rectB) const {
  for (int index = 0; index < 2; ++index) {
	if (a_rectA->m_min[index] > a_rectB->m_max[index] || a_rectB->m_min[index] > a_rectA->m_max[index]) {
	  return false;
	}
  }

  return true;
}

bool RTree::overlap(Rect* a_rectA, Rect* a_rectB) const {
  if (a_rectA->m_min[0] <= a_rectB->m_min[0] && a_rectB->m_max[0] <= a_rectA->m_max[0] && a_rectA->m_min[1] <= a_rectB->m_min[1] && a_rectB->m_max[1] <= a_rectA->m_max[1])
	return true;

  return false;
}

void RTree::reInsert(Node* a_node, ListNode** a_listNode) {
  ListNode* newListNode;

  newListNode = new ListNode();
  newListNode->m_node = a_node;
  newListNode->m_next = *a_listNode;
  *a_listNode = newListNode;
}


bool RTree::search(Node* a_node, Rect* a_rect, int& a_foundCount, std::vector<Points>& objs) const {
  if (a_node->isInternal()) {
	for (int index = 0; index < a_node->m_count; ++index) {
	  if (overlapSpecial(a_rect, &a_node->m_branch[index].m_rect)) {
		if (!search(a_node->m_branch[index].m_child, a_rect, a_foundCount, objs)) {
		  return false;
		}
	  }
	}
  } else {
	for (int index = 0; index < a_node->m_count; ++index) {
	  if (overlap(a_rect, &a_node->m_branch[index].m_rect)) {
		Points& id = a_node->m_branch[index].m_data;
		++a_foundCount;
		objs.push_back(id);
	  }
	}
  }

  return true;
}


bool RTree::getMBRs(std::vector<std::vector<Points>>& mbrs_n) {
  std::vector<Branch> v, w;
  mbrs_n.clear();
  std::vector<Points> mbrs;

  int n = (m_root->m_level);

  for (int i = 0; i < (m_root->m_count); i++) {
	v.push_back(m_root->m_branch[i]);
  }

  for (unsigned int i = 0; i < v.size(); i++) {
	Points q;
	Point p;

	p.first = v[i].m_rect.m_min[0];
	p.second = v[i].m_rect.m_min[1];
	q.push_back(p);
	p.first = v[i].m_rect.m_max[0];
	p.second = v[i].m_rect.m_max[1];
	q.push_back(p);
	mbrs.push_back(q);
  }
  mbrs_n.push_back(mbrs);
  mbrs.clear();

  while (n--) {
	for (unsigned int i = 0; i < v.size(); i++) {
	  for (int j = 0; j < (v[i].m_child->m_count); j++) {
		w.push_back(v[i].m_child->m_branch[j]);
	  }
	}

	v = w;
	for (unsigned int i = 0; i < v.size(); i++) {
	  Points q;
	  Point p;

	  p.first = v[i].m_rect.m_min[0];
	  p.second = v[i].m_rect.m_min[1];
	  q.push_back(p);
	  p.first = v[i].m_rect.m_max[0];
	  p.second = v[i].m_rect.m_max[1];
	  q.push_back(p);
	  mbrs.push_back(q);
	}

	mbrs_n.push_back(mbrs);
	w.clear();
	mbrs.clear();
  }
  return true;
}


double RTree::minMaxDist(Point point, Rect a_rect) {
  double rmk[2], rMi[2];

  if (point.first <= (a_rect.m_min[0] + a_rect.m_max[0]) / 2.0) {
	rmk[0] = a_rect.m_min[0];
  } else {
	rmk[0] = a_rect.m_max[0];
  }

  if (point.second >= (a_rect.m_min[1] + a_rect.m_max[1]) / 2.0) {
	rMi[1] = a_rect.m_min[1];
  } else {
	rMi[1] = a_rect.m_max[1];
  }

  double alpha[2];

  alpha[0] = (point.first - rmk[0]) * (point.first - rmk[0])
			 + (point.second - rMi[1]) * (point.second - rMi[1]);

  if (point.second <= (a_rect.m_min[1] + a_rect.m_max[1]) / 2.0) {
	rmk[1] = a_rect.m_min[1];
  } else {
	rmk[1] = a_rect.m_max[1];
  }

  if (point.first >= (a_rect.m_min[0] + a_rect.m_max[0]) / 2.0) {
	rMi[0] = a_rect.m_min[0];
  } else {
	rMi[0] = a_rect.m_max[0];
  }

  alpha[1] = (point.second - rmk[1]) * (point.second - rmk[1])
			 + (point.first - rMi[0]) * (point.first - rMi[0]);

  if (alpha[0] <= alpha[1]) {
	return alpha[0];
  } else {
	return alpha[1];
  }
}

double RTree::minDist(Point point, Rect a_rect) {
  double x, y;

  if (point.first < a_rect.m_min[0]) {
	x = a_rect.m_min[0];
  } else if (point.first > a_rect.m_max[0]) {
	x = a_rect.m_max[0];
  } else {
	x = point.first;
  }

  if (point.second < a_rect.m_min[1]) {
	y = a_rect.m_min[1];
  } else if (point.second > a_rect.m_max[1]) {
	y = a_rect.m_max[1];
  } else {
	y = point.second;
  }

  return (x - point.first) * (x - point.first) + (y - point.second) * (y - point.second);
}

bool RTree::nearest(int k, Point point, std::vector<Points>& objs) {
  objs.clear();
  std::vector<std::pair<double, Points>> distObjs;

  for (auto& obj : mObjs) {
	distObjs.push_back(std::make_pair(pointToPoly(point, obj), obj));
  }

  std::sort(distObjs.begin(), distObjs.end());

  int count = 0;
  for (auto& obj : distObjs) {
	if (count > k - 1) break;
	objs.push_back(obj.second);
	count++;
  }

  return true;
}

bool RTree::inRadius(Point point, double distance, std::vector<Points>& objs) {
  objs.clear();
  std::vector<std::pair<double, Points>> distObjs;

  for (auto& obj : mObjs) {
	distObjs.push_back(std::make_pair(pointToPoly(point, obj), obj));
  }

  std::sort(distObjs.begin(), distObjs.end());

  for (auto& obj : distObjs) {
	if (obj.first <= distance) {
	  objs.push_back(obj.second);
	}
  }

  return true;
}

Rect RTree::mbr(Points points) {
  auto x1 = points[0].first;
  auto x2 = points[0].first;
  auto y1 = points[0].second;
  auto y2 = points[0].second;

  if (points.size() == 1) {
	x1 -= 5;
	x2 += 5;
	y1 -= 5;
	y2 += 5;
  } else {
	for (unsigned int i = 1; i < points.size(); i++) {
	  if (points[i].first < x1) {
		x1 = points[i].first;
	  }
	  if (x2 < points[i].first) {
		x2 = points[i].first;
	  }
	  if (points[i].second < y1) {
		y1 = points[i].second;
	  }
	  if (y2 < points[i].second) {
		y2 = points[i].second;
	  }
	}
  }

  return { x1, y1, x2, y2 };
}

bool RTree::onSegment(Point p, Point q, Point r) {
  if (q.first <= std::max(p.first, r.first) && q.first >= std::min(p.first, r.first) && q.second <= std::max(p.second, r.second) && q.second >= std::min(p.second, r.second))
	return true;
  return false;
}

// To find orientation of ordered triplet (p, q, r).
// 0 --> p, q, r colinear
// 1 --> CW
// 2 --> CCW
int RTree::orientation(Point p, Point q, Point r) {
  auto val = (q.second - p.second) * (r.first - q.first) - (q.first - p.first) * (r.second - q.second);

  if (val == 0) return 0;
  return (val > 0) ? 1 : 2;
}

bool RTree::doIntersect(Point p1, Point q1, Point p2, Point q2) {
  auto o1 = orientation(p1, q1, p2);
  auto o2 = orientation(p1, q1, q2);
  auto o3 = orientation(p2, q2, p1);
  auto o4 = orientation(p2, q2, q1);

  if (o1 != o2 && o3 != o4)
	return true;

  if (o1 == 0 && onSegment(p1, p2, q1)) return true;

  if (o2 == 0 && onSegment(p1, q2, q1)) return true;

  if (o3 == 0 && onSegment(p2, p1, q2)) return true;

  if (o4 == 0 && onSegment(p2, q1, q2)) return true;

  return false;
}

// Adapted from: https://www.geeksforgeeks.org/how-to-check-if-a-given-point-lies-inside-a-polygon/
bool RTree::isInside(Points& polygon, Point p) {
  auto n = polygon.size();
  if (n < 3) return false;

  Point farRight{ std::numeric_limits<double>::infinity(), p.second };

  std::size_t count = 0, i = 0;
  do {
	auto next = (i + 1) % n;

	if (doIntersect(polygon[i], polygon[next], p, farRight)) {
	  if (orientation(polygon[i], p, polygon[next]) == 0)
		return onSegment(polygon[i], p, polygon[next]);

	  count++;
	}
	i = next;
  } while (i != 0);

  return count & 1;
}

double RTree::pointToLine(const Point& p, const Point& l1, const Point& l2) {
  double xDelta = l2.first - l1.first;
  double yDelta = l2.second - l1.second;

  double u = ((p.first - l1.first) * xDelta + (p.second - l1.second) * yDelta) / (xDelta * xDelta + yDelta * yDelta);

  Point closestPointOnLine;

  if (u < 0) {
	closestPointOnLine = l1;
  } else if (u > 1) {
	closestPointOnLine = l2;
  } else {
	closestPointOnLine = { l1.first + u * xDelta, l1.second + u * yDelta };
  }


  Point d = { p.first - closestPointOnLine.first, p.second - closestPointOnLine.second };
  return sqrt(d.first * d.first + d.second * d.second);
}


double RTree::pointToPoly(const Point& p, Points& poly) {

  if (isInside(poly, p)) {
	return 0;
  }

  if (poly.size() == 1) {
	return sqrt((p.first - poly[0].first) * (p.first - poly[0].first) + (p.second - poly[0].second) * (p.second - poly[0].second));
  }

  double result = std::numeric_limits<double>::infinity();

  for (int i = 0; i < poly.size(); i++) {
	auto previousIndex = i - 1;
	if (previousIndex < 0) {
	  previousIndex = poly.size() - 1;
	}

	Point currentPoint = poly.at(i);
	Point previousPoint = poly.at(previousIndex);

	double segmentDistance = pointToLine({ p.first, p.second }, previousPoint, currentPoint);

	if (segmentDistance < result) {
	  result = segmentDistance;
	}
  }

  return result;
}
