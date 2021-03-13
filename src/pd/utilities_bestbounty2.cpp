#include "utilities_bestbounty.h"

// --- Returns user list in JSON format grabbed from DB
// --- Input: dbQuery - SQL format returns users
//			db	  - DB connection
//			user	- current user object
string GetUserListInJSONFormat(string dbQuery, CMysql *db, CUser *user)
{
	MESSAGE_DEBUG("", "", "start");

	ostringstream					 ost;
	string							 sessid, lookForKey;
	unordered_set<unsigned long>	 setOfUserID;

	struct	ItemClass
	{
		string	userID;
		string	userLogin;
		string	userName;
		string	userNameLast;
		string	userSex;
		string	site_theme_id;
		string	email;
		string	email_changeable;
		string	cv;
		string	phone;
		string	country_code;
		string	isBlocked;
		string	address;
		string	userBirthday;
		string	userBirthdayAccess;
		string	userAppliedVacanciesRender;
		string	userCurrentCityID;
		string	userLastOnline;
		string	userLastOnlineSecondSinceY2k;
		string	helpdesk_subscription_S1_sms;
		string	helpdesk_subscription_S2_sms;
		string	helpdesk_subscription_S3_sms;
		string	helpdesk_subscription_S4_sms;
		string	helpdesk_subscription_S1_email;
		string	helpdesk_subscription_S2_email;
		string	helpdesk_subscription_S3_email;
		string	helpdesk_subscription_S4_email;
	};
	vector<ItemClass>		itemsList;
	auto					itemsCount = 0;

	if((itemsCount = db->Query(dbQuery)) > 0)
	{
		for(auto i = 0; i < itemsCount; ++i)
		{
			ItemClass	item;
			item.userID								= db->Get(i, "id");
			item.userLogin							= db->Get(i, "login");
			item.userName							= db->Get(i, "name");
			item.userNameLast						= db->Get(i, "nameLast");
			item.userSex							= db->Get(i, "sex");
			item.cv									= db->Get(i, "cv");
			item.phone								= db->Get(i, "phone");
			item.country_code						= db->Get(i, "country_code");
			item.isBlocked							= db->Get(i, "isblocked");
			item.address							= db->Get(i, "address");
			item.email								= db->Get(i, "email");
			item.email_changeable					= db->Get(i, "email_changeable");
			item.site_theme_id						= db->Get(i, "site_theme_id");
			item.userBirthday						= db->Get(i, "birthday");
			item.userBirthdayAccess					= db->Get(i, "birthdayAccess");
			item.userAppliedVacanciesRender			= db->Get(i, "appliedVacanciesRender");
			item.userCurrentCityID					= db->Get(i, "geo_locality_id");
			item.userLastOnline						= db->Get(i, "last_online");
			item.userLastOnlineSecondSinceY2k		= db->Get(i, "last_onlineSecondsSinceY2k");
			item.helpdesk_subscription_S1_email		= db->Get(i, "helpdesk_subscription_S1_email");
			item.helpdesk_subscription_S2_email		= db->Get(i, "helpdesk_subscription_S2_email");
			item.helpdesk_subscription_S3_email		= db->Get(i, "helpdesk_subscription_S3_email");
			item.helpdesk_subscription_S4_email		= db->Get(i, "helpdesk_subscription_S4_email");
			item.helpdesk_subscription_S1_sms		= db->Get(i, "helpdesk_subscription_S1_sms");
			item.helpdesk_subscription_S2_sms		= db->Get(i, "helpdesk_subscription_S2_sms");
			item.helpdesk_subscription_S3_sms		= db->Get(i, "helpdesk_subscription_S3_sms");
			item.helpdesk_subscription_S4_sms		= db->Get(i, "helpdesk_subscription_S4_sms");

			itemsList.push_back(item);
		}


		ost.str("");
		for(int i = 0; i < itemsCount; i++) 
		{

			if(setOfUserID.find(stol(itemsList[i].userID)) == setOfUserID.end())
			{
				string				userCurrentEmployment, avatarPath;
				string				numberUreadMessages;
				string				userFriendship;
				ostringstream		ost1;
				int					affected1;

				auto				userCurrentCity				= ""s;
				auto				userID						= itemsList[i].userID;
				auto				userLogin					= itemsList[i].userLogin;
				auto				userName					= itemsList[i].userName;
				auto				userNameLast				= itemsList[i].userNameLast;
				auto				userSex						= itemsList[i].userSex;
				auto				userAppliedVacanciesRender	= itemsList[i].userAppliedVacanciesRender;
				auto				userCurrentCityID			= itemsList[i].userCurrentCityID;
				auto				userLastOnline				= itemsList[i].userLastOnline;
				auto				userLastOnlineSecondSinceY2k= itemsList[i].userLastOnlineSecondSinceY2k;
				auto				isMe						= user && (userID == user->GetID());

				setOfUserID.insert(atol(userID.c_str()));

				// --- Defining title and company of user
				ost1.str("");
				ost1 << "SELECT `company_position`.`title` as `users_company_position_title`,  \
						`company`.`name` as `company_name`, `company`.`id` as `company_id`  \
						FROM `users_company` \
						LEFT JOIN  `company_position` ON `company_position`.`id`=`users_company`.`position_title_id` \
						LEFT JOIN  `company` 				ON `company`.`id`=`users_company`.`company_id` \
						WHERE `users_company`.`user_id`=\"" << userID << "\" and `users_company`.`current_company`='1' \
						ORDER BY  `users_company`.`occupation_start` DESC ";

				affected1 = db->Query(ost1.str());
				ost1.str("");
				ost1 << "[";
				if(affected1 > 0)
				{
					for(int j = 0; j < affected1; j++)
					{
						ost1 << "{ \
								\"companyID\": \"" << db->Get(j, "company_id") << "\", \
								\"company\": \"" << db->Get(j, "company_name") << "\", \
								\"title\": \"" << db->Get(j, "users_company_position_title") << "\" \
								}";
						if(j < (affected1 - 1)) ost1 << ", ";
					}
				}
				ost1 << "]";
				userCurrentEmployment = ost1.str(); 

				{
					CLog	log;

					log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: done with building employment list ", userCurrentEmployment);
				}

				// --- Get user avatars
				ost1.str("");
				ost1 << "select * from `users_avatars` where `userid`='" << userID << "' and `isActive`='1';";
				avatarPath = "empty";
				if(db->Query(ost1.str()))
				{
					ost1.str("");
					ost1 << "/images/avatars/avatars" << db->Get(0, "folder") << "/" << db->Get(0, "filename");
					avatarPath = ost1.str();
				}

				// --- Get friendship status
				userFriendship = "empty";
				if(user && db->Query("select * from `users_friends` where `userid`='" + user->GetID() + "' and `friendID`='" + userID + "';"))
				{
					userFriendship = db->Get(0, "state");
				}

				// --- Get presence status for chat purposes
				if(db->Query("select COUNT(*) as `number_unread_messages` from `chat_messages` where `fromType`='fromUser' and `fromID`='" + userID + "' and (`messageStatus`='unread' or `messageStatus`='sent' or `messageStatus`='delivered');"))
				{
					numberUreadMessages = db->Get(0, "number_unread_messages");
				}

				if(userCurrentCityID.length() && db->Query("SELECT `title` FROM `geo_locality` WHERE `id`=\"" + userCurrentCityID + "\";"))
				{
					userCurrentCity = db->Get(0, "title");
				}

				if(ost.str().length()) ost << ", ";

				ost << "{"
						  "\"id\": \""							<< userID << "\", "
						  "\"userID\": \""						<< userID << "\", "  // TODO: find usage of this in JS and remove
						  "\"name\": \""						<< userName << "\", "
						  "\"nameLast\": \""					<< userNameLast << "\","
						  "\"cv\": \""							<< itemsList[i].cv << "\","
						  "\"address\": \""						<< itemsList[i].address << "\","
						  "\"birthday\": \""					<< ((isMe || (itemsList[i].userBirthdayAccess == "public")) ? itemsList[i].userBirthday : "") << "\","
						  "\"birthdayAccess\": \""				<< itemsList[i].userBirthdayAccess << "\","
						  "\"sex\": \""							<< userSex << "\","
						  "\"userSex\": \""						<< userSex << "\","
						  "\"gifts\": ["						<< GetGiftListInJSONFormat("SELECT * FROM `gifts` WHERE `user_id`=\"" + userID + "\";", db, user) << "],"
						  "\"gifts_to_give\": ["				<< GetGiftToGiveListInJSONFormat("SELECT * FROM `gifts_to_give` WHERE `gift_id` in (SELECT `id` FROM `gifts` WHERE `user_id`=\"" + userID + "\");", db, user) << "],"
						  "\"last_online\": \""					<< userLastOnline << "\","
						  "\"last_online_diff\": \""			<< to_string(GetTimeDifferenceFromNow(userLastOnline)) << "\","
						  "\"last_onlineSecondsSinceY2k\": \""  << userLastOnlineSecondSinceY2k << "\","
						  "\"userFriendship\": \""				<< userFriendship << "\","
						  "\"avatar\": \""						<< avatarPath << "\","
						  "\"currentEmployment\": "				<< userCurrentEmployment << ", "
						  "\"geo_locality_id\": \""				<< userCurrentCityID << "\","
						  "\"city\": \""						<< userCurrentCity << "\","
						  "\"currentCity\": \""					<< userCurrentCity << "\", "
						  "\"numberUnreadMessages\": \""		<< numberUreadMessages << "\", "
						  "\"languages\": ["		 			<< GetLanguageListInJSONFormat("SELECT * FROM `language` WHERE `id` in (SELECT `language_id` FROM `users_language` WHERE `user_id`=\"" + userID + "\");", db) << "], "
						  "\"skills\": ["		 				<< GetSkillListInJSONFormat("SELECT * FROM `skill` WHERE `id` in (SELECT `skill_id` FROM `users_skill` WHERE `user_id`=\"" + userID + "\");", db) << "], "
						  "\"site_theme_id\": \""				<< itemsList[i].site_theme_id << "\","
						  "\"themes\": ["						<< GetSiteThemesInJSONFormat("SELECT * FROM `site_themes`", db, user) << "],"
						  "\"country_code\": \""				<< (isMe ? itemsList[i].country_code : "") << "\","
						  "\"phone\": \""						<< (isMe ? itemsList[i].phone: "") << "\","
						  "\"email\": \""						<< (isMe ? itemsList[i].email : "") << "\","
						  "\"email_changeable\": \""			<< (isMe ? itemsList[i].email_changeable : "") << "\","
						  "\"helpdesk_subscriptions_sms\": ["	<< (isMe ? quoted(itemsList[i].helpdesk_subscription_S1_sms) + "," + quoted(itemsList[i].helpdesk_subscription_S2_sms) + "," + quoted(itemsList[i].helpdesk_subscription_S3_sms) + "," + quoted(itemsList[i].helpdesk_subscription_S4_sms)  : "") << "],"
						  "\"helpdesk_subscriptions_email\": ["	<< (isMe ? quoted(itemsList[i].helpdesk_subscription_S1_email) + "," + quoted(itemsList[i].helpdesk_subscription_S2_email) + "," + quoted(itemsList[i].helpdesk_subscription_S3_email) + "," + quoted(itemsList[i].helpdesk_subscription_S4_email)  : "") << "],"
						  "\"subscriptions\":[" 				<< (isMe ? GetUserSubscriptionsInJSONFormat("SELECT * FROM `users_subscriptions` WHERE `user_id`=\"" + userID + "\";", db) : "") << "],"
						  "\"isBlocked\": \""					<< itemsList[i].isBlocked << "\","
						  "\"isMe\": \""						<< (isMe ? "yes" : "no") << "\" "
						"}";
			} // --- if user is not dupicated
		} // --- for loop through user list
	} // --- if sql-query on user selection success
	else
	{
		CLog	log;

		ost.str("");
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: there are users returned by request [", dbQuery, "]");
	}

	MESSAGE_DEBUG("", "", "finish [length" + to_string(ost.str().length()) + "]");

	return ost.str();
}

