# Create a static library called EventBuilderCore
add_library(EventBuilderCore STATIC)

# Specify the include directories for the EventBuilderCore library.
# SYSTEM PUBLIC means that these directories are treated as system headers and will propagate to targets that link this library.
target_include_directories(EventBuilderCore
    SYSTEM PUBLIC ../../vendor/spdlog/include   # Include directory for spdlog (third-party logging library)
                  ${ROOT_INCLUDE_DIRS}       # ROOT library include directories
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}          # Include the current source directory
           ${CMAKE_CURRENT_SOURCE_DIR}/../    # Include the parent directory of the current source
           ../vendor/yaml-cpp/include/       # Include directory for yaml-cpp (YAML parser library)
)

# Precompile headers for the EventBuilderCore library.
# This will precompile the specified header file (EventBuilder.h) to speed up compilation.
target_precompile_headers(EventBuilderCore PRIVATE ../EventBuilder.h)

# Specify the source files for the EventBuilderCore library.
# These files will be compiled into the static library.
target_sources(EventBuilderCore PRIVATE
    ChannelMap.cpp
    CompassRun.h
    FlagHandler.cpp
    MassLookup.h
    SFPAnalyzer.h
    Stopwatch.cpp
    ChannelMap.h
    CutHandler.cpp
    FlagHandler.h
    OrderChecker.cpp
    SFPPlotter.cpp
    Stopwatch.h
    CutHandler.h
    FP_kinematics.cpp
    OrderChecker.h
    SFPPlotter.h
    CompassFile.cpp
    EVBApp.cpp
    FP_kinematics.h
    ProgressCallback.h
    ShiftMap.cpp
    CompassFile.h
    EVBApp.h
    Logger.cpp
    ShiftMap.h
    CompassHit.h
    FastSort.cpp
    Logger.h
    SlowSort.cpp
    CompassRun.cpp
    FastSort.h
    MassLookup.cpp
    SFPAnalyzer.cpp
    SlowSort.h
    EVBWorkspace.cpp
    EVBWorkspace.h
    EVBParameters.h
)

# Link libraries to the EventBuilderCore library.
# These libraries are needed during the linking stage to ensure that necessary symbols are included.
target_link_libraries(EventBuilderCore PUBLIC
    SPSDict            # Link the SPSDict library
    ${ROOT_LIBRARIES}  # Link ROOT libraries (e.g., core, hist, etc.)
    yaml-cpp           # Link yaml-cpp library (for handling YAML files)
)

# Set properties for the EventBuilderCore library, specifying the output directory for the archive (static library) file.
set_target_properties(EventBuilderCore PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${EVB_LIBRARY_DIR})

# Define preprocessor macro YAML_CPP_STATIC_DEFINE to indicate that yaml-cpp is being used as a static library.
target_compile_definitions(EventBuilderCore PRIVATE YAML_CPP_STATIC_DEFINE)
