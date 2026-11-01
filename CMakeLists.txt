cmake_minimum_required(VERSION 3.22)
project(MisterArp VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_OSX_DEPLOYMENT_TARGET "12.0" CACHE STRING "" FORCE)

# JUCE 8.0.7 - defaults to the shared copy next to this project (Projects/JUCE)
if(NOT DEFINED JUCE_PATH)
  set(JUCE_PATH "${CMAKE_SOURCE_DIR}/../JUCE" CACHE PATH "Path to JUCE")
endif()
if(NOT EXISTS "${JUCE_PATH}/CMakeLists.txt")
  message(FATAL_ERROR "JUCE not found at ${JUCE_PATH} (git clone https://github.com/juce-framework/JUCE.git --branch 8.0.7)")
endif()
add_subdirectory("${JUCE_PATH}" juce_build)

juce_add_plugin(MisterArp
    COMPANY_NAME             "Tully EDM Vibe"
    COMPANY_COPYRIGHT        "(c)(p) 2026 Tully EDM Vibe"
    COMPANY_EMAIL            "info@tullyedmvibe.com"
    COMPANY_WEBSITE          "https://www.tullyedmvibe.com"
    BUNDLE_ID                com.TullyEDMVibe.MisterArp
    PLUGIN_MANUFACTURER_CODE Tull
    PLUGIN_CODE              Marp
    FORMATS                  AU VST3 Standalone
    PRODUCT_NAME             "Mister Arp"
    IS_SYNTH                 TRUE
    NEEDS_MIDI_INPUT         TRUE
    NEEDS_MIDI_OUTPUT        FALSE
    IS_MIDI_EFFECT           FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    VST3_CATEGORIES          "Instrument|Synth"
    AU_MAIN_TYPE             kAudioUnitType_MusicDevice
)

juce_add_binary_data(MisterArpData SOURCES Resources/TullyLogo.png)

target_sources(MisterArp PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
)

target_compile_definitions(MisterArp PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_link_libraries(MisterArp
    PRIVATE
        MisterArpData
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# Optional self-test + demo-video renderer (cmake -DMA_TESTS=ON)
option(MA_TESTS "Build the self-test and video console apps" OFF)
if(MA_TESTS)
  foreach(t MATest MAVideo)
    juce_add_console_app(${t} PRODUCT_NAME "${t}")
    target_compile_definitions(${t} PRIVATE JucePlugin_Name="Mister Arp" JucePlugin_VersionString="1.0.0" JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0)
    target_link_libraries(${t} PRIVATE MisterArpData juce::juce_audio_utils juce::juce_dsp PUBLIC juce::juce_recommended_config_flags)
  endforeach()
  target_sources(MATest PRIVATE Tests/ArpTest.cpp Source/PluginProcessor.cpp Source/PluginEditor.cpp)
  target_sources(MAVideo PRIVATE Tests/VideoRender.cpp Source/PluginProcessor.cpp Source/PluginEditor.cpp)
endif()
