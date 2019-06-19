#include "utilities.h"

static auto ReplaceWstringAccordingToMap(const wstring &src, const map<wstring, wstring> &replacements)
{
	auto	result(src);
	auto	pos = result.find(L"1"); // --- fake find to define type

	MESSAGE_DEBUG("", "", "start");

	for(auto &replacement : replacements)
	{
		pos = 0;

		while((pos = result.find(replacement.first, pos)) != string::npos)
		{
			result.replace(pos, replacement.first.length(), replacement.second);
		}
	}

	MESSAGE_DEBUG("", "", "finish");

	return result;
}

bool AmIMessageOwner(string messageID, CUser *user, CMysql *db)
{
	bool		result = false;
	
	{
		MESSAGE_DEBUG("", "", "start (messageID: " + messageID + ")");
	}

	if(messageID.length() && (messageID != "0"))
	{
		if(db->Query("SELECT `userId`,`srcType` FROM `feed` WHERE `actionTypeId`='11' AND `actionId`=\"" + messageID + "\";"))
		{
			string		messageOwnerID = db->Get(0, "userId");
			string		messageOwnerType = db->Get(0, "srcType");

			if((messageOwnerType == "user") && (messageOwnerID == user->GetID()))
			{
				result = true;
			}
			else if(messageOwnerType == "company")
			{
				if(db->Query("SELECT `id` FROM `company` WHERE `id`=\"" + messageOwnerID + "\" AND `admin_userID`=\"" + user->GetID() + "\";"))
				{
					result = true;
				}
				else
				{
					MESSAGE_ERROR("", "", "user(id:" + user->GetID() + ") not owning company(id:" + messageOwnerID + ")");
				}
			}
			else if(messageOwnerType == "group")
			{
				if(db->Query("SELECT `id` FROM `groups` WHERE `id`=\"" + messageOwnerID + "\" AND `owner_id`=\"" + user->GetID() + "\";"))
				{
					result = true;
				}
				else
				{
					MESSAGE_ERROR("", "", "user(id:" + user->GetID() + ") not owning group(id:" + messageOwnerID + ")");
				}
			}
			else
			{
				{
					MESSAGE_ERROR("", "", "unknown message type (id:" + messageID + ", type:" + messageOwnerType + ")");
				}
			}
		}
		else
		{
			{
				MESSAGE_ERROR("", "", "message.id(" + messageID + ") not found");
			}
		}
	}
	else
	{
		{
			MESSAGE_DEBUG("", "", "message.id(" + messageID + ") empty or 0");
		}
	}

	{
		MESSAGE_DEBUG("", "", "end (returning value is " + (result ? "true" : "false") + ")");
	}

	return result;	
}

pair<string, string> GetMessageOwner(string messageID, CUser *user, CMysql *db)
{
	string		messageOwnerID = "";
	string		messageOwnerType = "";
	
	{
		MESSAGE_DEBUG("", "", "start (messageID: " + messageID + ")");
	}

	if(db->Query("SELECT `userId`,`srcType` FROM `feed` WHERE `actionTypeId`='11' AND `actionId`=\"" + messageID + "\";"))
	{
		messageOwnerID = db->Get(0, "userId");
		messageOwnerType = db->Get(0, "srcType");
	}
	else
	{
		{
			MESSAGE_ERROR("", "", "message.id(" + messageID + ") not found");
		}
	}

	{
		MESSAGE_DEBUG("", "", "end (returning value pair<messageOwnerType, messageOwnerID>(" + messageOwnerType + ", " + messageOwnerID + ")");
	}

	return make_pair(messageOwnerType, messageOwnerID);
}

