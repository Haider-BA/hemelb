
# This file is part of HemeLB and is Copyright (C)
# the HemeLB team and/or their institutions, as detailed in the
# file AUTHORS. This software is provided under the terms of the
# license in the file LICENSE.
cmake_minimum_required (VERSION 2.8)

project(HemeLB)

#---- OPTION switches accepted by the build -------

option(HEMELB_BUILD_DEBUGGER "Build the built in debugger" ON)
# option(HEMELB_DEBUGGER_IMPLEMENTATION "Which implementation to use for the debugger" none)
# mark_as_advanced(HEMELB_DEBUGGER_IMPLEMENTATION)

option(HEMELB_VALIDATE_GEOMETRY "Validate geometry" OFF)
option(HEMELB_BUILD_TESTS_ALL "Build all the tests" ON)
option(HEMELB_BUILD_TESTS_UNIT "Build the unit-tests (HEMELB_BUILD_TESTS_ALL takes precedence)" ON)
option(HEMELB_BUILD_TESTS_FUNCTIONAL "Build the functional tests (HEMELB_BUILD_TESTS_ALL takes precedence)" ON)
option(HEMELB_USE_ALL_WARNINGS_GNU "Show all compiler warnings on development builds (gnu-style-compilers)" ON)
option(HEMELB_USE_STREAKLINES "Calculate streakline images" OFF)
option(HEMELB_USE_BOOST "Use Boost" ON)
option(HEMELB_DEPENDENCIES_SET_RPATH "Set runtime RPATH" ON)
option(HEMELB_STATIC_ASSERT "Use simple compile-time assertions" ON)
option(HEMELB_WAIT_ON_CONNECT "Wait for steering client" OFF)
option(HEMELB_BUILD_MULTISCALE "Build HemeLB Multiscale functionality" OFF)
option(HEMELB_IMAGES_TO_NULL "Write images to null" OFF)
option(HEMELB_USE_SSE3 "Use SSE3 intrinsics" OFF)
option(HEMELB_USE_VELOCITY_WEIGHTS_FILE "Use Velocity weights file" OFF)

set(HEMELB_EXECUTABLE "hemelb"
  CACHE STRING "File name of executable to produce")
set(HEMELB_READING_GROUP_SIZE 5
  CACHE INTEGER "Number of cores to use to read geometry file.")
set(HEMELB_LOG_LEVEL Info
	CACHE STRING "Log level, choose 'Critical', 'Error', 'Warning', 'Info', 'Debug' or 'Trace'" )
set(HEMELB_STEERING_LIB basic
	CACHE STRING "Steering library, choose 'basic' or 'none'" )
set(HEMELB_DEPENDENCIES_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../dependencies"
	CACHE FILEPATH "Path to find dependency find modules")
set(HEMELB_DEPENDENCIES_INSTALL_PATH ${HEMELB_DEPENDENCIES_PATH}
	CACHE FILEPATH "Path to find dependency includes and libraries")
set(HEMELB_OPTIMISATION "-O3" CACHE STRING "Optimisation level (can be blank or -O1 to -O4)")
set(HEMELB_LATTICE "D3Q15"
  CACHE STRING "Select the lattice type to use (D3Q15,D3Q19,D3Q27,D3Q15i)")
set(HEMELB_KERNEL "LBGK"
  CACHE STRING "Select the kernel to use (LBGK,EntropicAnsumali,EntropicChik,MRT,TRT,NNCY,NNCYMOUSE,NNC,NNTPL)")
set(HEMELB_WALL_BOUNDARY "SIMPLEBOUNCEBACK"
  CACHE STRING "Select the boundary conditions to be used at the walls (BFL,GZS,SIMPLEBOUNCEBACK,JUNKYANG)")
set(HEMELB_INLET_BOUNDARY "NASHZEROTHORDERPRESSUREIOLET"
  CACHE STRING "Select the boundary conditions to be used at the inlet (NASHZEROTHORDERPRESSUREIOLET,LADDIOLET)")
set(HEMELB_OUTLET_BOUNDARY "NASHZEROTHORDERPRESSUREIOLET"
  CACHE STRING "Select the boundary conditions to be used at the outlets (NASHZEROTHORDERPRESSUREIOLET,LADDIOLET)")
