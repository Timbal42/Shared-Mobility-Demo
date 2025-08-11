cmake_minimum_required(VERSION 3.10)

include(CMakePackageConfigHelpers)

project(blocksec2go VERSION 0.1 LANGUAGES C CXX)
set(PROJECT_PREFIX ${PROJECT_NAME}-${${PROJECT_NAME}_VERSION})

set(CMAKE_C_STANDARD 99)

find_package(ifx-error REQUIRED)
find_package(apdu REQUIRED)
find_package(protocol REQUIRED)
find_package(apduprotocol REQUIRED)

set(SOURCES src/blocksec2go.c)
set(HEADERS include/ifx/blocksec2go.h)

add_library(blocksec2go ${SOURCES} ${HEADERS})
add_library(ifx::blocksec2go ALIAS blocksec2go)
target_link_libraries(blocksec2go
    ifx::ifx-error
    ifx::apdu
    ifx::protocol
    ifx::apduprotocol
)
target_include_directories(blocksec2go
  PUBLIC
    "$<BUILD_INTERFACE:${${PROJECT_NAME}_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:include/${PROJECT_PREFIX}>"
)

# Prepare for packaging
set(INSTALL_RUNTIME_DIR "${CMAKE_INSTALL_BINDIR}")
set(INSTALL_CONFIG_DIR  "${CMAKE_INSTALL_LIBDIR}/${PROJECT_PREFIX}/cmake")
set(INSTALL_LIBRARY_DIR "${CMAKE_INSTALL_LIBDIR}/${PROJECT_PREFIX}")
set(INSTALL_ARCHIVE_DIR "${CMAKE_INSTALL_LIBDIR}/${PROJECT_PREFIX}/static")
set(INSTALL_INCLUDE_DIR "${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_PREFIX}/ifx")
set(PROJECT_CONFIG_VERSION_FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake")
set(PROJECT_CONFIG_FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config.cmake")
configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/data/${PROJECT_NAME}-config.cmake.in" ${PROJECT_CONFIG_FILE} INSTALL_DESTINATION ${INSTALL_CONFIG_DIR})
write_basic_package_version_file( "${PROJECT_CONFIG_VERSION_FILE}" COMPATIBILITY SameMajorVersion)
install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}-targets
        RUNTIME DESTINATION "${INSTALL_RUNTIME_DIR}"
        LIBRARY DESTINATION "${INSTALL_LIBRARY_DIR}"
        ARCHIVE DESTINATION "${INSTALL_ARCHIVE_DIR}")
install(FILES ${HEADERS} DESTINATION "${INSTALL_INCLUDE_DIR}")
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

  # add_dependencies(blocksec2go-documentation
  # ifx-error-documentation
  # apdu-documentation
  # protocol-documentation
  # apduprotocol-documentation
# )
endif()

