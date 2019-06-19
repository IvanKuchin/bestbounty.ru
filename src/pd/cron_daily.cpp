#include "cron_daily.h"
#include "cactivator.h"

bool CleanupActivators(CMysql *db)
{
	bool		result = true;

	MESSAGE_DEBUG("", "", "start");

	db->Query("DELETE FROM `activators` WHERE  `date`<=(now() - INTERVAL " + to_string(ACTIVATOR_SESSION_LEN) + " MINUTE);");
	
	MESSAGE_DEBUG("", "", "finish");

	return result;
}

bool RemoveOldCaptcha()
{
	DIR 		*dir;
	struct 		dirent *ent;
	bool		result = true; 
	string		dirName = IMAGE_CAPTCHA_DIRECTORY;
	time_t		now;

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: start");
	}

	time(&now);

	if ((dir = opendir( dirName.c_str() )) != NULL) 
	{
		/* print all the files and directories within directory */
		while ((ent = readdir (dir)) != NULL) 
		{
			struct	stat	sb;
			string	fileName = dirName + ent->d_name;

			if(stat(fileName.c_str(), &sb) == 0)
			{
				double		secondsBetween = difftime(now, sb.st_mtime);


				if(secondsBetween > 2600 * 24)
				{
					{
						CLog	log;
						log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: remove file [" + ent->d_name + "] created " + to_string(secondsBetween) + " secs ago");
					}
					unlink(fileName.c_str());
				}
			}
			else
			{
				{
					CLog	log;
					log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR: file stat [" + ent->d_name + "]");
				}
			}
		}
		closedir (dir);
	} 
	else 
	{
		/* could not open directory */
		result = false;
	}



	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: finish");
	}

	return result;
}

bool BirthdayNotificationToFriends(CMysql *db)
{
	bool 				result = false;
	long int			affected;
	vector<string>		birthdayUserList;

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: start");
	}

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

bool RemoveTempMedia(CMysql *db)
{
	bool	result = true;
	int		affected;

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: start");
	}

	affected = db->Query("SELECT * FROM `temp_media` WHERE `mediaType`=\"image\" AND `eventTimestamp`<DATE_SUB(CURDATE(), INTERVAL 2 DAY);");
	for(int i = 0; i < affected; ++i)
	{
		string	filename = IMAGE_TEMP_DIRECTORY + "/" + db->Get(i, "folder") + "/" + db->Get(i, "filename");

		if(isFileExists(filename))
		{
			{
				CLog	log;
				log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) +  "]: deleting file [filename=" + filename + "]");
			}
			unlink(filename.c_str());
		}
		else
		{
			CLog	log;
			log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) +  "]:ERROR: file doesn't exists  [filename=" + filename + "]");
		}
	}
	
	db->Query("DELETE FROM `temp_media` WHERE `mediaType`=\"image\" AND `eventTimestamp`<DATE_SUB(CURDATE(), INTERVAL 2 DAY);");

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: finish (result = " + (result ? "true" : "false") + ")");
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
	CStatistics		appStat;  // --- CStatistics must be firts statement to measure end2end param's
	CMysql			db;
	struct timeval	tv;

	{
		CLog	log;
		log.Write(DEBUG, __func__ + string("[") + to_string(__LINE__) + "]: " + __FILE__);
	}

	signal(SIGSEGV, crash_handler); 

	gettimeofday(&tv, NULL);
	srand(tv.tv_sec * tv.tv_usec * 100000);

	try
	{

		if(db.Connect(DB_NAME, DB_LOGIN, DB_PASSWORD) < 0)
		{
			CLog	log;

			log.Write(ERROR, string(__func__) + string("[") + to_string(__LINE__) + "]:ERROR: Can not connect to mysql database");
			throw CExceptionHTML("MySql connection");
		}

		//--- start of daily cron main functionality
		CleanupActivators(&db);

		//--- notify friends about public birthday
		BirthdayNotificationToFriends(&db);

		//--- notify guests about upcoming event
		EventNotificationToGuests(&db);

		//--- Remove temporarily media files
		RemoveTempMedia(&db);

		//--- Remove gifts reservation to give
		UpdateGiftsToGive(&db);

		//--- Remove old captcha
		RemoveOldCaptcha();

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

