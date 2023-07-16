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
#include "domain.h"


namespace tr_catalogue {


	class TransportCatalogue {
	public:
		using StopPair = std::pair<const domain::Stop*, const domain::Stop*>;
		TransportCatalogue() = default;

		// r-value reference òàê êàê íàäî ïåðåíåñòè òÿæåëûé îáúåêò, êîòîðûé îáðàçîâàëñÿ ïðè ñ÷èòûâàíèè äàííûõ
		// Ëèáî íóæíî îñòàâëÿòü const Stop& stop, íî òîãäà áóäåò ïðîèñõîäèòü êîïèðîâàíèå îáúåêòà, êîòîðûé íå íóæåí âíå ñïðàâî÷íèêà
		void AddStop(domain::Stop&& stop);
		const domain::Stop* FindStop(std::string_view stop_name) const;

		void AddBus(domain::Bus&& bus);
		const domain::Bus* FindBus(std::string_view bus_name) const;
        const std::deque<domain::Bus>& GetAllBuses() const{
            return buses_storage_;
        }
        
        const std::deque<domain::Stop>& GetAllStops() const{
            return stops_storage_;
        }
        
		void AddStopPairsDistances(const domain::Stop* from,const domain::Stop* to, const int distance);

		double ComputeGeoRouteLength(const domain::Bus* bus) const;

		size_t ComputeActualRouteLength(const domain::Bus* bus) const;
        
        const std::unordered_map<std::string_view, std::set<std::string_view>>& StopToBuses() const;
        
        const std::deque<const domain::Stop*> StopsWithBus() const;

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
		std::deque<domain::Stop> stops_storage_;
		std::deque<domain::Bus> buses_storage_;

		std::unordered_map<std::string_view, const domain::Stop*> stops_;
		std::unordered_map<std::string_view, const domain::Bus*> buses_;

		std::unordered_map<std::string_view, std::set<std::string_view>> stops_to_buses_;
		std::unordered_map<StopPair, size_t, StopPairHasher> stop_pairs_distances_;
	};
}