// --- Function returns list of images belongs to imageSet
// --- input: imageSetID, db
// --- output: list of image objects
string GetMessageImageList(string imageSetID, CMysql *db)
{
	ostringstream	ost;
	string		  result = "";

	MESSAGE_DEBUG("", "", "start");

	if(imageSetID != "0")
	{
		int				affected;
		
		ost.str("");
		ost << "select * from `feed_images` where `setID`='" << imageSetID << "';";
		affected = db->Query(ost.str());
		if(affected > 0) 
		{
			ost.str("");
			for(int i = 0; i < affected; i++)
			{
				if(i > 0) ost << "," << std::endl;
				ost << "{";
				ost << "\"id\":\"" << db->Get(i, "id") << "\",";
				ost << "\"folder\":\"" << db->Get(i, "folder") << "\",";
				ost << "\"filename\":\"" << db->Get(i, "filename") << "\",";
				ost << "\"mediaType\":\"" << db->Get(i, "mediaType") << "\",";
				ost << "\"isActive\":\"" << db->Get(i, "isActive") << "\"";
				ost << "}";
			}

			result = ost.str();
		}
	}

	{
		CLog			log;
		ostringstream	ost;

		ost.str();
		ost <<  "GetMessageImageList: end. returning string length " << result.length();
		log.Write(DEBUG, ost.str());
	}

	return result;
}

