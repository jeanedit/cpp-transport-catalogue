#include <iostream>
#include "input_reader.h"
#include "stat_reader.h"
#include "transport_catalogue.h"


int main() {
	tr_catalogue::TransportCatalogue transport_catalogue;
	tr_catalogue::input_handler::InputReader(transport_catalogue,std::cin);
	tr_catalogue::stat_handler::StatsReader(transport_catalogue,std::cin);
}