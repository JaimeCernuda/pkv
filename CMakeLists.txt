cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)
include_directories(include)
include_directories(common/include)

include_directories(${CMAKE_SOURCE_DIR}/common/include)
set(LIBS "-L${CMAKE_BINARY_DIR}/journal -L${CMAKE_BINARY_DIR}/common -L${CMAKE_BINARY_DIR}/pkv -L${CMAKE_BINARY_DIR}/pwriter")
set(INCLUDES "-I${CMAKE_SOURCE_DIR}/journal/include -I${CMAKE_SOURCE_DIR}/common/include -I${CMAKE_SOURCE_DIR}/pkv/include -I${CMAKE_SOURCE_DIR}/pwriter/include")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${LIBS} ${INCLUDES} -g -w")

set(PKV_COMMON common/include/pkv/configuration_manager.h common/include/pkv/daemonize.h common/include/pkv/data_structure.h)
set(PKV_LIB_SRC include/pkv/common/common.h include/pkv/client/client.h src/client/client.cpp  )
set(PKV_SERVER_SRC include/pkv/common/common.h include/pkv/server/server.h src/server/main.cpp)
set(PKV_TEST_SRC test/test.cpp)

add_library(pkv SHARED ${PKV_LIB_SRC} ${PKV_COMMON})
target_link_libraries(pkv -lbasket -lmpi -lpthread -lrt -lrpc -lboost_filesystem)

add_executable(pkv_server ${PKV_SERVER_SRC} ${PKV_COMMON})
target_link_libraries(pkv_server -lbasket -lmpi -lpthread -lrt -lrpc -lboost_filesystem)

add_executable(pkv_test ${PKV_TEST_SRC} ${PKV_COMMON})
add_dependencies(pkv_test pkv)
target_link_libraries(pkv_test -L${CMAKE_BINARY_DIR}/pkv pkv -lbasket -lmpi -lpthread -lrt -lrpc -lboost_filesystem)
