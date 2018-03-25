add_executable(fsm_minimal minimal.cpp)

target_link_libraries(fsm_minimal PUBLIC CppAIKit::CppAIKit)

target_compile_features(fsm_minimal PUBLIC cxx_std_17)

if (MSVC)
    target_compile_options(fsm_minimal PRIVATE /FAs /W4 /WX)
    target_compile_options(fsm_minimal PRIVATE /wd4309 /wd4324 /wd4389 /wd4127 /wd4267 /wd4146 /wd4201 /wd4464 /wd4514 /wd4701 /wd4820 /wd4365)
    target_compile_definitions(fsm_minimal PRIVATE _CRT_SECURE_NO_WARNINGS)
else ()
    target_compile_options(fsm_minimal PRIVATE
            -Wall -Wextra -Wpedantic
            -Wcast-align -Wconversion -Wold-style-cast -Wsign-conversion
            -Wredundant-decls -Wunused -Wno-missing-braces -Wshadow
            -Woverloaded-virtual -Wctor-dtor-privacy -Wnon-virtual-dtor
            -Winit-self -Werror=return-type -Wmissing-declarations -Wzero-as-null-pointer-constant -Wdouble-promotion
            -fno-exceptions -fno-strict-aliasing -fvisibility=hidden)
endif ()
