
#include "Server.hpp"


#include <iostream>

using namespace MetaSocket;
namespace MultipleChat {
	//static
	void Server::communicateFunc(int clntNum) {
		Server::getInstance()->m_mtx.lock();
		MetaSocket::TCPSocket* clntSock = Server::getInstance()->m_clntSockArr[clntNum];
		Server::getInstance()->m_mtx.unlock();

		std::cout << "Client " << clntNum << " is Connected" << std::endl;

		size_t recvLen;

		while (1) {
			byte* headerData = clntSock->recv(6, recvLen);
			if (headerData == nullptr || recvLen != 6) break;

			std::cout << clntNum << "'s Header : ";
			for (size_t i = 0; i < recvLen; i++)
				printf("%02X ", headerData[i]);
			std::cout << std::endl;

			unsigned short messageType =
				headerData[0] << 8 |
				headerData[1];
			int len =
				headerData[2] << 24 |
				headerData[3] << 16 |
				headerData[4] << 8 |
				headerData[5];

			//std::cout << "MsgType : " << messageType << " len : " << len << std::endl;

			byte* data = clntSock->recv(len, recvLen);
			if (data == nullptr || recvLen != len) break;


			Server::getInstance()->m_mtx.lock();

			size_t clntCnt = Server::getInstance()->m_clntSockArr.size();
			for (size_t i = 0; i < clntCnt; i++)
				if (i != clntNum && Server::getInstance()->m_clntSockArr[i] != nullptr) {
					std::cout << "Clnt " << clntNum << " -> " << i << std::endl;
					Server::getInstance()->m_clntSockArr[i]->send(headerData, 6);
					Server::getInstance()->m_clntSockArr[i]->send(data, len);
				}

			Server::getInstance()->m_mtx.unlock();

			delete headerData;
			delete data;
		}

		Server::getInstance()->m_mtx.lock();

		std::cout << "Client " << clntNum << " is Disconnected" << std::endl;

		clntSock->close(); delete clntSock;

		Server::getInstance()->m_threadArr[clntNum] = nullptr;
		Server::getInstance()->m_clntSockArr[clntNum] = nullptr;

		Server::getInstance()->m_mtx.unlock();
	}


	//non-static
	Server::~Server() {
		for (size_t i = 0; i < m_threadArr.size(); i++) {
			if (m_threadArr[i] != nullptr) {
				m_threadArr[i]->join();
				delete m_threadArr[i];
			}
			if (m_clntSockArr[i] != nullptr)
				delete m_clntSockArr[i];
		}
	}

	void Server::run(int port) {
		MetaSocket::TCPServer serv(port);
		
		serv.bind();
		serv.listen(5);

		while (true) {
			MetaSocket::TCPSocket* clntSock = serv.accept();

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

			std::thread* t = new std::thread(&communicateFunc, clntNum);

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
}