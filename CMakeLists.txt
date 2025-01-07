cmake_minimum_required(VERSION 3.24)
set(NAME "ContainerDistributionFramework" CACHE STRING "")

option(BUILD_TEST "Sets log level to debug." OFF)

# -------- Project ----------
project(
	${NAME}
	VERSION 3.0.0
	LANGUAGES CXX
)

macro(set_from_environment VARIABLE)
	if (NOT DEFINED ${VARIABLE} AND DEFINED ENV{${VARIABLE}})
	if(NOT DEFINED ${VARIABLE} AND DEFINED ENV{${VARIABLE}})
		set(${VARIABLE} $ENV{${VARIABLE}})
	endif ()
	endif()
endmacro()

set_from_environment(VCPKG_ROOT)

macro(find_commonlib_path)
	if(CommonLibName AND NOT ${CommonLibName} STREQUAL "")
		# Check extern
		find_path(CommonLibPath
			include/REL/Relocation.h
			PATHS 
			External/${CommonLibName})

		if(${CommonLibPath} STREQUAL "CommonLibPath-NOTFOUND")
			message(
				STATUS
				" ${CommonLibPath}."
			)
			# Check path
			set_from_environment(${CommonLibName}Path)
			set(CommonLibPath ${${CommonLibName}Path})
			#set(${CommonLibName}Path  $ENV{"External/CommonLibVR"})
			#set(CommonLibPath "External/CommonLibVR")
		endif()
	endif()
endmacro()

set_from_environment(VCPKG_ROOT)

#set_property(GLOBAL PROPERTY USE_FOLDERS ON)

#list(APPEND CMAKE_IGNORE_PATH "${PROJECT_BINARY_DIR}")
#list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/tools/SKSE-CMakeModules/Modules" )

#include(SKSEPlugin)
#set(SKSE_USE_XBYAK ON)
#set(SKSE_SUPPORT_VR ON)

add_compile_definitions(SKSE_SUPPORT_XBYAK)


add_compile_definitions(SKYRIMVR)
add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
set(CommonLibName "CommonLibVR")
set_from_environment(SkyrimVRPath)
set(SkyrimPath ${SkyrimVRPath})
set(SkyrimVersion "Skyrim VR")


find_path(CLIB_UTIL_INCLUDE_DIRS "ClibUtil/utils.hpp")

			set_from_environment(${CommonLibName}Path)
			set(CommonLibPath ${${CommonLibName}Path})
#find_commonlib_path()
message(
	STATUS
	"Building ${NAME} ${VERSION} for ${SkyrimVersion} at ${SkyrimPath} with ${CommonLibName} at ${CommonLibPath}."
)

if(DEFINED VCPKG_ROOT)
	set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
	set(VCPKG_TARGET_TRIPLET "x64-windows-static" CACHE STRING "")
else()
	message(
		WARNING
		"Variable VCPKG_ROOT is not set. Continuing without vcpkg."
	)
endif()




if (BUILD_TEST)
	add_compile_definitions(DEBUG)
endif()

if(MSVC)
	if(NOT ${CMAKE_GENERATOR} STREQUAL "Ninja")
		add_compile_options(
			/MP # Build with Multiple Processes
		)
	endif()
endif()

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_DEBUG OFF)

set(Boost_USE_STATIC_LIBS ON)

# ---- Dependencies ----
if(DEFINED CommonLibPath AND NOT ${CommonLibPath} STREQUAL "" AND IS_DIRECTORY ${CommonLibPath})
	add_subdirectory(${CommonLibPath} ${CommonLibName})
else()
	message(
		FATAL_ERROR
		"Variable ${CommonLibName}Path is not set or in extern/."
	)
endif()


find_package(jsoncpp CONFIG REQUIRED)

configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/cmake/Version.h.in
	${CMAKE_CURRENT_BINARY_DIR}/include/Version.h
	@ONLY
)

configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.rc.in
	${CMAKE_CURRENT_BINARY_DIR}/version.rc
	@ONLY
)

set(headers
    src/common/PCH.h
    src/hooks/hooks.h
    src/utilities/utilities.h
    src/RE/misc.h
    src/RE/offset.h
    src/conditions/actorValueCondition.h
    src/conditions/condition.h
    src/conditions/containerCondition.h
    src/conditions/globalCondition.h
    src/conditions/locationCondition.h
    src/conditions/locationKeywordCondition.h
    src/conditions/questCondition.h
    src/conditions/referenceCondition.h
    src/conditions/worldspaceCondition.h
    src/merchantCache/merchantCache.h
    src/settings/INISettings.h
    src/settings/JSONSettings.h
)

