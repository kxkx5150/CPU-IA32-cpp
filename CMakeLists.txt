cmake_minimum_required(VERSION 3.12)
project(cpp_app)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -ffast-math -DNDEBUG -s -fmerge-all-constants")
set(CMAKE_CXX_FLAGS_MINSIZEREL "-Os -ffast-math -DNDEBUG -s -fmerge-all-constants")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O3 -ffast-math -DNDEBUG -g -fmerge-all-constants")

link_libraries("-lm -lz")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/exe)
include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${PROJECT_SOURCE_DIR}/src/cpu)

file(GLOB sourcefiles 
"src/*.c"
"src/*.cpp"
"src/cpu/*.cpp"
)
add_executable(${PROJECT_NAME} ${sourcefiles}
)
find_package(OpenGL)
target_link_libraries(${PROJECT_NAME} ${OPENGL_LIBRARIES} SDL2_image SDL2_ttf SDL2 SDL2main)



