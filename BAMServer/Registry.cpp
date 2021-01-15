#pragma once

#include <iostream>
#include <fstream>
#include "Structset.h"

using namespace std;

void Registry::callADM(char get_id[], char get_pw[])					// 계정 생성 세션
{
	USER_USING uln;

	strcpy_s(uln.user_id, 20, get_id);
	strcpy_s(uln.user_pw, 20, get_pw);

	strcpy_s(get_id, 20, get_id);
	strcpy_s(get_pw, 20, get_pw);

	char user_file[30] = "\0";
	strcat_s(user_file, "admin\\");
	strcat_s(user_file, get_id);
	strcat_s(user_file, ".adm");

	ofstream reg; 
	reg.open(user_file);

	if (reg.fail())
	{
		system("mkdir admin");
		reg.open(user_file);
	}

	reg << get_id << endl;
	reg << get_pw << endl;

	reg.close();
}

bool Registry::PostRegistry(char message[], int value)
{
	USER_USING uln;

	char* context = NULL;
	
	strcpy_s(uln.user_id, 20, strtok_s(message, " ", &context));
	strcpy_s(uln.user_pw , 20, strtok_s(NULL, " ", &context));

	if (value == 1)
		cout << "call login" << endl;
		return getADM(uln.user_id, uln.user_pw);
	if (value == 2)
		cout << "call reg" << endl;
		return getADM(uln.user_id);
}

bool Registry::getADM(char out_id[])								// 계정 중복 여부 확인 세션
{
	USER_USING uln;

	strcpy_s(uln.user_id, 20, out_id);

	char user_file[30] = "\0";
	strcat_s(user_file, "admin\\");
	strcat_s(user_file, out_id);
	strcat_s(user_file, ".adm");

	ifstream reg;
	reg.open(user_file);

	if (reg.fail())
	{
		return true;
	}

	else
	{
		return false;
	}

	reg.close();
}

bool Registry::getADM(char out_id[], char out_pw[])					// 로그인 판단 세션
{
	char get_id[20] = "\0";
	char get_pw[20] = "\0";
	USER_USING uln;

	strcpy_s(uln.user_id, 20, out_id);
	strcpy_s(uln.user_pw, 20, out_pw);

	char user_file[30] = "\0";
	strcat_s(user_file, "admin\\");
	strcat_s(user_file, out_id);
	strcat_s(user_file, ".adm");

	ifstream reg;
	reg.open(user_file);

	if (reg.fail())
	{
		cout << "file open failed" << endl;
		return false;
	}

	else
	{
		reg.getline(get_id, 20);
		if (!strcmp(get_id, out_id))
		{
			cout << get_id << " : " << out_id << endl;
			reg.getline(get_pw, 20);
			if (!strcmp(get_pw, out_pw))
			{
				cout << get_pw << " : " << out_pw << endl;
				reg.close();
				return true;
			}
			reg.close();
			return false;
		}
	}

	reg.close();
}