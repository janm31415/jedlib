set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(HDRS
test_assert.h
test_buffer.h
test_edit.h
test_syntax_highlight.h
)
	
set(SRCS
test.cpp
test_assert.cpp
test_buffer.cpp
test_edit.cpp
test_syntax_highlight.cpp
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

add_custom_command(TARGET jedlib.tests POST_BUILD
   COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/../jedlib/jedlib_syntax.json" "$<TARGET_FILE_DIR:jedlib.tests>/jedlib_syntax.json")
