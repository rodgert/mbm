configure_file(words.txt ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
add_executable(benchmarks benchmarks.cpp)
target_link_libraries(benchmarks ${Boost_LIBRARIES})

add_test(benchmarks ${CMAKE_CURRENT_BINARY_DIR}/benchmarks)
