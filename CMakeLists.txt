cmake_minimum_required(VERSION 3.10)
project(DreamboatRPC)

find_package(Protobuf REQUIRED)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall")

set(OBJDIR obj)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/build)
include_directories(${PROTOBUF_INCLUDE_DIRS})

# Find all .cc files in the common directory
file(GLOB_RECURSE COMMON_SRCS ${CMAKE_SOURCE_DIR}/source/common/*.cc)

# Find all .cc files in the serialization directory
file(GLOB_RECURSE SERIALIZATION_SRCS ${CMAKE_SOURCE_DIR}/source/serialization/*.cc)

# Find all .cc files in the runtime directory
file(GLOB_RECURSE RUNTIME_SRCS ${CMAKE_SOURCE_DIR}/source/runtime/*.cc)

# Find all .cc files in the transport directory
file(GLOB_RECURSE TRANSPORT_SRCS ${CMAKE_SOURCE_DIR}/source/transport/*.cc)

# Find all .cc files in the client directory
file(GLOB_RECURSE CLIENT_SRCS ${CMAKE_SOURCE_DIR}/source/client/*.cc)

# Find all .cc files in the server directory
file(GLOB_RECURSE SERVER_SRCS ${CMAKE_SOURCE_DIR}/source/server/*.cc)

# Find all .proto files
file(GLOB_RECURSE PROTO_FILES ${CMAKE_SOURCE_DIR}/source/example/proto/*.proto)

# Generate C++ source and header files from .proto
EXEC_PROGRAM(protoc ARGS --cpp_out=${CMAKE_CURRENT_BINARY_DIR} ${PROTO_FILES} --proto_path=${CMAKE_SOURCE_DIR}/source/example/proto)

# Find all .proto generated .cc files
file(GLOB_RECURSE PROTO_SRCS ${CMAKE_CURRENT_BINARY_DIR}/*.pb.cc)

# Define targets for server and client
add_executable(Server ${CMAKE_SOURCE_DIR}/source/example/Server.cc
    ${CMAKE_SOURCE_DIR}/source/example/NameServer.cc ${CMAKE_SOURCE_DIR}/source/example/NameService.cc
    ${COMMON_SRCS} ${SERIALIZATION_SRCS} ${RUNTIME_SRCS} ${TRANSPORT_SRCS} ${SERVER_SRCS} ${PROTO_SRCS})

add_executable(Client ${CMAKE_SOURCE_DIR}/source/example/Client.cc
    ${CMAKE_SOURCE_DIR}/source/example/NameClient.cc ${CMAKE_SOURCE_DIR}/source/example/NameService.cc
    ${COMMON_SRCS} ${SERIALIZATION_SRCS} ${RUNTIME_SRCS} ${TRANSPORT_SRCS} ${CLIENT_SRCS} ${PROTO_SRCS})

# Declare macros for server and client
target_compile_definitions(Server PRIVATE BUILD_SERVER)
target_compile_definitions(Client PRIVATE BUILD_CLIENT)

# Link with necessary libraries
target_link_libraries(Server protobuf pthread)
target_link_libraries(Client protobuf pthread)

# Optionally specify build type (Debug, Release)
set(CMAKE_BUILD_TYPE Debug)
