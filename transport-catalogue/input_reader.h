#pragma once
#include <string_view>
#include <istream>

#include "transport_catalogue.h"

namespace tr_catalogue {
	namespace input_handler {

		Stop ParseStopQuery(std::string_view stop_info);

		void ParseStopQueryDistance(TransportCatalogue& transport_catalogue, std::string_view stop_info);


		Bus ParseBusQuery(const TransportCatalogue& transport_catalogue, std::string_view route_info);

		void InputReader(TransportCatalogue& transport_catalogue,std::istream&is);
	}
}