FIND_PACKAGE(FREEGLUT QUIET)
FIND_PACKAGE(GLUT QUIET)

IF(FREEGLUT_FOUND OR GLUT_FOUND)
    ADD_SUBDIRECTORY(SimpleMultiDisplay)
    ADD_SUBDIRECTORY(SimpleDisplayImage)
    ADD_SUBDIRECTORY(SimplePlot)

    ## These samples require Pangolin Var support
    IF(BUILD_PANGOLIN_VARS)
      ADD_SUBDIRECTORY(SimpleDisplay)

      ## Video Samples require Pangolin Video support
      IF(BUILD_PANGOLIN_VIDEO)
        ADD_SUBDIRECTORY(SimpleVideo)
        ADD_SUBDIRECTORY(SimpleRecord)
        ADD_SUBDIRECTORY(SimpleRepeatVideo)
      ENDIF()

      ADD_SUBDIRECTORY(VBODisplay)
    ENDIF()
ENDIF()
