
#include "MetaSocket.hpp"
#include "ClientManager.hpp"

#include <iostream>

namespace MultipleChat {
	//static
	void ClientManager::clntProcessThreadFunc(int clntNum) {
		ClientManager::getInstance()->m_mtx.lock();
		MetaSocket::TCPSocket* clntSock = ClientManager::getInstance()->m_clntSockArr[clntNum];
		ClientManager::getInstance()->m_mtx.unlock();

		std::cout << "Clnt Process Thread Join ( ClntNum : " << clntNum << " )" << std::endl;

		size_t recvLen;

		while (1) {
			byte* headerData = clntSock->recv(6, recvLen);
			if (headerData == nullptr || recvLen != 6) break;

			std::cout << clntNum << "'s Header : ";
			for (size_t i = 0; i < recvLen; i++)
				printf("%02X ", headerData[i]);
			std::cout << std::endl;
			
			unsigned short messageType = 
				headerData[0] << 8  | 
				headerData[1];
			int len =
				headerData[2] << 24 |
				headerData[3] << 16 |
				headerData[4] <<  8 |
				headerData[5];
			
			//std::cout << "MsgType : " << messageType << " len : " << len << std::endl;

			byte* data = clntSock->recv(len, recvLen);
			if (data == nullptr || recvLen != len) break;


			ClientManager::getInstance()->m_sendMtx.lock();

			size_t clntCnt = ClientManager::getInstance()->m_clntSockArr.size();
			for (size_t i = 0; i < clntCnt; i++)
				if (i != clntNum && ClientManager::getInstance()->m_clntSockArr[i] != nullptr) {
					std::cout << "Clnt " << clntNum << " -> " << i << std::endl;
					ClientManager::getInstance()->m_clntSockArr[i]->send(headerData, 6);
					ClientManager::getInstance()->m_clntSockArr[i]->send(data,     len);
				}

			ClientManager::getInstance()->m_sendMtx.unlock();

			delete headerData;
			delete data;
		}

		ClientManager::getInstance()->m_mtx.lock();

		clntSock->close(); delete clntSock;

		ClientManager::getInstance()->m_threadArr[clntNum]   = nullptr;
		ClientManager::getInstance()->m_clntSockArr[clntNum] = nullptr;
		
		ClientManager::getInstance()->m_mtx.unlock();
	}

	//non-static
	ClientManager::~ClientManager() {
		for (size_t i = 0; i < m_threadArr.size(); i++) {
			if (m_threadArr[i] != nullptr) {
				m_threadArr[i]->join();
				delete m_threadArr[i];
			}
			if (m_clntSockArr[i] != nullptr)
				delete m_clntSockArr[i];
		}
	}

	void ClientManager::processClnt(MetaSocket::TCPSocket* clntSock) {

		this->m_mtx.lock();

		size_t clntNum = 0; bool flag = false;
		for (size_t i = 0; i < m_threadArr.size(); i++) {
			if (m_threadArr[i] == nullptr) {
				clntNum = i;
				flag = true;
			}
		}
		if (clntNum == 0 && flag == false)
			clntNum = m_threadArr.size();

		std::thread* t = new std::thread(&clntProcessThreadFunc, clntNum);

		if (flag == false) {
			m_threadArr.push_back(t);
			m_clntSockArr.push_back(clntSock);
		}
		else {
			m_threadArr[clntNum] = t;
			m_clntSockArr[clntNum] = clntSock;
		}

		this->m_mtx.unlock();
	}
}