// input: ....
//		  includeStudents will add student counter
string GetLanguageListInJSONFormat(string dbQuery, CMysql *db, bool includeStudents/* = false*/)
{
	struct ItemClass {
		string	  id, title, photoFolder, photoFilename, studentUserList;
		string	  isComplained, complainedUserList;
	};

	ostringstream	   ostResult;
	int				 itemsCount;
	vector<ItemClass>   itemsList;

	MESSAGE_DEBUG("", "", "start");

	ostResult.str("");
	itemsCount = db->Query(dbQuery);
	if(itemsCount)
	{
		for(int i = 0; i < itemsCount; i++) 
		{
			ItemClass   item;

			item.id				 = db->Get(i, "id");
			item.title			  = db->Get(i, "title");
			item.photoFolder		= db->Get(i, "logo_folder");
			item.photoFilename	  = db->Get(i, "logo_filename");
			item.studentUserList	= "";

			itemsList.push_back(item);						
		}
		
		for(int i = 0; i < itemsCount; i++) 
		{

				if(includeStudents)
				{
					string temp = "";

					for(int j = 0; j < db->Query("SELECT * from `users_language` WHERE `language_id`=\"" + itemsList.at(i).id + "\";"); ++j)
					{
						if(temp.length()) temp += ",";
						temp += db->Get(j, "user_id");
					}

					itemsList.at(i).studentUserList = temp;
				}

				if(ostResult.str().length()) ostResult << ", ";

				ostResult << "{"
						  << "\"languageID\": \""				 << itemsList.at(i).id << "\", "
						  << "\"languageTitle\": \""			  << itemsList.at(i).title << "\", "
						  << "\"languagePhotoCoverFolder\": \""   << itemsList.at(i).photoFolder << "\","
						  << "\"languagePhotoCoverFilename\": \"" << itemsList.at(i).photoFilename << "\","
						  << "\"languageStudentsUserID\": ["	   << itemsList.at(i).studentUserList << "]"
						  << "}";
		} // --- for loop through user list
	} // --- if sql-query on user selection success
	else
	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: there are no languages returned by the request [", dbQuery, "]");
	}

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: finish (returning string length = " + to_string(ostResult.str().length()) + ")");
	}

	return ostResult.str();
}

