#pragma once

#pragma comment(lib, "ws2_32.lib")

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>

#include <string>
#include <vector>

namespace MetaSocket {
	class Socket {
	public:
		enum class AF { INET = AF_INET };
		enum class Type { TCP = SOCK_STREAM, UDP = SOCK_DGRAM };

	private:
		static int init();
		static int cleanUp();

		static int g_socketCnt;

	protected:
		AF          m_addressFamily;
		Type        m_protocol;

		SOCKET      m_socket;
		SOCKADDR_IN m_sockAddr;

	private:
		Socket();
	protected:
		Socket(const std::string, const unsigned int, AF, Type);
		Socket(const SOCKET, const SOCKADDR_IN);

	protected:
		virtual void  sendF(int&, const byte* data, const size_t size) const = 0;
		virtual byte* recvF(int&, const size_t size)                   const = 0;

	private:
		bool errorCheck(const int&) const;

	public:
		virtual ~Socket();

		void close();

		bool operator==(const Socket&) const;
		bool operator!=(const Socket&) const;

		Type        getSockType() const;
		SOCKET      getSocket()   const;
		SOCKADDR_IN getSockAddr() const;

		unsigned int setIp(const std::string ipStr);


		bool  send(const byte* data, size_t size) const;
		bool  send(const std::vector<byte>&)      const;
		bool  send(const std::string&)            const;

		byte*             recv(size_t size, size_t& recvLen) const;
		std::vector<byte> recv(size_t size)                  const;
	};
	class TCPSocket : public Socket {
	protected:
		void  sendF(int&, const byte*, const size_t) const override;
		byte* recvF(int&, const size_t)              const override;

	public:
		TCPSocket(const std::string ip, const unsigned int port);
		TCPSocket(const SOCKET sock, const SOCKADDR_IN sockAddr);

		~TCPSocket();
	};
	class UDPSocket : public Socket {
	private:
		SOCKADDR_IN m_peerAddr;

	protected:
		void  sendF(int&, const byte* data, const size_t size) const override;
		byte* recvF(int&, const size_t size)                   const override;

	public:
		UDPSocket(const std::string ip, const unsigned int port);
		UDPSocket(const SOCKET sock, const SOCKADDR_IN sockAddr);

		~UDPSocket();

		SOCKADDR_IN getPeerAddr();
	};

	class Client {
	protected:
		Client();

		~Client();
	};
	class TCPClient : public Client {
	private:
		TCPSocket * m_clntSocket;
	public:
		TCPClient(TCPSocket*);
		~TCPClient();

		int connect();

		TCPSocket* getSocket();
	};

	class Server {
	protected:
		Socket * m_servSocket;

		int m_port;
	protected:
		Server(int port);
		virtual ~Server();

		virtual int bind() = 0;
	};
	class TCPServer : public Server {
	public:
		TCPServer(int port);
		~TCPServer();

		int bind() override;

		int listen(size_t size);

		TCPSocket* accept();

		TCPSocket* getSocket();
	};
	class UDPServer : public Server {
	public:
		UDPServer(int port);
		~UDPServer();

		int bind() override;

		byte* recv(size_t size, size_t& len, UDPSocket*& clntSock);
		bool  send(const UDPSocket*, const byte*, const size_t len);
	};
}