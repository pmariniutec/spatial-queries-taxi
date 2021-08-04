#include <catch2/catch_all.hpp>

#include "rtree.h"
#include "loader.h"

RTree rt;

TEST_CASE("Load Data") {
  DataLoader::loadNeighborhoods(rt);
  DataLoader::loadTrips(rt);
}

TEST_CASE("Query 3") {
  std::vector<Points> objects;

  Point query_min = { -73.84859700000018, 40.871670000000115 };
  Point query_max = { -73.819056396484375, 42.105252075195312 };
  auto num = rt.search(query_min, query_max, objects);
  std::cout << "Found " << num << " trips starting inside the region\n";
  REQUIRE(num == 116);
}

TEST_CASE("Query 5") {
  std::vector<Points> objects;

  Point point{ -73.819056396484375, 42.105252075195312 };
  double distance = 0.2;
  rt.inRadius(point, distance, objects);
  std::cout << "Found " << objects.size() << " trips within " << distance << " units\n";
  REQUIRE(objects.size() == 7);
}
