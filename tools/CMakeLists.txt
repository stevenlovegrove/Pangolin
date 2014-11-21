find_package(FREEGLUT QUIET)
find_package(GLUT QUIET)

if(BUILD_PANGOLIN_GUI AND BUILD_PANGOLIN_VARS AND (FREEGLUT_FOUND OR GLUT_FOUND OR ANDROID) )
  if(BUILD_PANGOLIN_VIDEO)
    add_subdirectory(VideoViewer)
  endif()
endif()
