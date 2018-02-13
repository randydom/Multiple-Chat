#pragma once

#include "Singleton.hpp"

#include <vector>

namespace MultipleChat {
	class Server : public Singleton<Server> {
	private:

	public:
		void run();
	};
}