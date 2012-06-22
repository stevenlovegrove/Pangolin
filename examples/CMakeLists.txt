ADD_SUBDIRECTORY(SimpleMultiDisplay)

## These samples require Pangolin Var support
IF(BUILD_PANGOLIN_VARS)
  ADD_SUBDIRECTORY(SimpleDisplay)
  ADD_SUBDIRECTORY(SimpleDisplayImage)
  ADD_SUBDIRECTORY(SimplePlot)

  ## Video Samples require Pangolin Video support
  IF(BUILD_PANGOLIN_VIDEO)
    ADD_SUBDIRECTORY(SimpleVideo)
    ADD_SUBDIRECTORY(SimpleRecord)
    ADD_SUBDIRECTORY(SimpleRepeatVideo)
  ENDIF()

  ADD_SUBDIRECTORY(VBODisplay)
ENDIF()
