####################################################################
#
# PowerManga 
#
####################################################################

# TODO :
# - tested only in Windows/mingw gcc & Emscripten
# - need more options + config file

cmake_minimum_required(VERSION 2.8)

project(PowerManga C CXX)

SET(CMAKE_BUILD_TYPE Release)

#SET(MAKE_VERBOSE_MAKEFILE off)

# configuration file
#configure_file (
#	"${PROJECT_SOURCE_DIR}/config.h.in"
#	"${PROJECT_SOURCE_DIR}/src/config.h"
#)

if(NOT EMSCRIPTEN)
find_package(SDL)
find_package(SDL_mixer)
endif()

# assets path
if(EMSCRIPTEN)
SET(PATH_FILE_ASSETS "assets")
endif()

# ---------------------------------------------------------------------------------------
# emscripten
if(EMSCRIPTEN)

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O2")
#SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} --preload-file ${PATH_FILE_ASSETS} -s ALLOW_MEMORY_GROWTH=1")
SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} --preload-file ${PATH_FILE_ASSETS} -s TOTAL_MEMORY=40000000")
SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} --closure 0")
SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} --memory-init-file 1")

else()

# ---------------------------------------------------------------------------------------
# windows mingw gcc options
if(WIN32 AND MINGW)

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -march=native -mtune=native -std=gnu99")
SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -m32")
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -m32")
SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -pipe -Wall -Wextra")
SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Ofast -ffast-math -s -flto")
SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -DNDEBUG -D_WIN32")
SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -pipe -g -DDEBUG")
SET(CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -g")
SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -static -s -flto")

endif()

# ---------------------------------------------------------------------------------------
# windows visual studio options
if(WIN32 AND MSVC)

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /W3 /WX-")
SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /D \"_CRT_SECURE_NO_WARNINGS\" /D \"WIN32\" /D \"NDEBUG\" /D \"_WIN32\"")
SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /O2 /Ob1 /Oi /Ot /Oy /MT /GS- /EHsc /fp:fast /GL")
SET(CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /OPT:REF /LTCG")

endif()

# ---------------------------------------------------------------------------------------
# unix gcc options
if(UNIX AND NOT APPLE)

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -march=native -mtune=native")
SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -m32")
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -m32")
SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -pipe -Wall")
SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3 -fwhole-program -ffast-math -s -flto")
SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -fno-rtti")
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -s -flto")

endif()

# end EMSCRIPTEN
endif()


if(NOT EMSCRIPTEN)
include_directories(
	${INCLUDE_DIRECTORIES}
	${SDL_INCLUDE_DIR}
	${SDL_MIXER_INCLUDE_DIR}
)
endif()

# files
add_executable(
	powermanga
	src/config.h 
	src/powermanga.c
	src/bonus.c
	src/bonus.h 
	src/counter_shareware.c 
	src/counter_shareware.h 
	src/congratulations.c 
	src/congratulations.h 
	src/config_file.c 
	src/config_file.h 
	src/curve_phase.c 	
	src/curve_phase.h 
	src/display.c 
	src/display.h 
	src/display_sdl.c 
	src/display_x11.c 	
	src/electrical_shock.c 
	src/enemies.c 
	src/enemies.h 
	src/energy_gauge.c 
	src/energy_gauge.h 
	src/explosions.c 
	src/explosions.h 
	src/extra_gun.c 
	src/extra_gun.h 
	src/gfx_wrapper.c 
	src/gfx_wrapper.h 
	src/grid_phase.c 
	src/grid_phase.h 
	src/guardians.c 
	src/guardians.h 
	src/images.c 	
	src/images.h 
	src/inits_game.c 
	src/lispreader.c 
	src/lispreader.h 
	src/lonely_foes.c 
	src/lonely_foes.h 
	src/main.c 
	src/menu.c 
	src/menu.h 
	src/menu_sections.c 
	src/menu_sections.h 
	src/meteors_phase.c 
	src/meteors_phase.h 
	src/movie.c 
	src/movie.h 
	src/log_recorder.c 
	src/log_recorder.h 	
	src/options_panel.c 
	src/options_panel.h 
	src/powermanga.h 
	src/scalebit.c 
	src/scalebit.h 
	src/scale2x.c 
	src/scale2x.h 
	src/scale3x.c 	
	src/scale3x.h 
	src/sdl_mixer.c 
	src/sdl_mixer.h 
	src/satellite_protections.c 
	src/satellite_protections.h 
	src/scrolltext.c 	
	src/scrolltext.h 
	src/spaceship.c 
	src/spaceship.h 
	src/spaceship_weapons.c 
	src/spaceship_weapons.h 
	src/special_keys.c 
	src/sprites_string.c 
	src/sprites_string.h 
	src/shots.c 
	src/shots.h 
	src/shockwave.c
	src/shockwave.h 
	src/script_page.c
	src/script_page.h 
	src/starfield.c 
	src/starfield.h 
	src/text_overlay.c
	src/text_overlay.h
	src/texts.c
	src/texts.h
	src/tools.c
	src/tools.h
	
	src/gfxroutines.h
	src/gfxroutines.c 
	
)

if(NOT EMSCRIPTEN)
target_link_libraries(
	powermanga
	${SDL_LIBRARY}
	${SDL_MIXER_LIBRARY}
)
endif()

if(EMSCRIPTEN)
SET(CMAKE_EXECUTABLE_SUFFIX ".html")
endif()

SET(RELEASE_NAME "powermanga")
set_target_properties(powermanga PROPERTIES OUTPUT_NAME "${RELEASE_NAME}")
