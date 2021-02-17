project("cproto")

add_library(${PROJECT_NAME} STATIC
    src/cproto.cpp
    src/cprotomsg.cpp
    src/login.cpp
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        sub::jsoncpp_static
)

set(PROJECT_TEST cprototest)
add_executable(${PROJECT_TEST}
    src/cprototest.cpp
    src/cproto.cpp
)

target_include_directories(${PROJECT_TEST}
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)
target_link_libraries(${PROJECT_TEST}
    PUBLIC
    sub::jsoncpp_static
)