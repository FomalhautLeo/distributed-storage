project(Config)

# add library
file(GLOB sources CONFIGURE_DEPENDS src/*.cc)
add_library(${PROJECT_NAME} SHARED ${sources})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR})

#install
install(DIRECTORY ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR})