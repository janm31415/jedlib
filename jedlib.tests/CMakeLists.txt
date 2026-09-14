set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(HDRS
test_assert.h
test_buffer.h
test_edit.h
)
	
set(SRCS
test.cpp
test_assert.cpp
test_buffer.cpp
test_edit.cpp
)

# general build definitions
add_definitions(-D_SCL_SECURE_NO_WARNINGS)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

add_definitions(-DMEMORY_LEAK_TRACKING)

add_executable(jedlib.tests ${HDRS} ${SRCS})
source_group("Header Files" FILES ${hdrs})
source_group("Source Files" FILES ${srcs})

target_include_directories(jedlib.tests
    PRIVATE
	  ${CMAKE_CURRENT_SOURCE_DIR}/../
    )
	
target_link_libraries(jedlib.tests
    PRIVATE   
    jedlib 
    )
