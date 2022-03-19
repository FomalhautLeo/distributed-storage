## generate
configure_file("${PROJECT_SOURCE_DIR}/cmake/storage-setup.sh.in" "${PROJECT_BINARY_DIR}/storage-setup.sh" @ONLY)

## install
install(FILES "${PROJECT_BINARY_DIR}/storage-setup.sh" DESTINATION ${CMAKE_INSTALL_BINDIR} RENAME setup.sh)