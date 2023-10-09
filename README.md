# Transport Catalogue
Implementation of the transport catalogue that stores a database of bus stops and buses with their routes, supports SVG rendering, calculates optimal routes between stops, and provides information about stops, routes, SVG maps, and optimal routes upon request.
# Descripiton
The Transport Catalogue is a comprehensive digital repository designed to manage and provide access to essential information related to transportation services, particularly bus routes and stops. This catalogue serves as a centralized hub for storing, organizing, and retrieving data pertinent to the transport network. Key features and functionalities of the Transport Catalogue include:

1. Bus Stop Database: The catalogue maintains an extensive database of bus stops, each with detailed information such as location coordinates, stop names, and unique identifiers.

2. Bus Route Information: It stores comprehensive data about bus routes, including their starting and ending points, intermediate stops, schedules, and route maps.

3. SVG Rendering: The Transport Catalogue supports rendering of transport-related data in Scalable Vector Graphics (SVG) format. This feature enables the creation of dynamic, interactive maps displaying bus routes, stop   
   locations, and other relevant details.

4. Route Optimization: Users can leverage the catalogue to find the most efficient and optimal routes between two bus stops. The system takes into account factors such as distance and bus schedules to 
   provide users with the best route options.

5. JSON Interactions: The Transport Catalogue interacts with users through JSON files, enabling programmatic access and data retrieval. Users can access the catalogue's functionalities and query transport-related 
   information using JSON-based requests and responses.

6. SVG Map Generation: On request, the catalogue generates SVG maps that visually represent the transport network, highlighting bus routes, stops, and connections. These maps can be easily integrated into other 
   applications or used for informational purposes.

7. Customization and Extensibility: The Transport Catalogue is designed to be customizable and extensible, allowing to incorporate 
   additional features as required.

8. Optimal Route Planning: The catalogue's route optimization functionality helps users plan journeys by identifying the fastest and most convenient routes, reducing travel time and enhancing the overall passenger 
   experience.

# Использование
0. Установка всех необходимых компонентов.
1. Инициализация поискового сервера происходит с использованием стоп-слов.
2. Добавление документов в сервер.
3. Формирование очереди запросов.
4. Вывод результатов.
5. Тесты помогут более подробно разобраться с возможностями данного поискового сервера.
# Cистемные требования
1. C++17
2. Любой из следующих компиляторов: GCC,MSVC,CLANG.
# Планы по доработке:
1. Возможность добавления документов с помощью файла.
2. Графический интерфейс для удобной работы с сервером.
