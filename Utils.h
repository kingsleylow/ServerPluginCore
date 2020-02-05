#pragma once
#include "StdAfx.h"


using namespace std;
 

class logger;
class Utils
{
private:
	
public:
	Utils();
	~Utils();
	static bool Utils::isExistFolder(string path);
    static bool Utils::createFolder(string path);
	static string Utils::MD5Crypt(string strPlain);
	static int Utils::CheckGroup(char* grouplist, const char *group);
	static int Utils::CheckTemplate(char* expr, char* tok_end, const char* group, char* prev, int* deep);
};

