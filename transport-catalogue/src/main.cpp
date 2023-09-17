#include <iostream>


//
//int main() {
//    tr_catalogue::TransportCatalogue transport_catalogue;
//    tr_catalogue::json_reader::JsonReader jr(transport_catalogue,std::cin);
//    jr.ReadTransportDataBase();
//    renderer::MapRenderer map_renderer = jr.MapRenderFromJson();
//    tr_catalogue::router::TransportRouter transport_router = jr.TransportRouterFromJson();
//    tr_catalogue::RequestHandler request_handler(transport_catalogue, transport_router,map_renderer);
//    jr.ResponseRequests(std::cout, request_handler);
//}


#include "json_reader.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"
#include "serialization.h"

#include <fstream>
#include <iostream>
#include <string_view>

using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
	stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
	if (argc != 2) {
		PrintUsage();
		return 1;
	}

	const std::string_view mode(argv[1]);

	tr_catalogue::json_reader::JsonReader jr(std::cin);
	if (mode == "make_base"sv) {
		tr_catalogue::TransportCatalogue transport_catalogue = jr.TransportCatalogueFromJson();
		renderer::MapRenderer map_renderer = jr.MapRenderFromJson(transport_catalogue);
		tr_catalogue::router::TransportRouter transport_router = jr.TransportRouterFromJson(transport_catalogue);
		std::ofstream ofs(jr.ParseSerializationSettings().file,std::ios::binary);
		tr_catalogue::SerializeTransportCatalogue(ofs, transport_catalogue);
		tr_catalogue::SerializeMapRenderer(ofs, jr.ParseRenderSettings(),transport_catalogue.GetAllBuses());
		tr_catalogue::SerializeTransportRouter(ofs, transport_router);
	}
	else if (mode == "process_requests"sv) {
		auto transport_catalogue = tr_catalogue::DeserializeTransportCatalogue(jr.ParseSerializationSettings());
		auto map_renderer = tr_catalogue::DeserializeMapRenderer(jr.ParseSerializationSettings());	
		auto transport_router = tr_catalogue::DeserializeTransportRouter(jr.ParseSerializationSettings(), *transport_catalogue);
		tr_catalogue::RequestHandler request_handler(*transport_catalogue, *transport_router,*map_renderer);
		jr.ResponseRequests(std::cout, request_handler);
	}
	else {
		PrintUsage();
		return 1;
	}
}