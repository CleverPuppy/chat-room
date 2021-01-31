project("cproto")

add_library(${PROJECT_NAME} STATIC
    src/cproto.cpp
)

message(jsoncpp_include : ${jsoncpp_SOURCE_DIR}/include)
target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
        ${jsoncpp_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        jsoncpp
)

set(PROJECT_TEST cprototest)

add_executable(${PROJECT_TEST}
    src/cprototest.cpp
    src/cproto.cpp
)

target_include_directories(${PROJECT_TEST}
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
        ${jsoncpp_SOURCE_DIR}/include
)
target_link_libraries(${PROJECT_TEST}
    PUBLIC
        jsoncpp #have to use sharedlib ? #jsoncpp_static don't have Value::operator= implementation
)