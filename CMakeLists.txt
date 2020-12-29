cmake_minimum_required(VERSION 2.8.11)

#[[cmake_policy(VERSION 2.8.11)
if (POLICY CMP0076)
    cmake_policy(SET CMP0076 NEW)
endif()]]

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

set(PROJECT_WX melonDS)
project(${PROJECT_WX})

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

include(CheckSymbolExists)
function(detect_architecture symbol arch)
    if (NOT DEFINED ARCHITECTURE)
        set(CMAKE_REQUIRED_QUIET 1)
        check_symbol_exists("${symbol}" "" ARCHITECTURE_${arch})
        unset(CMAKE_REQUIRED_QUIET)

        # The output variable needs to be unique across invocations otherwise
        # CMake's crazy scope rules will keep it defined
        if (ARCHITECTURE_${arch})
            set(ARCHITECTURE "${arch}" PARENT_SCOPE)
            set(ARCHITECTURE_${arch} 1 PARENT_SCOPE)
            add_definitions(-DARCHITECTURE_${arch}=1)
        endif()
    endif()
endfunction()

detect_architecture("__x86_64__" x86_64)
detect_architecture("__i386__" x86)
detect_architecture("__arm__" ARM)
detect_architecture("__aarch64__" ARM64)

if (ARCHITECTURE STREQUAL x86_64 OR ARCHITECTURE STREQUAL ARM64)
	option(ENABLE_JIT "Enable x64 JIT recompiler" OFF)
endif()

if (ENABLE_JIT)
	add_definitions(-DJIT_ENABLED)
endif()

if (CMAKE_BUILD_TYPE STREQUAL Release)
	set(CMAKE_CXX_FLAGS_RELEASE "-O3 -flto")
	#option(ENABLE_LTO "Enable link-time optimization" ON)
else()
	#set(CMAKE_CXX_FLAGS "-g")
	#option(ENABLE_LTO "Enable link-time optimization" OFF)
endif()

option(ENABLE_OGLRENDERER "Enable OpenGL renderer" OFF)

if (ENABLE_OGLRENDERER)
    add_definitions(-DOGLRENDERER_ENABLED)
endif()

if (CMAKE_BUILD_TYPE STREQUAL Debug)
	set(CMAKE_CXX_FLAGS_DEBUG "-g")
	#add_compile_options(-Og)
endif()

#[[if(ENABLE_LTO)
	add_compile_options(-O3 -flto)
	set(CMAKE_CXX_STANDARD_LIBRARIES "-flto {CMAKE_CXX_STANDARD_LIBRARIES}")
	#add_link_options(-flto)

	if (${CMAKE_CXX_COMPILER_ID} STREQUAL Clang)
		find_program(LLD ld.lld)
		if (LLD)
			set(CMAKE_CXX_STANDARD_LIBRARIES "-fuse-ld=lld {CMAKE_CXX_STANDARD_LIBRARIES}")
			#add_link_options(-fuse-ld=lld)
		else()
			set(CMAKE_CXX_STANDARD_LIBRARIES "-fuse-linker-plugin {CMAKE_CXX_STANDARD_LIBRARIES}")
			#add_link_options(-fuse-linker-plugin)
		endif()
		#set(CMAKE_AR "llvm-ar")
	else()
		set(CMAKE_AR "gcc-ar")
	endif()
	#set(CMAKE_C_ARCHIVE_CREATE "<CMAKE_AR> qcs <TARGET> <LINK_FLAGS> <OBJECTS>")
	#set(CMAKE_C_ARCHIVE_FINISH   true)
	#set(CMAKE_CXX_ARCHIVE_CREATE "<CMAKE_AR> qcs <TARGET> <LINK_FLAGS> <OBJECTS>")
	#set(CMAKE_CXX_ARCHIVE_FINISH   true)
endif()]]

#add_compile_options(-fno-pic)
#set(CMAKE_CXX_STANDARD_LIBRARIES "-no-pie {CMAKE_CXX_STANDARD_LIBRARIES}")
#add_link_options(-no-pie)

#option(BUILD_LIBUI "Build libui frontend" ON)

add_subdirectory(src)

if (ANDROID)
	add_subdirectory(src/android)
else()
	if (BUILD_LIBUI)
		add_subdirectory(src/libui_sdl)
	endif()
endif()