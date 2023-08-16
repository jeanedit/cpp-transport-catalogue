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
	using StopPair = std::pair<const domain::Stop*, const domain::Stop*>;

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

	struct RoutingSettings {
		double bus_velocity;
		double bus_wait_time;
	};

	struct TimeBusSpan {
		double time;
		size_t span_count;
		friend bool operator <(const TimeBusSpan& lhs,const TimeBusSpan& rhs) {
			return lhs.time < rhs.time;
		}
	};


	struct StopPairHasher {
		size_t operator()(const StopPair& stop_pair) const {
			size_t first_stop_name_hash = std::hash<std::string>{}(stop_pair.first->name) * prime_multiply(1);
			size_t second_stop_name_hash = std::hash<std::string>{}(stop_pair.second->name) * prime_multiply(2);

			size_t first_coords_hash = std::hash<double>{}(stop_pair.first->geo_coords.lat) * prime_multiply(3)
				+ std::hash<double>{}(stop_pair.first->geo_coords.lng)* prime_multiply(4);

			size_t second_coords_hash = std::hash<double>{}(stop_pair.second->geo_coords.lat) * prime_multiply(5)
				+ std::hash<double>{}(stop_pair.second->geo_coords.lng)* prime_multiply(6);

			return first_stop_name_hash + second_stop_name_hash + first_coords_hash + second_coords_hash;
		}
	private:
		inline size_t prime_multiply(size_t count) const {
			size_t result = 1;
			for (size_t i = 0; i < count; ++i) {
				result *= prime_number;
			}
			return result;
		}
		const size_t prime_number = 7u;
	};



	namespace detail {
		size_t GetUniqueStops(std::vector<const Stop*> stops);
	}
}