string	GetUserSubscriptionsInJSONFormat(string sqlQuery, CMysql *db)
{
	int		affected;
	string	result;

	{
		MESSAGE_DEBUG("", "", "start");
	}

	affected = db->Query(sqlQuery);
	if(affected)
	{
		for(int i = 0; i < affected; i++)
		{
			if(result.length()) result += ",";
			result +=	"{";
			result +=	"\"id\":\"" + string(db->Get(i, "id")) + "\",";
			result +=	"\"entity_id\":\"" + string(db->Get(i, "entity_id")) + "\",";
			result +=	"\"entity_type\":\"" + string(db->Get(i, "entity_type")) + "\",";
			result +=	"\"eventTimestamp\":\"" + string(db->Get(i, "eventTimestamp")) + "\"";
			result +=	"}";
		}
	}
	else
	{
		{
			MESSAGE_DEBUG("", "", "user have no active subscriptions to any company or group");
		}
	}

	{
		MESSAGE_DEBUG("", "", "end (returning result length(" + to_string(result.length()) + ")");
	}

	return result;
}

string	SubscribeToCompany(string companyID, CUser *user, CMysql *db)
{
	ostringstream	ostResult;

	{
		MESSAGE_DEBUG("", "", "start");
	}

	ostResult.str("");

	if(user && companyID.length())
	{
		if(db->Query("SELECT * FROM `users_subscriptions` WHERE `user_id`=\"" + user->GetID() + "\" AND `entity_type`=\"company\" AND `entity_id`=\"" + companyID + "\";"))
		{
			{
				MESSAGE_ERROR("", "", "already subscribed [companyID: " + companyID + ", userID: " + user->GetID() + "]");
			}

			ostResult << "\"result\": \"success\", \"description\": \"вы уже подписаны на новости этой компании\"";
		}
		else
		{
			if(db->InsertQuery("INSERT INTO `users_subscriptions` SET "
								"`user_id`=\"" + user->GetID() + "\","
								"`entity_type`=\"company\","
								"`entity_id`=\"" + companyID + "\","
								"`eventTimestamp`=UNIX_TIMESTAMP();"))
			{
				ostResult << "\"result\": \"success\"";

				// --- insert notification into feed
				if(db->Query("SELECT `id` FROM `feed` WHERE `userId`=\"" + user->GetID() + "\" AND `srcType`=\"user\" AND `actionTypeId`=\"63\" AND `actionId`=\"" + companyID + "\";"))
				{
					MESSAGE_DEBUG("", "", "already subscribed [companyID: " + companyID + ", userID: " + user->GetID() + "]");
				}
				else
				{
					if(db->InsertQuery("INSERT INTO `feed` (`userId`, `actionTypeId`, `actionId`, `eventTimestamp`) values(\"" + user->GetID() + "\", \"63\", \"" + companyID + "\", NOW())"))
					{
					}
					else
					{
						MESSAGE_ERROR("", "", "inserting into `feed` table");
					}
				}
			}
			else
			{
				{
					MESSAGE_ERROR("", "", "error inserting into users subscription [companyID: " + companyID + ", userID: " + user->GetID() + "]");
				}

				ostResult << "\"result\": \"error\",\"description\": \"Ошибка БД\"";
			}
		}
	}
	else
	{
		{
			MESSAGE_ERROR("", "", "(companyID is empty) || (user == NULL) [companyID: " + companyID + ", userID: " + (user ? "not-NULL" : "NULL") + "]");
		}

		ostResult << "\"result\": \"error\",\"description\": \"Компания не найдена или пользователь неопределен\"";
	}


	{
		MESSAGE_DEBUG("", "", "end (returning result length(" + to_string(ostResult.str().length()) + ")");
	}

	return ostResult.str();
}

