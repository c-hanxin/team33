# tests/CMakeLists.txt
add_executable(nav_host_tests
    HostTests.cpp
)

target_include_directories(nav_host_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(nav_host_tests PRIVATE
    nav_engine
)

add_test(NAME HostTests COMMAND nav_host_tests)
