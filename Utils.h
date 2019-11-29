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
};

