#pragma once

#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>
#include <list>
#include <map>
#include <boost\bind.hpp>
#include <boost\asio.hpp>
#include <boost\smart_ptr.hpp>
#include <boost\function.hpp>
#include <boost\thread.hpp>
#include <boost\array.hpp>
#include <boost\signals2.hpp>
#include "Structset.h"

using namespace std;

const unsigned short PORT_NUMBER = 31400;

class TCP_Server;
class Session;

class TCP_Server : public boost::enable_shared_from_this<TCP_Server>
{
public:
	struct MAPDATA
	{
		boost::shared_ptr<Session> map_Session;
	};

public:
	TCP_Server(boost::asio::io_service& io_service) : m_acceptor(io_service,
			   boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), PORT_NUMBER))
	{
		cout << "서버 생성자 생성" << endl;
		StartAccept();
	}

	void PostWrite(boost::shared_ptr<map <int, TCP_Server::MAPDATA>> get_Session,
				   boost::array<boost::asio::mutable_buffer, 3> buf);

	~TCP_Server()
	{
		if (m_pSession != nullptr)
		{
			cout << "서버 소멸자 생성" << " , 참조수:"<< m_pSession.use_count() << endl;

			sig.disconnect_all_slots();
			m_pSession.reset();//			delete m_pSession;
		}
	}

private:
	int index = 0;
	
	map <int, MAPDATA>						m_Session;
	MAPDATA									mapdata;
	boost::shared_ptr<Session>				m_pSession;
	boost::shared_ptr<map <int, MAPDATA>>	pMap;
	boost::asio::ip::tcp::acceptor			m_acceptor;
	boost::signals2::signal <void()>		sig;

private:
	void StartAccept();
	void handle_accept(boost::shared_ptr<Session> pSession, 
				 const boost::system::error_code& error);

	void handle_write(boost::shared_ptr<Session> pSession,
					  boost::array<boost::asio::mutable_buffer, 3> buf);
};

class Session : public boost::enable_shared_from_this<Session>	//  Cilent 로 데이터를 받는 세션을 구성하는 클래스
{
public:
	Session(boost::asio::io_service& io_service, int index) : 
		m_Socket(io_service)
	{ this->index = index; }
	boost::asio::ip::tcp::socket &Socket(){ return m_Socket;}

	void PostReceive()
	{
		try
		{
			m_Socket.async_read_some(recv_buf,
				boost::bind(&Session::handle_receive, shared_from_this(),
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred));
		}

		catch (boost::bad_weak_ptr e)
		{
			cout << e.what() << endl;
		}
	}

	void GetMap(boost::shared_ptr<map <int, TCP_Server::MAPDATA>> &m_Session, 
				boost::shared_ptr<Session> pSession)
	{
		MAP = m_Session;
		m_pSession = pSession;
		cout << "세션 생성자 생성" << "맵사이즈: " << MAP->size() << 
									",  참조수: " << this->weak_from_this().use_count() << endl;

		boost::thread t_Recv(boost::bind(&Session::PostReceive, this));
		t_Recv.join();
	}

	void UpdateMap(boost::shared_ptr<map <int, TCP_Server::MAPDATA>> &m_Session)
	{
		MAP = m_Session;
	}

	void PostWrite(boost::array<boost::asio::mutable_buffer, 3> buf)
	{
		this->buf = buf;

		try
		{
			async_write(m_Socket, buf,
					boost::bind(&Session::handle_write, shared_from_this(),
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred));
			PostReceive();
		}

		catch (boost::bad_weak_ptr e)
		{
			cout << e.what() << endl;
		}
	}

	void  NomalWrite()
	{
		buf = { boost::asio::buffer(command), boost::asio::buffer(user_id), boost::asio::buffer(message) };
		async_write(m_Socket, buf,
			boost::bind(&Session::handle_write, shared_from_this(),
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
		buf.empty();
	}

public:
	USER_USING ULN;
	char status[20] = S;

private:
	void handle_write(const boost::system::error_code&, size_t);
	void handle_receive(const::boost::system::error_code& error, size_t bytes_transferred);

	void handle_uploadfile(const boost::system::error_code & error);
	void start_fileupload(char filename[]);

	void StartDownload();
	void handle_download(const boost::system::error_code & error, size_t bytes_transferred);
	void handle_readfile(const::boost::system::error_code & error, size_t bytes_transferred);

private:
	Registry newRegistry;
	boost::shared_ptr<TCP_Server>							TCP;
	boost::shared_ptr<Session>								m_pSession;
	boost::shared_ptr<map <int, TCP_Server::MAPDATA>>		MAP;

	int	index;

	char command[4]				= "\0";
	char user_id[20]			= "\0";
	char message[2000]			= "\0";

	char recv_command[4]		= "\0";
	char recv_id[20]			= "\0";
	char recv_message[2000]		= "\0";

	boost::array<boost::asio::mutable_buffer, 3>			buf =
	{
		boost::asio::buffer(command),
		boost::asio::buffer(user_id),
		boost::asio::buffer(message)
	};

	boost::array<boost::asio::mutable_buffer, 3>			recv_buf =
	{
		boost::asio::buffer(recv_command),
		boost::asio::buffer(recv_id),
		boost::asio::buffer(recv_message)
	};

	boost::asio::ip::tcp::socket			m_Socket;

private:
	char									f_write[2000] = "\0";;
	boost::asio::streambuf					request;
	ifstream								sourceFile;

private:
	boost::asio::streambuf					requestBuf;
	char									f_read[40860] = "\0";
	size_t									fileSize;
	ofstream								outputFile;

private:
	ofstream								chatLog;
	char									logFilename[30] = "\0";
};