string GetSkillListInJSONFormat(string dbQuery, CMysql *db)
{
	struct ItemClass {
		string	  id, title;
		string	  isComplained, complainedUserList;
	};

	ostringstream	   ostResult;
	int				 itemsCount;
	vector<ItemClass>   itemsList;

	MESSAGE_DEBUG("", "", "start");

	ostResult.str("");
	itemsCount = db->Query(dbQuery);
	if(itemsCount)
	{
		for(int i = 0; i < itemsCount; i++) 
		{
			ItemClass   item;

			item.id				 = db->Get(i, "id");
			item.title			  = db->Get(i, "title");

			itemsList.push_back(item);						
		}
		
		for(int i = 0; i < itemsCount; i++) 
		{

				if(ostResult.str().length()) ostResult << ", ";

				ostResult << "{"
						  << "\"id\": \""		<< itemsList.at(i).id << "\", "
						  << "\"title\": \""	<< itemsList.at(i).title << "\" "
						  << "}";
		} // --- for loop through user list
	} // --- if sql-query on user selection success
	else
	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: there are no skills returned by the request [", dbQuery, "]");
	}

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: finish (returning string length = " + to_string(ostResult.str().length()) + ")");
	}

	return ostResult.str();
}

string GetMessageSpam(string messageID, CMysql *db)
{
	return GetValueFromDB("select count(*) as `counter` from `feed_message_params` where `parameter`='spam' and messageID='" + messageID + "';", db);
}


// --- Function returns true or false depends on userID "spamed" it or not
// --- input: messageID, userID
// --- output: was this message "spamed" by particular user or not
string GetMessageSpamUser(string messageID, string userID, CMysql *db)
{
	return GetValueFromDB("select count(*) from `feed_message_params` where `parameter`='spam' and `messageID`='" + messageID + "' and `userID`='" + userID + "' ;", db);
}

