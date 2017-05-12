cmake_minimum_required(VERSION 3.7)
project (babel-performance-sdr)

include_directories("${PROJECT_BINARY_DIR}")
include_directories("${PROJECT_SOURCE_DIR}/includes")
include_directories(/usr/local/include)
include_directories(/opt/local/include)
link_directories(/usr/local/lib)
link_directories(/opt/local/lib)

add_executable( rxtx
        ${PROJECT_SOURCE_DIR}/rxtx.c
        )
add_executable( rx
        ${PROJECT_SOURCE_DIR}/rx.c
        )
add_executable( tx
        ${PROJECT_SOURCE_DIR}/tx.c
        )



target_link_libraries(rxtx bladeRF m c liquid)
target_link_libraries(rx bladeRF m c liquid)
target_link_libraries(tx bladeRF m c liquid)