set(HEMELB_COMPUTE_ARCHITECTURE "AMDBULLDOZER"
  CACHE STRING "Select the architecture of the machine being used (INTELSANDYBRIDGE,AMDBULLDOZER,NEUTRAL,ISBFILEVELOCITYINLET)")
set(HEMELB_WALL_INLET_BOUNDARY "NASHZEROTHORDERPRESSURESBB"
  CACHE STRING "Select the boundary conditions to be used at corners between walls and inlets (NASHZEROTHORDERPRESSURESBB,NASHZEROTHORDERPRESSUREBFL,LADDIOLETSBB,LADDIOLETBFL)")
set(HEMELB_WALL_OUTLET_BOUNDARY "NASHZEROTHORDERPRESSURESBB"
  CACHE STRING "Select the boundary conditions to be used at corners between walls and outlets (NASHZEROTHORDERPRESSURESBB,NASHZEROTHORDERPRESSUREBFL,LADDIOLETSBB,LADDIOLETBFL)")
set(HEMELB_POINTPOINT_IMPLEMENTATION Coalesce
	CACHE STRING "Point to point comms implementation, choose 'Coalesce', 'Separated', or 'Immediate'" )
set(HEMELB_GATHERS_IMPLEMENTATION Separated
	CACHE STRING "Gather comms implementation, choose 'Separated', or 'ViaPointPoint'" )
set(HEMELB_ALLTOALL_IMPLEMENTATION Separated
	CACHE STRING "Alltoall comms implementation, choose 'Separated', or 'ViaPointPoint'" )
option(HEMELB_SEPARATE_CONCERNS "Communicate for each concern separately" OFF)
	
