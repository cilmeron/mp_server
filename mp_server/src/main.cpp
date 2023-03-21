#include "mp_server.h"
#include "log.h"

int main(int argc, char* argv[])
{
    //Set Locale so we can use unicode in chat messages
    //setlocale(LC_ALL, "en_US.UTF-8");
    mp_server::Log::Init();
    _CORE_INFO("Server starting up.");
    mp_server::mp_server *server = new mp_server::mp_server();
    server->setPort(8042);
    server->create();
    return 0;
}



