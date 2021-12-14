#include <iostream>
#include <chrono>
#include <iomanip>

#include "loader.h"
#include "rtree.h"

int main() {
  std::cout << std::setprecision(9);

  RTree rt;

  DataLoader::loadTrips(rt);

  std::vector<Points> objects;

  auto start = std::chrono::high_resolution_clock::now();
  Point query_min = { -73.84859700000018, 40.871670000000115 };
  Point query_max = { -73.819056396484375, 42.105252075195312 };
  auto num = rt.search(query_min, query_max, objects);
  auto end = std::chrono::high_resolution_clock::now();
  std::cout << "Found " << num << " trips starting inside the region\n";
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
  std::cout << duration.count() << '\n';
}
