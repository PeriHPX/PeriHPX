# Copyright (c) 2019 Prashant K. Jha
# Copyright (c) 2019 Patrick Diehl
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

##############################################################################
# Quasi-static tests
##############################################################################

# 1D
add_test(NAME quasistatic.1D.elastic.mesh 
    COMMAND ${EXECUTABLE_OUTPUT_PATH}/mesh -i input_mesh.yaml -d 1
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/examples/qsModel/1D
)

add_test(NAME quasistatic.1D.elastic
COMMAND ${EXECUTABLE_OUTPUT_PATH}/PeriHPX -i input.yaml --hpx:threads=1
WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/examples/qsModel/1D
)

add_test(NAME quasistatic.1D.elastic.compare
COMMAND ${EXECUTABLE_OUTPUT_PATH}/dc -i input_compare.yaml -k fd_simple
WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/examples/qsModel/1D
)

# Depencies
set_tests_properties(quasistatic.1D.elastic PROPERTIES
  FIXTURES_SETUP quasistatic.1D.elastic.mesh)

set_tests_properties(quasistatic.1D.elastic.compare PROPERTIES
  FIXTURES_SETUP quasistatic.1D.elastic)

# The implicit test tooks too long for circle-ci and can only be run
# on desktops or clusters.
if(Enable_Expensive_Tests)

# 2D
add_test(NAME quasistatic.2D.elastic.mesh 
    COMMAND ${EXECUTABLE_OUTPUT_PATH}/mesh -i input_mesh.yaml -d 2
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/examples/qsModel/2D
)

add_test(NAME quasistatic.2D.elastic
COMMAND ${EXECUTABLE_OUTPUT_PATH}/PeriHPX -i input.yaml --hpx:threads=5
WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/examples/qsModel/2D
)

add_test(NAME quasistatic.2D.elastic.compare
COMMAND ${EXECUTABLE_OUTPUT_PATH}/dc -i input_compare.yaml -k fd_simple
WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/examples/qsModel/2D
)

endif()

##############################################################################
# Tools
##############################################################################

# Compare tool
add_test(NAME tools.dc.fd_simple 
    COMMAND ${EXECUTABLE_OUTPUT_PATH}/dc -i input.yaml -k fd_simple
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/examples/tools/dc/fd_simple
)

# Generate 2D mesh
add_test(NAME tools.mesh.2d
    COMMAND ${EXECUTABLE_OUTPUT_PATH}/mesh -i input_mesh.yaml -d 2
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/examples/tools/mesh/uniform_triangle
)



##############################################################################
# IO
##############################################################################

# Test the legacy vtk writer 
add_test(NAME quasistatic.1D.elastic_legacy_vtk
COMMAND ${EXECUTABLE_OUTPUT_PATH}/PeriHPX -i input_legacy_vtk.yaml --hpx:threads=1
WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/examples/qsModel/1D
)

# Depencies
set_tests_properties(quasistatic.1D.elastic_legacy_vtk PROPERTIES
  FIXTURES_SETUP quasistatic.1D.elastic.mesh)

# Read 2D traingle mesh generate with gmsh 2
add_test(NAME mesh.2d.read.triangle.gmsh_2
    COMMAND ${EXECUTABLE_OUTPUT_PATH}/PeriHPX -i input_2d_gmsh_triangle.yaml --hpx:threads=1
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/examples/io
)

# Read 2D quad mesh generate with gmsh 2
add_test(NAME mesh.2d.read.quad.gmsh_2
    COMMAND ${EXECUTABLE_OUTPUT_PATH}/PeriHPX -i input_2d_gmsh_quad.yaml --hpx:threads=1
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/examples/io
)

# Read 2D quad mesh withan intitial crack and output the crack
add_test(NAME mesh.2d.read.quad.gmsh-crack
    COMMAND ${EXECUTABLE_OUTPUT_PATH}/PeriHPX -i input_2d_gmsh_quad.yaml --hpx:threads=1
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/examples/io
)



##############################################################################
# Restart
##############################################################################

add_test(NAME explicit.restart.mesh
    COMMAND ${EXECUTABLE_OUTPUT_PATH}/mesh -i input_mesh.yaml -d 2
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/examples/fdModel/restart
)

add_test(NAME explicit.restart.run
COMMAND ${EXECUTABLE_OUTPUT_PATH}/PeriHPX -i input.yaml --hpx:threads=1
WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/examples/fdModel/restart
)

add_test(NAME explicit.restart.restart
COMMAND ${EXECUTABLE_OUTPUT_PATH}/PeriHPX -i restart.yaml --hpx:threads=1
WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/examples/fdModel/restart
)


add_test(NAME explicit.restart.compare
COMMAND ${EXECUTABLE_OUTPUT_PATH}/dc -i input_compare.yaml -k fd_simple
WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/examples/fdModel/restart
)


# Depencies
set_tests_properties(explicit.restart.run PROPERTIES
  FIXTURES_SETUP explicit.restart.mesh)

set_tests_properties(explicit.restart.restart PROPERTIES
  FIXTURES_SETUP explicit.restart.run)

  set_tests_properties(explicit.restart.compare PROPERTIES
  FIXTURES_SETUP explicit.restart.restart)