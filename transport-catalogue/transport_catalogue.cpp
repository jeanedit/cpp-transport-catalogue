#include "transport_catalogue.h"
#include <iostream>
#include <string_view>
#include <vector>
#include <numeric>
#include <functional>
#include <iomanip>
#include <algorithm>
using namespace domain;
namespace tr_catalogue {


	void TransportCatalogue::AddStop(Stop&& stop) {
		stops_storage_.push_back(std::move(stop));
		stops_[stops_storage_.back().name] = &stops_storage_.back();
		stops_to_buses_[stops_storage_.back().name];
	}

	const Stop* TransportCatalogue::FindStop(std::string_view stop_name) const {
		if (stops_.count(stop_name)) {
			return stops_.at(stop_name);
		}
		return nullptr;
	}

	void TransportCatalogue::AddBus(Bus&& bus) {
		buses_storage_.push_back(std::move(bus));
		buses_[buses_storage_.back().name] = &buses_storage_.back();
		for (const auto& stop : buses_storage_.back().route) {
			stops_to_buses_.at(stop->name).insert(buses_storage_.back().name);
		}
	}

	const Bus* TransportCatalogue::FindBus(std::string_view bus_name) const {
		if (buses_.count(bus_name)) {
			return buses_.at(bus_name);
		}
		return nullptr;
	}

	double TransportCatalogue::ComputeGeoRouteLength(const Bus* bus) const {

		double length = std::transform_reduce(bus->route.begin(), std::prev(bus->route.end()), std::next(bus->route.begin()), 0.0, std::plus{},
			[](const Stop* from, const Stop* to) {
				return geo::ComputeDistance(from->geo_coords, to->geo_coords);
			});

		return bus->is_loop ? length : length * 2;
	}
    
    const std::unordered_map<std::string_view, std::set<std::string_view>>& TransportCatalogue::StopToBuses() const{
        return stops_to_buses_;
    }

     const std::deque<const Stop*> TransportCatalogue::StopsWithBus() const{
         std::deque<const Stop*> stops_with_buses;
         for(const auto&[stop_name,bus_set]:stops_to_buses_){
             if(!bus_set.empty()){
                 stops_with_buses.push_back(FindStop(stop_name));
             }
         }
        std::sort(stops_with_buses.begin(),stops_with_buses.end(),[](const domain::Stop* lhs, const domain::Stop* rhs){
                return lhs->name < rhs->name;
            });
         return stops_with_buses;
     }
    
	size_t TransportCatalogue::ComputeActualRouteLength(const Bus* bus) const {
		size_t length = std::transform_reduce(bus->route.begin(), std::prev(bus->route.end()), std::next(bus->route.begin()), 0, std::plus{},
			[this](const Stop* from, const Stop* to) {
				auto stop_pair = std::make_pair(from, to);
				if (stop_pairs_distances_.count(stop_pair)) {
					return stop_pairs_distances_.at(stop_pair);
				}
				std::swap(stop_pair.first, stop_pair.second);
				return stop_pairs_distances_.at(stop_pair);
			});
		if (bus->is_loop) {
			return length;
		}
		length += std::transform_reduce(bus->route.rbegin(), std::prev(bus->route.rend()), std::next(bus->route.rbegin()), 0, std::plus{},
			[this](const Stop* from, const Stop* to) {
				auto stop_pair = std::make_pair(from, to);
				if (stop_pairs_distances_.count(stop_pair)) {
					return stop_pairs_distances_.at(stop_pair);
				}
				std::swap(stop_pair.first, stop_pair.second);
				return stop_pairs_distances_.at(stop_pair);
			});

		return length;
	}


	void TransportCatalogue::AddStopPairsDistances(const Stop* from,const Stop* to, const int distance) {
		stop_pairs_distances_[std::make_pair(from,to)] = distance;
	}
}