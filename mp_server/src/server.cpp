#include "server.h"

constexpr auto DEFAULT_CLIENT_RESERVE = 10;

namespace mp_server
{
	void TcpServer::subscribe(const server_observer_t& observer)
	{
		m_subscribers.push_back(observer);
	}
	
	void TcpServer::unsubscribeAll()
	{
		m_subscribers.clear();
	}

	void TcpServer::printClients()
	{
		for (size_t i = 0; i < m_clients.size(); ++i)
		{
			std::string connected = m_clients[i].isConnected() ? "True" : "False";
			_CORE_INFO("---------\nIP address: {0}\nConnected?: {1}\nSocket FD: {2}\nMessage: {3}", 
				m_clients[i].getIp(), connected, m_clients[i].getFileDescriptor(), m_clients[i].getErrorMessage());
		}
	}

	Client TcpServer::getClient(int id)
	{
		for (size_t i = 0; i < m_clients.size(); ++i)
		{
			if (m_clients[i].getId() == id)
			{
				return m_clients[i];
			}
		}
	}

	void TcpServer::receiveTask()
	{
		Client* client = &m_clients.back();

		while (client->isConnected())
		{
			char msg[MAX_PACKET_SIZE];
			int numOfBytesReceived = recv(client->getFileDescriptor(), msg, MAX_PACKET_SIZE, 0);
			if (numOfBytesReceived < 1)
			{
				client->setDisConnected();
				if (numOfBytesReceived == 0)
				{
					client->setErrorMessage("Client closed connection");
				}
				else
				{
					client->setErrorMessage(strerror(errno));
				}
				//This doesn't seem to be necessary because the handle seems to be always closed at that point
				//close(client->getFileDescriptor());
				publishClientDisconnected(*client);
				deleteClient(*client);
				break;
			}
			else
			{
				publishClientMsg(*client, msg, numOfBytesReceived);
			}
		}
	}

	void TcpServer::DisconnectClient(const Client& client)
	{
		publishClientDisconnected(client);
		return;
	}

	bool TcpServer::deleteClient(Client& client)
	{
		int clientIndex = -1;
		for (size_t i = 0; i < m_clients.size(); ++i)
		{
			if (m_clients[i] == client)
			{
				clientIndex = i;
				break;
			}
		}
		if (clientIndex > -1)
		{
			m_clients.erase(m_clients.begin() + clientIndex);
			return true;
		}
		return false;
	}

	void TcpServer::publishClientMsg(const Client& client, const char* msg, size_t msgSize)
	{
		for (size_t i = 0; i < m_subscribers.size(); ++i)
		{
			if (m_subscribers[i].wantedIp == client.getIp() || m_subscribers[i].wantedIp.empty())
			{
				if (m_subscribers[i].incoming_packet_func != NULL)
				{
					(*m_subscribers[i].incoming_packet_func)(client, msg, msgSize);
				}
			}
		}
	}

	void TcpServer::publishClientDisconnected(const Client& client)
	{
		for (size_t i = 0; i < m_subscribers.size(); i++)
		{
			if (m_subscribers[i].wantedIp == client.getIp())
			{
				if (m_subscribers[i].disconnected_func != NULL)
				{
					(*m_subscribers[i].disconnected_func)(client);
				}
			}
		}
	}

