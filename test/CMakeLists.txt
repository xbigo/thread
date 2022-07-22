cmake_minimum_required(VERSION 3.14)

project(ApeThreadTest LANGUAGES CXX)

amun_fetch_lib(catch
	GIT_REPOSITORY ../Catch2.git
	GIT_TAG v2.13.0 )

amun_add_test(ape thread
	FEATURES cxx_std_20
	SOURCES main.cpp queue.cpp threadpool.cpp
	INCLUDES ../include "${catch_SOURCE_DIR}/single_include"  ${gsl_include_dir} ${ape_config_include_dir}
	LINKS Ape::thread $<IF:$<BOOL:${UNIX}>,pthread,>
	FOLDER unit_tests
	DEFINES PUBLIC APE_BUILD_SHARED
	WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
	)
