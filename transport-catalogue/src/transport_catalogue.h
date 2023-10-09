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
		TransportCatalogue() = default;

		void AddStop(domain::Stop&& stop);
		const domain::Stop* FindStop(std::string_view stop_name) const;
		void AddBus(domain::Bus&& bus);
		const domain::Bus* FindBus(std::string_view bus_name) const;

        	inline const std::deque<domain::Bus>& GetAllBuses() const{
            	return buses_storage_;
        	}
        
        	inline const std::deque<domain::Stop>& GetAllStops() const{
            	return stops_storage_;
        	}
        
		void AddStopPairsDistances(const domain::Stop* from,const domain::Stop* to, int distance);
		double ComputeGeoRouteLength(const domain::Bus* bus) const;
		size_t ComputeActualRouteLength(const domain::Bus* bus) const;
        
        	const std::unordered_map<std::string_view, std::set<std::string_view>>& StopToBuses() const;
        
        	const std::deque<const domain::Stop*> StopsWithBus() const;

		size_t GetDistanceBetweenStops(const domain::Stop* from, const domain::Stop* to) const;

		inline const std::unordered_map<domain::StopPair, size_t, domain::StopPairHasher>& GetStopPairsDistances() const {
			return stop_pairs_distances_;
		}

		inline size_t GetIndexByStop(const domain::Stop* stop) const {
			return stops_to_ids.at(stop);
		}

		inline size_t GetIndexByBus(const domain::Bus* bus) const {
			return buses_to_ids.at(bus);
		}

	private:
		std::deque<domain::Stop> stops_storage_;
		std::deque<domain::Bus> buses_storage_;

		std::unordered_map<std::string_view, const domain::Stop*> stops_;
		std::unordered_map<std::string_view, const domain::Bus*> buses_;

		std::unordered_map<std::string_view, std::set<std::string_view>> stops_to_buses_;
		std::unordered_map<domain::StopPair, size_t, domain::StopPairHasher> stop_pairs_distances_;

		std::unordered_map<const domain::Stop*, size_t> stops_to_ids;
		std::unordered_map<const domain::Bus*, size_t> buses_to_ids;
	};
}
