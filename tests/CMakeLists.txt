# Include from a host that supplies Glade's dependencies and its glade target.
add_executable(glade_sprite_cpu_test SpriteAtlasTest.cpp)
target_compile_features(glade_sprite_cpu_test PRIVATE cxx_std_20)
target_include_directories(glade_sprite_cpu_test PRIVATE ../include)

if(TARGET glade AND TARGET glfw)
    add_executable(glade_sprite_render_test SpriteRenderTest.cpp)
    target_compile_features(glade_sprite_render_test PRIVATE cxx_std_20)
    target_compile_definitions(glade_sprite_render_test PRIVATE GLFW_INCLUDE_NONE GLEW_STATIC)
    target_link_libraries(glade_sprite_render_test PRIVATE glade glfw)
    if(APPLE)
        target_link_libraries(glade_sprite_render_test PRIVATE
            "-framework Cocoa" "-framework IOKit" "-framework CoreFoundation")
    endif()
    add_custom_target(glade_sprite_test_assets
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/../res/shaders/gl"
            "$<TARGET_FILE_DIR:glade_sprite_render_test>/sprite-test-assets/shaders/gl")
    add_dependencies(glade_sprite_render_test glade_sprite_test_assets)
endif()

if(TARGET glade)
    add_executable(glade_atlas_reader_test GladeTexAtlasReaderTest.cpp)
    target_compile_features(glade_atlas_reader_test PRIVATE cxx_std_20)
    target_link_libraries(glade_atlas_reader_test PRIVATE glade)
endif()

if(RAPIDJSON_INCLUDE_DIR)
    add_executable(glade_atlas_metadata_test GladeTexAtlasMetadataTest.cpp
        ../src/util/GladeTexAtlasReader.cpp)
    target_compile_features(glade_atlas_metadata_test PRIVATE cxx_std_20)
    target_include_directories(glade_atlas_metadata_test PRIVATE ../include "${RAPIDJSON_INCLUDE_DIR}")
endif()
