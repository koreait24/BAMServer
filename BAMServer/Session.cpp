#pragma once
#include <iostream>
#include <io.h>
#include <algorithm>
#include <string>
#include <list>
#include <filesystem>
#include <boost\bind.hpp>
#include <boost\asio.hpp>
#include <boost\signals2.hpp>
#include <boost\smart_ptr.hpp>
#include "Registry.h"
#include "Structset.h"
#include "TCP_Server.h"

void Session::handle_write(const boost::system::error_code&, size_t)
{
}
 
void Session::handle_receive(const::boost::system::error_code& error, size_t bytes_transferred)
{
	if (!m_Socket.is_open())
	{
		return;
	}

	TCP = boost::make_shared<TCP_Server>(m_Socket.get_io_service());
	TCP->weak_from_this();

	if (error)
	{
		if (error == boost::asio::error::eof)
		{
			cout << "클라이언트와 연결이 끊어졌습니다." << endl;;
		}

		else
		{
			cout << "error No : " << error.value() << " error Message: " << error.message() << endl;
		}

		strcpy_s(status, 20, C_D);																// Client Disconnected 상태
		m_Socket.close();
		recv_buf.empty();
		TCP.reset();
		return;
	}

	if (!strcmp(recv_command, "1") || !strcmp(recv_command, "2"))								// 1번 command는 로그인 세션, 2번 command는 회원가입 세션
	{
		strcpy_s(ULN.user_id, 20, "\0");
		strcpy_s(ULN.user_id, 20, "\0");

		char* context = NULL;

		if (!strcmp(recv_command, "1"))
		{
			cout << recv_message << endl;
			strcpy_s(message, 128, recv_message);
			if (newRegistry.PostRegistry(recv_message, 1))
			{
				strcpy_s(ULN.user_id, 20, strtok_s(message, " ", &context));
				strcpy_s(ULN.user_pw, 20, strtok_s(NULL, " ", &context));

				for (auto i = MAP->begin(); i!=MAP->end(); i++)
				{
					if (!strcmp(i->second.map_Session->ULN.user_id, ULN.user_id) && (i->first != this->index))
					{
						strcpy_s(status, 20, C_A);
					}
				}

				if (!strcmp(status, C_A))
				{
					strcpy_s(command, 4, "__1");
					strcpy_s(status, 20, S);
				}

				else
				{
					strcpy_s(status, 20, S_3);
					strcpy_s(command, 4, "1_");

					strcpy_s(user_id, 20, "Server");
					strcpy_s(message, 128, ULN.user_id);
					strcat_s(message, " 님의 접속을 환영합니다. (/?을 통해 도움말을 확인할 수 있습니다)");
					strcpy_s(logFilename, sizeof(logFilename), "Logs\\");
					strcat(logFilename, ULN.user_id);
					strcat(logFilename, "_Logs.txt");
					chatLog.open(logFilename);
				}
			}

			else
			{
				strcpy_s(command, 4, "_1");
			}
		}

		else if (!strcmp(recv_command, "2"))
		{
			cout << recv_message << endl;
			strcpy_s(message, 128, recv_message);
			if (!newRegistry.PostRegistry(recv_message, 2))
			{
				strcpy_s(status, 20, S_3);
				strcpy_s(command, 4, "2_"); 

				strcpy_s(ULN.user_id, 20, strtok_s(message, " ", &context));
				strcpy_s(ULN.user_pw, 20, strtok_s(NULL, " ", &context));

				newRegistry.callADM(ULN.user_id, ULN.user_pw);

				strcpy_s(user_id, 20, "Server");
				strcpy_s(message, 128, ULN.user_id);
				strcat_s(message, " 님의 접속을 환영합니다. (/?을 통해 도움말을 확인할 수 있습니다)");
				strcpy_s(logFilename, sizeof(logFilename), "Logs\\");
				strcat(logFilename, ULN.user_id);
				strcat(logFilename, "_Logs.txt");
				chatLog.open(logFilename);
			}
			else
			{
				strcpy_s(command, 4, "_2");
			}
		}
		NomalWrite();
		PostReceive();
		strcpy_s(message, 128, "\0");
	}

	if (!strcmp(recv_command, "3"))
	{
		if (!strcmp(recv_message, "/v"))												// /v 명령어는 클라이언트로 접속자 목록을 제공한다.
		{
			strcpy_s(command, 4, "3");
			strcpy_s(user_id, 20, "Server");
			strcpy_s(message, 128, "\0");

			for (auto i = MAP->begin();  i != MAP->end(); i++)
			{
				strcat_s(message, i->second.map_Session->ULN.user_id);
				if (i != MAP->end())
				{
					strcat_s(message, " | ");
				}
			}
			NomalWrite();
			PostReceive();
		}

		else if (!strcmp(recv_message, "/f"))											// 클라이언트에서 파일리스트 공개 요청을 받는다
		{
			strcpy_s(command, 4, "3");
			strcpy_s(user_id, 20, "Server");
			strcpy_s(message, 2000, "\0");

			string path = "C:\\Users\\USER\\Desktop\\Study\\BAM\\BAMServer\\BAMServer\\downloads\\*.*";

			struct _finddata_t fd;

			intptr_t handle;

			if ((handle = _findfirst(path.c_str(), &fd)) == -1L)
				cout << "No file in directory!" << endl;
			do
			{
				strcat(message, fd.name);
				strcat(message, " | ");
			} while (_findnext(handle, &fd) == 0);

			_findclose(handle);

			NomalWrite();
			PostReceive();
		}

		else
		{
			TCP->PostWrite(MAP, this->recv_buf);
		}
	}

	if (!strcmp(recv_command, "/u"))										// 클라이언트에서 업로드 요청을 받는다 - 서버는 다운로드 상태
	{
		StartDownload();
	}

	else if (!strcmp(recv_command, "/d"))										// 클라이언트에서 다운로드 요청을 받는다 - 서버는 업로드 상태
	{
		strcpy_s(command, 4, "/d");
		NomalWrite();

		strcpy_s(ULN.file_name, 100, "downloads\\");
		strcat_s(ULN.file_name, recv_message);

		start_fileupload(ULN.file_name);
	}

	TCP.reset();
	buf.empty();
	recv_buf.empty();
}

