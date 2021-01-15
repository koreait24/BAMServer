#pragma once

#include <iostream>
#include <algorithm>
#include <string>
#include <list>
#include <boost\asio.hpp>
#include <boost\bind.hpp>
#include <boost\thread.hpp>
#include "TCP_Client.h"
#include "Structset.h"

using namespace std;

int main()
{
	boost::asio::io_service io_service;
	boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string(SERVER_IP), PORT_NUMBER);

	TCP_Client client(io_service);
	client.Connect(endpoint);

	boost::thread thread(boost::bind(&boost::asio::io_service::run, &io_service));
	boost::thread t_Chat(boost::bind(&TCP_Client::PostChat, &client));
	boost::thread t_Recv(boost::bind(&TCP_Client::PostRecieve, &client));
	t_Chat.join();
	t_Recv.join();
	thread.join();

	cout << "匙飘况农 立加 辆丰" << endl;
	getchar();
	return 0;
}
