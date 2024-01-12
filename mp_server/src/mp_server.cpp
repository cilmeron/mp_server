#include "mp_server.h"
#include "log.h"
#include <thread>
#include <chrono>
#include <sstream>
#include <string>
#include <unordered_map>

namespace mp_server
{
	

	std::string createString(const char* data, size_t size)
	{
		return std::string(data, size);
	}
	void onIncomingAdmin(const Client& client, const char* msg, size_t size)
	{
		_CORE_INFO("Administrator sent some message");
	}
	void onIncomingClient(const Client& client, const char* msg, size_t size)
	{		
		std::string message = createString(msg, size);
		int lclid = client.getId();
		_CORE_INFO("Client {0} sent some message: {1}", lclid, message);
		const std::string delimiters = "\n\0";
		std::istringstream iss(message);
		std::string token;
		while (std::getline(iss, token, delimiters.front()))
		{
			if (!token.empty())
			{
				if (token.length() == 1 || token[1] != ':')
				{
					_CORE_INFO("This is probably a garbage message because it doesn't contain valid marker");
					return;
				}
				if (token[0] == 'T')
				{
					mp_server::processchat(token, lclid);
				}
				else if (token[0] == 'P')
				{
					mp_server::processping(client);
				}
				else if (token[0] == 'H')
				{
					mp_server::processhello(token, lclid, client);
				}
				else if (token[0] == 'M')
				{
					mp_server::processmove(token, lclid, client, "M");
				}
				else if (token[0] == 'A')
				{
					mp_server::processattack(token, lclid, client);
				}
				else if (token[0] == 'Z')
				{
					mp_server::timeout();
				}
				else if (token[0] == 'C')
				{
					mp_server::processmove(token, lclid, client, "C");
				}
				else if (token[0] == 'I')
				{
					mp_server::processhp(token, lclid, client);
				}
				else if (token[0] == 'K')
				{
					mp_server::processmove(token, lclid, client, "K");
				}
				else if (token[0] == 'S')
				{
					mp_server::processmove(token, lclid, client, "S");
				}
				else if (token[0] == 'Q')
				{
					mp_server::reset();
				}
			}
		}
	}

	void mp_server::reset()
	{
		std::lock_guard<std::mutex> lock(m_server.resetMutex);
		_CORE_INFO("Resetting server because a player quit the game.");
		m_server.playeronestring = "";
		m_server.playertwostring = "";
		m_server.playeroneunits.clear();
		m_server.playertwounits.clear();
		m_server.playerone = -1;
		m_server.playertwo = -1;
		std::string send = "Q:SUCCESS:|";
		m_server.sendToAllClients(send.c_str(), send.length());		
		_CORE_INFO("Reset done.");
	}


	void mp_server::timeout()
	{
		_CORE_INFO("Timeout condition reached - Calculate winner and send to clients");
		_CORE_INFO("#################");
		std::string send;
		if (m_server.playeroneunits.size() >= m_server.playertwounits.size())
		{
			_CORE_INFO("Player {0} won.", m_server.playeronestring);
			send = "G:" + m_server.playertwostring + ":0:|";
		}
		else
		{
			_CORE_INFO("Player {0} won.", m_server.playertwostring);
			send = "G:" + m_server.playeronestring + ":0:|";
		}
		m_server.sendToAllClients(send.c_str(), send.length());
	}
	
	std::string mp_server::processSingleMoveCommand(const std::string& command, const std::string& term)
	{
		std::string temp = "";
		if (!command.empty() && command[0] == term[0] && command.size() > 1 && command[1] != ':')
		{
			temp = term + ":" + command;
		}
		else
		{
			temp = command;
		}
		
		std::istringstream commandStream(temp);	
		std::string token;

		std::vector<std::string> components;
		while (std::getline(commandStream, token, ':'))
		{
			components.push_back(token);
		}

		// Assuming the components vector has the expected number of elements
		if (components.size() >= 4)
		{
			std::string playerName = components[1];
			std::string coordinates = components[2];
			std::string UnitID = components[3];
			std::string Type = "A";
			if (term._Equal("S"))
			{
				Type = components[4];
			}
			int id = std::stoi(UnitID);
			std::unordered_map<int, std::string>& temp = (playerName == m_server.playeronestring) ? m_server.playeroneunits : m_server.playertwounits;
			std::unordered_map<int, std::string>& temp2 = (playerName == m_server.playeronestring) ? m_server.playertwounits : m_server.playeroneunits;

			if (Type._Equal("O"))
			{
				auto it = temp2.find(id);
				if (it != temp2.end())
				{
					//found unit in opponent collection 
				}
				else
				{
					//not found - add to list
					temp2.insert(std::make_pair(id, coordinates));
				}
				return "S:" + playerName + ":" + coordinates + ":" + UnitID + ":" + Type + ":|";
			}
			if (Type._Equal("K"))
			{
				auto it = temp.find(id);
				if (it != temp.end())
				{
					//found unit in own collection - send new coord
				}
				else
				{
					//not found add
					temp.insert(std::make_pair(id, coordinates));
				}
				return "S:" + playerName + ":" + coordinates + ":" + UnitID + ":" + Type + ":|";
			}

			auto it = temp.find(id);
			if (it != temp.end())
			{
				if (term._Equal("K"))
				{
					return "";
				//int id = std::stoi(UnitID);	temp.erase(id);
//					if (temp.size() == 0)
	//				{
		//				//player has no more units - declare game over
			//			_CORE_INFO("#################");
				//		_CORE_INFO("Player {0} is DEAD", playerName);
					//	return "G:" + playerName + ":0:|";
					//}
				}
				else if (term._Equal("M"))
				{
					temp.erase(id);
					temp.insert(std::make_pair(id, coordinates));
				}
				return term + ":" + playerName + ":" + coordinates + ":" + UnitID + ":|";
			}
			else
			{
				if (term._Equal("C"))
				{
					try
					{
						auto result = temp.insert(std::make_pair(id, coordinates));
					}
					catch (const std::exception& e)
					{
						// Handle the exception (log, print, etc.)
						_CORE_ERROR("Something went wrong: {0}", e.what());
					}
					return term + ":" + playerName + ":" + coordinates + ":" + UnitID + ":|";
				}
				return term + ":NOTFOUND";
			}
		}
		return "";
	}

