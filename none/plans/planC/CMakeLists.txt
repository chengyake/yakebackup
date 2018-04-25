PROJECT(none)
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/core ${PROJECT_SOURCE_DIR}/utils /home/chengyake/AMD/AMDAPPSDK-3.0/include/ )

LINK_DIRECTORIES( /home/chengyake/AMD/AMDAPPSDK-3.0/lib/x86_64/  ${PROJECT_SOURCE_DIR}/out/lib )

#gcc main.c cmd.c matrix.c core.c debug.c core_kernel.cl  -I/home/chengyake/AMD/AMDAPPSDK-3.0/include/       -L/home/chengyake/AMD/AMDAPPSDK-3.0/lib/x86_64/ -lOpenCL  \
#                                                         -I/home/chengyake/project/none/plans/planB/utils/  -L/home/chengyake/project/none/plans/planB/utils -lnone_utils

# g++ HelloWorld.cpp -I /home/chengyake/AMD/AMDAPPSDK-3.0/include/   -L/home/chengyake/AMD/AMDAPPSDK-3.0/lib/x86_64/ -lOpenCL



ADD_SUBDIRECTORY( utils )

ADD_SUBDIRECTORY( core )

