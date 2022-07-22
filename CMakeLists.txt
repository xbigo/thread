cmake_minimum_required(VERSION 3.14)

project(ApeThread
	VERSION "0.2.0"
	DESCRIPTION "Thread related utilities"
	LANGUAGES CXX)

if (NOT DEFINED APE_SUPERPROJECT)
	find_package(Ape 0.2.0 QUIET)
	if (NOT Ape_FOUND)
		find_package(ApeAmun 0.2.0 QUIET)
		if (NOT ApeAmun_FOUND)
			include(FetchContent)
			FetchContent_Declare( amun
				GIT_REPOSITORY https://github.com/xbigo/amun.git
				GIT_TAG v0.2.0
				)
			FetchContent_MakeAvailable(amun)
			include(${amun_SOURCE_DIR}/lib/amun/amun.cmake)
		endif()
		amun_fetch_lib( ape_config
			GIT_REPOSITORY ../config.git
			GIT_TAG v0.2.0
			)
		add_library(ape_config INTERFACE)
		target_include_directories(ape_config INTERFACE "${ape_config_SOURCE_DIR}/include")
	endif()
endif()
amun_disable_build_in_source()

amun_check_build(THREAD NORMAL INSTALL TEST)

get_target_property(ape_config_include_dir ape_config INTERFACE_INCLUDE_DIRECTORIES)

if(${THREAD_NORMAL})
	amun_fetch_lib( gsl
		GIT_REPOSITORY ../GSL.git
		GIT_TAG v3.1.0
		)
	set (gsl_include_dir "${gsl_SOURCE_DIR}/include")

	set(sources src/threadpool.cpp)
	set(headers queue.hpp threadpool.hpp threadpool.ipp)
	list(TRANSFORM headers PREPEND "include/ape/thread/")
	list(APPEND all_sources ${headers} ${sources})

	if (DEFINED APE_SUPERPROJECT)
		add_library(Ape::thread ALIAS ape )
		target_sources(ape PRIVATE ${all_sources})
		set(target_library "ape")
	else()
		option(APE_BUILD_SHARED "Enable shared library build" ON )
		if (APE_BUILD_SHARED)
			add_library(ape_thread SHARED ${all_sources})
			target_compile_definitions(ape_thread PRIVATE APE_BUILD_SHARED)
		else()
			add_library(ape_thread STATIC ${all_sources})
			target_compile_definitions(ape_thread PRIVATE APE_BUILD_STATIC)
		endif()
		add_library(Ape::thread ALIAS ape_thread )
		target_compile_features(ape_thread PUBLIC cxx_std_20)
		target_compile_definitions(ape_thread PUBLIC
			$<$<CXX_COMPILER_ID:MSVC>:
			_SCL_SECURE_NO_WARNINGS
			_SILENCE_CXX17_UNCAUGHT_EXCEPTION_DEPRECATION_WARNING
			>)
		set(target_library "ape_thread")
		if(UNIX)
			target_link_libraries(ape_thread pthread)
		endif()
	endif()
	if (MSVC)
		target_compile_options(${target_library} PUBLIC "/Zc:__cplusplus")
	endif()
	target_include_directories(${target_library}  PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
		$<BUILD_INTERFACE:${gsl_include_dir}>
		$<BUILD_INTERFACE:${ape_config_include_dir}>
		$<INSTALL_INTERFACE:include>
		)

	amun_fake_project(ape_thread_
		SOURCES ${headers} src/dummy.cpp
		INCLUDES include  ${gsl_include_dir} ${ape_config_include_dir}
		FOLDER Libs
		FEATURES INTERFACE cxx_std_20)
endif()

if(${THREAD_INSTALL})
	amun_get_cmake_install_dir(THREAD_INSTALL_DIR)

	if (NOT DEFINED APE_SUPERPROJECT)
		amun_install_targets(${THREAD_INSTALL_DIR} ape_thread)
		amun_install_config_version(${THREAD_INSTALL_DIR} NOARCH)
	endif()

	install(DIRECTORY include/ DESTINATION include)
endif()

if(${THREAD_TEST})
	amun_enable_testing()
	add_subdirectory(test)
endif()
