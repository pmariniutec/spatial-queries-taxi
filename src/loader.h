#ifndef LOADER_H
#define LOADER_H

#include <vector>
#include <fstream>

#include "rtree.h"
#include "csvUtils.h"

class DataLoader {
 public:
  static void loadTrips(RTree& rt) {
	std::fstream in{ "res/green_tripdata_2015-01.csv" };

	bool first = true;
	for (auto& row : CSVRange(in)) {

	  if (first) {
		first = false;
		continue;
	  }

	  auto puLon = std::stod(std::string(row[5])),
		   puLat = std::stod(std::string(row[6])),
		   doLon = std::stod(std::string(row[7])),
		   doLat = std::stod(std::string(row[8]));


	  Points points;
	  points.emplace_back(puLon, puLat, PointType::PICKUP);
	  points.emplace_back(doLon, doLat, PointType::DROPOFF);

	  auto rect = rt.mbr(points);
	  rt.insert(rect.m_min, rect.m_max, points);
	}
  }
};


#endif
