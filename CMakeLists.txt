cmake_minimum_required(VERSION 3.27)

project(smallsh C)

# C standard
set(CMAKE_C_STANDARD 11)

# GNU extensions (for asprintf)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_GNU_SOURCE")

# specify the gettext include and library directories
include_directories(/opt/homebrew/opt/gettext/include)
link_directories(/opt/homebrew/opt/gettext/lib)

# Add the executable for smallsh
add_executable(smallsh main.c)

# link with the libintl library
target_link_libraries(smallsh intl)
