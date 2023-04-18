#pragma once
#pragma comment(lib, "Ws2_32.lib")
#include "server_observer.h"
#include "pipe_ret_t.h"
#include "log.h"

#ifdef __linux
	#include <sys/socket.h>
	#include <unistd.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
#elif _WIN32
	#include <WinSock2.h>
	#include <WS2tcpip.h>
	#include <Windows.h>
#else
#endif

#define MAX_PACKET_SIZE 4096

namespace mp_server
{
	class TcpServer
	{
	public:
		pipe_ret_t start(int port);
		Client acceptClient(unsigned int timeout);
		bool deleteClient(Client& client);
		void subscribe(const server_observer_t& observer);
		void unsubscribeAll();
		void DisconnectClient(const Client& client);
		pipe_ret_t sendToAllClients(const char* msg, size_t size);
		pipe_ret_t sendToClient(const Client & client, const char* msg, size_t size);
		pipe_ret_t finish();
		void printClients();
	private:
		int m_clientindex;
		int m_sockfd;
		struct sockaddr_in m_serverAddress;
		struct sockaddr_in m_clientAddress;
		fd_set m_fds;
		std::vector<Client> m_clients;
		std::vector<server_observer_t> m_subscribers;
		std::thread* m_threadHandle;
		void publishClientMsg(const Client& client, const char* msg, size_t msgSize);
		void publishClientDisconnected(const Client& client);
		void receiveTask();
	};
}


