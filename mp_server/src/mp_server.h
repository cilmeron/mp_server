#pragma once
#include "server.h";
#include "client.h";

namespace mp_server
{
	class mp_server
	{
	public:
		void create();
		static void setPort(int port) { m_port = port; };
	private:
		TcpServer m_server;
		static int m_port;
		server_observer_t observer_clients, observer_administration;
	};
}