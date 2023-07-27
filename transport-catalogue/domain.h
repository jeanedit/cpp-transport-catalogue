#pragma once
#include <string>
#include <string_view>
#include <vector>
#include <set>
#include "geo.h"


namespace domain{
	struct Stop {
		std::string name;
		geo::Coordinates geo_coords;
	};


	struct Bus {
		std::string name;
		bool is_loop;
		std::vector<const Stop*> route;
	};

    struct StopStats{
        size_t request_id;
        const std::set<std::string_view>* buses;
    };
    
    
	struct BusStats {
		std::string_view bus_name;
        size_t request_id = 0u;
		size_t stops = 0u;
		size_t unique_stops = 0u;
		size_t route_length = 0u;
		double curvature = 0.0;
	};

	namespace detail {
		size_t GetUniqueStops(std::vector<const Stop*> stops);
	}
}