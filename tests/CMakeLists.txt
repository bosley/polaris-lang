link_directories(${CPPUTEST_LIBRARIES})

include_directories(
  ${CPPUTEST_INCLUDE_DIRS}
  ../
)

add_executable(polaris_unit_tests
        polaris_tests.cpp
        ${POLARIS_SOURCES}
        main.cpp)

target_link_libraries(polaris_unit_tests
  ${CPPUTEST_LDFLAGS}
)

add_custom_command(TARGET polaris_unit_tests COMMAND ./polaris_unit_tests POST_BUILD)