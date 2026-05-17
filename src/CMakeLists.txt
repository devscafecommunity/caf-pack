# caf-pack/src/CMakeLists.txt

# Find dependencies
find_package(PNG REQUIRED)
find_package(ZLIB REQUIRED)

# caf-pack library
add_library(caf-pack-lib
    Packer.cpp
    AssetProcessor.cpp
    TextureProcessor.cpp
    AudioProcessor.cpp
)

target_include_directories(caf-pack-lib 
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(caf-pack-lib
    PUBLIC PNG::PNG ZLIB::ZLIB
)

target_compile_definitions(caf-pack-lib PRIVATE HAVE_PNG)

# caf-pack CLI tool
add_executable(caf-pack
    main.cpp
)

target_link_libraries(caf-pack PRIVATE caf-pack-lib)

install(TARGETS caf-pack DESTINATION bin)