auto GetUsersID_BySearchString(const string &lookForKey, bool include_myself, CMysql *db, CUser *user) -> vector<string>
{
	MESSAGE_DEBUG("", "", "start(" + lookForKey + ")");

	vector<string>	result, searchWords;

	if(qw(lookForKey, searchWords))
	{
		// --- single search word can be name, surname, company name 
		if(searchWords.size() == 1)		
		{
			MESSAGE_DEBUG("", "", "single word search");

			// --- Looking through name, last name
			auto query = 
					"SELECT `users`.`id` FROM `users`"
					"LEFT JOIN `users_company` on `users_company`.`user_id` = `users`.`id` "
					"LEFT JOIN `company` on `company`.`id`=`users_company`.`company_id` "
					"WHERE "
					"("
						" `users`.`isActivated`='Y' AND `users`.`isblocked`='N' "
						+ (include_myself ? "" : " AND `users`.`id`!=\"" + user->GetID() + "\" ") +
					") "
					" AND "
					"("
						"("
							"`users`.`name` LIKE \"%" 	+ lookForKey + "%\" or `users`.`nameLast` LIKE \"%" 	+ lookForKey + "%\""
						")"
						" OR "
						"("
							"`users_company`.`current_company`='1' AND `company`.`name` LIKE \"%" + lookForKey + "%\""
						")"
					")";

			result = GetValuesFromDB(query, db);
		}
		if(searchWords.size() == 2)
		{
			MESSAGE_DEBUG("", "", "two words search");

			auto query = 
					"SELECT `users`.`id` FROM `users` "
					" LEFT JOIN `users_company` on `users_company`.`user_id` = `users`.`id` "
					" LEFT JOIN `company` on `company`.`id`=`users_company`.`company_id` "
					" WHERE "
					"("
						" `users`.`isActivated`='Y' AND `users`.`isblocked`='N' "
						+ (include_myself ? "" : " AND `users`.`id`!=\"" + user->GetID() + "\" ") +
					") "
					" AND "
					"("
						"("
							" `users_company`.`current_company`='1' "
							" AND "
							"( "
							" 	`company`.`name` like \"%" 	+ searchWords[0] + "%\" or "
							"	`company`.`name` like \"%" 	+ searchWords[1] + "%\" "
							")"
							" AND "
							"( "
							" 	`users`.`name` like \"%" 		+ searchWords[0] + "%\" or "
							" 	`users`.`name` like \"%" 		+ searchWords[1] + "%\" or "
							" 	`users`.`nameLast` like \"%" 	+ searchWords[0] + "%\" or "
							" 	`users`.`nameLast` like \"%" 	+ searchWords[1] + "%\" "
							" )"
						")"
						" OR "
						"("
							" ( "
							" 	`users`.`name` like \"%" 		+ searchWords[1] + "%\" AND "
							" 	`users`.`nameLast` like \"%" 	+ searchWords[0] + "%\" "
							" ) "
							" or "
							" ( "
							" 	`users`.`name` like \"%" 		+ searchWords[0] + "%\" AND "
							" 	`users`.`nameLast` like \"%" 	+ searchWords[1] + "%\" "
							" ) "
						")"
					")";

			result = GetValuesFromDB(query, db);
		}
		if(searchWords.size() == 3)
		{
			MESSAGE_DEBUG("", "", "three words search");

			auto	query = 
					"SELECT `users`.`id` FROM `users` "
					" LEFT JOIN `users_company` on `users_company`.`user_id` = `users`.`id` "
					" LEFT JOIN `company` on `company`.`id`=`users_company`.`company_id` "
					" WHERE "
					"("
						" `users`.`isActivated`='Y' AND `users`.`isblocked`='N' "
						+ (include_myself ? "" : " AND `users`.`id`!=\"" + user->GetID() + "\" ") +
					") "
					" AND "
					"("
						"("
							"`users_company`.`current_company`='1' and "
							"( "
							"	`company`.`name` like \"%" 		+ searchWords[0] + "%\" or "
							"	`company`.`name` like \"%" 		+ searchWords[1] + "%\" or "
							"	`company`.`name` like \"%" 		+ searchWords[2] + "%\" "
							")"
							" AND "
							"( "
							"	`users`.`name` like \"%" 		+ searchWords[0] + "%\" or "
							"	`users`.`name` like \"%" 		+ searchWords[1] + "%\" or "
							"	`users`.`name` like \"%" 		+ searchWords[2] + "%\" or "
							"	`users`.`nameLast` like \"%" 	+ searchWords[0] + "%\" or "
							"	`users`.`nameLast` like \"%" 	+ searchWords[1] + "%\" or "
							"	`users`.`nameLast` like \"%" 	+ searchWords[2] + "%\""
							")"
						")"
						" OR "
						"("
							"("
							"	`users`.`name` like \"%" 		+ searchWords[1] + "%\" and "
							"	`users`.`nameLast` like \"%" 	+ searchWords[0] + "%\" "
							")"
							" OR "
							"("
							"	`users`.`name` like \"%" 		+ searchWords[0] + "%\" and "
							"	`users`.`nameLast` like \"%" 	+ searchWords[1] + "%\" "
							")"
							" OR "
							"("
							"	`users`.`name` like \"%" 		+ searchWords[2] + "%\" and "
							"	`users`.`nameLast` like \"%" 	+ searchWords[0] + "%\" "
							")"
							" OR "
							"("
							"	`users`.`name` like \"%" 		+ searchWords[0] + "%\" and "
							"	`users`.`nameLast` like \"%" 	+ searchWords[2] + "%\" "
							")"
							" OR "
							"("
							"	`users`.`name` like \"%" 		+ searchWords[1] + "%\" and "
							"	`users`.`nameLast` like \"%" 	+ searchWords[2] + "%\" "
							")"
							" OR "
							"("
							"	`users`.`name` like \"%" 		+ searchWords[2] + "%\" and "
							"	`users`.`nameLast` like \"%" 	+ searchWords[1] + "%\" "
							")"
						")"
					")";

			result = GetValuesFromDB(query, db);
		}
	}
	else
	{
		MESSAGE_DEBUG("", "", "search request is empty");
	}

	MESSAGE_DEBUG("", "", "finish");

	return result;
}

auto GetUserListInJSONFormat_BySearchString(const string &lookForKey, bool include_myself, CMysql *db, CUser *user) -> string
{
	MESSAGE_DEBUG("", "", "start(" + lookForKey + ")");

	auto			users_id	= GetUsersID_BySearchString(lookForKey, include_myself, db, user);
	auto			result		= (users_id.size() ? GetUserListInJSONFormat("SELECT * FROM `users` WHERE `id` IN (" + join(users_id, ",") + ");", db, user) : "");

	MESSAGE_DEBUG("", "", "finish");

	return result;
}

