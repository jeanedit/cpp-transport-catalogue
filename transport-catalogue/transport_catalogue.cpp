#include "transport_catalogue.h"
#include <iostream>
#include <string_view>
#include <vector>
#include <numeric>
#include <functional>
#include <iomanip>
#include <algorithm>

namespace tr_catalogue {

	namespace detail {
		size_t GetUniqueStops(std::vector<const Stop*> stops) {
			std::sort(stops.begin(), stops.end(), [](const Stop* lhs, const Stop* rhs) { return lhs->name < rhs->name;});
			auto last = std::unique(stops.begin(), stops.end(), [](const Stop* lhs, const Stop* rhs) { return lhs->name == rhs->name; });
			return std::distance(stops.begin(),last);
		}
	}

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




	BusStats TransportCatalogue::GetBusStats(std::string_view bus_name) const {
		BusStats bus_stats;
		const Bus* bus = FindBus(bus_name);
		if (bus == nullptr) {
			return bus_stats;
		}
		bus_stats.bus_name = bus->name;
		bus_stats.stops = bus->is_loop ? bus->route.size() : bus->route.size() * 2 - 1;
		bus_stats.unique_stops = detail::GetUniqueStops(bus->route);

		bus_stats.route_length = ComputeActualRouteLength(bus);
		bus_stats.curvature = static_cast<double>(ComputeActualRouteLength(bus)) / ComputeGeoRouteLength(bus);
		return bus_stats;
	}

	const std::set<std::string_view>* TransportCatalogue::StopAllBuses(std::string_view stop_name) const {
		if (stops_to_buses_.count(stop_name)) {
			return &stops_to_buses_.at(stop_name);
		}
		return nullptr;
	}

	void TransportCatalogue::AddStopPairsDistances(const Stop* from,const Stop* to, const int distance) {
		stop_pairs_distances_[std::make_pair(from,to)] = distance;
	}
}