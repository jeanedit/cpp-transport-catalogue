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

## Step 1: Build the Project using CMake, protobuf 3, and Prepare JSON Data**

1. Ensure that you have CMake and a C++17 compatible compiler installed on your system.

2. Ensure that you have protobuf 3 installed on your system.

3. Clone or download the Transport Catalogue project from its source repository.

4. Navigate to the project directory and create a build directory (e.g., `build`) inside it.

5. Open a terminal or command prompt and navigate to the build directory.

6. Run the CMake command to generate the build files. Specify the protobuf directory using `-DProtobuf_DIR=<protobuf_directory>`. For example:

   ```
   cmake -DCMAKE_PREFIX_PATH=<protobuf_directory> ..
   ```

   Replace `<protobuf_directory>` with the actual path to the directory where protobuf is installed on your system.

7. Build the project using `make` (on Linux) or `cmake --build .` (on Windows).

## Step 2: Prepare JSON Data

1. Create JSON files that contain required data following the expected JSON format specified by the Transport Catalogue. Your JSON file should include the following sections:

   - `serialization_settings`: Specify the file where the catalogue should serialize its data.

   - `routing_settings`: Define routing parameters such as bus wait time and bus velocity.

   - `render_settings`: Configure rendering settings, including the width, height, padding, and appearance of the rendered map.

   - `base_requests`: Add transportation data, including buses and stops, in the desired format.
     
   - `stat_requests` : Include statistical requests for information about buses, stops, routes, and maps. This setting is used in the "process_requests" stage to retrieve statistical data.

2. You can find more example JSON files in the "json_example" folder within the Transport Catalogue project. These examples can help you understand the expected format and structure for your JSON data.

## Step 3: Make Base

1. The "make_base" step involves running the following command to automatically read the provided JSON file, serialize the data into binary protobuf format using Protobuf 3, and create the Transport Catalogue database:

   ```shell
   ./transport_catalogue -m make_base -c path/to/base.json
   ```
2. This step creates the transport directory database based on base_requests and serializes it to a file.
   
## Step 4: Process Requests

1. After the base is established, you can process statistical requests. These requests can include queries about buses, stops, routes, and maps. The Transport Catalogue responds to these requests using the pre-built database, providing valuable information and insights.

   ```shell
   ./transport_catalogue -m process_requests -c path/to/stat.json
   ```
2. This step deserializes the database from a file and uses it to respond to stat_requests.

*Responses to different types of requests, including bus details, stop information, route data, and rendered maps in SVG format, are automatically generated and provided by the Transport Catalogue program.*

# System requirements
1. CMake 3.18
2. Any of the following compilers: GCC, Clang or MSVC.
3. Protobuf 3
# Development plans:
1. Create a user interface for more convenient interaction with the program.
   
2. Render maps using more advanced technologies, such as OpenGL.
   
3. Support new formats in addition to JSON.

4. Add rendering in a more convenient format, not just in SVG.