void Session::handle_uploadfile(const boost::system::error_code& error)
{
	if (!error)
	{
		if (sourceFile.eof() == false)
		{
			sourceFile.read(f_write, (streamsize)boost::asio::buffer(f_write).size());
			if (sourceFile.gcount() <= 0)
			{
				cout << "read file error" << endl;
				chatLog << __FUNCTION__ << " : " << "read file error" << endl;
				PostReceive();
				return;
			}

			cout << "send : "		  << sourceFile.gcount()
				 << "bytes, total = " << sourceFile.tellg() << " bytes\n";

			char _buf[20] = "\0";

			strcpy_s(user_id, 20, itoa(sourceFile.gcount(), _buf, 10));  

			buf.empty();
			buf = { boost::asio::buffer(command), boost::asio::buffer(user_id), boost::asio::buffer(f_write) };

			boost::asio::async_write(m_Socket, buf,
				boost::bind(&Session::handle_write, this,
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred));
			handle_uploadfile(error);

			if (error)
			{
				std::cout << "send error : " << error.message() << endl;
				chatLog << "send error : " << error.message() << endl;
				sourceFile.close();
				PostReceive();
				return;
			}
		}
		else
		{
			cout << "send success : " << error.message() << endl;
			chatLog << __FUNCTION__ << " : " << "send success : " << error.message() << endl;
			strcpy_s(f_write, sizeof(f_write), "\0");
			sourceFile.close();
			PostReceive();
			return;
		}
	}
}