set(sources
    src/hooks/hooks.cpp
    src/export/SKSEPlugin.cpp
    src/common/PCH.cpp
    src/conditions/actorValueCondition.cpp
    src/conditions/containerCondition.cpp
    src/conditions/globalCondition.cpp
    src/conditions/locationCondition.cpp
    src/conditions/locationKeywordCondition.cpp
    src/conditions/questCondition.cpp
    src/conditions/referenceCondition.cpp
    src/conditions/worldspaceCondition.cpp
    src/merchantCache/merchantCache.cpp
    src/settings/INISettings.cpp
    src/settings/JSONSettings.cpp
)

source_group(
	TREE
		${CMAKE_CURRENT_SOURCE_DIR}
	FILES
		${CMAKE_CURRENT_BINARY_DIR}/include/Version.h
		${headers}
		${sources}
)

add_library(
	${PROJECT_NAME}
	SHARED
	${headers}
	${sources}
	${CMAKE_CURRENT_BINARY_DIR}/include/Version.h
	${CMAKE_CURRENT_BINARY_DIR}/version.rc
)


target_compile_features(
	${PROJECT_NAME}
	PRIVATE
	cxx_std_23
)

target_compile_definitions(
	${PROJECT_NAME}
	PRIVATE
	_UNICODE
)

target_include_directories(
	${PROJECT_NAME}
	PRIVATE
		#$<INSTALL_INTERFACE:src>
		${CMAKE_CURRENT_BINARY_DIR}/include
		${CMAKE_CURRENT_SOURCE_DIR}/include
		${CMAKE_CURRENT_SOURCE_DIR}/src
		${CLIB_UTIL_INCLUDE_DIRS}
)

target_link_libraries(
		"${PROJECT_NAME}"
		PRIVATE
			${CommonLibName}::${CommonLibName}
			JsonCpp::JsonCpp
)

target_precompile_headers(
	${PROJECT_NAME}
	PRIVATE
		src/common/PCH.h
)

#set_target_properties(CommonLibVR PROPERTIES
#	FOLDER External
#)

if(MSVC)
	target_compile_options(
		${PROJECT_NAME}
		PRIVATE
		/sdl # Enable Additional Security Checks
		/utf-8 # Set Source and Executable character sets to UTF-8
		/Zi # Debug Information Format

		/permissive- # Standards conformance
		/Zc:preprocessor # Enable preprocessor conformance mode

		/wd4200 # nonstandard extension used : zero-sized array in struct/union

		"$<$<CONFIG:DEBUG>:>"
		"$<$<CONFIG:RELEASE>:/Zc:inline;/JMC-;/Ob3>"
	)

	target_link_options(
		${PROJECT_NAME}
		PRIVATE
		"$<$<CONFIG:DEBUG>:/INCREMENTAL;/OPT:NOREF;/OPT:NOICF>"
		"$<$<CONFIG:RELEASE>:/INCREMENTAL:NO;/OPT:REF;/OPT:ICF;/DEBUG:FULL>"
	)
endif()

set(DATA_DIR "${PROJECT_SOURCE_DIR}/data")

install(
	FILES "${DATA_DIR}/SKSE/Plugins/replaceme.ini"
	DESTINATION "SKSE/Plugins"
)

include(components.cmake)

# ---- Post build ----
if(DEFINED ENV{SKYRIM_MODS_FOLDER} AND IS_DIRECTORY "$ENV{SKYRIM_MODS_FOLDER}")
	if(BUILD_TEST)
		set(OUTPUT_FOLDER "$ENV{SKYRIM_MODS_FOLDER}/${PROJECT_NAME} - Test")
	else() 
		set(OUTPUT_FOLDER "$ENV{SKYRIM_MODS_FOLDER}/${PROJECT_NAME}")
	endif()
endif()

if(DEFINED OUTPUT_FOLDER)
    set(DLL_FOLDER "${OUTPUT_FOLDER}/SKSE/Plugins")

    add_custom_command(
        TARGET "${PROJECT_NAME}"
        POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_FILE:${PROJECT_NAME}>" "${DLL_FOLDER}/$<TARGET_FILE_NAME:${PROJECT_NAME}>"
		COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_PDB_FILE:${PROJECT_NAME}>" "${DLL_FOLDER}/$<TARGET_PDB_FILE_NAME:${PROJECT_NAME}>"
        VERBATIM
    )
endif()