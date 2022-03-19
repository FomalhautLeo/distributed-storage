project(CacheServer.exe)

# build
add_executable(${PROJECT_NAME} CacheServer.cc)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(${PROJECT_NAME} CacheServer)

#install
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR}/CacheServer)
#install(FILES CacheMonitor.sh DESTINATION ${CMAKE_INSTALL_BINDIR}/CacheServer)