string	UnsubscribeFromCompany(string companyID, CUser *user, CMysql *db)
{
	ostringstream	ostResult;

	{
		MESSAGE_DEBUG("", "", "start");
	}

	ostResult.str("");

	if(user && companyID.length())
	{
		if(db->Query("SELECT `id` FROM `users_subscriptions` WHERE `user_id`=\"" + user->GetID() + "\" AND `entity_type`=\"company\" AND `entity_id`=\"" + companyID + "\";"))
		{
			string		subscriptionID = db->Get(0, "id");

			if(db->Query("SELECT `admin_userID` FROM `company` WHERE `id`=\"" + companyID + "\" AND `admin_userID`=\"" + user->GetID() + "\";"))
			{
				{
					MESSAGE_DEBUG("", "", "user can't unsubscribe from own company [companyID: " + companyID + ", userID: " + user->GetID() + "]");
				}

				ostResult << "\"result\": \"error\",\"description\": \"Вы не можете отписаться от собственной компании\"";
			}
			else
			{
				db->Query("DELETE FROM `users_subscriptions` WHERE `id`=\"" + subscriptionID + "\";");
				ostResult << "\"result\": \"success\"";

				// --- remove notification from feed
				if(db->Query("SELECT `id` FROM `feed` WHERE `userId`=\"" + user->GetID() + "\" AND `srcType`=\"user\" AND `actionTypeId`=\"63\" AND `actionId`=\"" + companyID + "\";"))
				{
					db->Query("DELETE FROM `feed` WHERE `userId`=\"" + user->GetID() + "\" AND `actionTypeId`=\"63\" AND `actionId`=\"" + companyID + "\";");
					if(db->isError())
					{
						MESSAGE_ERROR("", "", "removeing form `feed` table");
					}
				}
				else
				{
					MESSAGE_DEBUG("", "", "feed subscription missed [companyID: " + companyID + ", userID: " + user->GetID() + "]");
				}

			}
		}
		else
		{
			{
				MESSAGE_ERROR("", "", "user not subscribed to company [companyID: " + companyID + ", userID: " + user->GetID() + "]");
			}

			ostResult << "\"result\": \"success\", \"description\": \"Вы не подписаны\"";
		}
	}
	else
	{
		{
			MESSAGE_ERROR("", "", "(companyID is empty) || (user == NULL) [companyID: " + companyID + ", userID: " + (user ? "not-NULL" : "NULL") + "]");
		}

		ostResult << "\"result\": \"error\",\"description\": \"Компания не найдена или пользователь неопределен\"";
	}


	{
		MESSAGE_DEBUG("", "", "end (returning result length(" + to_string(ostResult.str().length()) + ")");
	}

	return ostResult.str();
}

string	SubscribeToGroup(string groupID, CUser *user, CMysql *db)
{
	ostringstream	ostResult;

	{
		MESSAGE_DEBUG("", "", "start");
	}

	ostResult.str("");

	if(user && groupID.length())
	{
		if(db->Query("SELECT * FROM `users_subscriptions` WHERE `user_id`=\"" + user->GetID() + "\" AND `entity_type`=\"group\" AND `entity_id`=\"" + groupID + "\";"))
		{
			{
				MESSAGE_ERROR("", "", "already subscribed [groupID: " + groupID + ", userID: " + user->GetID() + "]");
			}

			ostResult << "\"result\": \"success\", \"description\": \"вы уже подписаны на новости этой группы\"";
		}
		else
		{
			if(db->InsertQuery("INSERT INTO `users_subscriptions` SET "
								"`user_id`=\"" + user->GetID() + "\","
								"`entity_type`=\"group\","
								"`entity_id`=\"" + groupID + "\","
								"`eventTimestamp`=UNIX_TIMESTAMP();"))
			{
				ostResult << "\"result\": \"success\"";

				// --- insert notification into feed
				if(db->Query("SELECT `id` FROM `feed` WHERE `userId`=\"" + user->GetID() + "\" AND `srcType`=\"user\" AND `actionTypeId`=\"64\" AND `actionId`=\"" + groupID + "\";"))
				{
					MESSAGE_DEBUG("", "", "already subscribed [groupID: " + groupID + ", userID: " + user->GetID() + "]");
				}
				else
				{
					if(db->InsertQuery("INSERT INTO `feed` (`userId`, `actionTypeId`, `actionId`, `eventTimestamp`) values(\"" + user->GetID() + "\", \"64\", \"" + groupID + "\", NOW())"))
					{
					}
					else
					{
						MESSAGE_ERROR("", "", "inserting into `feed` table");
					}
				}
			}
			else
			{
				{
					MESSAGE_ERROR("", "", "error inserting into users subscription [groupID: " + groupID + ", userID: " + user->GetID() + "]");
				}

				ostResult << "\"result\": \"error\",\"description\": \"Ошибка БД\"";
			}
		}
	}
	else
	{
		{
			MESSAGE_ERROR("", "", "(groupID is empty) || (user == NULL) [groupID: " + groupID + ", userID: " + (user ? "not-NULL" : "NULL") + "]");
		}

		ostResult << "\"result\": \"error\",\"description\": \"Компания не найдена или пользователь неопределен\"";
	}


	{
		MESSAGE_DEBUG("", "", "end (returning result length(" + to_string(ostResult.str().length()) + ")");
	}

	return ostResult.str();
}