static auto GetChecklistItemInJSONFormat(string dbQuery, CMysql *db, CUser *user) -> string
{
	MESSAGE_DEBUG("", "", "start");

	struct ItemClass 
	{
		string	id;
		string	event_checklist_id;
		string	checklist_predefined_id;
		string	category;
		string	title;
		string	state;
		string	price;
		string	comment;
		string	eventTimestamp;
	};

	vector<ItemClass>	items;
	auto				result = ""s;
	auto				affected = db->Query(dbQuery);

	if(affected)
	{
		for(int i = 0; i < affected; i++)
		{
			ItemClass	item;

			item.id								= db->Get(i, "id");
			item.event_checklist_id				= db->Get(i, "event_checklist_id");
			item.checklist_predefined_id		= db->Get(i, "checklist_predefined_id");
			item.category						= db->Get(i, "category");
			item.title							= db->Get(i, "title");
			item.state							= db->Get(i, "state");
			item.price							= db->Get(i, "price");
			item.comment						= db->Get(i, "comment");
			item.eventTimestamp					= db->Get(i, "eventTimestamp");

			items.push_back(item);
		}

		for(auto &item: items)
		{
			if(result.length()) result +=",";

			result += "{";
			result += "\"id\": \""				  	+ item.id + "\",";
			result += "\"title\": \""				+ item.title + "\",";
			result += "\"category\": \""			+ item.category + "\",";
			result += "\"state\": \""				+ item.state + "\",";
			result += "\"price\": \""				+ item.price + "\",";
			result += "\"comment\": \""				+ item.comment + "\",";
			result += "\"eventTimestamp\": \""		+ item.eventTimestamp + "\"";
			result += "}";
		}
	}
	else
	{
		MESSAGE_DEBUG("", "", "no items assigned to the checklist");
	}



	MESSAGE_DEBUG("", "", "finish");

	return result;
}


auto GetEventChecklistInJSONFormat(const string &dbQuery, CMysql *db, CUser *user) -> string
{
	MESSAGE_DEBUG("", "", "start");

	struct ItemClass 
	{
		string	id;
		string	title;
		string	event_id;
		string	eventTimestamp;
	};

	vector<ItemClass>	checklists;
	auto				result = ""s;
	auto				affected = db->Query(dbQuery);

	if(affected)
	{
		for(int i = 0; i < affected; i++)
		{
			ItemClass	checklist;

			checklist.id = db->Get(i, "id");
			checklist.title = db->Get(i, "title");
			checklist.event_id = db->Get(i, "event_id");
			checklist.eventTimestamp = db->Get(i, "eventTimestamp");

			checklists.push_back(checklist);
		}

		for(auto &checklist: checklists)
		{
			if(result.length()) result +=",";

			result += "{";
			result += "\"id\": \""				  	+ checklist.id + "\",";
			result += "\"title\": \""				+ checklist.title + "\",";
			result += "\"items\": ["				+ GetChecklistItemInJSONFormat(Get_ChecklistItemsByChecklistID_sqlquery(checklist.id), db, user) + "],";
			result += "\"eventTimestamp\": \""		+ checklist.eventTimestamp + "\"";
			result += "}";

		}

	}
	else
	{
		MESSAGE_DEBUG("", "", "check list not found");
	}


	MESSAGE_DEBUG("", "", "finish");

	return result;
}

auto GetFavoriteChecklistItemsInJSONFormat(const string &dbQuery, CMysql *db, CUser *user) -> string
{
	MESSAGE_DEBUG("", "", "start");

	struct ItemClass 
	{
		string	id;
		string	category;
		string	title;
		string	type;
		string	favorite;
	};

	vector<ItemClass>	items;
	auto				result = ""s;
	auto				affected = db->Query(dbQuery);

	if(affected)
	{
		for(int i = 0; i < affected; i++)
		{
			ItemClass	item;

			item.id								= db->Get(i, "id");
			item.category						= db->Get(i, "category");
			item.title							= db->Get(i, "title");
			item.type							= db->Get(i, "type");
			item.favorite						= db->Get(i, "favorite");

			items.push_back(item);
		}

		for(auto &item: items)
		{
			if(result.length()) result +=",";

			result += "{";
			result += "\"id\": \""				  	+ item.id + "\",";
			result += "\"title\": \""				+ item.title + "\",";
			result += "\"category\": \""			+ item.category + "\",";
			result += "\"type\": \""				+ item.type + "\",";
			result += "\"favorite\": \""			+ item.favorite + "\"";
			result += "}";
		}
	}
	else
	{
		MESSAGE_DEBUG("", "", "no items assigned to the checklist");
	}



	MESSAGE_DEBUG("", "", "finish");

	return result;
}

