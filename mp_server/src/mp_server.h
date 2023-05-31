#pragma once
#include "server.h";
#include "client.h";

namespace mp_server
{
	class mp_server
	{
	public:
		void create();
		static void setPort(int port) { m_port = port;};
		static void processchat(const char* msg, int id);
		static void processhello(const char* msg, int id);
		static void processping(const Client& client);
	private:
		inline static TcpServer m_server;
		static int m_port;
		server_observer_t observer_clients, observer_administration;
		static int playerone;
		static int playertwo;
	};
}