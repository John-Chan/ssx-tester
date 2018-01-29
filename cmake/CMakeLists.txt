CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

#Project
PROJECT (ssx-tester CXX C)



SET(ssx_project_DIR ${PROJECT_SOURCE_DIR}/..)
SET(ssx_src_DIR ${ssx_project_DIR}/src)
SET(mysoow_src_DIR ${ssx_project_DIR}/3rd/mysoow/src)


#add_definitions( -DBOOST_ALL_DYN_LINK ) 
set(Boost_USE_MULTITHREADED ON)
if (MSVC)
  set(Boost_USE_STATIC_LIBS ON)
  set(Boost_USE_STATIC_RUNTIME ON)
endif (MSVC)

# USE STATIC RUNNTIME
if (MSVC)
    set(CompilerFlags
        CMAKE_CXX_FLAGS
        CMAKE_CXX_FLAGS_DEBUG
        CMAKE_CXX_FLAGS_RELEASE
        CMAKE_C_FLAGS
        CMAKE_C_FLAGS_DEBUG
        CMAKE_C_FLAGS_RELEASE
        )
    foreach(CompilerFlag ${CompilerFlags})
        string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
    endforeach()
endif(MSVC)

# min boost version 1.55 for predef
find_package (Boost 1.55.0 REQUIRED COMPONENTS system filesystem thread date_time chrono atomic)
if (Boost_FOUND)
  # From the offical documentation:
  # Add include directories to the build. [...] If the SYSTEM option is given,
  # the compiler will be told the directories are meant as system include
  # directories on some platforms (signalling this setting might achieve effects
  # such as the compiler skipping warnings [...])."
  include_directories (SYSTEM ${Boost_INCLUDE_DIR})
  MESSAGE(STATUS "Boost_INCLUDE_DIR:" ${Boost_INCLUDE_DIR})
  # From the offical documentation:
  # "Specify directories in which the linker will look for libraries. [...] Note
  # that this command is rarely necessary. Library locations returned by
  # find_package() and find_library() are absolute paths. Pass these absolute
  # library file paths directly to the target_link_libraries() command. CMake
  # will ensure the linker finds them."
  #link_directories (${Boost_LIBRARY_DIRS})
else(Boost_FOUND)
  MESSAGE(STATUS "the boost library not found,set BOOST_ROOT or (BOOST_INCLUDEDIR,BOOST_LIBRARYDIR) first")
  
  #set (BOOST_ROOT "C:/MinGW")
  #set (BOOST_INCLUDEDIR "C:/MinGW/include")
  #set (BOOST_LIBRARYDIR "C:/MinGW/lib")
endif ()

#MESSAGE(STATUS "ADD BOOST LIB DIR:" ${BOOST_LIB32_DIR})
#LINK_DIRECTORIES(${BOOST_LIB32_DIR})

if (MSVC)
  add_definitions(/wd4244 /wd4267 /wd4018 /wd4355 /wd4800 /wd4251 /wd4996 /wd4146 /wd4305)
endif (MSVC)

#if (MSVC)
#  string(REPLACE "/" "\\" relay_src_WIN32_PATH ${relay_src_DIR})
#  MESSAGE(STATUS "relay_src_WIN32_PATH:" ${relay_src_WIN32_PATH})
#  configure_file(extract_includes.bat.in extract_includes.bat)
#endif (MSVC)

#get_filename_component(logcpp_source_dir ${LOG4CPP_SRC_DIR} PATH)
#MESSAGE(STATUS "logcpp_source_dir:" ${logcpp_source_dir})
 

INCLUDE_DIRECTORIES(${ssx_src_DIR})



if (MSVC)
  # Add the "lib" prefix for generated .lib outputs.
  set(LIB_PREFIX lib)
else (MSVC)
  # When building with "make", "lib" prefix will be added automatically by
  # the build tool.
  set(LIB_PREFIX)
endif (MSVC)

include(build_ssxtester.cmake)