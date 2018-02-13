
#include "MetaSocket.hpp"
#include "ClientManager.hpp"

#include <iostream>

namespace MultipleChat {
	//static
	void ClientManager::clntProcessThreadFunc(int clntNum, MetaSocket::TCPSocket* clntSock) {
		std::cout << "Clnt Process Thread Join ( ClntNum : " << clntNum << " )" << std::endl;

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