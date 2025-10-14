include_directories(
        "${CMAKE_SOURCE_DIR}/source"
)

add_library(aeolus
        SHARED
        source/addsynth.cpp
        source/lfqueue.cpp # array allocation
        source/rankwave.cpp # sample generation
        source/rngen.cpp # random number generation
        source/exp2ap.cpp # power of 2, specific function
        source/asection.cpp # reverb
        source/messages.cpp # messages for inter-process communication
        source/model.cpp # complete model of the entire organ
        source/scales.cpp # tuning scales
        source/slave.cpp # not clear, maybe for tasks that take longer?
        source/division.cpp #Synthesis of a group of registers (a division)
        source/reverb.cpp # The reverb part
        source/audio.cpp # Audio synthesis in general
        source/imidi.cpp
)

find_library( # Sets the name of the path variable.
        android
        #log-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log
)

target_link_libraries(
        aeolus
        clthreads
        log
)
