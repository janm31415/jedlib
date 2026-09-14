
set(HDRS
buffer.h
file_utils.h
namespace.h
utf8.h
utils.h
)
	
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SRCS
buffer.cpp
file_utils.cpp
utils.cpp
)

# general build definitions
add_definitions(-D_SCL_SECURE_NO_WARNINGS)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

add_definitions(-DMEMORY_LEAK_TRACKING)

add_library(jedlib STATIC ${HDRS} ${SRCS})
source_group("Header Files" FILES ${hdrs})
source_group("Source Files" FILES ${srcs})

target_include_directories(jedlib 
  PUBLIC
	  ${CMAKE_CURRENT_SOURCE_DIR}/../cpp-rrb
    )
	
target_link_libraries(jedlib
    PUBLIC
    )
	
