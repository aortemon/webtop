add_library(util INTERFACE)

target_include_directories(util 
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/util
)


add_library(logger STATIC
    logger/logger.cpp
)

target_include_directories(logger 
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} 
)

add_library(readers STATIC
    reader/parsers/cpu_parser.cpp
    reader/parsers/memory_parser.cpp
    reader/parsers/network_parser.cpp
    reader/parsers/disk_parser.cpp
    reader/parsers/process_parser.cpp
    reader/parsers/loadavg_parser.cpp
    reader/parsers/uptime_parser.cpp
    reader/parsers/dotenv_parser.cpp
    reader/file_reader.cpp
)

target_include_directories(readers 
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(readers 
    PUBLIC util
    PUBLIC logger
)

add_library(aggregator STATIC
    aggregator/analizers/cpu_analizer.cpp
    aggregator/analizers/proc_analizer.cpp
    aggregator/analizers/network_analizer.cpp
    aggregator/builders/disk_builder.cpp
    aggregator/builders/loadavg_builder.cpp
    aggregator/builders/memory_builder.cpp
    aggregator/builders/proc_builder.cpp
    aggregator/builders/uptime_builder.cpp
    aggregator/aggregator.cpp
)

target_include_directories(aggregator 
    PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(aggregator 
    PUBLIC readers
    PRIVATE logger
)

add_library(server STATIC
    server/handlers/metrics_handler.cpp
    server/handlers/process_handler.cpp
    server/handlers/sse_handler.cpp
    server/handlers/static_handler.cpp
    server/router.cpp
    server/http_server.cpp
)

target_include_directories(server 
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(server 
    PUBLIC aggregator
    PRIVATE logger
    PRIVATE nlohmann_json::nlohmann_json
)

add_executable(webtop main.cpp)

target_include_directories(webtop 
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(webtop 
    PRIVATE server
    PRIVATE nlohmann_json::nlohmann_json
)
