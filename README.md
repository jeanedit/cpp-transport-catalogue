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

# Transport Catalogue User Guide

## Step 1: Build the Project**

1. Ensure that you have CMake and a C++17 compatible compiler installed on your system.

2. Clone or download the Transport Catalogue project from its source repository.

3. Navigate to the project directory and create a build directory (e.g., `build`) inside it.

4. Open a terminal or command prompt and navigate to the build directory.

5. Run the CMake command to generate the build files. Specify the protobuf directory using `-DProtobuf_DIR=<protobuf_directory>`. For example:

   ```
   cmake -DCMAKE_PREFIX_PATH=<protobuf_directory> ..
   ```

   Replace `<protobuf_directory>` with the actual path to the directory where protobuf is installed on your system.

6. Build the project using `make` (on Linux) or `cmake --build .` (on Windows).

## Step 2: Prepare JSON Data

1. Create a JSON file that contains your transportation data, following the expected JSON format specified by the Transport Catalogue. Your JSON file should include the following sections:

   - `serialization_settings`: Specify the file where the catalogue should serialize its data.

   - `routing_settings`: Define routing parameters such as bus wait time and bus velocity.

   - `render_settings`: Configure rendering settings, including the width, height, padding, and appearance of the rendered map.

   - `base_requests`: Add transportation data, including buses and stops, in the desired format.


## Step 3: Automatic Serialization and Deserialization with Protocol Buffers

1. The Transport Catalogue program relies on Protocol Buffers (protobuf) for automatic data serialization and deserialization.

2. During the build process, CMake should be configured with the location of the protobuf installation directory using `-DProtobuf_DIR=<protobuf_directory>`.

3. The "make_base" stage automatically reads the provided JSON file, serializes the data into binary protobuf format using Protobuf 3, and creates the Transport Catalogue database.
   
4. The "process_requests" stage processes incoming JSON requests and provides responses based on the data that has been automatically deserialized from the JSON file. The Transport Catalogue program handles this deserialization process for you.

## Step 4: Response to Requests

1. Configure the Transport Catalogue to handle various types of JSON-based requests, such as querying bus details, stop information, finding routes, and generating maps in SVG format.

2. Define the types of requests you want to support, and specify them in your JSON request file.

3. Process incoming JSON requests and provide responses based on the data that has been automatically deserialized from the JSON file. The Transport Catalogue program handles this deserialization process for you.

**Example Responses to Requests:**

# Cистемные требования
1. C++17
2. Любой из следующих компиляторов: GCC,MSVC,CLANG.
# Планы по доработке:
1. Возможность добавления документов с помощью файла.
2. Графический интерфейс для удобной работы с сервером.