# Add warnings flags to development build types
if (HEMELB_USE_ALL_WARNINGS_GNU)
	set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
	set( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")
endif()
if(HEMELB_OPTIMISATION)
	set( CMAKE_CXX_FLAGS_RELEASE ${HEMELB_OPTIMISATION})
	set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${HEMELB_OPTIMISATION}")
endif()


add_definitions(-DHEMELB_CODE)
add_definitions(-DHEMELB_READING_GROUP_SIZE=${HEMELB_READING_GROUP_SIZE})
add_definitions(-DHEMELB_LATTICE=${HEMELB_LATTICE})
add_definitions(-DHEMELB_KERNEL=${HEMELB_KERNEL})
add_definitions(-DHEMELB_WALL_BOUNDARY=${HEMELB_WALL_BOUNDARY})
add_definitions(-DHEMELB_INLET_BOUNDARY=${HEMELB_INLET_BOUNDARY})
add_definitions(-DHEMELB_OUTLET_BOUNDARY=${HEMELB_OUTLET_BOUNDARY})
add_definitions(-DHEMELB_WALL_INLET_BOUNDARY=${HEMELB_WALL_INLET_BOUNDARY})
add_definitions(-DHEMELB_WALL_OUTLET_BOUNDARY=${HEMELB_WALL_OUTLET_BOUNDARY})
add_definitions(-DHEMELB_COMPUTE_ARCHITECTURE=${HEMELB_COMPUTE_ARCHITECTURE})
add_definitions(-DHEMELB_LOG_LEVEL=${HEMELB_LOG_LEVEL})

if(HEMELB_VALIDATE_GEOMETRY)
  add_definitions(-DHEMELB_VALIDATE_GEOMETRY)
endif()

if (NOT HEMELB_USE_STREAKLINES)
	add_definitions(-DNO_STREAKLINES)
endif()

if (HEMELB_WAIT_ON_CONNECT)
	add_definitions(-DHEMELB_WAIT_ON_CONNECT)
endif()

if (NOT HEMELB_STATIC_ASSERT)
	add_definitions(-DHEMELB_NO_STATIC_ASSERT)
endif()

if (HEMELB_IMAGES_TO_NULL)
	add_definitions(-DHEMELB_IMAGES_TO_NULL)
endif()

if (HEMELB_USE_SSE3)
	add_definitions(-DHEMELB_USE_SSE3)
	set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse3")
	set( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -msse3")
        set( CMAKE_CXX_FLAGS_RELEASE "${HEMELB_OPTIMISATION} -msse3")
endif()

if (HEMELB_USE_VELOCITY_WEIGHTS_FILE)
    add_definitions(-DHEMELB_USE_VELOCITY_WEIGHTS_FILE)
endif()

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" "${HEMELB_DEPENDENCIES_PATH}/Modules/")
list(APPEND CMAKE_INCLUDE_PATH ${HEMELB_DEPENDENCIES_INSTALL_PATH}/include)
list(APPEND CMAKE_LIBRARY_PATH ${HEMELB_DEPENDENCIES_INSTALL_PATH}/lib)

#--- Set up runtime search path for DLLs -----
if(HEMELB_DEPENDENCIES_SET_RPATH)
	SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
	SET(CMAKE_INSTALL_RPATH "${HEMELB_DEPENDENCIES_INSTALL_PATH}/lib")
endif()

include(build_environment)
include(platform_checks)

if(HAVE_ISNAN)
  add_definitions(-DHAVE_ISNAN)
endif()

if(HAVE_STD_ISNAN)
  add_definitions(-DHAVE_STD_ISNAN)
endif()

if(HAVE_RUSAGE)
  add_definitions(-DHAVE_RUSAGE)
endif()

if(LINUX_SCANDIR)
  add_definitions(-DLINUX_SCANDIR)
endif()

if(NOT HAVE_XDRUINTXX_T)
  #Without the standard names for the xdr functions, create aliases for the existing ones.
  add_definitions(-Dxdr_uint16_t=xdr_u_int16_t)
  add_definitions(-Dxdr_uint32_t=xdr_u_int32_t)
  add_definitions(-Dxdr_uint64_t=xdr_u_int64_t)
endif()

include(mpi)
include(dependencies)

#-------------Resources -----------------------

set(BUILD_RESOURCE_PATH ${PROJECT_BINARY_DIR}/resources)
file(MAKE_DIRECTORY ${BUILD_RESOURCE_PATH})
set(INSTALL_RESOURCE_PATH ${CMAKE_INSTALL_PREFIX}/share/hemelb/resources)

configure_file (
  "${PROJECT_SOURCE_DIR}/resources/path_parameters.h.in"
  "${PROJECT_BINARY_DIR}/resources/path_parameters.h"
  )
 
# add the binary tree to the search path for include files
# so that we will find TutorialConfig.h
include_directories("${PROJECT_BINARY_DIR}")

# ----------- HemeLB ------------------
if (APPLE)
	add_definitions(-DHEMELB_CFG_ON_BSD -DHEMELB_CFG_ON_OSX)
endif()
if (HEMELB_USE_DEBUGGER)
	# Force a debug build, because the debugger makes no sense without debug symbols
	set(CMAKE_BUILD_TYPE DEBUG)
endif()

set(root_sources SimulationMaster.cc)
add_executable(${HEMELB_EXECUTABLE} main.cc ${root_sources})

include_directories(${PROJECT_SOURCE_DIR})
set(package_subdirs
	configuration
	extraction
	reporting
	steering
	vis
	lb
	geometry
	net
	debug
	util
	io
	log
        colloids
	)
foreach(subdir ${package_subdirs})
	set(lib "hemelb_${subdir}")
	list(APPEND heme_libraries ${lib})
	add_subdirectory(${subdir})
endforeach()
add_subdirectory(resources)
target_link_libraries(${HEMELB_EXECUTABLE} 
	${heme_libraries}
	${MPI_LIBRARIES}
	${PARMETIS_LIBRARIES}
	${TINYXML_LIBRARIES}
	${Boost_LIBRARIES}
	${CTEMPLATE_LIBRARIES}
	${ZLIB_LIBRARIES}
    ${MPWide_LIBRARIES}
	)
INSTALL(TARGETS ${HEMELB_EXECUTABLE} RUNTIME DESTINATION bin)
list(APPEND RESOURCES resources/report.txt.ctp resources/report.xml.ctp)

# ----------- HemeLB Multiscale ------------------
if (HEMELB_BUILD_MULTISCALE)
	if (APPLE)
		add_definitions(-DHEMELB_CFG_ON_BSD -DHEMELB_CFG_ON_OSX)
	endif()
	if (HEMELB_USE_DEBUGGER)
		# Force a debug build, because the debugger makes no sense without debug symbols
		set(CMAKE_BUILD_TYPE DEBUG)
	endif()
	
	set(root_sources SimulationMaster.cc multiscale/MultiscaleSimulationMaster.h)
	add_executable(multiscale_hemelb mainMultiscale.cc ${root_sources})
	include_directories(${PROJECT_SOURCE_DIR})
	set(package_subdirs
		configuration
		extraction
		reporting
		steering
		vis
		lb
		geometry
		net
		debug
		util
		io
		log
                colloids
                multiscale
		)
	foreach(subdir ${package_subdirs})
		set(lib "hemelb_${subdir}")
		list(APPEND heme_libraries ${lib})
	endforeach()
        add_subdirectory(multiscale)
	target_link_libraries(multiscale_hemelb
		${heme_libraries}
		${MPI_LIBRARIES}
		${PARMETIS_LIBRARIES}
		${TINYXML_LIBRARIES}
		${Boost_LIBRARIES}
		${CTEMPLATE_LIBRARIES}
		${ZLIB_LIBRARIES}
                ${MPWide_LIBRARIES}
		)
	INSTALL(TARGETS multiscale_hemelb RUNTIME DESTINATION bin)
	list(APPEND RESOURCES resources/report.txt.ctp resources/report.xml.ctp)
endif()

# ----------- HEMELB unittests ---------------
if(HEMELB_BUILD_TESTS_ALL OR HEMELB_BUILD_TESTS_UNIT)
	#------CPPUnit ---------------
	find_package(CPPUnit REQUIRED)
	include_directories(${CPPUNIT_INCLUDE_DIR})
	add_executable(unittests_hemelb ${root_sources})
	add_subdirectory(unittests)
	target_link_libraries(unittests_hemelb 
		hemelb_unittests 
		${heme_libraries}
		${MPI_LIBRARIES}
		${PARMETIS_LIBRARIES}
		${TINYXML_LIBRARIES}
		${CPPUNIT_LIBRARY}
		${Boost_LIBRARIES}
		${CTEMPLATE_LIBRARIES}
		${ZLIB_LIBRARIES}
		${MPWide_LIBRARIES}
		${CMAKE_DL_LIBS}) #Because on some systems CPPUNIT needs to be linked to libdl
	INSTALL(TARGETS unittests_hemelb RUNTIME DESTINATION bin)
	list(APPEND RESOURCES unittests/resources/four_cube.gmy unittests/resources/four_cube.xml unittests/resources/four_cube_multiscale.xml
		unittests/resources/config.xml unittests/resources/config0_2_0.xml
		unittests/resources/config_file_inlet.xml unittests/resources/iolet.txt 
		unittests/resources/config-velocity-iolet.xml unittests/resources/config_new_velocity_inlets.xml
    unittests/resources/velocity_inlet.txt.weights.txt
		unittests/resources/xmltest.xml unittests/resources/config_file_velocity_inlet.xml unittests/resources/velocity_inlet.txt 
		)
endif()

# ----------- HEMELB functionaltests ---------------
if(HEMELB_BUILD_TESTS_ALL OR HEMELB_BUILD_TESTS_FUNCTIONAL)
	#------CPPUnit ---------------
	find_package(CPPUnit REQUIRED)
	include_directories(${CPPUNIT_INCLUDE_DIR})
	add_executable(functionaltests_hemelb ${root_sources})
	add_subdirectory(functionaltests/cpptests)
	target_link_libraries(functionaltests_hemelb 
		hemelb_functionaltests 
		${heme_libraries}
		${MPI_LIBRARIES}
		${PARMETIS_LIBRARIES}
		${TINYXML_LIBRARIES}
		${CPPUNIT_LIBRARY}
		${Boost_LIBRARIES}
		${CTEMPLATE_LIBRARIES}
		${MPWide_LIBRARIES}
		${ZLIB_LIBRARIES}
		${CMAKE_DL_LIBS}) #Because on some systems CPPUNIT needs to be linked to libdl
	INSTALL(TARGETS functionaltests_hemelb RUNTIME DESTINATION bin)
endif()

#-------- Copy and install resources --------------

foreach(resource ${RESOURCES})
	configure_file(${PROJECT_SOURCE_DIR}/${resource} ${BUILD_RESOURCE_PATH} COPYONLY)
	get_filename_component(resource_leaf ${resource} NAME)
	INSTALL(FILES ${BUILD_RESOURCE_PATH}/${resource_leaf} DESTINATION share/hemelb/resources)
endforeach()
