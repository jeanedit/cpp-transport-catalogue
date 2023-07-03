#pragma once
#include <string_view>
#include "transport_catalogue.h"

namespace tr_catalogue {
	namespace stat_handler {
		std::string_view BusStatsQuery(std::string_view bus_stats);


		void ParseStatsQuery(const TransportCatalogue& ts);
	}
}