string	UnsubscribeFromGroup(string groupID, CUser *user, CMysql *db)
{
	ostringstream	ostResult;

	{
		MESSAGE_DEBUG("", "", "start");
	}

	ostResult.str("");

	if(user && groupID.length())
	{
		if(db->Query("SELECT `id` FROM `users_subscriptions` WHERE `user_id`=\"" + user->GetID() + "\" AND `entity_type`=\"group\" AND `entity_id`=\"" + groupID + "\";"))
		{
			string		subscriptionID = db->Get(0, "id");

			if(db->Query("SELECT `owner_id` FROM `groups` WHERE `id`=\"" + groupID + "\" AND `owner_id`=\"" + user->GetID() + "\";"))
			{
				{
					MESSAGE_DEBUG("", "", "user can't unsubscribe from own group [groupID: " + groupID + ", userID: " + user->GetID() + "]");
				}

				ostResult << "\"result\": \"error\",\"description\": \"Вы не можете отписаться от собственной группы\"";
			}
			else
			{
				db->Query("DELETE FROM `users_subscriptions` WHERE `id`=\"" + subscriptionID + "\";");
				ostResult << "\"result\": \"success\"";
				
				// --- remove notification from feed
				if(db->Query("SELECT `id` FROM `feed` WHERE `userId`=\"" + user->GetID() + "\" AND `srcType`=\"user\" AND `actionTypeId`=\"64\" AND `actionId`=\"" + groupID + "\";"))
				{
					db->Query("DELETE FROM `feed` WHERE `userId`=\"" + user->GetID() + "\" AND `actionTypeId`=\"64\" AND `actionId`=\"" + groupID + "\";");
					if(db->isError())
					{
						MESSAGE_ERROR("", "", "removeing form `feed` table");
					}
				}
				else
				{
					MESSAGE_DEBUG("", "", "feed subscription missed [groupID: " + groupID + ", userID: " + user->GetID() + "]");
				}
			}
		}
		else
		{
			{
				MESSAGE_ERROR("", "", "user not subscribed to group [groupID: " + groupID + ", userID: " + user->GetID() + "]");
			}

			ostResult << "\"result\": \"success\", \"description\": \"Вы не подписаны\"";
		}
	}
	else
	{
		{
			MESSAGE_ERROR("", "", "(groupID is empty) || (user == NULL) [groupID: " + groupID + ", userID: " + (user ? "not-NULL" : "NULL") + "]");
		}

		ostResult << "\"result\": \"error\",\"description\": \"Компания не найдена или пользователь неопределен\"";
	}


	{
		MESSAGE_DEBUG("", "", "end (returning result length(" + to_string(ostResult.str().length()) + ")");
	}

	return ostResult.str();
}

bool isBotIP(string ip)
{
	bool			result = false;
	vector<string>	botsIPs = {
		// --- Google bots
		string("66.249.64."),
		string("66.249.65."),
		string("66.249.66."),
		string("66.249.67."),
		string("66.249.68."),
		string("66.249.69."),
		string("66.249.71."),
		string("66.249.72."),
		string("66.249.73."),
		string("66.249.74."),
		string("66.249.75."),
		string("66.249.76."),
		string("66.249.77."),
		string("66.249.78."),
		string("66.249.79."),
		string("66.249.80."),
		string("66.249.81."),
		string("66.249.82."),
		string("66.249.83."),
		string("66.249.84."),
		string("66.249.85."),
		string("66.249.86."),
		string("66.249.87."),
		string("66.249.88."),
		string("66.249.89."),
		string("66.249.90."),
		string("66.249.91."),
		string("66.249.92."),
		string("66.249.93."),
		string("66.249.94."),
		string("66.249.95.")
		};

	{
		MESSAGE_DEBUG("", "", "start");
	}

	if(ip.length())
	{
		for(string &tmp : botsIPs)
		{
			if(ip.find(tmp) != string::npos)
			{
				result = true;
				break;
			}
		}
	}
	else
	{
		{
			MESSAGE_DEBUG("", "", "IP is empty");
		}
	}

	{
		MESSAGE_DEBUG("", "", "end (result  = " + (result ? "true" : "false") + ")");
	}

	return result;
}