void Session::start_fileupload(char filename[])
{
	cout << __FUNCTION__ << endl;
	sourceFile.open(filename, ios_base::binary|ios_base::ate);

	if (!sourceFile)
	{
		cout << "failed to open : " << filename << endl;
		chatLog << __FUNCTION__ << " : " << "failed to open : " << filename << endl;
		strcpy_s(command, "e");
		NomalWrite();
		PostReceive();
		return;
	}

	size_t fileSize = sourceFile.tellg();
	sourceFile.seekg(0);
	char _buf[20] = "\0";
	ostream requestStream(&request);

	strcpy_s(command, 4, "/d");
	strcpy_s(user_id, 20, itoa(fileSize, _buf, 10));
	strcpy_s(message, 2000, filename);

	buf.empty();
	buf = {boost::asio::buffer(command), boost::asio::buffer(user_id), boost::asio::buffer(message)};

	boost::asio::async_write(m_Socket, buf,
			boost::bind(&Session::handle_uploadfile, shared_from_this(),
			boost::asio::placeholders::error));
}

void Session::StartDownload()
{
	cout << __FUNCTION__ << endl;
	m_Socket.async_read_some(recv_buf,
		boost::bind(&Session::handle_download, shared_from_this(),
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
}

void Session::handle_download(const boost::system::error_code &error, size_t bytes_transferred)
{
	chatLog << __FUNCTION__ << " : " << recv_command << " : " << recv_id << endl;
	if (error)
	{
		cout << "error No : " << error.value() << " error Message: " << error.message() << endl;
		chatLog << __FUNCTION__ << " : " << "error No : " << error.value() << " error Message: " << error.message() << endl;
		PostReceive();
		return;
	}

	cout << __FUNCTION__ << " : " << recv_command << endl;
	if (!strcmp(recv_command, "e"))
	{
		chatLog << __FUNCTION__ << " : " << "download failed!" << endl;
		cout << "download failed!" << endl;
		PostReceive();
		return;
	}

	cout << __FUNCTION__ << "(" << bytes_transferred << ")"
		<< ", in_avail = " << recv_id
		<< ", size = " << recv_id
		<< ", file_dir = " << recv_message
		<< "\n" << endl;

	istream requestStream(&requestBuf);
	string filePath = recv_message;
	requestStream >> filePath;
	requestStream >> recv_id;

	requestStream.read(f_read, 2);

	std::cout << filePath << " size is " << 
		recv_id << ", tellg = " <<
		requestStream.tellg() << endl;

	filePath = recv_message;

	outputFile.open(filePath.c_str(), ios_base::binary);

	if (!outputFile)
	{
		cout << "file open failed : " << filePath << endl;
		chatLog << __FUNCTION__ << " : " << "file open failed : " << filePath << endl;
		PostReceive();
		return;
	}

	do
	{
		requestStream.read(f_read, (streamsize)strlen(recv_message));
		cout << __FUNCTION__ << " : " << " write  : " << strlen(recv_message) << " Bytes.\n";
		outputFile.write(f_read, requestStream.gcount());
	} while (requestStream.gcount() > 0);

	m_Socket.async_read_some(recv_buf,
		boost::bind(&Session::handle_readfile, shared_from_this(),
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
}

void Session::handle_readfile(const::boost::system::error_code& error, size_t bytes_transferred)
{
	if (bytes_transferred > 0)
	{
		outputFile.write(recv_message, (streamsize)atoi(recv_id));
		cout << __FUNCTION__ << " : " << " recv : " << recv_id << " Bytes : " << endl;
		if (outputFile.tellp() >= (streamsize)atoi(recv_id))
		{
			strcpy_s (recv_command, 4, "\0");
			outputFile.close();
			PostReceive();
			return;
		}
	}

	if (error)
	{
		cout << error.message() << endl;
		chatLog << __FUNCTION__ << " : " << error.message() << endl;
		return;
	}

	m_Socket.async_read_some(recv_buf,
		boost::bind(&Session::handle_readfile, shared_from_this(),
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
}