	pipe_ret_t TcpServer::start(int port)
	{
		m_sockfd = 0;
		m_clients.reserve(DEFAULT_CLIENT_RESERVE);
		m_subscribers.reserve(1);
		pipe_ret_t ret;
		m_clientindex = 0;
		#ifdef __linux
		#elif _WIN32
			WSADATA wsaData;
			WSAStartup(MAKEWORD(2, 2), &wsaData);
		#else
		#endif
		m_sockfd = socket(AF_INET, SOCK_STREAM, 0);

		if (m_sockfd == -1)
		{
			ret.success = false;
 			ret.msg = "Couldn't open socket - ";
			ret.msg += strerror(errno);
		#ifdef __linux
		#elif _WIN32
			ret.msg += WSAGetLastError();
		#else
		#endif
			return ret;
		}

		char option = '1';
		setsockopt(m_sockfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
		memset(&m_serverAddress, 0, sizeof(m_serverAddress));
		m_serverAddress.sin_family = AF_INET;
		m_serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
		m_serverAddress.sin_port = htons(port);

		int bindSuccess = bind(m_sockfd, (struct sockaddr*)&m_serverAddress, sizeof(m_serverAddress));

		if (bindSuccess == -1)
		{
			ret.success = false;
			ret.msg = strerror(errno);
			return ret;
		}

		const int clientsQueueSize = 5;
		int listenSuccess = listen(m_sockfd, clientsQueueSize);

		if (listenSuccess == -1)
		{
			ret.success = false;
			ret.msg = strerror(errno);
			return ret;
		}

		ret.success = true;
		return ret;

	}

	Client TcpServer::acceptClient(unsigned int timeout)
	{
		socklen_t sosize = sizeof(m_clientAddress);
		Client newclient;

		if (timeout > 0)
		{
			struct timeval tv;
			tv.tv_sec = 2;
			tv.tv_usec = 0;
			FD_ZERO(&m_fds);
			FD_SET(m_sockfd, &m_fds);
			int selectRet = select(m_sockfd + 1, &m_fds, NULL, NULL, &tv);
			if (selectRet == -1)
			{
				newclient.setErrorMessage(strerror(errno));
				return newclient;
			}
			else if (selectRet == 0) // Timeout
			{
				newclient.setErrorMessage("Connection to client timed out");
				return newclient;
			}
			else if (!FD_ISSET(m_sockfd, &m_fds))
			{
				newclient.setErrorMessage("File descriptor not set");
				return newclient;
			}
		}

		int file_descriptor = accept(m_sockfd, (struct sockaddr*)&m_clientAddress, &sosize);
		if (file_descriptor == -1)
		{
			newclient.setErrorMessage(strerror(errno));
			return newclient;
		}

		newclient.setFileDescriptor(file_descriptor);
		newclient.setConnected();
		newclient.setIp(inet_ntoa(m_clientAddress.sin_addr));
		newclient.setId(m_clientindex);
		m_clients.push_back(newclient);
		m_clients.back().setThreadHandler(std::bind(&TcpServer::receiveTask, this));
		m_clientindex++;
		return newclient;
	}

	pipe_ret_t TcpServer::sendToAllClients(const char* msg, size_t size)
	{
		pipe_ret_t ret;
		for (size_t i = 0; i < m_clients.size(); ++i)
		{
			ret = sendToClient(m_clients[i], msg, size);
			if (!ret.success)
			{
				return ret;
			}
		}
		ret.success = true;
		return ret;
	}

	pipe_ret_t TcpServer::sendToClient(const Client& client, const char* msg, size_t size)
	{
		pipe_ret_t ret;
		int numBytesSent = send(client.getFileDescriptor(), (char*)msg, size, 0);
		if (numBytesSent < 0)
		{
			ret.success = false;
			ret.msg = strerror(errno);
			return ret;
		}
		if ((unsigned int)numBytesSent < size)
		{
			ret.success = false;
			char lmsg[100];
			sprintf(lmsg, "Only %d bytes out of %lu was sent to client", numBytesSent, size);
			ret.msg = lmsg;
			return ret;
		}
		ret.success = true;
		return ret;
	}

	pipe_ret_t TcpServer::finish() 
	{
		pipe_ret_t ret;
		for (size_t i = 0; i < m_clients.size(); ++i)
		{
			m_clients[i].setDisConnected();
			if (close(m_clients[i].getFileDescriptor()) == -1)
			{
				ret.success = false;
				ret.msg = strerror(errno);
				return ret;
			}
		}

		if (close(m_sockfd) == -1)
		{
			ret.success = false;
			ret.msg = strerror(errno);
			return ret;
		}
		m_clients.clear();
		ret.success = true;
		return ret;
	}
}
