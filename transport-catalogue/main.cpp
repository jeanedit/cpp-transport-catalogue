#include <iostream>
#include "json_reader.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"


int main() {
    tr_catalogue::TransportCatalogue transport_catalogue;
    tr_catalogue::json_reader::JsonReader jr(transport_catalogue,std::cin);
    jr.ReadJson();
    renderer::MapRenderer map_renderer = jr.MapRenderFromJson();
    tr_catalogue::router::TransportRouter transport_router = jr.TransportRouterFromJson();
    tr_catalogue::RequestHandler request_handler(transport_catalogue, transport_router,map_renderer);
    jr.ResponseRequests(std::cout, request_handler);
}