cmake_minimum_required(VERSION 3.10)
project(DreamboatRPC)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall")

set(OBJDIR obj)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Find all .cc files in the common directory
file(GLOB COMMON_SRCS ${CMAKE_SOURCE_DIR}/source/common/*.cc)

# Create a static library or add the objects directly to the targets
add_library(common_objs OBJECT ${COMMON_SRCS})

# Link with pthread (if needed)
target_link_libraries(common_objs pthread)

# Find all .cc files in the serialization directory
file(GLOB SERIALIZATION_SRCS ${CMAKE_SOURCE_DIR}/source/serialization/*.cc)

# Create a static library or add the objects directly to the targets
add_library(serialization_objs OBJECT ${SERIALIZATION_SRCS})

# Find all .cc files in the runtime directory
file(GLOB_RECURSE RUNTIME_SRCS ${CMAKE_SOURCE_DIR}/source/runtime/*.cc)

# Create a static library or add the objects directly to the targets
add_library(runtime_objs OBJECT ${RUNTIME_SRCS})

# Find all .cc files in the server directory
file(GLOB TRANSPORT_SRCS ${CMAKE_SOURCE_DIR}/source/server/*.cc)

# Create a static library or add the objects directly to the targets
add_library(transport_objs OBJECT ${TRANSPORT_SRCS})

# Find all .cc files in the client directory
file(GLOB CLIENT_SRCS ${CMAKE_SOURCE_DIR}/source/client/*.cc)

# Create a static library or add the objects directly to the targets
add_library(client_objs OBJECT ${CLIENT_SRCS})

# Find all .cc files in the server directory
file(GLOB SERVER_SRCS ${CMAKE_SOURCE_DIR}/source/server/*.cc)

# Create a static library or add the objects directly to the targets
add_library(server_objs OBJECT ${SERVER_SRCS})

# Define targets for server and client
add_executable(Server ${CMAKE_SOURCE_DIR}/source/example/NameService.cc ${CMAKE_SOURCE_DIR}/source/example/NameServer.cc
    ${COMMON_SRCS} ${SERIALIZATION_SRCS} ${RUNTIME_SRCS} ${TRANSPORT_SRCS})
add_executable(Client ${CMAKE_SOURCE_DIR}/source/example/NameService.cc ${CMAKE_SOURCE_DIR}/source/example/NameClient.cc
    ${COMMON_SRCS} ${SERIALIZATION_SRCS} ${RUNTIME_SRCS} ${TRANSPORT_SRCS})

# Link with necessary libraries
target_link_libraries(Server protobuf pthread)
target_link_libraries(Client protobuf pthread)

# Optionally specify build type (Debug, Release)
set(CMAKE_BUILD_TYPE Debug)
