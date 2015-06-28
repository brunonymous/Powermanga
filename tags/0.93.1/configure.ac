AC_PREREQ(2.50)
dnl AC_INIT(src/powermanga.c)
AC_INIT([Powermanga], [0.93.1], [http://sf.net/projects/powermanga/bugs], [powermanga])
AC_CONFIG_AUX_DIR(autotools)
AC_CANONICAL_SYSTEM
#AM_INIT_AUTOMAKE(powermanga,0.90)
AM_INIT_AUTOMAKE([foreign])
AC_CONFIG_HEADER(config.h)

dnl  Check our build tools
AC_PROG_CC
AM_PROG_AS
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h stdlib.h unistd.h])

AC_ARG_ENABLE(x11,
[  --enable-x11            X11 support (default disabled)],
enable_x11=yes, enable_x11=no)
AC_ARG_ENABLE(devel,
[  --enable-devel          Development version (default disabled)],
enable_devel=yes, enable_devel=no)
AC_ARG_ENABLE(sound,
[  --disable-sound         Disable sound],
disable_sound=yes, disable_sound=no)
AC_ARG_ENABLE(pngexport,
[  --disable-pngexport     Disables the option to export the sprites in PNG],
disable_png_export=yes, disable_png_export=no)


dnl  Check for X
if test "x${enable_x11}" = "xyes"; then
  AC_MSG_RESULT([Use the Xlib library for display]);
  AC_DEFINE(POWERMANGA_X11, 1, Define to enable X11 support)
  AC_PATH_X
  XLIB_CFLAGS="-I${x_includes}"
  AC_CHECK_LIB(Xxf86dga, XF86DGAQueryExtension, [
    XLIB_LIBS="-lXext -lX11 -lXxf86dga"
  ],[
    AC_MSG_ERROR(Could not find -lXext -lX11)
  ])
  AC_SUBST(XLIB_CFLAGS)
  AC_SUBST(XLIB_LIBS)
else
  AC_DEFINE(POWERMANGA_SDL, 1, Define to enable SDL support)
  AC_MSG_RESULT([Use the SDL for display]);
fi

dnl  Check for SDL
AM_PATH_SDL

if test "x${enable_devel}" = "xyes"; then
  dnl If defined [Ctrl] + [V], [Ctrl] + [P], [Ctrl] + [B] are available
  AC_DEFINE(UNDER_DEVELOPMENT,  1, Define to enable development version)
  AC_DEFINE(USE_MALLOC_WRAPPER, 1, Define to use a malloc wrapper)
  CFLAGS="-Wall -Werror -pedantic -Wextra -std=gnu99 -g $CFLAGS"
else
  CFLAGS="-O3 -Wall -Wextra -std=gnu99 $CFLAGS"
fi

dnl  Check for SDL_mixer
dnl LDFLAGS_save="${LDFLAGS} ${SDL_LIBS}"

if test "x${disable_sound}" = "xno"; then
  AC_DEFINE(USE_SDLMIXER,  1, [Enable sound (SDL Mixer)])
  AC_CHECK_LIB(SDL_mixer, Mix_GetChunk, [
    SDL_LIBS="-lSDL_mixer ${SDL_LIBS}"
  ],[
    AC_MSG_ERROR(Could not find -lSDL_mixer)
])
else
  AC_MSG_RESULT([The sound has been disabled]);
fi

dnl ttf and sdl_image for shareware version only
dnl AC_CHECK_LIB(SDL_ttf,TTF_Init,,[AC_MSG_ERROR([SDL_ttf])])
dnl SDL_LIBS="$SDL_LIBS -lSDL_ttf"
dnl AC_CHECK_LIB(SDL_image, main, LIBS="$SDL_LIBS -lSDL_image", AC_MSG_ERROR([*** SDL_image library not found!])
dnl                                      )
dnl AC_CHECK_LIB(z, gzopen,, AC_MSG_ERROR([*** zlib is missing]))

dnl  Check for PNG 
if test "x${disable_png_export}" = "xno"; then
  AC_DEFINE(PNG_EXPORT_ENABLE,  1, [Enable option to export sprites to PNG)])
  AC_CHECK_HEADER(png.h,,
     AC_MSG_ERROR([no png.h; please install libpng12-dev or equivalent]))
  AC_CHECK_LIB(png, main, [
    LDFLAGS="-lpng ${LDFLAGS}"
    SDL_LIBS="-lpng ${SDL_LIBS}"
  ],[
   AC_MSG_ERROR(Could not find -lpng)
  ])
fi

dnl  Check for asm support
AC_MSG_CHECKING(for x86 assembly support for arch ${target_cpu})
case "${target_cpu}" in
  i*86) ac_cv_need_asm=yes ;;
  *) ac_cv_need_asm=no ;;
esac
#ac_cv_need_asm=no
AC_MSG_RESULT(${ac_cv_need_asm})
AM_CONDITIONAL(ASSEMBLY, test "${ac_cv_need_asm}" = "yes")

AC_C_RESTRICT

dnl  Output makefiles
AC_OUTPUT(
  Makefile
  data/curves/Makefile
  data/levels/curves_phase/Makefile
  data/levels/grids_phase/Makefile
  data/Makefile
  graphics/Makefile
  graphics/bitmap/Makefile
  graphics/bitmap/fonts/Makefile
  graphics/sprites/Makefile
  graphics/sprites/guardians/Makefile
  graphics/sprites/spaceships/Makefile
  graphics/sprites/stars/Makefile
  graphics/sprites/meteors/Makefile
  src/Makefile
  sounds/Makefile
  sounds/handheld_console/Makefile
  texts/Makefile)


echo "

Powermanga ready for building!

  enable-x11        : $enable_x11
  enable_devel      : $enable_devel
  disable_sound     : $disable_sound
  disable_png_export: $disable_png_export
  PNG_EXPORT_ENABLE : $PNG_EXPORT_ENABLE 
  LDFLAGS           : $LDFLAGS

Type 'make' to build.

"