string GetGiftListInJSONFormat(string dbQuery, CMysql *db, CUser *user)
{
	struct ItemClass 
	{
		string	id;
		string	link;
		string	title;
		string	description;
		string	folder;
		string	filename;
		string	requested_quantity;
		string	gained_quantity;
		string	estimated_price;
		string	user_id;
		string	eventTimestamp;
	};

	ostringstream			ost, ostFinal;
	string					sessid, lookForKey;
	int						affected;
	vector<ItemClass>		groupsList;

	{
		MESSAGE_DEBUG("", "", "start");
	}

	ostFinal.str("");

	if((affected = db->Query(dbQuery)) > 0)
	{
		int			groupCounter = affected;

		groupsList.reserve(groupCounter);  // --- reserving allows avoid moving vector in memory
											// --- to fit vector into continous memory piece

		for(int i = 0; i < affected; i++)
		{
			ItemClass	gift;

			gift.id = db->Get(i, "id");
			gift.link = db->Get(i, "link");
			gift.title = db->Get(i, "title");
			gift.description = db->Get(i, "description");
			gift.folder = db->Get(i, "logo_folder");
			gift.filename = db->Get(i, "logo_filename");
			gift.requested_quantity = db->Get(i, "requested_quantity");
			gift.gained_quantity = db->Get(i, "gained_quantity");
			gift.estimated_price = db->Get(i, "estimated_price");
			gift.user_id = db->Get(i, "user_id");
			gift.eventTimestamp = db->Get(i, "eventTimestamp");

			groupsList.push_back(gift);
		}

		for(int i = 0; i < groupCounter; i++)
		{
				if(ostFinal.str().length()) ostFinal << ", ";

				ostFinal << "{";
				ostFinal << "\"id\": \""				  	<< groupsList[i].id << "\",";
				ostFinal << "\"link\": \""					<< groupsList[i].link << "\",";
				ostFinal << "\"title\": \""					<< groupsList[i].title << "\",";
				ostFinal << "\"description\": \""			<< groupsList[i].description << "\",";
				ostFinal << "\"logo_folder\": \""			<< groupsList[i].folder << "\",";
				ostFinal << "\"logo_filename\": \""			<< groupsList[i].filename << "\",";
				ostFinal << "\"requested_quantity\": \""	<< groupsList[i].requested_quantity << "\",";
				ostFinal << "\"gained_quantity\": \""		<< groupsList[i].gained_quantity << "\",";
				ostFinal << "\"estimated_price\": \""		<< groupsList[i].estimated_price << "\",";
				ostFinal << "\"user_id\": \""				<< groupsList[i].user_id << "\",";
				ostFinal << "\"eventTimestamp\": \""		<< groupsList[i].eventTimestamp << "\"";
				ostFinal << "}";
		} // --- for loop through gift list

	} // --- if sql-query on gift selection success
	else
	{
		MESSAGE_DEBUG("", "", "there are no gifts returned by request [" + dbQuery + "]");
	}

	{
		MESSAGE_DEBUG("", "", "end (result length = " + to_string(ostFinal.str().length()) + ")");
	}

	return ostFinal.str();
}


bool isAdverseWordsHere(string text, CMysql *db)
{
	bool			result = false;
	int				affected;

	{
		MESSAGE_DEBUG("", "", "start");
	}

	affected = db->Query("SELECT * FROM `dictionary_adverse`;");
	if(affected)
	{
		text = toLower(text);

		for(int i = 0; i < affected; i++)
		{
			string		checkingWord = db->Get(i, "word");

			if(text.find(checkingWord) != string::npos)
			{
				{
					MESSAGE_DEBUG("", "", "adverse word.id[" + db->Get(i, "id") + "]");
				}

				result = true;
				break;
			}
		}
	}
	else
	{
		{
			MESSAGE_DEBUG("", "", "adverse_word table is empty");
		}

	}

	
	{
		MESSAGE_DEBUG("", "", "end (result  = " + (result ? "true" : "false") + ")");
	}
	
	return result;
}

