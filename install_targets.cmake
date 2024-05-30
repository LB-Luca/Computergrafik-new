function(install_target TARGET_NAME)

    # Define the installation directory for executables and files
    set(INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/${TARGET_NAME}")

    # Startup bat file
    set(EXECUTABLE "${TARGET_NAME}")
    set(PARAMETERS "../assets shaders")

    if (WIN32)
        configure_file(${CMAKE_SOURCE_DIR}/start_target.bat.in ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}.bat @ONLY)
    elseif(UNIX)
        configure_file(${CMAKE_SOURCE_DIR}/start_target.sh.in ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}.sh @ONLY)                
    endif()

    # Install the executable files
    install(TARGETS ${TARGET_NAME}
        RUNTIME DESTINATION ${INSTALL_DIR}
    )

    # Install the batch or sh file
    if (WIN32)
        set(START_SCRIPT "${TARGET_NAME}.bat")
    elseif(UNIX)
        set(START_SCRIPT "${TARGET_NAME}.sh")
    endif()

    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${START_SCRIPT}
        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE WORLD_EXECUTE
        DESTINATION ${INSTALL_DIR}
    )

    # Install shaders
    set(SHADER_DIR "${CMAKE_SOURCE_DIR}/${TARGET_NAME}/shaders")
    install(DIRECTORY ${SHADER_DIR}
        DESTINATION ${INSTALL_DIR}        
    )

endfunction()