#pragma once
#include <stdlib.h>
#include <boost\asio.hpp>

#define		S			"first section"

#define		S_1_1		"log_in_id"
#define		S_1_2		"log_in_pw"
#define		S_1_3		"log_in_complete"

#define		S_2_1		"reg_id"
#define		S_2_2		"reg_pw"

#define		S_3			"chat_on"

#define		C_D			"client disconnected"
#define		C_A			"Connection Already"

class Session;

struct USER_USING
{
	char user_id[20];
	char user_pw[20];
	char file_name[100];
};

struct STATUS
{
	int user_wait;
	int user_connect;
};

struct Registry
{
	void callADM(char get_id[], char get_pw[]);
	bool PostRegistry(char message[], int value);
	bool getADM(char out_id[], char out_pw[]);
	bool getADM(char out_id[]);
};