string ParseGPSLongitude(const string longitudeStr)
{
	string  result = "";
	smatch  cm;
	regex   format1("[-+]?[[:digit:]]+(\\.[[:digit:]]+)?");
	regex   format2("([EW])\\:[[:space:]]*([[:digit:]]+)((\\/)([[:digit:]]+))?\\,[[:space:]]*([[:digit:]]+)((\\/)([[:digit:]]+))?\\,[[:space:]]*([[:digit:]]+)((\\/)([[:digit:]]+))?", regex_constants::ECMAScript | regex_constants::icase);
	regex   format3(".*unknown.*");
	regex   format4(".[[:space:]]*");

	MESSAGE_DEBUG("", "", "start(" + longitudeStr + ")");

	// --- format: +74.56 or 74.56 or 74
	if(regex_match(longitudeStr, format1))
	{
		result = longitudeStr;
	}
	else if(regex_match(longitudeStr, cm, format2))
	{
		string	ref = cm[1];
		string  degreeNumerator = cm[2];
		string  degreeDivisor = cm[5];
		string  minutesNumerator = cm[6];
		string  minutesDivisor = cm[9];
		string  secondsNumerator = cm[10];
		string  secondsDivisor = cm[13];
		float	temp = 0;

		if(!degreeDivisor.length() || !stof(degreeDivisor)) degreeDivisor = "1";
		if(!minutesDivisor.length() || !stof(minutesDivisor)) minutesDivisor = "1";
		if(!secondsDivisor.length() || !stof(secondsDivisor)) secondsDivisor = "1";

		if(stof(minutesNumerator)/stof(minutesDivisor) > 60)
		{
			// --- minutes couldn't be greater than 60
			{
				MESSAGE_ERROR("", "", " " + minutesNumerator + "/" + minutesDivisor + " minutes can't be greater 60");
			}
		}
		else if(stof(secondsNumerator)/stof(secondsDivisor) > 60)
		{
			// --- seconds couldn't be greater than 60
			{
				MESSAGE_ERROR("", "", " " + secondsNumerator + "/" + secondsDivisor + " seconds can't be greater 60");
			}
		}
		else
		{
			temp = stof(degreeNumerator)/stof(degreeDivisor) + stof(minutesNumerator)/stof(minutesDivisor)/60 + stof(secondsNumerator)/stof(secondsDivisor)/3600;
			result = (ref.length() && (toupper(ref[0]) == 'W') ? "-" : "+") + to_string(temp);
		}

	}
	else if(regex_match(longitudeStr, format3))
	{
	}
	else if(regex_match(longitudeStr, format4))
	{
	}
	else if(longitudeStr.length() == 0)
	{
	}
	else
	{
		{
			MESSAGE_ERROR("", "", " longitude(" + longitudeStr + ") didn't match any pattern");
		}
	}

	MESSAGE_DEBUG("", "", "finish (" + result + ")");

	return result;
}

