#pragma once

#include "Singleton.hpp"
#include <vector>
#include <thread>
#include <mutex>

namespace MultipleChat {
	class MetaSocket::TCPSocket;

	class ClientManager : public Singleton<ClientManager> {
	private:
		std::vector<std::thread*>           m_threadArr;
		std::vector<MetaSocket::TCPSocket*> m_clntSockArr;

		std::mutex                          m_mtx;
		std::mutex                          m_sendMtx;

		static void clntProcessThreadFunc(int clntNum);

	public:
		~ClientManager();

		void processClnt(MetaSocket::TCPSocket*);
	};
}