cmake_minimum_required(VERSION 3.10)

project(blocksec2go-example VERSION 0.1 LANGUAGES C CXX)

# set(BUILD-DOCUMENTATION OFF) #Onemegoči add_dependencies tarče

set(PROJECT_PREFIX ${PROJECT_NAME}-${${PROJECT_NAME}_VERSION})

set(USE_I2C ON)

add_subdirectory(libs/ifx-error)
add_subdirectory(libs/logger)
add_subdirectory(libs/console-logger)
add_subdirectory(libs/timer)
add_subdirectory(libs/chronotimer)
add_subdirectory(libs/crc)
add_subdirectory(libs/protocol)
add_subdirectory(libs/apdu)
add_subdirectory(libs/apduprotocol)
add_subdirectory(libs/i2c)
add_subdirectory(libs/rpi-i2c)
add_subdirectory(libs/t1prime)
add_subdirectory(libs/blocksec2go)
add_subdirectory(libs/aes)
add_subdirectory(libs/aes-openssl)
add_subdirectory(libs/scp03)
add_subdirectory(src/se_interface)

find_package(ifx-error)
find_package(chronotimer)
find_package(t1prime)
find_package(blocksec2go)
find_package(scp03)

 # Documentation
find_package(Doxygen)
if (NOT DOXYGEN_FOUND)
  message(WARNING "Doxygen is required to build the documentation")
else()
  set(DOXYGEN_IN "${CMAKE_CURRENT_SOURCE_DIR}/data/Doxyfile.in")
  set(DOXYFILE "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile")
  set(DOXYGEN_OUT "${CMAKE_CURRENT_BINARY_DIR}/docs")
  configure_file("${DOXYGEN_IN}" "${DOXYFILE}")

  add_custom_target( documentation
    COMMAND "${DOXYGEN_EXECUTABLE}" "${DOXYFILE}" -b
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    COMMENT "Generating API documentation using doxygen"
    VERBATIM)

#  # TODO: Fix dependency management
#  add_dependencies(documentation
#    apdu-documentation
#    rpi-i2c-documentation
#    t1prime-documentation
#    chronotimer-documentation
#    blocksec2go-documentation
#    protocol-documentation
#    scp03-documentation
#    i2c-documentation
#  )
#    add_dependencies(rpi-i2c-documentation
#    ifx-error-documentation
#    protocol-documentation
#    i2c-documentation
#    timer-documentation
#  )
#  add_dependencies(i2c-documentation   
#   protocol-documentation
#  )
#  add_dependencies(apdu-documentation
#    ifx-error-documentation
#  )
#  add_dependencies(apduprotocol-documentation
#    ifx-error-documentation
#    apdu-documentation
#    protocol-documentation
#  )
#  add_dependencies(chronotimer-documentation
#    ifx-error-documentation
#    timer-documentation
#  )
#  add_dependencies(protocol-documentation
#    ifx-error-documentation
#  )
#  add_dependencies(t1prime-documentation
#    ifx-error-documentation
#    protocol-documentation
#    crc-documentation
#    i2c-documentation
#    timer-documentation
#  )
#  add_dependencies(timer-documentation
#    ifx-error-documentation
#  )

endif()