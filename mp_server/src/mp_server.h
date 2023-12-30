#pragma once
#include "server.h"
#include "client.h"

namespace mp_server
{
	class mp_server
	{
	public:
		void create();
		void setPort(int port) { m_server.m_port = port;};
		static void processchat(std::string msg, int id);
		static void processhello(std::string msg, int id, const Client& client);
		static void processattack(std::string msg, int id, const Client& client);
		static void processmove(std::string msg, int id, const Client& client, const std::string& delim);
		static std::string processSingleMoveCommand(const std::string& command, const std::string& term);
		static std::string processSingleAttackCommand(const std::string& command, const std::string& term);
		static void processping(const Client& client);
		static void reset();
	private:
		inline static TcpServer m_server;
		server_observer_t observer_clients, observer_administration;
	};
}