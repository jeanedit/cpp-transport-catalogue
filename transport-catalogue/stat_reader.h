#pragma once
#include <string_view>
#include <istream>
#include <set>
#include "transport_catalogue.h"


namespace tr_catalogue {
	namespace stat_handler {
		using namespace std::string_view_literals;
		/* Query Types */
		const std::string_view bus_sv = "Bus"sv;
		const std::string_view stop_sv = "Stop"sv;

		std::string_view BusStatsQuery(std::string_view bus_stats);
		std::string_view StopStatsQuery(std::string_view stop_query);

		void PrintBusStats(std::string_view bus_name,const BusStats& bus_stats);

		void PrintStopStats(std::string_view bus_name,const std::set<std::string_view>* buses);

		void StatsReader(const TransportCatalogue& ts, std::istream& is);

		std::ostream& operator << (std::ostream& os, const BusStats& bus_stats);
	}
}

