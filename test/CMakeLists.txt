cmake_minimum_required(VERSION 3.14)

project(ApeThreadTest LANGUAGES CXX)

amun_fetch_lib(catch
	GIT_REPOSITORY ../Catch2.git
	GIT_TAG v3.1.0 )

amun_add_test(ape thread
	FEATURES cxx_std_20
	SOURCES main.cpp queue.cpp threadpool.cpp
	INCLUDES ../include ${gsl_include_dir} ${ape_config_include_dir}
	LINKS Ape::thread $<IF:$<BOOL:${UNIX}>,pthread,> Catch2::Catch2WithMain
	FOLDER unit_tests
	DEFINES PUBLIC APE_BUILD_SHARED CATCH_CONFIG_ENABLE_BENCHMARKING
	WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
	)
