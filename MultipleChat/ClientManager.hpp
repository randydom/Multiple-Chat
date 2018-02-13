#pragma once

#include "Singleton.hpp"
#include <vector>
#include <thread>

namespace MultipleChat {
	class MetaSocket::TCPSocket;

	class ClientManager : public Singleton<ClientManager> {
	private:
		std::vector<std::thread*> m_clntArr;

		static void clntProcessThreadFunc(int clntNum, MetaSocket::TCPSocket* clntSock);

	public:
		~ClientManager();

		void processClnt(MetaSocket::TCPSocket*);
	};
}