	std::string mp_server::processSingleHPCommand(const std::string& command, const std::string& term)
	{
		std::string temp = "";
		if (!command.empty() && command[0] == term[0] && command.size() > 1 && command[1] != ':')
		{
			temp = term + ":" + command;
		}
		else
		{
			temp = command;
		}

		std::istringstream commandStream(temp);
		std::string token;

		std::vector<std::string> components;
		while (std::getline(commandStream, token, ':'))
		{
			components.push_back(token);
		}

		// Assuming the components vector has the expected number of elements
		if (components.size() == 5)
		{
			std::string playerName = components[1];
			std::string hp = components[2];
			std::string current = components[3];
			std::string UID = components[4];
			int hpint = std::stoi(hp);
			int currentint = std::stoi(current);
			int res = currentint - hpint;
			if (res < 0)
				res = 0;
			if (res == 0)
			{
				int id = std::stoi(UID);
				std::unordered_map<int, std::string>& temp = (playerName == m_server.playeronestring) ? m_server.playeroneunits : m_server.playertwounits;
				auto it = temp.find(id);
				if (it != temp.end())
				{
					temp.erase(id);
					if (temp.size() == 0)
					{
						//player has no more units - declare game over
						_CORE_INFO("#################");
						_CORE_INFO("Player {0} is DEAD", playerName);
						std::string gameover = "G:" + playerName + ":0:|";
						m_server.sendToAllClients(gameover.c_str(), gameover.length());
					}
					std::string returndeath = "K:" + playerName + ":O:" + UID + ":|";
					m_server.sendToAllClients(returndeath.c_str(), returndeath.length());
					}
			}
			return term + ":" + playerName + ":" + UID + ":" + std::to_string(res) + ":|";
		}
		return "";
	}

	std::string mp_server::processSingleAttackCommand(const std::string& command, const std::string& term)
	{
		std::string temp = "";
		if (!command.empty() && command[0] == term[0] && command.size() > 1 && command[1] != ':')
		{
			temp = term + ":" + command;
		}
		else
		{
			temp = command;
		}

		std::istringstream commandStream(temp);
		std::string token;

		std::vector<std::string> components;
		while (std::getline(commandStream, token, ':'))
		{
			components.push_back(token);
		}

		// Assuming the components vector has the expected number of elements
		if (components.size() == 4)
		{
			std::string playerName = components[1];
			std::string Victim = components[2];
			std::string Attacker = components[3];
			return term + ":" + playerName + ":" + Victim + ":" + Attacker + ":|";
		}
		return "";
	}

	void mp_server::processmove(std::string msg, int id, const Client& client, const std::string& delim)
	{
		_CORE_INFO("Processing "+delim+" command");

		std::istringstream messageStream(msg);
		size_t pos = 0;
		std::string token;
		while ((pos = msg.find(delim)) != std::string::npos)
		{
			token = msg.substr(0, pos);
			msg.erase(0, pos + delim.length());
			std::string send = processSingleMoveCommand(delim + token, delim);
			if (send.length() > 1)
			{
				_CORE_INFO("SENDING: " + send);
				m_server.sendToAllClients(send.c_str(), send.length());
			}
		}

		std::string send = processSingleMoveCommand(delim + msg, delim);
		if (send.length() > 1)
		{
			_CORE_INFO("SENDING: " + send);
			m_server.sendToAllClients(send.c_str(), send.length());
		}
		
	}

