cmake_minimum_required(VERSION 3.23)

project(transport_catalogue CXX)
set(CMAKE_CXX_STANDARD 17)

find_package(Protobuf REQUIRED)
find_package(Threads REQUIRED)

set(PROTO_DIR proto)
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS 
		      ${PROTO_DIR}/svg.proto 
		      ${PROTO_DIR}/map_renderer.proto 
		      ${PROTO_DIR}/transport_catalogue.proto 
		      ${PROTO_DIR}/graph.proto 
		      ${PROTO_DIR}/transport_router.proto)

set(SRC_DIR src)
file(GLOB TRANSPORTCATALOGUE_FILES "${SRC_DIR}/*.h" "${SRC_DIR}/*.cpp")


add_executable(transport_catalogue ${PROTO_SRCS} ${PROTO_HDRS} ${TRANSPORTCATALOGUE_FILES})
target_include_directories(transport_catalogue PUBLIC ${Protobuf_INCLUDE_DIRS})
target_include_directories(transport_catalogue PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

string(REPLACE "protobuf.lib" "protobufd.lib" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")
string(REPLACE "protobuf.a" "protobufd.a" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")

target_link_libraries(transport_catalogue "$<IF:$<CONFIG:Debug>,${Protobuf_LIBRARY_DEBUG},${Protobuf_LIBRARY}>" Threads::Threads)
