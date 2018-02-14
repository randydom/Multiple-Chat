
#include "Server.hpp"


using namespace MultipleChat;
int main() {
	Server::getInstance()->run(8888);

	return 0;
}