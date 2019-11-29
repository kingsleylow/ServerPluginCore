#pragma once
class TradeTask
{
public:
	TradeTask();
	~TradeTask();
	bool disable;
	string portal_id;
	string follower_id;
	string master_id;
	double ratio;
	string symbol;
	int strategy;
	int max_vol;
	int min_vol;
	string task_id;


	int master_server_id;
	int follower_server_id;

};

