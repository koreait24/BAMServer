#include <iostream>
#include <boost\bind.hpp>
#include <boost\smart_ptr.hpp>
#include <boost\asio.hpp>
#include <boost\thread.hpp>
#include <fstream>
#include "TCP_Server.h"

using namespace std;

int main()
{
	boost::asio::io_service io_service;
	TCP_Server server(io_service);
	boost::thread thread(boost::bind(&boost::asio::io_service::run, &io_service));
	thread.join();

	cout << "네트워크 접속 종료" << endl;
	getchar();
	return 0;
}
