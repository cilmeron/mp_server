#pragma once

#include <string>
#include <functional>
#include <thread>

class Client 
{
public:
	~Client();
	bool operator ==(const Client& other);
	void setFileDescriptor(int sockfd) { m_sockfd = sockfd; }
	int getFileDescriptor() const { return m_sockfd; }
	void setIp(const std::string& ip) { m_ip = ip; }
	std::string getIp() const { return m_ip; }
	int getId() const { return m_id; }
	void setId(int _id) { m_id = _id; }
	void setErrorMessage(const std::string& msg) { m_errorMsg = msg; }
	std::string getErrorMessage() const { return m_errorMsg; }
	void setConnected() { m_isConnected = true; }
	void setDisConnected() { m_isConnected = false; }
	bool isConnected() { return m_isConnected; }
	void setThreadHandler(std::function<void(void)> func) { m_threadHandler = new std::thread(func); }
private:
	int m_sockfd = 0;
	std::string m_ip = "";
	std::string m_errorMsg = "";
	bool m_isConnected;
	std::thread* m_threadHandler = nullptr;
	int m_id;
};
