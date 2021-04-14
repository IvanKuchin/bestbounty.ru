#include "cron_daily_pd.h"
#include "cactivator.h"

bool BirthdayNotificationToFriends(CMysql *db)
{
	bool 				result = false;
	long int			affected;
	vector<string>		birthdayUserList;

	MESSAGE_DEBUG("", "", "start");

	affected = db->Query("SELECT `id` FROM `users` WHERE `birthday` LIKE DATE_FORMAT(NOW(), '%d/%m/%') AND `birthdayAccess`=\"public\" AND `isblocked`=\"N\";");
	for(long int i = 0; i < affected; ++i)
	{
		birthdayUserList.push_back(db->Get(i, "id"));
	}

	for(auto &birthdayUserID: birthdayUserList)
	{
		vector<string>		userListToNotify;
		string				sqlQuery = "";

		userListToNotify.clear();
		affected = db->Query("SELECT `id` FROM `users` WHERE `id` IN (SELECT `friendID` FROM `users_friends` WHERE `userid`=\"" + birthdayUserID + "\" and `state`=\"confirmed\") AND `isblocked`=\"N\";");
		for(long int i = 0; i < affected; ++i)
		{
			if(sqlQuery.length()) sqlQuery += ",";
			sqlQuery += "(\"\", \"" + string(db->Get(i, "id")) + "\", \"58\", \"" + birthdayUserID + "\", UNIX_TIMESTAMP())";
		}

		if(sqlQuery.length())
		{
			if(db->InsertQuery("INSERT INTO `users_notification` (`title`, `userId`, `actionTypeId`, `actionId`, `eventTimestamp`) VALUES " + sqlQuery + ";"))
			{
			}
			else
			{
				CLog	log;
				log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR: inserting into `users_notifications` table");
			}
		}
		else
		{
			MESSAGE_DEBUG("", "", "friend list is empty, noone have to be notified");
		}
	}
	

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: finish");
	}

	return result;
}

bool EventNotificationToGuests(CMysql *db)
{
	bool 				result = false;
	long int			affected;
	vector<string>		eventIDList;

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: start");
	}

	affected = db->Query("SELECT `id` FROM `events` WHERE `startTimestamp`>=UNIX_TIMESTAMP() AND `startTimestamp`<(UNIX_TIMESTAMP() + 36 * 60 * 60) AND `isBlocked`=\"N\";");
	for(long int i = 0; i < affected; ++i)
	{
		eventIDList.push_back(db->Get(i, "id"));
	}

	for(auto &eventID: eventIDList)
	{
		vector<string>		userListToNotify;
		string				sqlQuery = "";

		userListToNotify.clear();
		affected = db->Query("SELECT `id` FROM `users` WHERE ((`id` IN (SELECT `user_id` FROM `event_guests` WHERE `event_id`=\"" + eventID + "\")) OR (`id` IN (SELECT `user_id` FROM `event_hosts` WHERE `event_id`=\"" + eventID + "\"))) AND `isblocked`=\"N\";");
		for(long int i = 0; i < affected; ++i)
		{
			if(sqlQuery.length()) sqlQuery += ",";
			sqlQuery += "(\"\", \"" + string(db->Get(i, "id")) + "\", \"70\", \"" + eventID + "\", UNIX_TIMESTAMP())";
		}

		if(sqlQuery.length() && db->InsertQuery("INSERT INTO `users_notification` (`title`, `userId`, `actionTypeId`, `actionId`, `eventTimestamp`) VALUES " + sqlQuery + ";"))
		{
		}
		else
		{
			CLog	log;
			log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR: inserting into `users_notifications` table");
		}
	}
	

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: finish");
	}

	return result;
}

bool UpdateGiftsToGive(CMysql *db)
{
	bool	result = true;

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: start");
	}

	db->Query("DELETE FROM `gifts_to_give` WHERE `eventTimestamp`<UNIX_TIMESTAMP()-(`reserve_period`*60*60*24);");

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: finish (result = " + (result ? "true" : "false") + ")");
	}

	return result;
}

int main()
{
	CStatistics		appStat;  // --- CStatistics must be first statement to measure end2end param's
	c_config		config(CONFIG_DIR);
	CMysql			db;
	struct timeval	tv;

	MESSAGE_DEBUG("", "", __FILE__);

	signal(SIGSEGV, crash_handler); 

	gettimeofday(&tv, NULL);
	srand(tv.tv_sec * tv.tv_usec * 100000);  /* Flawfinder: ignore */

	try
	{

		if(db.Connect(&config) < 0)
		{
			CLog	log;

			log.Write(ERROR, string(__func__) + string("[") + to_string(__LINE__) + "]:ERROR: Can not connect to mysql database");
			throw CExceptionHTML("MySql connection");
		}

		//--- start of daily cron main functionality

		//--- notify friends about public birthday
		BirthdayNotificationToFriends(&db);

		//--- notify guests about upcoming event
		EventNotificationToGuests(&db);

		//--- Remove gifts reservation to give
		UpdateGiftsToGive(&db);

		//--- end of daily cron main functionality
	}
	catch(CExceptionHTML &c)
	{
		CLog	log;

		c.SetLanguage("ru");
		c.SetDB(&db);

		log.Write(DEBUG, string(__func__) + ": catch CExceptionHTML: DEBUG exception reason: [", c.GetReason(), "]");

		return(0);
	}
	catch(CException &c)
	{
		CLog 	log;

		log.Write(ERROR, string(__func__) + ": catch CException: exception: ERROR  ", c.GetReason());

		return(-1);
	}
	catch(exception& e)
	{
		CLog 	log;
		log.Write(ERROR, string(__func__) + ": catch(exception& e): catch standard exception: ERROR  ", e.what());

		return(-1);
	}

	return(0);
}

