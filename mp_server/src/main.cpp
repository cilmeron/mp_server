#include "mp_server.h"
#include "log.h"

int main(int argc, char* argv[])
{
    //Set Locale so we can use unicode in chat messages
    setlocale(LC_ALL, "en_US.UTF-8");
    mp_server::Log::Init();
    mp_server::mp_server::create();
    //std::cout << "我" << std::endl; Test Unicode
    _CORE_INFO("Can we see this character: 我?");
    //m_StartTime = std::chrono::steady_clock::now();
    //Let's start the server on a specific port
    return 0;
}



