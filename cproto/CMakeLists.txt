project("cproto")

add_library(${PROJECT_NAME}
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

