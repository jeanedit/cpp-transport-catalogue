#include "domain.h"
#include <algorithm>

/*
 * В этом файле вы можете разместить классы/структуры, которые являются частью предметной области
 * (domain) вашего приложения и не зависят от транспортного справочника. Например Автобусные
 * маршруты и Остановки.
 *
 * Их можно было бы разместить и в transport_catalogue.h, однако вынесение их в отдельный
 * заголовочный файл может оказаться полезным, когда дело дойдёт до визуализации карты маршрутов:
 * визуализатор карты (map_renderer) можно будет сделать независящим от транспортного справочника.
 *
 * Если структура вашего приложения не позволяет так сделать, просто оставьте этот файл пустым.
 *
 */
namespace domain{
	namespace detail {
		size_t GetUniqueStops(std::vector<const Stop*> stops) {
			std::sort(stops.begin(), stops.end(), [](const Stop* lhs, const Stop* rhs) { return lhs->name < rhs->name;});
			auto last = std::unique(stops.begin(), stops.end(), [](const Stop* lhs, const Stop* rhs) { return lhs->name == rhs->name; });
			return std::distance(stops.begin(),last);
		}
	}
}