string ParseGPSLatitude(const string latitudeStr)
{
	string  result = "";
	smatch  cm;
	regex   format1("[-+]?[[:digit:]]+(\\.[[:digit:]]+)?");
	regex   format2("([NS])\\:[[:space:]]*([[:digit:]]+)((\\/)([[:digit:]]+))?\\,[[:space:]]*([[:digit:]]+)((\\/)([[:digit:]]+))?\\,[[:space:]]*([[:digit:]]+)((\\/)([[:digit:]]+))?", regex_constants::ECMAScript | regex_constants::icase);
	regex   format3(".*unknown.*");
	regex   format4(".[[:space:]]*");

	{
		MESSAGE_DEBUG("", "", "start(" + latitudeStr + ")");
	}
	
	// --- format: +74.56 or 74.56 or 74
	if(regex_match(latitudeStr, regex(format1)))
	{
		result = latitudeStr;
	}
	else if(regex_match(latitudeStr, cm, format2))
	{
		string	ref = cm[1];
		string  degreeNumerator = cm[2];
		string  degreeDivisor = cm[5];
		string  minutesNumerator = cm[6];
		string  minutesDivisor = cm[9];
		string  secondsNumerator = cm[10];
		string  secondsDivisor = cm[13];
		float	temp = 0;

		if(!degreeDivisor.length() || !stof(degreeDivisor)) degreeDivisor = "1";
		if(!minutesDivisor.length() || !stof(minutesDivisor)) minutesDivisor = "1";
		if(!secondsDivisor.length() || !stof(secondsDivisor)) secondsDivisor = "1";

		if(stof(minutesNumerator)/stof(minutesDivisor) > 60)
		{
			// --- minutes couldn't be greater than 60
			{
				MESSAGE_ERROR("", "", " " + minutesNumerator + "/" + minutesDivisor + " minutes can't be greater 60");
			}
		}
		else if(stof(secondsNumerator)/stof(secondsDivisor) > 60)
		{
			// --- seconds couldn't be greater than 60
			{
				MESSAGE_ERROR("", "", " " + secondsNumerator + "/" + secondsDivisor + " seconds can't be greater 60");
			}
		}
		else
		{
			temp = stof(degreeNumerator)/stof(degreeDivisor) + stof(minutesNumerator)/stof(minutesDivisor)/60 + stof(secondsNumerator)/stof(secondsDivisor)/3600;
			result = (ref.length() && (toupper(ref[0]) == 'S') ? "-" : "+") + to_string(temp);
		}

	}
	else if(regex_match(latitudeStr, format3))
	{
	}
	else if(regex_match(latitudeStr, format4))
	{
	}
	else if(latitudeStr.length() == 0)
	{
	}

	else
	{
			{
				MESSAGE_ERROR("", "", " latitude(" + latitudeStr + ") didn't match any pattern");
			}
	}

	{
		MESSAGE_DEBUG("", "", "finish (" + result + ")");
	}

	return result;
}

string ParseGPSAltitude(const string altitudeStr)
{
	string  result = "";
	smatch  cm;
	regex   format1("[-+]?[[:digit:]]+(\\.[[:digit:]]+)?");
	regex   format2(".*\\:[[:space:]]*([[:digit:]]+)(\\/)([[:digit:]]+).*");
	regex   format3(".*unknown.*");
	regex   format4(".[[:space:]]*");
	regex   format5("([[:digit:]]+)(\\/)([[:digit:]]+)\n.*");
	regex   format6("[^[:digit:]]*([[:digit:]]+)(\\/)([[:digit:]]+)[^[:digit:]]*");

	MESSAGE_DEBUG("", "", "start(" + altitudeStr + ")");
	
	// --- format: +74.56 or 74.56 or 74
	if(regex_match(altitudeStr, regex(format1)))
	{
		result = altitudeStr;
	}
	else if(regex_match(altitudeStr, cm, format2) || regex_match(altitudeStr, cm, format5) || regex_match(altitudeStr, cm, format6))
	{
		string		altitudeNumerator = cm[1];
		string		altitudeDivisor = cm[3];
		float		temp = 0;

		if(!altitudeDivisor.length() || !stof(altitudeDivisor)) altitudeDivisor = "1";

		temp = stof(altitudeNumerator)/stof(altitudeDivisor);
		result = (temp > 0 ? "+" : "") + to_string(temp);
	}
	else if(regex_match(altitudeStr, format3))
	{
	}
	else if(regex_match(altitudeStr, format4))
	{
	}
	else if(altitudeStr.length() == 0)
	{
	}
	else
	{
		MESSAGE_ERROR("", "", "altitude(" + altitudeStr + ") didn't match any pattern");
	}

	MESSAGE_DEBUG("", "", "finish (" + result + ")");

	return result;
}

