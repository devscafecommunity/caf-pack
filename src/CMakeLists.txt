# caf-pack/src/CMakeLists.txt

# Find dependencies
find_package(PNG REQUIRED)
find_package(ZLIB REQUIRED)
find_package(zstd REQUIRED)

# caf-pack library
add_library(caf-pack-lib
    Packer.cpp
    AssetProcessor.cpp
    TextureProcessor.cpp
    AudioProcessor.cpp
    MeshProcessor.cpp
    HeaderGenerator.cpp
    Reader.cpp
    Registry.cpp
)

target_include_directories(caf-pack-lib 
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(caf-pack-lib
    PUBLIC PNG::PNG ZLIB::ZLIB zstd::libzstd_shared
)

target_compile_definitions(caf-pack-lib PRIVATE HAVE_PNG)

# caf-pack CLI tool
add_executable(caf-pack
    main.cpp
    TerrainConvert.cpp
)

target_link_libraries(caf-pack PRIVATE caf-pack-lib)

# `--convert-terrain` (legacy .cterrain -> native .terrain) shares the engine's container code, so it exists when this
# tree is built inside Caffeine (caffeine-core is defined before caf-pack is added); standalone builds omit it.
if(TARGET caffeine-core)
    target_link_libraries(caf-pack PRIVATE caffeine-core)
    target_compile_definitions(caf-pack PRIVATE CAF_PACK_WITH_TERRAIN=1)
endif()

install(TARGETS caf-pack DESTINATION bin)

