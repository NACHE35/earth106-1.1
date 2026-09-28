cmake_minimum_required(VERSION 3.22)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")   # runtime statique : pas besoin de VC++ Redistributable
project(Earth106 VERSION 1.1.0)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.4 GIT_SHALLOW ON)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(Earth106
    COMPANY_NAME "NACHE"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE Nach
    PLUGIN_CODE Er06
    FORMATS VST3
    VST3_CATEGORIES Instrument Synth
    PRODUCT_NAME "Earth 106")

target_sources(Earth106 PRIVATE
    Source/PluginProcessor.cpp Source/PluginEditor.cpp)

target_compile_definitions(Earth106 PUBLIC
    JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)

# Police "New Rocker" (OFL) pour "by NACHE" : téléchargée par le workflow, optionnelle
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/Assets/NewRocker-Regular.ttf)
    juce_add_binary_data(EarthAssets SOURCES Assets/NewRocker-Regular.ttf)
    target_link_libraries(Earth106 PRIVATE EarthAssets)
    target_compile_definitions(Earth106 PRIVATE HAS_FONT=1)
endif()

target_link_libraries(Earth106 PRIVATE
    juce::juce_audio_utils
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_lto_flags)
