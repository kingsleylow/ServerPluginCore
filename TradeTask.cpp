#include "stdafx.h"
#include "TradeTask.h"
#define STRATEGY_ERROR -1
#define STRATEGY_POSITIVE_BY_ORDER 0 /// trace order
#define STRATEGY_NEGATIVE_BY_ORDER 1
#define STRATEGY_TRADE_BY_ORDER 2 /// trace summary

#define INVAILD_SERVER_ID -1

#define NO_RECONCILIATION 0
#define ALL_RECONCILIATION 1
#define CONDITION_RECONCILIATION 2

#define STRATEGY_ERROR -1
#define STRATEGY_POSITIVE 0
#define STRATEGY_NEGATIVE 1

#define DEFAULT_RATIO 1
#define DEFAULT_MAX_VOL 0
TradeTask::TradeTask()
{
	this->follower_disable = true;
	this->portal_id = "";
	this->follower_id = "";

	this->follower_ratio = DEFAULT_RATIO;
	 
 
	this->follower_max_vol = DEFAULT_MAX_VOL;

	this->task_id = "";
	this->auto_reconciliation = NO_RECONCILIATION;

	this->master_server_id = INVAILD_SERVER_ID;
	this->follower_server_id= INVAILD_SERVER_ID;
	this->master_disable = true;
	this->master_strategy = STRATEGY_ERROR;
	this->master_ratio = DEFAULT_RATIO;
	this->master_id = "";
}


TradeTask::~TradeTask()
{
}


 