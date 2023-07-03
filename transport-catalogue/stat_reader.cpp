#include "stat_reader.h"
#include "transport_catalogue.h"

#include <string_view>
#include <string>
#include <iostream>

namespace tr_catalogue {
	namespace stat_handler {
		using namespace std::string_view_literals;
		/* Query Types */
		const std::string_view bus_sv= "Bus"sv; 
		const std::string_view stop_sv = "Stop"sv;

		std::string_view BusStatsQuery(std::string_view bus_query) {
			bus_query.remove_prefix(bus_sv.size() + 1);
			return bus_query;
		}

		std::string_view StopStatsQuery(std::string_view stop_query) {
			stop_query.remove_prefix(stop_sv.size() + 1);
			return stop_query;
		}

		void ParseStatsQuery(const TransportCatalogue& ts) {
			size_t stats_count;
			std::cin >> stats_count;
			std::cin.ignore();
			std::string stats_query;
			for (size_t i = 0; i < stats_count; ++i) {
				std::getline(std::cin, stats_query);
				if (stats_query.find("Bus") != stats_query.npos)
				{
					std::string_view bus_name = BusStatsQuery(stats_query);
					BusStats bus_stats = ts.GetBusStats(bus_name);
					if (bus_stats.bus_name.empty()) {
						std::cout << "Bus " << bus_name << ": not found" << std::endl;
					}
					else {
						std::cout << bus_stats << std::endl;
					}
				}
				else {
					std::string_view stop_name = StopStatsQuery(stats_query);
					auto buses = ts.StopAllBuses(stop_name);
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
			}
		}
	}
}
