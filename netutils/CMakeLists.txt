project(netutils)

add_library(netutils STATIC
    src/netutils.cpp    
)

target_include_directories(netutils
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)