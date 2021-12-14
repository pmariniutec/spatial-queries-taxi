#include <catch2/catch_all.hpp>

#include "rtree.h"
#include "loader.h"

#include <vector>

#include <iostream>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/box.hpp>

#include <boost/geometry/index/rtree.hpp>
#include <boost/foreach.hpp>

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;

typedef bg::model::point<double, 2, bg::cs::cartesian> point;
typedef bg::model::box<point> box;
typedef std::pair<box, unsigned> value;

RTree rt;
bgi::rtree<value, bgi::quadratic<16>> refrt;


TEST_CASE("Load Data") {
  DataLoader::loadTrips(rt);
}

TEST_CASE("Trips started inside rectangular region") {
  std::vector<Points> objects;

  Point query_min = { -73.84859700000018, 40.871670000000115 };
  Point query_max = { -73.819056396484375, 42.105252075195312 };
  auto num = rt.search(query_min, query_max, objects);
  std::cout << "Found " << num << " trips starting inside the region\n";

  /*
  box query_box(point(-73.84859700000018, 40.871670000000115), point(-73.819056396484375, 42.105252075195312));
  std::vector<value> result_s;
  refrt.query(bgi::intersects(query_box), std::back_inserter(result_s));
  std::cout << "spatial query result:" << std::endl;
  BOOST_FOREACH (value const& v, result_s)
	std::cout << bg::wkt<box>(v.first) << " - " << v.second << std::endl;
  */

  REQUIRE(num == 116);
}
