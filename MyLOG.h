#pragma once
#include "StdAfx.h"
class MyLOG
{

private:
	static  MyLOG* instance;
	MyLOG();
	~MyLOG();
public:
	static MyLOG* getInstance();

	void LOG_I(string msg);
};

