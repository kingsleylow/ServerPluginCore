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


int Utils::CheckGroup(char* grouplist, const char *group)
{
	//--- проверки
	if (grouplist == NULL || group == NULL) return(FALSE);
	//--- проходим? по всем группа?
	char *tok_start = grouplist, end;
	int  res = TRUE, deep = 0, normal_mode;
	while (*tok_start != 0)
	{
		//--- пропусти?за?ты?
		while (*tok_start != 0 && *tok_start == ',') tok_start++;
		//---
		if (*tok_start == '!') { tok_start++; normal_mode = FALSE; }
		else                 normal_mode = TRUE;
		//--- найдем границ?токена
		char *tok_end = tok_start;
		while (*tok_end != ',' && *tok_end != 0) tok_end++;
		end = *tok_end; *tok_end = NULL;
		//---
		char *tp = tok_start;
		const char *gp = group;
		char *prev = NULL;
		//--- проходим по токену
		res = TRUE;
		while (tp != tok_end && *gp != NULL)
		{
			//--- нашл?звёздочк? проверяем ка?регэкс?
			if (*tp == '*')
			{
				deep = 0;
				if ((res = Utils::CheckTemplate(tp, tok_end, gp, prev, &deep)) == TRUE)
				{
					*tok_end = end;
					return(normal_mode);
				}
				break;
			}
			//--- просто проверяем
			if (*tp != *gp) { *tok_end = end; res = FALSE; break; }
			tp++; gp++;
		}
		//--- восстанавливае?
		*tok_end = end;
		//--- проверяем, мы нашл?точную цитату ?вс?хорошо?
		if (*gp == NULL && (tp == tok_end || *tp == '*') && res == TRUE) return(normal_mode);
		//--- перехо??следующему токену
		if (*tok_end == 0) break;
		tok_start = tok_end + 1;
	}
	//---
	return(FALSE);
}

int Utils::CheckTemplate(char* expr, char* tok_end, const char* group, char* prev, int* deep)
{
	char  tmp = 0;
	char *lastwc, *prev_tok;
	const char *cp;
	//---- check recursion depth
	if ((*deep)++ >= 10) return(FALSE);
	//---- skip repeats of start (*)
	while (*expr == '*' && expr != tok_end) expr++;
	if (expr == tok_end) return(TRUE);
	//---- find next star or end of line
	lastwc = expr;
	while (*lastwc != '*' && *lastwc != 0) lastwc++;
	//---- temporarily limit the line
	if ((tmp = *(lastwc)) != 0) // token not last in line?
	{
		tmp = *(lastwc); *(lastwc) = 0;
		if ((prev_tok = (char*)strstr(group, expr)) == NULL) { if (tmp != 0) *(lastwc) = tmp; return(FALSE); }
		*(lastwc) = tmp;
	}
	else // last token...
	{
		//---- checks
		cp = group + strlen(group);
		for (; cp >= group; cp--)
			if (*cp == expr[0] && strcmp(cp, expr) == 0) return(TRUE);
		return(FALSE);
	}
	//---- the order is broken?
	if (prev != NULL && prev_tok <= prev) return(FALSE);
	prev = prev_tok;
	//----
	group = prev_tok + (lastwc - expr - 1);
	//---- it is end?
	if (lastwc != tok_end) return CheckTemplate(lastwc, tok_end, group, prev, deep);
	//----
	return(TRUE);
}