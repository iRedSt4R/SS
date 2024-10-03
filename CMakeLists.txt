cmake_minimum_required(VERSION 3.10)
project(GeneralArcadeSS)

add_executable(GeneralArcadeSS WIN32 ScreenSaver.cpp)

set_target_properties(GeneralArcadeSS PROPERTIES OUTPUT_NAME "GeneralArcadeSS.scr")

# Optimization flags for smaller executable
if(MSVC)
    target_compile_options(GeneralArcadeSS PRIVATE /O1 /Os /GL /RTC-)
    target_link_options(GeneralArcadeSS PRIVATE /LTCG /OPT:REF /OPT:ICF)
endif()