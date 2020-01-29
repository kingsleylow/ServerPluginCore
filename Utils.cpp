#include "Stdafx.h"
#include "Utils.h"
#include "shlwapi.h"
#include "dbghelp.h"
#pragma comment(lib, "Dbghelp.lib")
#pragma comment(lib, "Shlwapi.lib")
 
#include "md5.h" 
  
Utils::Utils()
{

}


Utils::~Utils()
{
}

 


bool Utils::isExistFolder(string path) {
	if (PathIsDirectory(path.c_str())) {		 
		return true;
	}
	else {		 
		return false;
	}
}

bool Utils::createFolder(string path) {

	if (!PathIsDirectory(path.c_str())) {
		if (MakeSureDirectoryPathExists(path.c_str())) {		 
			return true;
		}
		else {	 
			return false;
		}
	}

	return false;
}



string Utils::MD5Crypt(string strPlain) {
	MD5_CTX mdContext;
	int bytes = 0;
	unsigned char data[1024] = { 0 };

	MD5Init(&mdContext);
	MD5Update(&mdContext, (unsigned char*)const_cast<char*>(strPlain.c_str()), strPlain.size());
	MD5Final(&mdContext);

	string md5;
	char buf[3];
	for (int i = 0; i < 16; i++)
	{
		sprintf_s(buf, "%02x", mdContext.digest[i]);
		md5.append(buf);
	}
	return md5;
}
