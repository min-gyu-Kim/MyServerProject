add_executable(StressTestClient)
target_sources(StressTestClient PRIVATE
    main.cpp
    PollThread.hpp
    PollThread.cpp
    RingBuffer.hpp
    Session.cpp
    Session.hpp
)