string ParseGPSSpeed(const string speedStr)
{
	string  result = "";
	smatch  cm;
	regex   format1("[-+]?[[:digit:]]+(\\.[[:digit:]]+)?");
	regex   format2("K\\:[[:space:]]*([[:digit:]]+)(\\/)([[:digit:]]+)");
	regex   format3(".*unknown.*");
	regex   format4(".[[:space:]]*");

	{
		MESSAGE_DEBUG("", "", "start(" + speedStr + ")");
	}

	// --- format: +74.56 or 74.56 or 74
	if(regex_match(speedStr, regex(format1)))
	{
		result = speedStr;
	}
	else if(regex_match(speedStr, cm, format2))
	{
		string	speedNumerator = cm[1];
		string	speedDivisor = cm[3];
		float		temp = 0;

		if(!speedDivisor.length() || !stof(speedDivisor)) speedDivisor = "1";

		temp = stof(speedNumerator)/stof(speedDivisor);
		result = (temp > 0 ? "+" : "") + to_string(temp);
	}
	else if(regex_match(speedStr, format3))
	{
	}
	else if(regex_match(speedStr, format4))
	{
	}
	else if(speedStr.length() == 0)
	{
	}
	else
	{
			{
				MESSAGE_ERROR("", "", " speed(" + speedStr + ") didn't match any pattern");
			}
	}

	{
		MESSAGE_DEBUG("", "", "finish (" + result + ")");
	}

	return result;
}

auto	stod_noexcept(const string &num) -> double
{
	auto	result = 0.0;
	auto	decimal_point = (localeconv()->decimal_point ? localeconv()->decimal_point[0] : ',');
	auto	lc_original = ""s;
	auto	lc_en = "en_US.utf8"s;

	if(decimal_point != '.')
	{
		lc_original = setlocale(LC_NUMERIC, NULL);
		setlocale(LC_NUMERIC, lc_en.c_str());
		// --- commented due to performance
		// MESSAGE_DEBUG("", "", "temporary switch locale from " + lc_original + " to " + lc_en)
	}

	try
	{
		result = stod(num);
	}
	catch(...)
	{
		MESSAGE_ERROR("", "", "can't convert " + num + " to double");
	}

	if(decimal_point != '.')
	{
		setlocale(LC_NUMERIC, lc_original.c_str());
		// --- commented due to performance
		// MESSAGE_DEBUG("", "", "switch locale back to " + lc_original)
	}

	return result;
}

auto	MaskSymbols(string src, int first_pos, int last_pos) -> string
{
	MESSAGE_DEBUG("", "", "start");

	for(auto i = first_pos; i < last_pos; ++i)
		src[i] = '.';

	MESSAGE_DEBUG("", "", "finish");

	return src;
}

auto ConvertHTMLToText(const wstring &src) -> wstring
{

	auto					result = src;
	map<wstring, wstring>	map_replacement_1 = {
		{L"&quot;", L"\""},
		{L"&gt;", L">"},
		{L"&lt;", L"<"}
	};

	MESSAGE_DEBUG("", "", "start");

	result = ReplaceWstringAccordingToMap(result, map_replacement_1);

	MESSAGE_DEBUG("", "", "finish");

	return result;
}

auto ConvertHTMLToText(const string &src) -> string
{
	return(wide_to_multibyte(ConvertHTMLToText(multibyte_to_wide(src))));
}

// Convert UTF-8 byte string to wstring
auto multibyte_to_wide(std::string const& s) -> wstring
{
	std::wstring_convert<std::codecvt_utf8<wchar_t> > conv;
	return conv.from_bytes(s);
}

// Convert wstring to UTF-8 byte string
auto wide_to_multibyte(std::wstring const& s) -> string
{
	std::wstring_convert<std::codecvt_utf8<wchar_t> > conv;
	return conv.to_bytes(s);
}

auto CutTrailingZeroes(string number) -> string
{
    size_t   dec_point_place = number.find(".");
    
    if(dec_point_place != string::npos)
    {
        bool   domore = true;
        int    symbols_to_cut = 0;

        for(string::reverse_iterator rit = number.rbegin(); domore && (rit != number.rend()); ++rit)
        {
            if(*rit == '0') ++symbols_to_cut;
            else if(*rit == '.') { ++symbols_to_cut; domore = false;}
            else domore = false;
        }

        number.erase(number.length() - symbols_to_cut);
    }
    
    return number;
}
