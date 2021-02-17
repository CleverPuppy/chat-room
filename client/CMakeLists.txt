project(client)

add_executable( ${PROJECT_NAME}
    src/client.cpp
    src/main.cpp
)

target_include_directories( ${PROJECT_NAME}
PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    ${netutils_SOURCE_DIR}/include
    ${server_SOURCE_DIR}/include
    ${cproto_SOURCE_DIR}/include
)

target_link_libraries( ${PROJECT_NAME}
PUBLIC
    cproto
    netutils
)