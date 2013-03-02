# Find Pangolin (https://github.com/stevenlovegrove/Pangolin)
FIND_PACKAGE(Pangolin REQUIRED)
INCLUDE_DIRECTORIES(${Pangolin_INCLUDE_DIRS})
LINK_DIRECTORIES(${Pangolin_LIBRARY_DIRS})
LINK_LIBRARIES(${Pangolin_LIBRARIES})

ADD_EXECUTABLE(SimpleRecord main.cpp)

# In your own code, you can ommit these lines
IF( ${PANGOLIN_EXAMPLE} )
  add_dependencies(SimpleRecord ${LIBRARY_NAME})
ENDIF()
