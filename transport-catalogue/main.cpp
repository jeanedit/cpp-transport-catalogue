#include <iostream>
#include "json_reader.h"
#include "transport_catalogue.h"
#include "map_renderer.h"


int main() {
    tr_catalogue::TransportCatalogue transport_catalogue;
    tr_catalogue::json_reader::JsonReader jr(transport_catalogue,std::cin);
    jr.ReadJson();
    renderer::MapRenderer mr = jr.MapRenderFromJson();
    tr_catalogue::RequestHandler rq(transport_catalogue,mr);
    jr.ResponseRequests(std::cout,rq);
}
