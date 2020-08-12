cmake_minimum_required(VERSION 3.14)

project(ApeThreadTest LANGUAGES CXX)

amun_fetch_lib(catch
	GIT_REPOSITORY ../Catch2.git
	GIT_TAG v2.13.0 )

add_executable(ape_thread_test main.cpp queue.cpp threadpool.cpp)
target_compile_features(ape_thread_test PUBLIC cxx_std_17)
target_include_directories(ape_thread_test PUBLIC ../include
	"${catch_SOURCE_DIR}/single_include"
	${gsl_include_dir}
	${ape_config_include_dir})
target_compile_definitions(ape_thread_test PUBLIC APE_BUILD_SHARED) 
target_link_libraries(ape_thread_test Ape::thread)
if(UNIX)
	target_link_libraries(ape_thread_test pthread)
endif()

add_test(NAME run_thread_test COMMAND ape_thread_test WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>" )
