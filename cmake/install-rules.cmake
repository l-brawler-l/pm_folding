if(PROJECT_IS_TOP_LEVEL)
  set(
      CMAKE_INSTALL_INCLUDEDIR "include/xi-${PROJECT_VERSION}"
      CACHE STRING ""
  )
  set_property(CACHE CMAKE_INSTALL_INCLUDEDIR PROPERTY TYPE PATH)
endif()

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

# find_package(<package>) call for consumers to find this project
set(package xi)

install(
    DIRECTORY
    include/
    "${PROJECT_BINARY_DIR}/export/"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    COMPONENT xi_Development
)

install(
    TARGETS xi_xi
    EXPORT xiTargets
    RUNTIME #
    COMPONENT xi_Runtime
    LIBRARY #
    COMPONENT xi_Runtime
    NAMELINK_COMPONENT xi_Development
    ARCHIVE #
    COMPONENT xi_Development
    INCLUDES #
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

write_basic_package_version_file(
    "${package}ConfigVersion.cmake"
    COMPATIBILITY SameMajorVersion
)

# Allow package maintainers to freely override the path for the configs
set(
    xi_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/${package}"
    CACHE STRING "CMake package config location relative to the install prefix"
)
set_property(CACHE xi_INSTALL_CMAKEDIR PROPERTY TYPE PATH)
mark_as_advanced(xi_INSTALL_CMAKEDIR)

install(
    FILES cmake/install-config.cmake
    DESTINATION "${xi_INSTALL_CMAKEDIR}"
    RENAME "${package}Config.cmake"
    COMPONENT xi_Development
)

install(
    FILES "${PROJECT_BINARY_DIR}/${package}ConfigVersion.cmake"
    DESTINATION "${xi_INSTALL_CMAKEDIR}"
    COMPONENT xi_Development
)

install(
    EXPORT xiTargets
    NAMESPACE xi::
    DESTINATION "${xi_INSTALL_CMAKEDIR}"
    COMPONENT xi_Development
)

if(PROJECT_IS_TOP_LEVEL)
  include(CPack)
endif()
