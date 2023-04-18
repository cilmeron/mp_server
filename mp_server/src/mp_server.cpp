#pragma once
#include "mp_server.h"
#include "log.h"
#include <thread>
#include <chrono>
#include <sstream>
#include <string>

namespace mp_server
{
	void onIncomingAdmin(const Client& client, const char* msg, size_t size)
	{
		_CORE_INFO("Administrator sent some message");
	}
	void onIncomingClient(const Client& client, const char* msg, size_t size)
	{
		int lclid = client.getId();
		_CORE_INFO("Client {0} sent some message", lclid);
		char* rmsg;
		const char s[2] = "\n";
		rmsg = strtok(const_cast<char*>(msg), s);
		if (rmsg[0] == 'T')
		{
			mp_server::processchat(rmsg, lclid);
		}
		else if (rmsg[0] == 'P')
		{
			mp_server::processping(client);
			
		}
	}
	void mp_server::processping(const Client& client)
	{
		std::string reply = "PONG:|";
		const char* repl = reply.c_str();
		m_server.sendToClient(client, repl, strlen(repl));
	}
	void mp_server::processchat(const char* msg, int id)
	{
		std::stringstream data(msg);
		std::string l;
		std::string name = "Player" + std::to_string(id);
		int cc = 0;
		while (std::getline(data, l, ':'))
		{
			if (cc == 0)
			{

			}
			else
			{
				std::string reply;
				reply = "T:" + name + ":" + l + ":|";
				const char* repl = reply.c_str();
				m_server.sendToAllClients(repl, strlen(repl));
			}
			cc++;
		}

	}
	void onDisconnect(const Client& client)
	{
		_CORE_INFO("Client {0} (ip:{1}) disconnected. {2}", client.getId(), client.getIp(), client.getErrorMessage());
	}
	int mp_server::m_port;
	void mp_server::create()
	{
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