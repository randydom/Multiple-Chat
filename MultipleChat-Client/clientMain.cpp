
#include "MetaSocket.hpp"

#include <vector>
#include <iostream>
#include <thread>


using namespace MetaSocket;


void recvDataFuuc(TCPSocket* sock) {
	size_t recvLen;
	while (1) {
		byte* headerData = sock->recv(6, recvLen);
		if (headerData == nullptr || recvLen != 6) break;

		//std::cout << "header : ";
		//for (size_t i = 0; i < recvLen; i++)
		//	printf("%02X ", headerData[i]);
		//std::cout << std::endl;

		unsigned short messageType =
			headerData[0] << 8 |
			headerData[1];
		int dataLen =
			headerData[2] << 24 |
			headerData[3] << 16 |
			headerData[4] << 8 |
			headerData[5];

		delete headerData;

		//std::cout << "Header : " << messageType << " " << dataLen << std::endl;

		byte* data = sock->recv(dataLen, recvLen);
		if (data == nullptr || recvLen != dataLen) break;

		size_t pos = 0;

		unsigned short len;
		std::string name;
		std::string msg;

		len = 
			data[pos] << 8 |
			data[pos+1];
		pos += 2;

		for (size_t i = 0; i < len; i++)
			name += data[pos++];
		len =
			data[pos] << 8 |
			data[pos+1];
		pos += 2;
		for (size_t i = 0; i < len; i++)
			msg += data[pos++];


		std::cout << name << " : " << msg << std::endl;
	}
}

int main() {

	std::string name;
	std::cout << "name : ";
	std::cin >> name;

	TCPClient clnt(new TCPSocket("127.0.0.1", 8888));
	clnt.connect();

	auto sock = clnt.getSocket();

	std::thread t = std::thread(&recvDataFuuc, sock);

	while (1) {
		std::vector<byte> vec;
		vec.push_back(0x00);
		vec.push_back(0x01);

		std::string str;
		std::cin >> str;
		if (str == "exit")
			break;

		vec.push_back((byte)(name.length() + str.length() + 4) >> 8);
		vec.push_back((byte)(name.length() + str.length() + 4) >> 16);
		vec.push_back((byte)(name.length() + str.length() + 4) >> 24);
		vec.push_back((byte)(name.length() + str.length() + 4) >> 32);

		vec.push_back((byte)name.length() >> 24);
		vec.push_back((byte)name.length() >> 32);
		for (size_t i = 0; i < name.length(); i++)
			vec.push_back(name[i]);

		vec.push_back((byte)str.length() >> 24);
		vec.push_back((byte)str.length() >> 32);
		for (size_t i = 0; i < str.length(); i++)
			vec.push_back(str[i]);


		//std::cout << "Data : ";
		//for (auto iter : vec)
		//	printf("%02X ", iter);
		//std::cout << std::endl;


		sock->send(vec);

	}

	sock->close();

	t.join();

	return 0;
}