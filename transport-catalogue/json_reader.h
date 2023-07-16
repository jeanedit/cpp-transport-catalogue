#pragma once
#include <string_view>
#include <istream>


#include "transport_catalogue.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "json.h"



namespace tr_catalogue {
	namespace json_reader {
        
class JsonReader{
    public:
        JsonReader(TransportCatalogue& tr_catalogue,std::istream& is):transport_catalogue_(tr_catalogue),is_(is),json_document_(json::Load(is_)){}
    
        void ReadJson();
    
        const json::Document& GetJsonDocument() const;
    
        void ResponseRequests(std::ostream& os,const RequestHandler& rq) const;
    
        renderer::RenderSettings ParseRenderSettings() const;
        renderer::MapRenderer MapRenderFromJson() const;
    
    private:
        TransportCatalogue& transport_catalogue_;
        std::istream& is_;
        json::Document json_document_;
                                                                                                   
    json::Dict StopResponse(int requst_id,const std::set<std::string_view>* buses) const;
        json::Dict BusResponse(int request_id,const std::optional<domain::BusStats>& bs) const;
       json::Dict MapResponse(int request_id,const svg::Document& render_doc) const;
    
        domain::Stop ParseStopQuery(const json::Node& type_stop) const;

		void ParseStopQueryDistance(const json::Node& type_stop);

		domain::Bus ParseBusQuery(const json::Node& type_bus) const;
    
        svg::Color ParseColor(const json::Node& node) const;
    	
};

}
}
