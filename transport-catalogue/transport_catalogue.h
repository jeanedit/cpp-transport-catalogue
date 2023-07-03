#pragma once
#include <string>
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <ostream>
#include <set>
#include <vector>
#include <functional>
#include "geo.h"


namespace tr_catalogue {

	struct Stop {
		std::string name;
		geo::Coordinates geo_coords;
	};


	struct Bus {
		std::string name;
		bool is_loop;
		std::vector<const Stop*> route;
		std::unordered_set<const Stop*> unique_stops;
	};

	struct BusStats {
		std::string_view bus_name;
		size_t stops = 0u;
		size_t unique_stops = 0u;
		size_t route_length = 0u;
		double curvature = 0.0;
	};

	std::ostream& operator << (std::ostream& os, const BusStats& bus_stats);

	class TransportCatalogue {
	public:
		using StopPair = std::pair<const Stop*, const Stop*>;
		TransportCatalogue() = default;

		void AddStop(const Stop stop);
		const Stop* FindStop(std::string_view stop_name) const;

		void AddBus(const Bus bus);
		const Bus* FindBus(std::string_view bus_name) const;

		BusStats GetBusStats(std::string_view bus_name) const;

		const std::set<std::string_view>* StopAllBuses(std::string_view stop_name) const;

		void AddStopPairsDistances(StopPair stop_pair, const int distance);

		double ComputeGeoRouteLength(const Bus* bus) const;

		size_t ComputeActualRouteLength(const Bus* bus) const;


	private:
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
		std::deque<Stop> stops_storage_;
		std::deque<Bus> buses_storage_;

		std::unordered_map<std::string_view, const Stop*> stops_;
		std::unordered_map<std::string_view, const Bus*> buses_;

		std::unordered_map<std::string_view, std::set<std::string_view>> stops_to_buses_;
		std::unordered_map<StopPair, size_t, StopPairHasher> stop_pairs_distances_;
	};
}