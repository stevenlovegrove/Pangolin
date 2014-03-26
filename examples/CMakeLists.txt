find_package(FREEGLUT QUIET)
find_package(GLUT QUIET)
find_package(CUDA QUIET)

# All examples depend on Pangolin GUI
if(BUILD_PANGOLIN_GUI AND (FREEGLUT_FOUND OR GLUT_FOUND OR ANDROID) )
    add_subdirectory(HelloPangolin)
    add_subdirectory(SimpleMultiDisplay)
    add_subdirectory(SimpleDisplayImage)
    add_subdirectory(SimplePlot)

    ## These samples require Pangolin Var support
    if(BUILD_PANGOLIN_VARS)
      add_subdirectory(SimpleDisplay)

      ## Video Samples require Pangolin Video support
      if(BUILD_PANGOLIN_VIDEO)
        add_subdirectory(SimpleVideo)
        add_subdirectory(SimpleRecord)
        add_subdirectory(SimpleRepeatVideo)
      endif()

#      This sample fails on many platforms, so exclude it for now,
#      until we can create a better cmake test for support.
#      ## VBODisplay Sample requires CUDA
#      if( CUDA_FOUND )
#        add_subdirectory(VBODisplay)
#      endif()
    endif()
endif()
