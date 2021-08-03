#include <iostream>
#include <iomanip>

#include "loader.h"
#include "rtree.h"

int main() {
  std::cout << std::setprecision(9);

  RTree rt;

  DataLoader::loadNeighborhoods(rt);
  DataLoader::loadTrips(rt);

  std::vector<Points> objects;

  {
	// Query 1

  }

  {
	// Query 2

  }

  {
	// Query 3
	Point query_min = { -73.84859700000018, 40.871670000000115 };
	Point query_max = { -73.819056396484375, 42.105252075195312 };
	auto num = rt.search(query_min, query_max, objects);
	std::cout << "Found " << num << " trips starting inside the region\n";
  }

  {
	// Query 5
	Point point{ -73.819056396484375, 42.105252075195312 };
	double distance = 0.2;
	rt.inRadius(point, distance, objects);
	std::cout << "Found " << objects.size() << " trips within " << distance << " units\n";
  }
}