auto GetFavoriteChecklistCategoriesInJSONFormat(const string &dbQuery, CMysql *db, CUser *user) -> string
{
	MESSAGE_DEBUG("", "", "start");

	struct ItemClass 
	{
		string	id;
		string	title;
		string	event_id;
		string	favorite;
	};

	vector<ItemClass>	items;
	auto				result = ""s;
	auto				affected = db->Query(dbQuery);

	if(affected)
	{
		for(int i = 0; i < affected; i++)
		{
			ItemClass	item;

			item.id								= db->Get(i, "id");
			item.title							= db->Get(i, "title");
			item.event_id						= db->Get(i, "event_id");
			item.favorite						= db->Get(i, "favorite");

			items.push_back(item);
		}

		for(auto &item: items)
		{
			if(result.length()) result +=",";

			result += "{";
			result += "\"id\": \""				  	+ item.id + "\",";
			result += "\"title\": \""				+ item.title + "\",";
			result += "\"event_id\": \""			+ item.event_id + "\",";
			result += "\"favorite\": \""			+ item.favorite + "\"";
			result += "}";
		}
	}
	else
	{
		MESSAGE_DEBUG("", "", "no items assigned to the checklist");
	}



	MESSAGE_DEBUG("", "", "finish");

	return result;
}

auto amIAllowedToChangeEvent(const string id, CMysql *db, CUser *user) -> string
{
	MESSAGE_DEBUG("", "", "start");

	auto	error_message	= ""s;
	auto	event_id		= GetValueFromDB("SELECT `id` FROM `events` WHERE `id`=" + quoted(id) + " AND `owner_id`=" + quoted(user->GetID()) + ";", db);

	if(event_id.length())
	{
	}
	else
	{
		error_message = gettext("mandatory parameter missed");
		MESSAGE_ERROR("", "", error_message);
	}

	MESSAGE_DEBUG("", "", "finish");

	return error_message;
}

auto addMissedChecklistItems(const string &from_checklist_id, string const &to_checklist_id, CMysql *db) -> string
{
	MESSAGE_DEBUG("", "", "start (" + from_checklist_id + " -> " + to_checklist_id + ")");

	auto			error_message		= ""s;
	auto			item_ids			= GetValuesFromDB("SELECT `checklist_predefined_id` FROM `checklist_items` where `event_checklist_id`=" + quoted(from_checklist_id) + " AND `checklist_predefined_id` not in (SELECT `checklist_predefined_id` FROM `checklist_items` WHERE `event_checklist_id`=" + quoted(to_checklist_id) + ")", db);
	auto			sql_query			= "INSERT INTO `checklist_items` (`event_checklist_id`, `checklist_predefined_id`, `eventTimestamp`) VALUES ";
	vector<string>	sql_values;

	for(auto &item_id: item_ids)
	{
		sql_values.push_back("(" + quoted(to_checklist_id) + ", " + quoted(item_id) + ", UNIX_TIMESTAMP())");
	}

	if(sql_values.size())
	{
		db->Query(sql_query + join(sql_values, ","));
	}

	MESSAGE_DEBUG("", "", "finish");

	return error_message;
}

auto addChecklistItem(string const &to_checklist_id, const string &title, const string &category, CMysql *db, CUser *user) -> string
{
	MESSAGE_DEBUG("", "", "start (" + to_checklist_id + ", " + title + ", " + category + ")");

	auto	error_message				= ""s;
	auto	checklist_predefined_id		= GetValueFromDB("SELECT `id` FROM `checklist_predefined` where `title`=" + quoted(title) + " AND `category`=" + quoted(category) + ";", db);

	if(checklist_predefined_id.empty())
	{
		checklist_predefined_id = to_string(db->InsertQuery("INSERT INTO `checklist_predefined` (`category`, `title`) VALUES (" + quoted(category) + ", " + quoted(title) + ");"));
	}

	auto	checklist_item_id			= GetValueFromDB("SELECT `id` FROM `checklist_items` WHERE `event_checklist_id`=" + quoted(to_checklist_id) + " AND `checklist_predefined_id`=" + quoted(checklist_predefined_id) + ";", db);

	if(checklist_item_id.empty())
	{
		checklist_predefined_id = to_string(db->InsertQuery("INSERT INTO `checklist_items` (`event_checklist_id`, `checklist_predefined_id`, `eventTimestamp`) VALUES (" + quoted(to_checklist_id) + ", " + quoted(checklist_predefined_id) + ", UNIX_TIMESTAMP());"));
	}

	MESSAGE_DEBUG("", "", "finish");

	return error_message;
}


