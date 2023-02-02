#指定cmake的最旧版本(版本较低时会报错)
cmake_minimum_required(VERSION 3.10)

#定义项目信息用
project(chat)

#配置编译选项
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -g)
#set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -std=c++11)
set(CMAKE_CXX_STANDARD 11)            #设置C++11
set(CMAKE_CXX_STANDARD_REQUIRED on)

#配置最终的可执行文件输出路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

#头文件搜索路径
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/include/server)
include_directories(${PROJECT_SOURCE_DIR}/include/server/db)
include_directories(${PROJECT_SOURCE_DIR}/include/server/model)
include_directories(${PROJECT_SOURCE_DIR}/include/server/redis)
include_directories(${PROJECT_SOURCE_DIR}/thirdparty)

#静态库路径   (不理解，为什么只要mysql)
link_directories(/usr/lib/mysql)

#加载子目录(子目录下要有CMakeLists.txt文件)
add_subdirectory(src)