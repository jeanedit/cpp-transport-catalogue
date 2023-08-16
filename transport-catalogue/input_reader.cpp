#include "input_reader.h"
#include "transport_catalogue.h"
#include "geo.h"
#include "stat_reader.h"

#include<iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <utility>
#include <string_view>
#include <deque>
using namespace domain;

namespace tr_catalogue {
	namespace input_handler {
		using namespace std::string_view_literals;
		Stop ParseStopQuery(std::string_view stop_info) {
			stop_info.remove_prefix(stop_info.find_first_of(' ') + 1);
			std::string stop_name = std::string(stop_info.substr(0, stop_info.find_first_of(':')));
			stop_info.remove_prefix(stop_name.size() + 2); //  remove Stop name, ':' and ' '
			std::string latitude = std::string(stop_info.substr(0, stop_info.find_first_of(',')));
			stop_info.remove_prefix(latitude.size() + 2); // remove Stop name, ',' and ' '
			std::string longitude = std::string(stop_info.substr(0, stop_info.find_first_of(',')));
			return { stop_name,geo::Coordinates{std::stod(latitude),std::stod(longitude)} };
		}

		void ParseStopQueryDistance(TransportCatalogue& transport_catalogue, std::string_view stop_info) {
			// Extract Stop name
			stop_info.remove_prefix(stop_info.find_first_of(' ') + 1);
			std::string_view stop_name = stop_info.substr(0, stop_info.find_first_of(':'));
			size_t found_comma = stop_info.find_first_of(',');
			// Remove Stop name and geo cooridnates to handle distances to other stops (if exists)
			stop_info.find_first_of(',', found_comma + 1) == stop_info.npos ? stop_info.remove_prefix(stop_info.size()) : stop_info.remove_prefix(stop_info.find_first_of(',', found_comma + 1) + 2);
			while (!stop_info.empty()) {
				std::string_view distance = stop_info.substr(0, stop_info.find_first_of('m'));
				size_t first_space = stop_info.find_first_of(' ');
				stop_info.remove_prefix(stop_info.find_first_of(' ', first_space + 1) + 1); // remove before stop name
				found_comma = stop_info.find_first_of(',');
				std::string_view stop_to = stop_info.substr(0, found_comma);
				found_comma == stop_info.npos ? stop_info.remove_prefix(stop_info.size()) : stop_info.remove_prefix(found_comma + 2); // remove before next distance

				transport_catalogue.AddStopPairsDistances(transport_catalogue.FindStop(stop_name), transport_catalogue.FindStop(stop_to), stoi(std::string(distance)));
			}
		}




		Bus ParseBusQuery(const TransportCatalogue& transport_catalogue, std::string_view route_info) {
			Bus bus;
			bus.is_loop = (route_info.find_first_of('>') != route_info.npos);
			const char route_separator = bus.is_loop ? '>' : '-';
			route_info.remove_prefix(route_info.find_first_of(' ') + 1);
			std::string_view bus_name = route_info.substr(0, route_info.find_first_of(':'));
			bus.name = bus_name;
			route_info.remove_prefix(bus_name.size() + 2);
			while (!route_info.empty()) {
				std::string_view stop = route_info.substr(0, route_info.find_first_of(route_separator) - 1);
				bus.route.push_back(transport_catalogue.FindStop(stop));
				route_info.find_first_of(route_separator) == route_info.npos ? route_info.remove_prefix(stop.size()) : route_info.remove_prefix(stop.size() + 3);
			}
			return bus;
		}


		void InputReader(TransportCatalogue& transport_catalogue,std::istream& is) {
			size_t db_data_count; // number of database data
			is >> db_data_count;
			is.ignore();

			std::string dataline;
			std::vector<std::string> queries;
			queries.reserve(db_data_count);
			for (size_t i = 0; i < db_data_count; ++i) {
				std::getline(std::cin, dataline);
				if (dataline.substr(0, 4) == "Stop"sv) {

					transport_catalogue.AddStop(ParseStopQuery(dataline));
				}
				queries.push_back(std::move(dataline));

			}

			for (const std::string_view query : queries) {
				if (query.substr(0, 4) == "Stop"sv) {
					ParseStopQueryDistance(transport_catalogue, query);
				}
				else {
					transport_catalogue.AddBus(ParseBusQuery(transport_catalogue, query));
				}

			}
		}
	}
}