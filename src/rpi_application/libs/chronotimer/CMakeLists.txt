cmake_minimum_required(VERSION 3.10)

# Disable in-source builds
set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

# Configure project
project(chronotimer VERSION 0.0.1 LANGUAGES C CXX)
set(PROJECT_PREFIX ${PROJECT_NAME}-${${PROJECT_NAME}_VERSION})

# Utility includes
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# Compiler settings
set(CMAKE_CXX_STANDARD 11)

# Input files
set(SOURCES src/chronotimer.cpp)

# Dependencies
find_package(ifx-error REQUIRED)
find_package(timer REQUIRED)

# Build library
add_library(${PROJECT_NAME} ${SOURCES})
add_library(ifx::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
set_target_properties(${PROJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_link_libraries(${PROJECT_NAME} PUBLIC ifx::ifx-error ifx::timer)

# Prepare for packaging
set(INSTALL_RUNTIME_DIR "${CMAKE_INSTALL_BINDIR}")
set(INSTALL_CONFIG_DIR  "${CMAKE_INSTALL_LIBDIR}/${PROJECT_PREFIX}/cmake")
set(INSTALL_LIBRARY_DIR "${CMAKE_INSTALL_LIBDIR}/${PROJECT_PREFIX}")
set(INSTALL_ARCHIVE_DIR "${CMAKE_INSTALL_LIBDIR}/${PROJECT_PREFIX}/static")
set(PROJECT_CONFIG_VERSION_FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake")
set(PROJECT_CONFIG_FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config.cmake")
configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/data/${PROJECT_NAME}-config.cmake.in" ${PROJECT_CONFIG_FILE} INSTALL_DESTINATION ${INSTALL_CONFIG_DIR})
write_basic_package_version_file( "${PROJECT_CONFIG_VERSION_FILE}" COMPATIBILITY SameMajorVersion)
install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}-targets
        RUNTIME DESTINATION "${INSTALL_RUNTIME_DIR}"
        LIBRARY DESTINATION "${INSTALL_LIBRARY_DIR}"
        ARCHIVE DESTINATION "${INSTALL_ARCHIVE_DIR}")
install(FILES "${PROJECT_CONFIG_VERSION_FILE}" "${PROJECT_CONFIG_FILE}" DESTINATION "${INSTALL_CONFIG_DIR}")
install(EXPORT ${PROJECT_NAME}-targets FILE ${PROJECT_NAME}-targets.cmake NAMESPACE ifx:: DESTINATION "${INSTALL_CONFIG_DIR}")
export(EXPORT ${PROJECT_NAME}-targets FILE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-targets.cmake" NAMESPACE ifx::)
export(PACKAGE ${PROJECT_NAME})

# Documentation
find_package(Doxygen)
if (NOT DOXYGEN_FOUND)
  message(WARNING "Doxygen is required to build the documentation")
else()
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
