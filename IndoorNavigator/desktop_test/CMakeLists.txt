# desktop_test/CMakeLists.txt
add_executable(nav_desktop_runner
    main.cpp
    CliUI.cpp
)

target_include_directories(nav_desktop_runner PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(nav_desktop_runner PRIVATE
    nav_engine
)
