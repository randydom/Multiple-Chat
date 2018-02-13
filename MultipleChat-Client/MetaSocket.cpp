#include "MetaSocket.hpp"

#include <iostream>


namespace MetaSocket {
	//static
	int Socket::g_socketCnt = 0;

	int Socket::init() {
		WSADATA wsaData;
		if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
			std::cout << "[Socket Class] " << "WSAStartup Error" << std::endl;

			return 0;
		}

		return 1;
	}

	int Socket::cleanUp() {
		WSACleanup();

		return 1;
	}

	//nont-static
	Socket::Socket() {
		if (Socket::g_socketCnt == 0)
			Socket::init();
		Socket::g_socketCnt++;
	}

	Socket::Socket(const std::string ip, const unsigned int port, AF addressFamily, Type protocolType)
		: Socket() {

		ZeroMemory(&m_sockAddr, sizeof(m_sockAddr));
		m_sockAddr.sin_family = AF_INET;
		m_sockAddr.sin_port = htons(port);
		this->setIp(ip);

		int proto = protocolType == Type::TCP ? IPPROTO_TCP : IPPROTO_UDP;

		this->m_addressFamily = addressFamily;
		this->m_protocol = protocolType;
		this->m_socket = ::socket(
			static_cast<int>(this->m_addressFamily),
			static_cast<int>(this->m_protocol),
			proto
		);

		if (this->m_socket == INVALID_SOCKET)
			std::cout << "[Socket Class] " << "socket() error" << std::endl;
	}

	Socket::Socket(const SOCKET sock, const SOCKADDR_IN sockAddr)
		: Socket() {

		this->m_socket = sock;
		this->m_sockAddr = sockAddr;
	}

	Socket::~Socket() {
		close();

		Socket::g_socketCnt--;
		if (Socket::g_socketCnt == 0)
			Socket::cleanUp();
	}

	void Socket::close() {
		if (this->m_isOpen) ::closesocket(m_socket);
		this->m_isOpen = false;
	}

	bool Socket::operator==(const Socket& T) const {
		return (
			this->m_socket == T.m_socket &&
			std::memcmp(&this->m_sockAddr, &T.m_sockAddr, sizeof this->m_sockAddr) == 0
			);
	}
	bool Socket::operator!=(const Socket& T) const {
		return !this->operator==(T);
	}

	Socket::Type Socket::getSockType() const { return m_protocol; }
	SOCKET       Socket::getSocket()   const { return m_socket; }
	SOCKADDR_IN  Socket::getSockAddr() const { return m_sockAddr; }

	unsigned int Socket::setIp(const std::string ipStr) {
		if (ipStr.empty() || ipStr == "")
			m_sockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
		else
			m_sockAddr.sin_addr.s_addr = inet_addr(ipStr.c_str());

		return m_sockAddr.sin_addr.s_addr;
	}

	bool Socket::errorCheck(const int& result) const {
		if (result == SOCKET_ERROR) {
			//std::cout << "[Socket Class] " << "SOCKET_ERROR" << std::endl;
			return true;
		}
		return false;
	}

	bool Socket::send(const byte* data, size_t len) const {
		int result;

		this->sendF(result, data, len);

		if (errorCheck(result))
			return false;

		return true;
	}
	bool Socket::send(const std::vector<byte>& byteVec) const {
		return this->send(byteVec.data(), byteVec.size());
	}
	bool Socket::send(const std::string& str) const {
		return this->send((byte*)str.c_str(), str.length());
	}


	byte* Socket::recv(size_t len, size_t& recvLen) const {
		int result;

		byte* buf = this->recvF(result, len);

		if (errorCheck(result))
			return nullptr;

		recvLen = result;

		return buf;
	}
	std::vector<byte> Socket::recv(size_t len) const {
		size_t recvLen;
		byte* buf = this->recv(len, recvLen);

		std::vector<byte> vec(recvLen);
		for (size_t i = 0; i < recvLen; i++)
			vec[i] = buf[i];

		return vec;
	}
	TCPSocket::TCPSocket(const std::string ip, const unsigned int port)
		: Socket(ip, port, Socket::AF::INET, Socket::Type::TCP) { }

	TCPSocket::TCPSocket(const SOCKET sock, const SOCKADDR_IN sockAddr)
		: Socket(sock, sockAddr) { }

	TCPSocket::~TCPSocket() { }


	void TCPSocket::sendF(int& out, const byte* data, const size_t size) const {
		out = ::send(
			m_socket,
			(char*)data,
			size,
			0
		);
	}

	byte* TCPSocket::recvF(int& out, const size_t size) const {
		byte* buf = new byte[size];

		out = ::recv(
			m_socket,
			(char*)buf,
			size,
			0
		);

		return buf;
	}
	UDPSocket::UDPSocket(const std::string ip, const unsigned int port)
		: Socket(ip, port, Socket::AF::INET, Socket::Type::UDP) { }

	UDPSocket::UDPSocket(const SOCKET sock, const SOCKADDR_IN sockAddr)
		: Socket(sock, sockAddr) { }

	UDPSocket::~UDPSocket() { }

	void UDPSocket::sendF(int& out, const byte* data, const size_t size) const {
		out = ::sendto(
			m_socket,
			(char*)data,
			size,
			0,
			(SOCKADDR *)&m_sockAddr,
			sizeof(m_sockAddr)
		);
	}

	byte* UDPSocket::recvF(int& out, const size_t size) const {
		byte* buf = new byte[size];

		int addrlen = sizeof(m_peerAddr);

		out = ::recvfrom(
			m_socket,
			(char*)buf,
			size,
			0,
			(SOCKADDR *)&m_peerAddr,
			&addrlen
		);

		return buf;
	}

	SOCKADDR_IN UDPSocket::getPeerAddr() {
		return this->m_peerAddr;
	}

	Client::Client() { }

	Client::~Client() { }

	TCPClient::TCPClient(TCPSocket* sock) {
		this->m_clntSocket = sock;
	}
	TCPClient::~TCPClient() {
		if (m_clntSocket != nullptr)
			delete m_clntSocket;
	}

	int TCPClient::connect() {
		int result = ::connect(
			m_clntSocket->getSocket(),
			(SOCKADDR*)&(m_clntSocket->getSockAddr()),
			sizeof(m_clntSocket->getSockAddr())
		);

		if (result == SOCKET_ERROR) {
			std::cout << "[Socket Class] " << "Connect Error" << std::endl;

			delete this->m_clntSocket;
			this->m_clntSocket = nullptr;

			return 0;
		}

		return 1;
	}

	TCPSocket* TCPClient::getSocket() {
		return this->m_clntSocket;
	}

	Server::Server(int port) {
		this->m_port = port;
	}

	Server::~Server() {
		delete this->m_servSocket;
	}

	TCPServer::TCPServer(int port)
		: Server::Server(port) {

		this->m_servSocket = new TCPSocket("", port);
	}
	TCPServer::~TCPServer() { }


	int TCPServer::bind() {
		const SOCKADDR_IN servAddr = m_servSocket->getSockAddr();
		if (::bind(
			m_servSocket->getSocket(),
			(SOCKADDR*)&servAddr,
			sizeof(servAddr)
		) == SOCKET_ERROR) {
			std::cout << "[Socket Class] " << "TCPServer Bind Error" << std::endl;
			return 0;
		}

		return 1;
	}

	int TCPServer::listen(size_t size) {
		if (::listen(
			m_servSocket->getSocket(),
			size
		) == SOCKET_ERROR) {
			std::cout << "[Socket Class] " << "TCPServer Listen Error" << std::endl;
			return 0;
		}

		return 1;
	}

	TCPSocket* TCPServer::accept() {
		SOCKADDR_IN clntAddr;
		int szClntAddr = sizeof(clntAddr);

		SOCKET clntSock = ::accept(
			m_servSocket->getSocket(),
			(SOCKADDR*)&clntAddr,
			&szClntAddr
		);

		if (clntSock == INVALID_SOCKET) {
			std::cout << "[Socket Class] " << "TCPServer Accept Error" << std::endl;
			return nullptr;
		}

		TCPSocket* clntCSock = nullptr;

		clntCSock = new TCPSocket(clntSock, clntAddr);

		return clntCSock;
	}

	TCPSocket* TCPServer::getSocket() {
		return dynamic_cast<TCPSocket*>(this->m_servSocket);
	}
	UDPServer::UDPServer(int port)
		: Server::Server(port) {

		this->m_servSocket = new UDPSocket("", port);
	}

	UDPServer::~UDPServer() { }

	int UDPServer::bind() {
		const SOCKADDR_IN servAddr = (m_servSocket->getSockAddr());
		if (::bind(
			m_servSocket->getSocket(),
			(SOCKADDR*)&servAddr,
			sizeof(servAddr)
		) == SOCKET_ERROR) {
			std::cout << "[Socket Class] " << "UDPServer Bind Error" << std::endl;

			return 0;
		}

		return 1;
	}

	byte* UDPServer::recv(size_t size, size_t& len, UDPSocket*& clntSock) {

		byte* data = this->m_servSocket->recv(size, len);

		UDPSocket* tSock = new UDPSocket(
			this->m_servSocket->getSocket(),
			dynamic_cast<UDPSocket*>(
				this->m_servSocket
				)->getPeerAddr()
		);

		if (clntSock == nullptr)
			clntSock = tSock;
		else
			if (*clntSock != *tSock)
				clntSock = tSock;

		return data;
	}

	bool UDPServer::send(const UDPSocket* clntSock, const byte* data, size_t len) {
		return clntSock->send(data, len);
	}
}