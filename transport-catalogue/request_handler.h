#pragma once
#include "transport_catalogue.h"
#include "map_renderer.h"
#include <optional>
#include <set>


namespace tr_catalogue {
class RequestHandler {
public:
    // MapRenderer понадобится в следующей части итогового проекта
    RequestHandler(const TransportCatalogue& db, const renderer::MapRenderer& renderer):db_(db),renderer_(renderer){}

    // Возвращает информацию о маршруте (запрос Bus)
    std::optional<domain::BusStats> GetBusStat(const std::string_view& bus_name) const;

    // Возвращает маршруты, проходящие через
    const std::set<std::string_view>* GetBusesByStop(const std::string_view& stop_name) const;

    // Этот метод будет нужен в следующей части итогового проекта
    svg::Document RenderMap() const;

private:
    // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
    const TransportCatalogue& db_;
    const renderer::MapRenderer& renderer_;
};
    }