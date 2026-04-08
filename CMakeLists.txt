cmake_minimum_required(VERSION 3.20)
project(snakelius VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# =============== 寻找包 (系统级) ===============
find_package(SDL3 REQUIRED CONFIG)
find_package(SDL3_image REQUIRED CONFIG)
find_package(SDL3_ttf REQUIRED CONFIG)
find_package(OpenGL REQUIRED)
find_package(imgui REQUIRED CONFIG)

# =============== 源文件列表 ===============
set(SOURCES
    main.cpp
    # GameDestroy.cpp
    # GameInit.cpp
    # GameRun.cpp
    # Resources.cpp
    # Render.cpp
)

# =============== 可执行文件 ===============
add_executable(main ${SOURCES})

# =============== 包含目录 ===============
target_include_directories(main PRIVATE ${CMAKE_SOURCE_DIR})

# =============== 链接库 ===============
target_link_libraries(main PRIVATE 
    SDL3::SDL3
    SDL3_image::SDL3_image
    SDL3_ttf::SDL3_ttf
    OpenGL::GL
    imgui::imgui
)

# =============== 复制 Assets 文件夹 ===============
set(ASSETS_SOURCE_DIR ${CMAKE_SOURCE_DIR}/Assets)
set(ASSETS_BINARY_DIR ${CMAKE_BINARY_DIR}/Assets)
if(EXISTS ${ASSETS_SOURCE_DIR})
    add_custom_target(copy_assets ALL
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${ASSETS_BINARY_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${ASSETS_SOURCE_DIR} ${ASSETS_BINARY_DIR}
        COMMENT "Copying Assets directory to build folder"
    )
    add_dependencies(main copy_assets)
else()
    message(WARNING "Assets directory not found at ${ASSETS_SOURCE_DIR}")
endif()
