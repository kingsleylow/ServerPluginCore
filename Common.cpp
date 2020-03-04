#include "stdafx.h"
#include "Common.h"


int  GetToOrderComment(char comm[]) {

	string comStr = string(comm);
	int len = strlen(comm);

	if (comStr.find("to") != std::string::npos

		) {
		try
		{

			size_t pos = comStr.find("#");
			if (pos == std::string::npos) {
				// no #
				return 0;

			}
			int end = len - 1;
			for (int i = pos + 1; i < len; i++) {
				if (!isdigit(comm[i])) {
					end = i;
				}
			}

			string sub = comStr.substr(pos + 1, end - pos);
			return stoi(sub);


		}
		catch (const std::exception&)
		{
			return 0;
		}

	}

	return 0;

}

int GetFromOrderComment(char comm[]) {

	string comStr = string(comm);
	int len = strlen(comm);

	if (comStr.find("from") != std::string::npos

		) {
		try
		{

			size_t pos = comStr.find("#");
			if (pos == std::string::npos) {
				// no #
				return 0;

			}
			int end = len - 1;
			for (int i = pos + 1; i < len; i++) {
				if (!isdigit(comm[i])) {
					end = i;
				}
			}

			string sub = comStr.substr(pos + 1, end - pos);
			return stoi(sub);

		}
		catch (const std::exception&)
		{
			return 0;
		}


	}

	return 0;

}


int GetOOrderNumberFromComment(const char comm[]) {
	string comStr = string(comm);
	int len = strlen(comm);

	if (comStr.find(string(ORDER_COMMENT_PRE)) != std::string::npos

		) {

		try
		{
			size_t pos = comStr.find(string(ORDER_COMMENT_PRE));
			int end = len - 1;
			for (int i = pos + strlen(ORDER_COMMENT_PRE); i < len; i++) {
				if (!isdigit(comm[i])) {
					end = i;
				}
			}
			string sub = comStr.substr(pos + strlen(ORDER_COMMENT_PRE), end - pos);
			return stoi(sub);
		}
		catch (const std::exception&)
		{
			return 0;
		}

	}



	return 0;
}

 