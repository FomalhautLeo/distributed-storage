project(Client)

# build
file(GLOB sources src/*.cc)
add_library(${PROJECT_NAME} SHARED ${sources})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(${PROJECT_NAME} Config)

# add main ELF
add_subdirectory(main)

#install
install(DIRECTORY ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR})