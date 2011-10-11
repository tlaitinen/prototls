project(libprototls)
cmake_minimum_required(VERSION 2.8)

set(CMAKE_BUILD_TYPE_SHARED_LIBS ON)
set(ROOT ${CMAKE_SOURCE_DIR})
set(CMAKE_INSTALL_PREFIX ${ROOT}/dist)
include_directories(${ROOT}/threadpool
                    ${ROOT}/include)
file(GLOB srcs src/*.cpp)
add_library(prototls ${srcs})
target_link_libraries(prototls gnutls protobuf pthread boost_thread)
install(TARGETS prototls DESTINATION 
        ${CMAKE_INSTALL_PREFIX}/lib)

install(FILES ${ROOT}/include/prototls.hpp 
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include)
file(GLOB headers include/prototls/*.hpp)
foreach(header ${headers})
    install(FILES ${header} 
            DESTINATION ${CMAKE_INSTALL_PREFIX}/include/prototls)
endforeach(header)


add_custom_command(OUTPUT ${ROOT}/example/my_protocol.pb.cc
                          ${ROOT}/example/my_protocol.pb.h
                   COMMAND
                          protoc ${ROOT}/example/my_protocol.proto
                                 --proto_path=${ROOT}/example
                                 --cpp_out=${ROOT}/example)
add_library(my_protocol example/my_protocol.pb.cc)
add_executable(testserver example/server.cpp)
add_executable(testclient example/client.cpp)
target_link_libraries(testserver my_protocol prototls)
target_link_libraries(testclient my_protocol prototls)

