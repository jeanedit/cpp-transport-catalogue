#include "stat_reader.h"
#include "transport_catalogue.h"

#include <string_view>
#include <string>
#include <iostream>
#include <iomanip>
using namespace domain;

namespace tr_catalogue {
	namespace stat_handler {
		std::string_view BusStatsQuery(std::string_view bus_query) {
			bus_query.remove_prefix(bus_sv.size() + 1);
			return bus_query;
		}

		std::string_view StopStatsQuery(std::string_view stop_query) {
			stop_query.remove_prefix(stop_sv.size() + 1);
			return stop_query;
		}

		void PrintBusStats(std::string_view bus_name,const BusStats& bus_stats) {
			if (bus_stats.bus_name.empty()) {
				std::cout << "Bus " << bus_name << ": not found" << std::endl;
			}
			else {
				std::cout << bus_stats << std::endl;
			}
		}

		void PrintStopStats(std::string_view stop_name, const std::set<std::string_view>* buses) {
			if (buses == nullptr) {
				std::cout << "Stop " << stop_name << ": not found" << std::endl;
			}
			else if (buses->empty()) {
				std::cout << "Stop " << stop_name << ": no buses" << std::endl;
			}
			else {
				std::cout << "Stop " << stop_name << ": buses";
				for (const auto& bus : *buses) {
					std::cout << ' ';
					std::cout << bus;
				}
				std::cout << std::endl;
			}
		}

		void StatsReader(const TransportCatalogue& ts,std::istream& is) {
			size_t stats_count;
			is >> stats_count;
			is.ignore();
			std::string stats_query;
			for (size_t i = 0; i < stats_count; ++i) {
				std::getline(is, stats_query);
				if (stats_query.find("Bus") != stats_query.npos)
				{
					std::string_view bus_name = BusStatsQuery(stats_query);
					BusStats bus_stats = ts.GetBusStats(bus_name);
					PrintBusStats(bus_name,bus_stats);
				}
				else {
					std::string_view stop_name = StopStatsQuery(stats_query);
					auto buses = ts.StopAllBuses(stop_name);
					PrintStopStats(stop_name,buses);
				}
			}
		}

		std::ostream& operator << (std::ostream& os, const BusStats& bus_stats) {
			os << "Bus " << bus_stats.bus_name << ": "
				<< bus_stats.stops << " stops on route, "
				<< bus_stats.unique_stops << " unique stops, "
				<< bus_stats.route_length << " route length, "
				<< std::setprecision(6) << bus_stats.curvature << " curvature";
			return os;
		}
	}
}
