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
		try
		{
			int lclid = client.getId();
			_CORE_INFO("Client {0} sent some message: {1}", lclid, msg);
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
			else if (rmsg[0] == 'H')
			{
				mp_server::processhello(rmsg, lclid, client);
			}
		}
		catch (...)
		{
			_CORE_WARN("Error_W");
		}
	}
	std::string mp_server::playeronestring;
	std::string mp_server::playertwostring;

	void mp_server::processhello(const char* msg, int id, const Client& client)
	{
		_CORE_INFO("Processing HELLO");
		std::stringstream data(msg);
		std::string l;
		int cc = 0;
		while (std::getline(data, l, ':'))
		{
			try
			{
				if (cc == 0)
				{

				}
				else
				{
					bool reconnect = false;
					std::string pkinds = "";
					int playerkind = m_server.findPlayer(l);
					if (playerkind != 4)
					{
						_CORE_INFO("Player has reconnected and old connection has been dropped, updating info");
						Client* c = m_server.getClient(id);
						c->setPlayerKind(playerkind);
						if (playerkind == 1)
						{
							playerone = id;
							pkinds = "A";
						}
						else if (playerkind == 2)
						{
							playertwo = id;
							pkinds = "B";
						}
						else
						{
							pkinds = "G";
						}
						reconnect = true;
					}
					else
					{
						//No old connections but that doesn't mean that we don't have a reconnection let's check against string save
						if (playeronestring == l)
						{
							playerone = id;
							pkinds = "A";
							reconnect = true;
						}
						else if (playertwostring == l)
						{
							playertwo = id;
							pkinds = "B";
							reconnect = true;
						}
					}
					if (reconnect == true)
					{
						//No need to send updated playerinfo to everyone but we have to tell this player who he is
						std::string pinfo = "H:" + l + ":" + pkinds + ":|";
						const char* reply = pinfo.c_str();
						m_server.sendToClient(client, reply, strlen(reply));
						return;
					}
					_CORE_INFO("I think the player name is {0}", l);
					Client* c = m_server.getClient(id);
					c->setPlayerName(l);
					_CORE_INFO("We set the client playername to {0}", c->getPlayerName());
					std::string number = "H:" + l + ":";
					if (playerone == 0)
					{
						playerone = id;
						c->setPlayerKind(1);
						playeronestring = l;
						number += "A";
					}
					else if (playertwo == 0)
					{
						playertwo = id;
						c->setPlayerKind(2);
						playertwostring = l;
						number += "B";
					}
					else
					{
						c->setPlayerKind(3);
						number += "G";
					}
					number += ":|";
					const char* repl = number.c_str();
					m_server.sendToAllClients(repl, strlen(repl));
					return;
				}
				cc++;
			}
			catch (...)
			{
				_CORE_WARN("Error_H");
			}
		}
	}
	void mp_server::processping(const Client& client)
	{
		_CORE_INFO("Processing PING");
		std::string reply = "PONG:|";
		const char* repl = reply.c_str();
		m_server.sendToClient(client, repl, strlen(repl));
	}
	void mp_server::processchat(const char* msg, int id)
	{
		_CORE_INFO("Processing Chat");
		std::stringstream data(msg);
		std::string l;
		//std::string name = "Player" + std::to_string(id);
		std::string name = m_server.getClient(id)->getPlayerName();
		int cc = 0;
		try
		{
		while (std::getline(data, l, ':'))
		{
			try
			{
			if (cc == 0)
			{

			}
			else
			{
				_CORE_INFO("Prepping reply");
				std::string reply;
				reply = "T:" + name + ":" + l + ":|";
				const char* repl = reply.c_str();
				m_server.sendToAllClients(repl, strlen(repl));
				return;
			}
			cc++;
			}
			catch (...)
			{
				_CORE_WARN("Error");
			}
		}
		}
		catch (...)
		{
			_CORE_WARN("Error_O");
		}

	}
	void onDisconnect(const Client& client)
	{
		_CORE_INFO("Client {0} (ip:{1}) disconnected. {2}", client.getId(), client.getIp(), client.getErrorMessage());
	}
	int mp_server::m_port;
	int mp_server::playerone;
	int mp_server::playertwo;

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
		playerone = 0;
		playertwo = 0;
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
