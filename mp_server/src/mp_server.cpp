#pragma once
#include "mp_server.h"
#include "log.h"
#include <thread>
#include <chrono>

namespace mp_server
{
	void onIncomingAdmin(const Client& client, const char* msg, size_t size)
	{
		_CORE_INFO("Administrator sent some message");
	}
	void onIncomingClient(const Client& client, const char* msg, size_t size)
	{
		_CORE_INFO("Client sent some message");
	}
	void onDisconnect(const Client& client)
	{
		_CORE_INFO("Client {0} disconnected. {1}", client.getIp(), client.getErrorMessage());
	}
	int mp_server::m_port;
	void mp_server::create()
	{
		m_port = 8080;
		pipe_ret_t runServer = m_server.start(m_port);
		if (runServer.success)
		{
			_CORE_INFO("Server setup successful");
		}
		else
		{
			_CORE_ERROR("Server setup failed with error: {0}", runServer.msg);
			return;
		}
		//register observer/listener
		observer_administration.incoming_packet_func = onIncomingAdmin;
		observer_clients.incoming_packet_func = onIncomingClient;
		observer_administration.disconnected_func = onDisconnect;
		observer_clients.disconnected_func = onDisconnect;
		observer_administration.wantedIp = "127.0.0.1"; // Only allow local administration
		observer_clients.wantedIp = ""; // Allow any ip for client connections
		m_server.subscribe(observer_administration);
		m_server.subscribe(observer_clients);
		//Server loop
		while (true)
		{
			Client client = m_server.acceptClient(0);
			if (client.isConnected())
			{
				_CORE_INFO("Client with IP '{0}' connected.", client.getIp());
				m_server.printClients();
			}
			else
			{
				_CORE_ERROR("Client connection failed ({0})", client.getErrorMessage());
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		}
	}
	
}