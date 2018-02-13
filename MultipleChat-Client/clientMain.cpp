
#include "MetaSocket.hpp"

#include <vector>
#include <iostream>
using namespace MetaSocket;
int main() {
	TCPClient clnt(new TCPSocket("127.0.0.1", 8888));
	clnt.connect();

	auto sock = clnt.getSocket();

	while (1) {

		std::vector<byte> vec;
		vec.push_back(0x00);
		vec.push_back(0x01);

		std::string str;
		std::cout << "Msg : ";
		std::cin >> str;

		vec.push_back((byte)str.length() >> 8);
		vec.push_back((byte)str.length() >> 16);
		vec.push_back((byte)str.length() >> 24);
		vec.push_back((byte)str.length() >> 32);

		for (size_t i = 0; i < str.length(); i++)
			vec.push_back(str[i]);

		sock->send(vec);

	}
	return 0;
}