cmake_minimum_required( VERSION 2.8 )

set( CMAKE_MODULE_PATH
 ${PROJECT_SOURCE_DIR}/cmake )

find_package( CPPUnit REQUIRED )

if( CMAKE_COMPILER_IS_GNUCXX )
  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror" )
  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-parameter" )
endif()

add_executable( cppunit-text-runner CppUnitTextRunner.cc )
target_link_libraries( cppunit-text-runner dl ${CPPUNIT_LIBRARIES} pthread )

if( ${CMAKE_SYSTEM_NAME} STREQUAL "Linux" )
  include( GNUInstallDirs )
endif()

if( APPLE )
  set( CMAKE_INSTALL_BINDIR "bin" )
endif()

install(
  TARGETS cppunit-text-runner
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} )
