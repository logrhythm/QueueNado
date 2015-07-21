# ==================================================================
# 2014 by KjellKod.cc. This is PUBLIC DOMAIN to use at your own
# risk and comes with no warranties.
#
# This code is yours to share, use and modify with no strings attached
# and no restrictions or obligations.
# ===================================================================

cmake_minimum_required (VERSION 2.6)
ENABLE_LANGUAGE(CXX)
#set(CMAKE_BUILD_TYPE Release)

project (QueueNado)
set(PROJECT_SRC ${QueueNado_SOURCE_DIR}/src)
set(DIR_UNIT_TEST ${QueueNado_SOURCE_DIR}/test)
set(DIR_3RDPARTY ${QueueNado_SOURCE_DIR}/3rdparty)
set(CMAKE_INSTALL_RPATH "/usr/local/probe/lib:/usr/local/probe/lib64")
set(CMAKE_SKIP_BUILD_RPATH TRUE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
find_library(G3LOG g3logger PATHS /usr/local/probe/lib )
find_library(G3LOG_ROTATE g3logrotate PATHS /usr/local/probe/lib)
find_library(BOOST_THREAD boost_thread PATHS /usr/local/probe/lib )
find_library(BOOST_SYSTEM boost_thread PATHS /usr/local/probe/lib )
find_library(CQueueNado czmq PATHS /usr/local/probe/lib )
find_library(ZMQ zmq PATHS /usr/local/probe/lib )
find_library(ZLIB z PATHS /usr/local/probe/lib )
find_library(TCMALLOC tcmalloc PATHS /usr/local/probe/lib )


MESSAGE(" PROJECT_SRC = : ${PROJECT_SRC}")
include_directories(${PROJECT_SRC})
SET(ACTIVE_CPP0xx_DIR "Release")


#cmake -DCMAKE_CXX_COMPILER=clang++ ..
  # WARNING: If Clang for Linux does not work with full C++11 support it might be your
  # installation that is faulty. When I tested Clang on Ubuntu I followed the following
  # description
  # 1) http://kjellkod.wordpress.com/2013/09/23/experimental-g2log-with-clang/
  # 2) https://github.com/maidsafe/MaidSafe/wiki/Hacking-with-Clang-llvm-abi-and-llvm-libc
IF ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
   MESSAGE("")
   MESSAGE("cmake for Clang ")
   if (APPLE)
       set(PLATFORM_LINK_LIBRIES c++abi)
   else()
       set(PLATFORM_LINK_LIBRIES rt c++abi)
   endif()
  set(CMAKE_CXX_FLAGS "-Wall -std=c++11 -pthread -stdlib=libc++ -Wunused -D_GLIBCXX_USE_NANOSLEEP")

ELSEIF("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
   MESSAGE("")
   MESSAGE("cmake for GCC ")
   if (APPLE)
       set(CMAKE_CXX_FLAGS "-Wall -Wunused -std=c++11 -pthread -D_GLIBCXX_USE_NANOSLEEP")
   else()
       set(PLATFORM_LINK_LIBRIES rt)
       set(CMAKE_CXX_FLAGS "-Wall -rdynamic -Wunused -std=c++11 -pthread -D_GLIBCXX_USE_NANOSLEEP")
   endif()

ELSEIF(MSVC)
      # VC11 bug: http://code.google.com/p/googletest/issues/detail?id=408
      # add_definition(-D_VARIADIC_MAX=10)
      # https://github.com/anhstudios/swganh/pull/186/files
      ADD_DEFINITIONS (/D_VARIADIC_MAX=10)
      MESSAGE(STATUS "- MSVC: Set variadic max to 10 for MSVC compatibility")
      # Remember to set set target properties if using GTEST similar to done below on target "unit_test"
      # "set_target_properties(unit_test PROPERTIES COMPILE_DEFINITIONS "GTEST_USE_OWN_TR1_TUPLE=0")
   MESSAGE("")
   MESSAGE("Windows: Please run the command [cmake -DCMAKE_BUILD_TYPE=Release -G \"Visual Studio 11\" ..]")
   MESSAGE("if cmake finishes OK, do 'msbuild TestOfQueueNado.sln /p:Configuration=Release'")
   MESSAGE("then run 'Release\\TestOfQueueNado.exe'")
   MESSAGE("")
ENDIF()


# GENERIC STEPS
file(GLOB SRC_FILES ${PROJECT_SRC}/*.h ${PROJECT_SRC}/*.hpp ${PROJECT_SRC}/*.cpp ${PROJECT_SRC}/*.ipp)
 
# Create the QueueNado library
include_directories(${PROJECT_SRC})
add_library(QueueNado ${SRC_FILES})
set_target_properties(QueueNado PROPERTIES LINKER_LANGUAGE CXX)
add_library(QueueNado_shared SHARED ${SRC_FILES})
set_target_properties(QueueNado_shared PROPERTIES LINKER_LANGUAGE CXX)


# create the unit tests
# =========================

set(GTEST_DIR ${DIR_3RDPARTY}/gtest-1.7.0)
set(GTEST_INCLUDE_DIRECTORIES ${GTEST_DIR}/include ${GTEST_DIR} ${GTEST_DIR}/src)
MESSAGE( "Attempt to build gtest. gtest directory: " ${GTEST_DIR})
include_directories(${GTEST_INCLUDE_DIRECTORIES})
add_library(gtest_170_lib ${GTEST_DIR}/src/gtest-all.cc)
set_target_properties(gtest_170_lib PROPERTIES COMPILE_DEFINITIONS "GTEST_HAS_RTTI=0")
enable_testing(true)



# obs see this: http://stackoverflow.com/questions/9589192/how-do-i-change-the-number-of-template-arguments-supported-by-msvcs-stdtupl
# and this: http://stackoverflow.com/questions/2257464/google-test-and-visual-studio-2010-rc
#include_directories(${DIR_UNIT_TEST})
include_directories(test)
  file(GLOB TEST_SRC_FILES "test/*.cpp")

  add_executable(UnitTestRunner 3rdparty/test_main.cpp ${TEST_SRC_FILES} )
  set_target_properties(${test} PROPERTIES COMPILE_DEFINITIONS "GTEST_HAS_TR1_TUPLE=0")
  set_target_properties(${test} PROPERTIES COMPILE_DEFINITIONS "GTEST_HAS_RTTI=0")
  target_link_libraries(UnitTestRunner  QueueNado gtest_170_lib boost_thread
boost_system ${ZLIB} ${G3LOG} ${G3LOG_ROTATE} FileIO DeathKnell stdc++ zmq czmq tcmalloc ${PLATFORM_LINK_LIBRIES} -Wl,-rpath -Wl,. -Wl,-rpath -Wl,/usr/local/probe/lib -Wl,-rpath -Wl,/usr/local/probe/lib64)
IF( NOT(MSVC))
  set_target_properties(${test} PROPERTIES COMPILE_FLAGS "-isystem -pthread ")
ENDIF( NOT(MSVC))
