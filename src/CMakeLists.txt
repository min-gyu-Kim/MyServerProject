# TODO: windows 환경일 떄 추가
if(UNIX AND NOT APPLE)
    add_subdirectory(lib-core)
else(WINDOWS)
    add_subdirectory(exe-StressTest)
endif()
