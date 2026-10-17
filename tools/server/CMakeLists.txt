include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

# server-context containing the core server logic, used by llama-server and CLI

set(TARGET server-context)

add_library(${TARGET} STATIC
    server-chat.cpp
    server-chat.h
    server-task.cpp
    server-task.h
    server-queue.cpp
    server-queue.h
    server-common.cpp
    server-common.h
    server-context.cpp
    server-context.h
    server-tools.cpp
    server-tools.h
)

if (BUILD_SHARED_LIBS)
    set_target_properties(${TARGET} PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()

target_include_directories(${TARGET} PRIVATE ../mtmd)
target_include_directories(${TARGET} PRIVATE ../omni)
target_include_directories(${TARGET} PRIVATE ../omni/voxcpm2)
target_include_directories(${TARGET} PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(${TARGET} PRIVATE llama-common omni mtmd voxcpm2_runtime ${CMAKE_THREAD_LIBS_INIT})

# llama-omni-server executable (standalone omni HTTP API)

if (LLAMA_OPENSSL)
    find_package(OpenSSL)
endif()

set(TARGET_OMNI llama-omni-server)

add_executable(${TARGET_OMNI}
    server-omni.cpp
    session.h
    session.cpp
    protocol.h
    protocol.cpp
    ws_handler.h
    ws_handler.cpp
)
install(TARGETS ${TARGET_OMNI} RUNTIME)

target_link_libraries(${TARGET_OMNI} PRIVATE omni llama-common cpp-httplib ${CMAKE_THREAD_LIBS_INIT})
if (LLAMA_OPENSSL AND OpenSSL_FOUND)
    target_link_libraries(${TARGET_OMNI} PRIVATE OpenSSL::SSL OpenSSL::Crypto)
    target_compile_definitions(${TARGET_OMNI} PRIVATE CPPHTTPLIB_OPENSSL_SUPPORT)
endif()
target_compile_features(${TARGET_OMNI} PRIVATE cxx_std_17)
target_include_directories(${TARGET_OMNI} PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories(${TARGET_OMNI} PRIVATE ${CMAKE_SOURCE_DIR}/vendor/cpp-httplib)
target_include_directories(${TARGET_OMNI} PRIVATE ${CMAKE_SOURCE_DIR}/vendor)

# llama-tts-server executable (standalone VoxCPM2 TTS HTTP API)

set(TARGET_VOXCPM2 llama-tts-server)

add_executable(${TARGET_VOXCPM2} server-voxcpm2.cpp)
install(TARGETS ${TARGET_VOXCPM2} RUNTIME)

target_link_libraries(${TARGET_VOXCPM2} PRIVATE voxcpm2_runtime llama-common cpp-httplib ${CMAKE_THREAD_LIBS_INIT})
if (LLAMA_OPENSSL AND OpenSSL_FOUND)
    target_link_libraries(${TARGET_VOXCPM2} PRIVATE OpenSSL::SSL OpenSSL::Crypto)
    target_compile_definitions(${TARGET_VOXCPM2} PRIVATE CPPHTTPLIB_OPENSSL_SUPPORT)
endif()
target_compile_features(${TARGET_VOXCPM2} PRIVATE cxx_std_17)
target_include_directories(${TARGET_VOXCPM2} PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories(${TARGET_VOXCPM2} PRIVATE ${CMAKE_SOURCE_DIR}/vendor/cpp-httplib)
target_include_directories(${TARGET_VOXCPM2} PRIVATE ${CMAKE_SOURCE_DIR}/vendor)

# llama-server executable (standard OpenAI-compatible chat server, real SSE streaming)
# 恢复上游 b9d15b8 误删的 chat server target。
# 注意：server.cpp 是自包含的单体版本（自带 main()，内部完整定义 server_task /
# server_context 等全部结构，仅 include utils.hpp），与 server-context 静态库
# (server-context.cpp 等拆分版) 是两套并存的实现。二者一起链接会产生大量
# 重复符号。因此本 target 只编 server.cpp，不链 server-context/server-http/
# server-models，直接依赖底层库 llama-common/omni/mtmd/voxcpm2_runtime/cpp-httplib。
set(TARGET_SERVER llama-server)

add_executable(${TARGET_SERVER}
    server.cpp
    builtin-tools.cpp
)
target_include_directories(${TARGET_SERVER} PRIVATE ${CMAKE_SOURCE_DIR}/vendor)
install(TARGETS ${TARGET_SERVER} RUNTIME)

target_link_libraries(${TARGET_SERVER} PRIVATE
    llama-common omni mtmd voxcpm2_runtime cpp-httplib ${CMAKE_THREAD_LIBS_INIT})
if (LLAMA_OPENSSL AND OpenSSL_FOUND)
    target_link_libraries(${TARGET_SERVER} PRIVATE OpenSSL::SSL OpenSSL::Crypto)
    target_compile_definitions(${TARGET_SERVER} PRIVATE CPPHTTPLIB_OPENSSL_SUPPORT)
endif()
target_compile_features(${TARGET_SERVER} PRIVATE cxx_std_17)
target_include_directories(${TARGET_SERVER} PRIVATE ../mtmd)
target_include_directories(${TARGET_SERVER} PRIVATE ../omni)
target_include_directories(${TARGET_SERVER} PRIVATE ../omni/voxcpm2)
target_include_directories(${TARGET_SERVER} PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories(${TARGET_SERVER} PRIVATE ${CMAKE_SOURCE_DIR}/vendor/cpp-httplib)
target_include_directories(${TARGET_SERVER} PRIVATE ${CMAKE_SOURCE_DIR}/vendor)
# 生成的 WebUI 资源头(index.html.gz.hpp / loading.html.hpp)由 build/generated 提供
target_include_directories(${TARGET_SERVER} PRIVATE ${CMAKE_BINARY_DIR}/generated)
