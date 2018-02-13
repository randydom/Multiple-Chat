
#include "MetaSocket.hpp"
#include "ClientManager.hpp"

#include <iostream>

namespace MultipleChat {
	//static
	void ClientManager::clntProcessThreadFunc(int clntNum, MetaSocket::TCPSocket* clntSock) {
		std::cout << "Clnt Process Thread Join ( ClntNum : " << clntNum << " )" << std::endl;

		size_t recvLen;

		while (1) {
			byte* headerData = clntSock->recv(6, recvLen);
			if (headerData == nullptr || recvLen != 6) break;

			std::cout << "header : ";
			for (size_t i = 0; i < recvLen; i++)
				printf("%02X ", headerData[i]);
			std::cout << std::endl;
			
			unsigned short id = headerData[0] << 8 | headerData[1];
			int len =
				headerData[2] << 24 |
				headerData[3] << 16 |
				headerData[4] << 8 |
				headerData[5];

			delete headerData;
			
			std::cout << "id : " << id << " len : " << len << std::endl;

			byte* data = clntSock->recv(len, recvLen);
			if (data == nullptr || recvLen != len) break;
			
			std::cout << "data : ";
			for (size_t i = 0; i < recvLen; i++)
				printf("%c", data[i]);
			std::cout << std::endl;

			delete data;
		}
		clntSock->close();
		delete clntSock;

		std::cout << "connection end" << std::endl;

		ClientManager::getInstance()->m_clntArr[clntNum] = nullptr;
	}

	//non-static
	ClientManager::~ClientManager() {
		for (size_t i = 0; i < m_clntArr.size(); i++) {
			if (m_clntArr[i] != nullptr) {
				m_clntArr[i]->join();
				delete m_clntArr[i];
			}
		}
	}

	void ClientManager::processClnt(MetaSocket::TCPSocket* clntSock) {
		size_t clntNum = 0; bool flag = false;
		for (size_t i = 0; i < m_clntArr.size(); i++) {
			if (m_clntArr[i] == nullptr) {
				clntNum = i;
				flag = true;
			}
		}
		if (clntNum == 0 && flag == false)
			clntNum = m_clntArr.size();

		std::thread* t = new std::thread(&clntProcessThreadFunc, clntNum, clntSock);

		if (flag == false)
			m_clntArr.push_back(t);
		else
			m_clntArr[clntNum] = t;
	}
}