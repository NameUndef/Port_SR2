cmake_minimum_required(VERSION 3.20)

add_executable(${LAUNCHER_APP_NAME} "main.cpp")
target_link_libraries(${LAUNCHER_APP_NAME} 
    PRIVATE ${ENGINE_LIB_NAME}
    PUBLIC SDL2::SDL2main 
    # PUBLIC SDL2::SDL2 
    # PUBLIC SDL2_image::SDL2_image 
    # PUBLIC SDL2_mixer::SDL2_mixer 
    # PUBLIC SDL2_net::SDL2_net 
    # PUBLIC SDL2_ttf::SDL2_ttf
    )
install(TARGETS ${LAUNCHER_APP_NAME})
