cmake_minimum_required(VERSION 3.10)

# Disable in-source builds
set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

# Configure project
project(t1prime VERSION 0.0.1 LANGUAGES C CXX)
set(PROJECT_PREFIX ${PROJECT_NAME}-${${PROJECT_NAME}_VERSION})

# Utility includes
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# Compile switches
option(USE_I2C "USE_I2C" OFF)

# Compiler settings
set(CMAKE_C_STANDARD 99)

# Input files
set(SOURCES src/t1prime.c src/t1prime.h src/t1prime/datastructures.h)
set(HEADERS include/ifx/t1prime.h)
set(TESTS tests/t1prime-tests-main.cpp tests/t1prime-datastructures-tests.cpp tests/t1prime-protocol-tests.cpp tests/timer-mock.cpp)
if (${USE_I2C})
  list(APPEND TESTS tests/i2c-mock.cpp)
else()
  list(APPEND TESTS tests/spi-mock.cpp)
endif()

# Dependencies
find_package(ifx-error REQUIRED)
find_package(protocol REQUIRED)
find_package(crc REQUIRED)
find_package(timer REQUIRED)
if (${USE_I2C})
  find_package(i2c REQUIRED)
else()
  find_package(spi REQUIRED)
endif()

# Build library
if (${USE_I2C})
  add_compile_definitions(INTERFACE_I2C)
endif()
add_library(${PROJECT_NAME} ${SOURCES} ${HEADERS})
add_library(ifx::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
set_target_properties(${PROJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_link_libraries(${PROJECT_NAME} PUBLIC ifx::ifx-error ifx::protocol ifx::crc ifx::timer)
if (${USE_I2C})
  target_link_libraries(${PROJECT_NAME} PUBLIC ifx::i2c)
else()
  target_link_libraries(${PROJECT_NAME} PUBLIC ifx::spi)
endif()
target_include_directories(${PROJECT_NAME} 
  PUBLIC
    $<BUILD_INTERFACE:${${PROJECT_NAME}_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include/${PROJECT_PREFIX}>
  PRIVATE
    src)

# Prepare for packaging
set(INSTALL_RUNTIME_DIR ${CMAKE_INSTALL_BINDIR})
set(INSTALL_CONFIG_DIR  ${CMAKE_INSTALL_LIBDIR}/${PROJECT_PREFIX}/cmake)
set(INSTALL_LIBRARY_DIR ${CMAKE_INSTALL_LIBDIR}/${PROJECT_PREFIX})
set(INSTALL_ARCHIVE_DIR ${CMAKE_INSTALL_LIBDIR}/${PROJECT_PREFIX}/static)
set(INSTALL_INCLUDE_DIR ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_PREFIX}/ifx)
set(PROJECT_CONFIG_VERSION_FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake")
set(PROJECT_CONFIG_FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config.cmake")
configure_package_config_file(data/${PROJECT_NAME}-config.cmake.in ${PROJECT_CONFIG_FILE} INSTALL_DESTINATION ${INSTALL_CONFIG_DIR})
write_basic_package_version_file( ${PROJECT_CONFIG_VERSION_FILE} COMPATIBILITY SameMajorVersion)
install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}-targets
        RUNTIME DESTINATION ${INSTALL_RUNTIME_DIR}
        LIBRARY DESTINATION ${INSTALL_LIBRARY_DIR}
        ARCHIVE DESTINATION ${INSTALL_ARCHIVE_DIR})
install(FILES ${HEADERS} DESTINATION ${INSTALL_INCLUDE_DIR})
install(FILES ${PROJECT_CONFIG_VERSION_FILE} ${PROJECT_CONFIG_FILE} DESTINATION ${INSTALL_CONFIG_DIR})
install(EXPORT ${PROJECT_NAME}-targets FILE ${PROJECT_NAME}-targets.cmake NAMESPACE ifx:: DESTINATION ${INSTALL_CONFIG_DIR})
export(EXPORT ${PROJECT_NAME}-targets FILE ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-targets.cmake NAMESPACE ifx::)
export(PACKAGE ${PROJECT_NAME})

# Tests
include (CTest)
option(Catch2_LOCAL_CHECKOUT "Flag if Catch2 has been checked out locally and included in parent project" OFF)
if (${Catch2_LOCAL_CHECKOUT})
  set(Catch2_FOUND ON)
else()
  find_package(Catch2 CONFIG)
endif()
if (${Catch2_FOUND})
  add_executable(${PROJECT_NAME}-tests ${TESTS})
  add_dependencies(${PROJECT_NAME}-tests ${PROJECT_NAME})
  target_link_libraries(${PROJECT_NAME}-tests PRIVATE ${PROJECT_NAME} Catch2::Catch2)
  target_include_directories(${PROJECT_NAME}-tests PRIVATE src)
  add_test(NAME ${PROJECT_NAME}-tests COMMAND ${PROJECT_NAME}-tests)
endif()

# Documentation
find_package(Doxygen)
if (NOT DOXYGEN_FOUND)
  message(WARNING "Doxygen is required to build the documentation")
else()
  if (${USE_I2C})
    set(I2C_DOCUMENTATION_TAG "\"${i2c_DIR}/docs/i2c.tag\"=\"${i2c_DIR}/docs/html\"")
  else()
    set(SPI_DOCUMENTATION_TAG "\"${spi_DIR}/docs/spi.tag\"=\"${spi_DIR}/docs/html\"")
  endif()
  set(DOXYGEN_IN "${CMAKE_CURRENT_SOURCE_DIR}/data/Doxyfile.in")
  set(DOXYFILE "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile")
  set(DOXYGEN_OUT "${CMAKE_CURRENT_BINARY_DIR}/docs")
  configure_file("${DOXYGEN_IN}" "${DOXYFILE}")

  add_custom_target( ${PROJECT_NAME}-documentation
    COMMAND "${DOXYGEN_EXECUTABLE}" "${DOXYFILE}"
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    COMMENT "Generating API documentation using doxygen"
    VERBATIM)
  install(DIRECTORY "${DOXYGEN_OUT}" DESTINATION "${CMAKE_INSTALL_DOCDIR}")
endif()
