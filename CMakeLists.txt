cmake_minimum_required(VERSION 3.14)

project(ApeThread
	VERSION "0.1.0"
	DESCRIPTION "Thread related utilities"
	LANGUAGES CXX)

if (NOT DEFINED APE_SUPERPROJECT)
	find_package(Ape 0.1.0 QUIET)
	if (NOT Ape_FOUND)
		find_package(ApeAmun 0.1.0 QUIET)
		if (NOT ApeAmun_FOUND)
			include(FetchContent)
			FetchContent_Declare( amun
				GIT_REPOSITORY https://github.com/xbigo/amun.git
				)
			FetchContent_MakeAvailable(amun)
			include(${amun_SOURCE_DIR}/lib/amun/amun.cmake)
		endif()
		amun_fetch_lib( ape_config
			GIT_REPOSITORY ../config.git
			#GIT_TAG
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
		GIT_REPOSITORY https://github.com/microsoft/GSL.git
		GIT_TAG v3.1.0
		)
	set (gsl_include_dir "${gsl_SOURCE_DIR}/include")

	add_library(ape_thread INTERFACE)
	add_library(Ape::thread ALIAS ape_thread )
	target_compile_features(ape_thread INTERFACE cxx_std_17)
	target_compile_definitions(ape_thread INTERFACE
		$<$<CXX_COMPILER_ID:MSVC>:
		_SCL_SECURE_NO_WARNINGS
		_SILENCE_CXX17_UNCAUGHT_EXCEPTION_DEPRECATION_WARNING
		>)
	target_include_directories(ape_thread INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:include>)
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


