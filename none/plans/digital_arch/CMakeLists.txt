PROJECT(cstock)
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

INCLUDE_DIRECTORIES(/home/chengyake/AMD/AMDAPPSDK-3.0/include/ )

LINK_DIRECTORIES( /home/chengyake/AMD/AMDAPPSDK-3.0/lib/x86_64/ )

#gcc main.c cmd.c matrix.c core.c debug.c core_kernel.cl  -I/home/chengyake/AMD/AMDAPPSDK-3.0/include/       -L/home/chengyake/AMD/AMDAPPSDK-3.0/lib/x86_64/ -lOpenCL  \
#                                                         -I/home/chengyake/project/none/plans/planB/utils/  -L/home/chengyake/project/none/plans/planB/utils -lnone_utils

# g++ HelloWorld.cpp -I /home/chengyake/AMD/AMDAPPSDK-3.0/include/   -L/home/chengyake/AMD/AMDAPPSDK-3.0/lib/x86_64/ -lOpenCL

AUX_SOURCE_DIRECTORY(. DIR_SRCS)
SET(CMAKE_C_FLAGS "-O3 -fopenmp")
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})  



ADD_EXECUTABLE(cstock ${DIR_SRCS})

#TARGET_LINK_LIBRARIES(cstock libOpenCL.so.1 libpthread.so.1)




#ADD_SUBDIRECTORY( utils )
#ADD_SUBDIRECTORY( core )

