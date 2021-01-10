project("cproto")

add_library(${PROJECT_NAME} STATIC
    src/cproto.cpp
)


target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include/
)

link_directories(
    ${PROJECT_SOURCE_DIR}/lib
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        jsoncpp
)

set(PROJECT_TEST cprototest)

add_executable(${PROJECT_TEST}
    src/cprototest.cpp
    src/cproto.cpp
)

target_include_directories(${PROJECT_TEST}
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include/
)
target_link_libraries(${PROJECT_TEST}
    PRIVATE
        jsoncpp
)
