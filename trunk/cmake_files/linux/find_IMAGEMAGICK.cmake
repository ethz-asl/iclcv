set(IMAGE_MAGICK_WAS_ON ${ICL_XDEP_IMAGEMAGICK_ON})
icl_check_external_package(IMAGEMAGICK Magick++.h GraphicsMagick++ lib include/GraphicsMagick /usr HAVE_IMAGEMAGICK_COND FALSE)
if(HAVE_IMAGEMAGICK_COND)
  set(IMAGEMAGICK_LIBS_l GraphicsMagick++)
  add_definitions( -DICL_USE_GRAPHICSMAGICK)
else()
  set(ICL_XDEP_IMAGEMAGICK_ON ${IMAGE_MAGICK_WAS_ON})
  icl_check_external_package(IMAGEMAGICK Magick++.h Magick++ lib include/ImageMagick /usr HAVE_IMAGEMAGICK_COND TRUE)
  if(HAVE_IMAGEMAGICK_COND)
    set(IMAGEMAGICK_LIBS_l Magick++)
    add_definitions( -DICL_USE_IMAGEMAGICK)
  endif()
endif()