# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\appkurs_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\appkurs_autogen.dir\\ParseCache.txt"
  "appkurs_autogen"
  )
endif()
