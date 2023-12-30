cmake_minimum_required(VERSION 3.14)

project(ApeThreadTest LANGUAGES CXX)

amun_fetch_lib(Catch2
	GIT_REPOSITORY ../../catchorg/Catch2
	GIT_TAG v2.13.10 )

amun_add_test(ape thread
	FEATURES cxx_std_20
	SOURCES main.cpp queue.cpp threadpool.cpp
	INCLUDES ../include  ${gsl_include_dir} ${ape_config_include_dir}
	LINKS Ape::thread Catch2::Catch2 $<IF:$<BOOL:${UNIX}>,pthread,>
	FOLDER unit_tests
	DEFINES PUBLIC APE_BUILD_SHARED
	)