auto GetChecklistIDByEventID_CreateIfMissed(const string &event_id, CMysql *db, CUser *user) -> string
{
	MESSAGE_DEBUG("", "", "start (" + event_id + ")");

	auto	checklist_id		= GetValueFromDB("SELECT `id` FROM `event_checklists` WHERE `event_id`=" + quoted(event_id) + ";", db);

	if(checklist_id.empty())
	{
		checklist_id = to_string(db->InsertQuery("INSERT INTO `event_checklists` (`event_id`, `eventTimestamp`) VALUES (" + quoted(event_id) + ", UNIX_TIMESTAMP());"));
	}

	MESSAGE_DEBUG("", "", "finish (checklist_id = " + checklist_id + ")");

	return checklist_id;
}


auto RemoveChecklistsByEventID(const string &event_id, CMysql *db, CUser *user) -> string
{
	MESSAGE_DEBUG("", "", "start (" + event_id + ")");

	auto	error_message		= ""s;
	auto	checklist_items_id	= GetValuesFromDB(Get_ChecklistItemsIDByChecklistID_sqlquery(Get_ChecklistItemsIDByEventID(event_id)), db);

	if(checklist_items_id.size())
	{
		db->Query("DELETE FROM `checklist_items` WHERE `id` IN (" + join(checklist_items_id, ",") + ")");
		if(db->isError())
		{
			error_message = gettext("SQL syntax error");
		}

		if(error_message.empty())
		{
			db->Query("DELETE FROM `event_checklists` WHERE `event_id` IN (" + event_id + ")");
			if(db->isError())
			{
				error_message = gettext("SQL syntax error");
			}
		}
	}
	else
	{
		MESSAGE_DEBUG("", "", "no checklists associated with event.id(" + event_id + ")");
	}

	MESSAGE_DEBUG("", "", "finish");

	return error_message;
}

string GetChatMessagesInJSONFormat(string dbQuery, CMysql *db)
{
	ostringstream	result, ost;
	int				affected;

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: start");
	}
	result.str("");

	affected = db->Query(dbQuery);
	if(affected)
	{
		for(int i = 0; i < affected; i++)
		{
			result << (i ? "," : "") << "{\
				\"id\": \""						<< db->Get(i, "id") << "\", \
				\"message\": \"" 				<< ReplaceDoubleQuoteToQuote(db->Get(i, "message")) << "\", \
				\"fromType\": \"" 				<< db->Get(i, "fromType") << "\",\
				\"fromID\": \""					<< db->Get(i, "fromID") << "\",\
				\"toType\": \""			 		<< db->Get(i, "toType") << "\",\
				\"toID\": \""	 				<< db->Get(i, "toID") << "\",\
				\"messageStatus\": \""		  << db->Get(i, "messageStatus") << "\",\
				\"messageType\": \""			<< db->Get(i, "messageType") << "\",\
				\"eventTimestampDelta\": \""	<< GetHumanReadableTimeDifferenceFromNow(db->Get(i, "eventTimestamp")) << "\",\
				\"secondsSinceY2k\": \""		<< db->Get(i, "secondsSinceY2k") << "\",\
				\"eventTimestamp\": \""			<< db->Get(i, "eventTimestamp") << "\"\
			}";
		}
	}
	
	{
		CLog	log;
		log.Write(DEBUG, __func__ + string("[") + to_string(__LINE__) + string("]: end"));
	}

	return  result.str();
}

string GetUnreadChatMessagesInJSONFormat(CUser *user, CMysql *db)
{
	ostringstream	result, ost;
	int				affected;

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: start");
	}

	result.str("");

	ost.str("");
	ost << "select * from `chat_messages` where `toID`='" << user->GetID() << "' and (`messageStatus`='unread' or `messageStatus`='delivered' or `messageStatus`='sent');";
	affected = db->Query(ost.str());
	if(affected)
	{
		for(int i = 0; i < affected; i++)
		{
			result << (i ? "," : "") << "{\
				\"id\": \""					<< db->Get(i, "id") << "\", \
				\"message\": \"" 			<< ReplaceDoubleQuoteToQuote(db->Get(i, "message")) << "\", \
				\"fromType\": \"" 			<< db->Get(i, "fromType") << "\",\
				\"fromID\": \""				<< db->Get(i, "fromID") << "\",\
				\"toType\": \""			 	<< db->Get(i, "toType") << "\",\
				\"toID\": \""	 			<< db->Get(i, "toID") << "\",\
				\"messageType\": \""		<< db->Get(i, "messageType") << "\",\
				\"messageStatus\": \""		<< db->Get(i, "messageStatus") << "\",\
				\"eventTimestamp\": \""		<< db->Get(i, "eventTimestamp") << "\"\
			}";
		}
	}
	
	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: end");
	}

	return	result.str();
}

string GetDefaultActionLoggedinUser(void)
{
	MESSAGE_DEBUG("", "", "start");

	MESSAGE_DEBUG("", "", "finish");

	return LOGGEDIN_USER_DEFAULT_ACTION;
}
