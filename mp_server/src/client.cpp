#include "client.h"


	//Destructor
Client::~Client()
{
	if (m_threadHandler != nullptr && m_threadHandler->joinable())
	{
		m_threadHandler->detach();
	}
}

bool Client::operator == (const Client& other)
{
	if ((this->m_sockfd == other.m_sockfd) && (this->m_ip == other.m_ip))
	{
		return true;
	}
	return false;
}
