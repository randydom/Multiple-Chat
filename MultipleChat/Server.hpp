#pragma once

#include "Singleton.hpp"
#include "MetaSocket.hpp"

#include <vector>
#include <thread>
#include <mutex>

namespace MultipleChat {

	class Server : public Singleton<Server> {
	private:
		std::vector<std::thread*>           m_threadArr;
		std::vector<MetaSocket::TCPSocket*> m_clntSockArr;

		std::mutex m_mtx;

	public:
		static void communicateFunc(int clntNum);

	public:
		void run(int port);

		~Server();
	};
}