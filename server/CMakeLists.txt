project(server)

set(SOURCES
    src/main.cpp
    src/server.cpp
    src/people.cpp
    src/usermanager.cpp
    src/roommanager.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
        ${netutils_SOURCE_DIR}/include
        ${cproto_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
       netutils
       cproto
)