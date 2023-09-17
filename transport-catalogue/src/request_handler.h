#pragma once
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"

#include <optional>
#include <set>


namespace tr_catalogue {
    class RequestHandler {
    public:
        RequestHandler(const TransportCatalogue& tc, const router::TransportRouter& tr, const renderer::MapRenderer& renderer)
            :tc_(tc),
            tr_(tr),
            renderer_(renderer)
        {
        }

        // returns information about bus
        std::optional<domain::BusStats> GetBusStat(std::string_view bus_name) const;
    
        // returns information about the buses that go through a stop
        const std::set<std::string_view>* GetBusesByStop(std::string_view stop_name) const;
    
        // returns an optimal route between two stops
        std::optional<router::OptimalRoute> GetOptimalRoute(std::string_view from, std::string_view to) const;
    
        svg::Document RenderMap() const;
    
    private:
        const TransportCatalogue& tc_;
        const router::TransportRouter& tr_;
        const renderer::MapRenderer& renderer_;
    
    };
} // end of namespace tr_catalogue