	void mp_server::processhp(std::string msg, int id, const Client& client)
	{
		_CORE_INFO("Processing HP Update");
		std::istringstream messageStream(msg);
		size_t pos = 0;
		const std::string delim = "I";
		std::string token;
		while ((pos = msg.find(delim)) != std::string::npos)
		{
			token = msg.substr(0, pos);
			msg.erase(0, pos + delim.length());
			std::string send = processSingleHPCommand(delim + token, delim);
			if (send.length() > 1)
			{
				_CORE_INFO("SENDING: " + send);
				m_server.sendToAllClients(send.c_str(), send.length());
			}
		}

		std::string send = processSingleHPCommand(delim + msg, delim);
		if (send.length() > 1)
		{
			_CORE_INFO("SENDING: " + send);
			m_server.sendToAllClients(send.c_str(), send.length());
		}
	}
	void mp_server::processattack(std::string msg, int id, const Client& client)
	{
		_CORE_INFO("Processing Attack command");

		std::istringstream messageStream(msg);
		size_t pos = 0;
		const std::string delim = "A";
		std::string token;
		while ((pos = msg.find(delim)) != std::string::npos)
		{
			token = msg.substr(0, pos);
			msg.erase(0, pos + delim.length());
			std::string send = processSingleAttackCommand(delim + token, delim);
			if (send.length() > 1)
			{
				_CORE_INFO("SENDING: " + send);
				m_server.sendToAllClients(send.c_str(), send.length());
			}
		}

		std::string send = processSingleAttackCommand(delim + msg, delim);
		if (send.length() > 1)
		{
			_CORE_INFO("SENDING: " + send);
			m_server.sendToAllClients(send.c_str(), send.length());
		}
	}
	
	void mp_server::processhello(std::string msg, int id, const Client& client)
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
					m_server.findPlayer(l); //Clean up old existing connections if any
					if (m_server.playeronestring == l)
					{
						m_server.playerone = id;
						pkinds = "A";
						reconnect = true;
					}
					else if (m_server.playertwostring == l)
					{
						m_server.playertwo = id;
						pkinds = "B";
						reconnect = true;
					}
					
					if (reconnect == true)
					{
						//No need to send updated playerinfo to everyone but we have to tell this player who he is
						std::string pinfo = "H:" + l + ":" + pkinds + ":R|";
						const char* reply = pinfo.c_str();
						_CORE_INFO("Sending playerinfo to reconnected player {0}", pinfo.c_str());
						m_server.sendToClient(client, reply, strlen(reply));
						std::this_thread::sleep_for(std::chrono::milliseconds(10));
						//Now we send unit infos
						for (const auto& entry : m_server.playeroneunits)
						{
							const auto& key = entry.first;
							const auto& value = entry.second;
							std::string send = "C:" + m_server.playeronestring + ":" + entry.second + ":" + std::to_string(entry.first) + ":|";
							m_server.sendToClient(client, send.c_str(), send.length());
						}
						for (const auto& entry : m_server.playertwounits)
						{
							const auto& key = entry.first;
							const auto& value = entry.second;
							std::string send = "C:" + m_server.playertwostring + ":" + entry.second + ":" + std::to_string(entry.first) + ":|";
							m_server.sendToClient(client, send.c_str(), send.length());
						}
						return;
					}
					
					_CORE_INFO("I think the player name is {0}", l);
					Client* c = m_server.getClient(id);
					c->setPlayerName(l);
					_CORE_INFO("We set the client playername to {0}", c->getPlayerName());
					std::string number = "H:" + l + ":";
					if (m_server.playerone == -1)
					{
						m_server.playerone = id;
						c->setPlayerKind(1);
						m_server.playeronestring = l;
						number += "A";
					}
					else if (m_server.playertwo == -1)
					{
						m_server.playertwo = id;
						c->setPlayerKind(2);
						m_server.playertwostring = l;
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
					//Now we send unit infos
					std::this_thread::sleep_for(std::chrono::milliseconds(10));
					for (const auto& entry : m_server.playeroneunits)
					{
						const auto& key = entry.first;
						const auto& value = entry.second;
						std::string send = "C:" + m_server.playeronestring + ":" + entry.second + ":" + std::to_string(entry.first) + ":|";
						m_server.sendToClient(client, send.c_str(), send.length());
					}
					for (const auto& entry : m_server.playertwounits)
					{
						const auto& key = entry.first;
						const auto& value = entry.second;
						std::string send = "C:" + m_server.playertwostring + ":" + entry.second + ":" + std::to_string(entry.first) + ":|";
						m_server.sendToClient(client, send.c_str(), send.length());
					}
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
	void mp_server::processchat(std::string msg, int id)
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
			if (cc == 0 || cc == 1)
			{

			}
			else
			{
				_CORE_INFO("Prepping reply");
				std::string reply;
				reply = "T:" + name + ":" + l + ":|";
				_CORE_INFO(reply);
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

	void mp_server::create()
	{
		pipe_ret_t runServer = m_server.start(m_server.m_port);
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
		m_server.playerone = -1;
		m_server.playertwo = -1;
		m_server.playeroneunits = std::unordered_map<int, std::string>();
		m_server.playertwounits = std::unordered_map<int, std::string>();
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
