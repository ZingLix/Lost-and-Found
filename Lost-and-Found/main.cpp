#include <iostream>
#include "boost/asio.hpp"
#include "Server.h"


int main(int argc, char* argv[]) {
	Server s(9981);
	s.start();
}

