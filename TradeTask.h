#pragma once
class TradeTask
{
public:
	TradeTask();
	~TradeTask();
	bool follower_disable;
	string portal_id;
	string follower_id;
	
	double follower_ratio;
 
 
	int follower_max_vol;
 
	string task_id;
	int auto_reconciliation;

	int master_server_id;
	int follower_server_id;
	bool master_disable;
	int master_strategy;
	double master_ratio;
	string master_id;
	double follower_max_drawback;
};

