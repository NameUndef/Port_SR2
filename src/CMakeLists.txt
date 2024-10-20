
list(APPEND SRC
  "main.cpp")
list(APPEND SRC ${IMGUI_CPP_FILES_LIST})

add_executable(${PROJECT_NAME} ${SRC})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wfloat-equal -Wno-error=float-equal -Wswitch-default -Wno-error=switch-default -Wno-error=zero-as-null-pointer-constant -Wzero-as-null-pointer-constant")

target_link_libraries(${PROJECT_NAME} PUBLIC 
SDL2::SDL2main 
SDL2::SDL2 
SDL2_image::SDL2_image 
SDL2_mixer::SDL2_mixer 
SDL2_net::SDL2_net 
SDL2_ttf::SDL2_ttf
nlohmann_json::nlohmann_json
flecs::flecs
lua
${SDL_GPU_LIBRARY})

install(TARGETS ${PROJECT_NAME})
