#pragma once

#include <iostream>
#include <algorithm>
#include <string>
#include <list>
#include <map>
#include <boost\bind.hpp>
#include <boost\asio.hpp>
#include "TCP_Server.h"

void TCP_Server::StartAccept()
{
	try
	{
		m_pSession = boost::make_shared<Session>(m_acceptor.get_io_service(), index);

		m_acceptor.async_accept(m_pSession->Socket(),
				boost::bind(&TCP_Server::handle_accept, this, m_pSession,
				boost::asio::placeholders::error));
	}

	catch (boost::bad_weak_ptr e)
	{
		cout << e.what() << endl;
	}
}

void TCP_Server::handle_accept(boost::shared_ptr<Session> pSession, 
						 const boost::system::error_code& error)
{
	if (m_Session.size() < 100)
	{
		if (!error)
		{
			cout << "클라이언트 접속 성공" << endl;

			mapdata.map_Session = pSession;

			cout << m_Session.size() << endl;

			m_Session.insert(pair<int, MAPDATA>(index, mapdata));

			for (auto i = m_Session.begin(); i != m_Session.end();)
			{
				if (!strcmp(i->second.map_Session->status, C_D))									// Client Disconnected 상태인 세션을 맵에서 제거
				{
					cout << i->second.map_Session.get() << " : " << i->second.map_Session->status << " is deleted " << endl;
					i = m_Session.erase(i);
				}

				else
				{
					cout << i->second.map_Session.get() << " : " << i->second.map_Session->status << endl;
					++i; 
				}
			}

			pMap = boost::make_shared<map <int, MAPDATA>>(m_Session);

			for (auto i = m_Session.begin(); i != m_Session.end(); ++i)								// 갱신된 맵의 상태를 모든 세션에 전달
			{
				i->second.map_Session->UpdateMap(pMap);
			}

			pSession->GetMap(pMap, pSession);

			index++;

			boost::thread t_Accept = boost::thread(boost::bind(&TCP_Server::StartAccept, this));
			t_Accept.join();

			pSession.reset();
			m_pSession.reset();
			pMap.reset();
		}
	}
}

void TCP_Server::handle_write(boost::shared_ptr<Session> pSession, 
							  boost::array<boost::asio::mutable_buffer, 3> buf)
{
	pSession->PostWrite(buf);
	
}

void TCP_Server::PostWrite(boost::shared_ptr<map <int, TCP_Server::MAPDATA>> get_Session,
						   boost::array<boost::asio::mutable_buffer, 3> buf)
{
	cout << get_Session->size() << endl;

	for (auto i = get_Session->begin(); i != get_Session->end(); ++i)
	{
		if (!strcmp(i->second.map_Session->status, S_3))											// Chat On 상태인 세션에만 채팅 전달
		{
			cout << "send to : " << i->second.map_Session.get() << ", message : " << (char*)buf[2].data() << endl;
			sig.connect(0, boost::bind(&TCP_Server::handle_write, this, i->second.map_Session, buf));
		}
	}

	sig();
	sig.disconnect_all_slots();

	buf.empty();
}