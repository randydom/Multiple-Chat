
#include "Server.hpp"

#include "MetaSocket.hpp"
#include "ClientManager.hpp"

namespace MultipleChat {
	void Server::run() {
		MetaSocket::TCPServer serv(8888);
		
		serv.bind();
		serv.listen(5);

		while (true) {
			MetaSocket::TCPSocket* clnt = serv.accept();

			ClientManager::getInstance()->processClnt(clnt);
		}
	}
}