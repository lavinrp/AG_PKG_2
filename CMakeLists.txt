cmake_minimum_required(VERSION 3.0.0)
set (CMAKE_CXX_STANDARD 17)
project(AG_PKG_2 VERSION 0.1.0)

add_executable(AG_PKG_2 main.cpp)

# dependencies
add_subdirectory("nana")
find_package (Threads)
find_package(Poco REQUIRED COMPONENTS Foundation Net Util Zip NetSSL)
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost REQUIRED COMPONENTS Filesystem)
find_package(nlohmann_json 3.2.0 REQUIRED)

# link libraries
target_link_libraries(AG_PKG_2 PRIVATE ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(AG_PKG_2 PRIVATE Poco::Net Poco::Foundation Poco::Util Poco::Zip Poco::NetSSL)
target_link_libraries(AG_PKG_2 PRIVATE nana)
if(Boost_FOUND)
	include_directories(${Boost_INCLUDE_DIRS})
	target_link_libraries(AG_PKG_2 PRIVATE ${Boost_LIBRARIES})
endif()
target_link_libraries(AG_PKG_2 PRIVATE nlohmann_json::nlohmann_json)