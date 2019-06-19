#include "event.h"

static auto RemoveEventHosts(string event_host, CMysql &db)
{
	auto	result = true;

	MESSAGE_DEBUG("", "", "start");

	db.Query("DELETE FROM `users_notification` WHERE `actionTypeId`=\"67\" AND (`actionId` IN (SELECT `id` FROM `event_hosts` WHERE `id`=\"" + event_host + "\"));");
	if(db.isError())
	{
		result = false;
		MESSAGE_ERROR("", "", "fail to remove user.notifications belongs to event_host.id(" + event_host + ")");
	}

	db.Query("DELETE FROM `event_hosts` WHERE `id`=\"" + event_host + "\";");
	if(db.isError())
	{
		result = false;
		MESSAGE_ERROR("", "", "fail to remove event_host.id(" + event_host + ")");
	}

	MESSAGE_DEBUG("", "", "finish");

	return result;
}

int main()
{
	CStatistics		appStat;  // --- CStatistics must be firts statement to measure end2end param's
	CCgi			indexPage(EXTERNAL_TEMPLATE);
	CUser			user;
	string			action, partnerID;
	CMysql			db;
	struct timeval	tv;

	{
		MESSAGE_DEBUG("", action, __FILE__);
	}

	signal(SIGSEGV, crash_handler); 

	gettimeofday(&tv, NULL);
	srand(tv.tv_sec * tv.tv_usec * 100000);

	try
	{

		indexPage.ParseURL();
		indexPage.AddCookie("lng", "ru", "", "", "/");

		if(!indexPage.SetTemplate("index.htmlt"))
		{
			MESSAGE_DEBUG("", action, "template file was missing");
			throw CException("Template file was missing");
		}

		if(db.Connect(DB_NAME, DB_LOGIN, DB_PASSWORD) < 0)
		{
			MESSAGE_DEBUG("", action, "Can not connect to mysql database");
			throw CExceptionHTML("MySql connection");
		}

		indexPage.SetDB(&db);

		action = CheckHTTPParam_Text(indexPage.GetVarsHandler()->Get("action"));
		MESSAGE_DEBUG("", "", "action taken from HTTP is " + action);

		// --- generate common parts
		{
			// --- it has to be run before session, because session relay upon Apache environment variable
			if(RegisterInitialVariables(&indexPage, &db, &user)) {}
			else
			{
				MESSAGE_ERROR("", "", "RegisterInitialVariables failed, throwing exception");
				throw CExceptionHTML("environment variable error");
			}

			//------- Generate session
			action = GenerateSession(action, &indexPage, &db, &user);
		}
		// ------------ end generate common parts

		MESSAGE_DEBUG("", "", "pre-condition if(action == \"" + action + "\")");


		if(action == "events_list")
		{
			ostringstream	ost;
			string			strPageToGet, strFriendsOnSinglePage;

			{
				MESSAGE_DEBUG("", action, "start");
			}

			if(user.GetLogin() == "Guest")
			{
				ostringstream	ost;

				{
					MESSAGE_DEBUG("", action, "re-login required");
				}

				ost.str("");
				ost << "/?rand=" << GetRandom(10);
				indexPage.Redirect(ost.str());
			}
			else
			{
				indexPage.RegisterVariableForce("title_head", "События");

				strFriendsOnSinglePage	= indexPage.GetVarsHandler()->Get("FriendsOnSinglePage");
				strPageToGet 			= indexPage.GetVarsHandler()->Get("page");
				if(strPageToGet.empty()) strPageToGet = "0";
				{
					MESSAGE_DEBUG("", action, "page " + strPageToGet + " requested");
				}

				indexPage.RegisterVariableForce("myFirstName", user.GetName());
				indexPage.RegisterVariableForce("myLastName", user.GetNameLast());


				if(!indexPage.SetTemplate("events_list.htmlt"))
				{
					MESSAGE_DEBUG("", action, "can't find template my_network.htmlt");
					throw CExceptionHTML("user not activated");
				} // if(!indexPage.SetTemplate("my_network.htmlt"))
			}


			{
				MESSAGE_DEBUG("", action, "finish");
			}
		}

		if((action == "AJAX_getFindEventsListAutocomplete") || (action == "AJAX_getFindEventsList") || (action == "AJAX_findEventByID") || (action == "AJAX_getMyEventsList"))
		{
			ostringstream	ost, ostFinal;
			string			sessid, lookForKey, eventsList;
			vector<string>	searchWords;

			{
				MESSAGE_DEBUG("", action, "start");
			}

			// --- Initialization
			ostFinal.str("");

			if(user.GetLogin() == "Guest")
			{
				ostringstream	ost;

				{
					MESSAGE_DEBUG("", action, "re-login required");
				}

				ost.str("");
				ost << "/?rand=" << GetRandom(10);
				indexPage.Redirect(ost.str());
			}

			lookForKey = CheckHTTPParam_Text(indexPage.GetVarsHandler()->Get("lookForKey"));

			if( (lookForKey.length() >= 3) || ((action == "AJAX_findEventByID") && lookForKey.length())  || (action == "AJAX_getMyEventsList")) 
			{
				ostringstream	ost;

				// --- Looking through event name
				ost.str("");
				if(action == "AJAX_findEventByID")
					ost << "SELECT * FROM `events` WHERE (`owner_id`=\"" + user.GetID() + "\" OR `isBlocked`='N') AND (`id`=\"" << lookForKey << "\");";
				else if (action == "AJAX_getMyEventsList")
					ost << "SELECT * FROM `events` WHERE"
								" (`owner_id`=\"" << user.GetID() << "\") "
								" OR "
								" (`id` IN (SELECT `event_id` FROM `event_hosts` WHERE `user_id`=\"" + user.GetID() + "\")) "
								" OR "
								" ("
									" (`isBlocked`='N') "
									" AND "
									" (`id` IN (SELECT `event_id` FROM `event_guests` WHERE `user_id`=\"" + user.GetID() + "\")) "
								" );";
				else
					ost << "SELECT * FROM `events` WHERE "
								"("
									"(`owner_id`=\"" << user.GetID() << "\") "
									"OR "
									"("
										"`isBlocked`='N' "
										"AND "
										"("
											"`accessType`=\"everyone\" "
											"OR "
											"(`id` IN (SELECT `event_id` FROM `event_hosts` WHERE `user_id`=\"" + user.GetID() + "\")) "
											" OR "
											"(`id` IN (SELECT `event_id` FROM `event_guests` WHERE `user_id`=\"" + user.GetID() + "\")) "
										")"
									")"
								") "
								"AND "
								"((`title` like \"%" << lookForKey << "%\") OR (`description` like \"%" << lookForKey << "%\"));";

				ostFinal << "{\"status\":\"success\",\"events\":[" << GetEventListInJSONFormat(ost.str(), &db, &user) << "]}";

			}
			else
			{
				{
					MESSAGE_DEBUG("", action, "searching key is empty");
				}
				ostFinal << "{\"status\":\"error\",\"description\":\"searching key is empty or less then 3\", \"events\":[]}";
			}

			indexPage.RegisterVariableForce("result", ostFinal.str());

			if(!indexPage.SetTemplate("json_response.htmlt"))
			{
				MESSAGE_DEBUG("", action, "template file json_response.htmlt was missing");
				throw CException("Template file was missing");
			}

			{
				MESSAGE_DEBUG("", action, "final response [length = " + to_string(ostFinal.str().length()) + "]");
			}
		}

		if((action == "AJAX_blockEvent") || (action == "AJAX_unblockEvent"))
		{
			ostringstream   ostResult;

			{
				MESSAGE_DEBUG("", action, "start");
			}

			ostResult.str("");
			if(user.GetLogin() == "Guest")
			{
				{
					MESSAGE_DEBUG("", action, "re-login required");
				}

				ostResult << "{\"result\":\"error\",\"description\":\"re-login required\"}";
			}
			else
			{
				string		eventID;

				eventID = CheckHTTPParam_Number(indexPage.GetVarsHandler()->Get("id"));

				if(eventID.length())
				{
					if(db.Query("SELECT `id` FROM `event_hosts` WHERE `event_id`=\"" + eventID + "\" AND `user_id`=\"" + user.GetID() + "\";"))
					{
						string	dbQuery;

						if(action == "AJAX_blockEvent")
							dbQuery = "UPDATE `events` SET `isBlocked`=\"Y\" WHERE `id`=\"" + eventID + "\";";
						if(action == "AJAX_unblockEvent")
							dbQuery = "UPDATE `events` SET `isBlocked`=\"N\" WHERE `id`=\"" + eventID + "\";";

						db.Query(dbQuery);
						if(!db.isError())
						{
							ostResult	<< "{"
										<< "\"result\":\"success\","
										<< "\"events\":[" << GetEventListInJSONFormat("SELECT * FROM `events` WHERE `id`=\"" + eventID + "\";", &db, &user) << "]"
										<< "}";
						}
						else
						{

							{
								MESSAGE_ERROR("", action, ":AJAX_blockEvent: updating DB");
							}

							ostResult.str("");
							ostResult << "{";
							ostResult << "\"result\" : \"error\",";
							ostResult << "\"description\" : \"ошибка БД\"";
							ostResult << "}";
						}
					}
					else
					{
						MESSAGE_ERROR("", action, "AJAX_blockEvent: eventID [" + eventID + "] doesn't belongs to you");

						ostResult << "{\"result\":\"error\",\"description\":\"Вы не можете блокировать событие\",\"events\":[]}";
					}

				}
				else
				{
					{
						MESSAGE_DEBUG("", action, "in eventID [" + eventID + "]");
					}

					ostResult << "{\"result\":\"error\",\"description\":\"ERROR in eventID\",\"events\":[]}";
				}
			}



			indexPage.RegisterVariableForce("result", ostResult.str());

			if(!indexPage.SetTemplate("json_response.htmlt"))
			{
				MESSAGE_DEBUG("", action, "template file json_response.htmlt was missing");
				throw CException("Template file json_response.htmlt was missing");
			}  // if(!indexPage.SetTemplate("AJAX_precreateNewOpenVacancy.htmlt"))

			{
				MESSAGE_DEBUG("", action, "start");
			}
		}

		if(action == "AJAX_updateEventDescription")
		{
			ostringstream	ostFinal;

			{
				MESSAGE_DEBUG("", action, "start");
			}

			ostFinal.str("");

			if(user.GetLogin() == "Guest")
			{
				ostringstream	ost;

				{
					MESSAGE_DEBUG("", action, "re-login required");
				}

				ostFinal << "{\"result\":\"error\",\"description\":\"re-login required\"}";
			}
			else
			{
				string			value, id;

				value = CheckHTTPParam_Text(indexPage.GetVarsHandler()->Get("description"));
				id = CheckHTTPParam_Number(indexPage.GetVarsHandler()->Get("eventid"));

				if(value.length() && id.length()) 
				{
					if(db.Query("SELECT `event_id` FROM `event_hosts` WHERE `event_id`=\"" + id + "\" AND `user_id`=\"" + user.GetID() + "\";"))
					{
						db.Query("UPDATE `events` SET `description`=\"" + value + "\"  WHERE `id`=\"" + id + "\";");

						if(!db.isError())
						{
							ostFinal.str("");
							ostFinal << "{";
							ostFinal << "\"result\" : \"success\",";
							ostFinal << "\"description\" : \"\"";
							ostFinal << "}";
						}
						else
						{

							{
								MESSAGE_ERROR("", action, ":AJAX_updateEventDescription: updating DB");
							}

							ostFinal.str("");
							ostFinal << "{";
							ostFinal << "\"result\" : \"error\",";
							ostFinal << "\"description\" : \"error updating DB\"";
							ostFinal << "}";
						}
					}
					else
					{
						{
							MESSAGE_ERROR("", action, ":AJAX_updateEventDescription: user.id(" + user.GetID() + ") is not an event(" + id + ") host");
						}

						ostFinal.str("");
						ostFinal << "{";
						ostFinal << "\"result\" : \"error\",";
						ostFinal << "\"description\" : \"вы не можете изменить данные события\"";
						ostFinal << "}";
					}
				}
				else
				{
					ostringstream	ost;
					{
						MESSAGE_DEBUG("", action, "AJAX_updateEventDescription: id(" + id + ") or value(" + value + ") is empty");
					}

					ostFinal.str("");
					ostFinal << "{";
					ostFinal << "\"result\" : \"error\",";
					ostFinal << "\"description\" : \"пустые параметры id или value\"";
					ostFinal << "}";
				}
			}

			indexPage.RegisterVariableForce("result", ostFinal.str());

			if(!indexPage.SetTemplate("json_response.htmlt"))
			{
				MESSAGE_DEBUG("", action, "template file json_response.htmlt was missing");
				throw CException("Template file was missing");
			}

			{
				MESSAGE_DEBUG("", action, "end");
			}

		}

		if(action == "AJAX_removeEvent")
		{
			ostringstream	ostFinal;

			{
				MESSAGE_DEBUG("", action, "start");
			}

			ostFinal.str("");

			if(user.GetLogin() == "Guest")
			{
				ostringstream	ost;

				{
					MESSAGE_DEBUG("", action, "re-login required");
				}

				ostFinal << "{\"result\":\"error\",\"description\":\"re-login required\"}";
			}
			else
			{
				string	id;

				id = CheckHTTPParam_Number(indexPage.GetVarsHandler()->Get("id"));

				if(id.length()) 
				{
					if(db.Query("SELECT `id` FROM `events` WHERE `id`=\"" + id + "\" AND `owner_id`=\"" + user.GetID() + "\";"))
					{

						db.Query("DELETE FROM `users_notification` WHERE `actionTypeId`=\"70\" AND `actionId`=\"" + id + "\";");
						db.Query("DELETE FROM `users_notification` WHERE `actionTypeId`=\"69\" AND (`actionId` IN (SELECT `id` FROM `event_guests` WHERE `event_id`=\"" + id + "\"));");
						db.Query("DELETE FROM `users_notification` WHERE `actionTypeId`=\"68\" AND (`actionId` IN (SELECT `id` FROM `event_guests` WHERE `event_id`=\"" + id + "\"));");
						db.Query("DELETE FROM `users_notification` WHERE `actionTypeId`=\"67\" AND (`actionId` IN (SELECT `id` FROM `event_hosts` WHERE `event_id`=\"" + id + "\"));");

						db.Query("DELETE FROM `event_hosts` WHERE `event_id`=\"" + id + "\";");
						db.Query("DELETE FROM `event_guests` WHERE `event_id`=\"" + id + "\";");

						db.Query("DELETE FROM `events` WHERE `id`=\"" + id + "\";");

						if(db.isError())
						{

							{
								MESSAGE_DEBUG("", action, "updating DB");
							}

							ostFinal.str("");
							ostFinal << "{";
							ostFinal << "\"result\" : \"error\",";
							ostFinal << "\"description\" : \"error deleting event DB\"";
							ostFinal << "}";
						}
						else
						{
							ostFinal.str("");
							ostFinal << "{";
							ostFinal << "\"result\" : \"success\",";
							ostFinal << "\"description\" : \"\"";
							ostFinal << "}";
						}
					}
					else
					{
						{
							MESSAGE_DEBUG("", action, "user.id(" + user.GetID() + ") is not an event(" + id + ") host");
						}

						ostFinal.str("");
						ostFinal << "{";
						ostFinal << "\"result\" : \"error\",";
						ostFinal << "\"description\" : \"Удалить событие может только владелец\"";
						ostFinal << "}";
					}
				}
				else
				{
					ostringstream	ost;
					{
						MESSAGE_DEBUG("", action, "id is empty");
					}

					ostFinal.str("");
					ostFinal << "{";
					ostFinal << "\"result\" : \"error\",";
					ostFinal << "\"description\" : \"event_id не задан\"";
					ostFinal << "}";
				}
			}

			indexPage.RegisterVariableForce("result", ostFinal.str());

			if(!indexPage.SetTemplate("json_response.htmlt"))
			{
				MESSAGE_DEBUG("", action, "template file json_response.htmlt was missing");
				throw CException("Template file was missing");
			}

			{
				MESSAGE_DEBUG("", action, "end");
			}
		}

		if(action == "edit_event")
		{
			ostringstream	ost;

			{
				MESSAGE_DEBUG("", action, "start");
			}

			if(user.GetLogin() == "Guest")
			{
				ostringstream	ost;

				{
					MESSAGE_DEBUG("", action, "re-login required");
				}

				ost.str("");
				ost << "/?rand=" << GetRandom(10);
				indexPage.Redirect(ost.str());
			}

			indexPage.RegisterVariableForce("title", "Редактирование события");

			if(!indexPage.SetTemplate("edit_event.htmlt"))
			{
				MESSAGE_DEBUG("", action, "template file edit_event.htmlt was missing");
				throw CException("Template file edit_event.htmlt was missing");
			}  // if(!indexPage.SetTemplate("edit_event.htmlt"))

			{
				MESSAGE_DEBUG("", action, "end");
			}
		} 	// if(action == "edit_event")

		if(action == "AJAX_updateAccessType")
		{
			string			value, id;
			ostringstream	ostFinal;

			{
				MESSAGE_DEBUG("", action, "start");
			}

			if(user.GetLogin() == "Guest")
			{
				ostringstream	ost;

				{
					MESSAGE_DEBUG("", action, "re-login required");
				}

				ost.str("");
				ostFinal.str("");
				ostFinal << "{";
				ostFinal << "\"result\" : \"error\",";
				ostFinal << "\"description\" : \"re-login required\"";
				ostFinal << "}";
			}
			else
			{

				value = toLower(CheckHTTPParam_Text(indexPage.GetVarsHandler()->Get("value")));
				value = value.substr(0, 64);
				id = CheckHTTPParam_Number(indexPage.GetVarsHandler()->Get("eventid"));
				ostFinal.str("");

				if(value.length() && id.length()) 
				{
					if(db.Query("SELECT `id` FROM `event_hosts` WHERE `event_id`=\"" + id + "\" AND `user_id`=\"" + user.GetID() + "\";"))
					{
							db.Query("UPDATE `events` SET `accessType`=\"" + value + "\"  WHERE `id`=\"" + id + "\";");

							if(!db.isError())
							{
								ostFinal.str("");
								ostFinal << "{";
								ostFinal << "\"result\" : \"success\",";
								ostFinal << "\"description\" : \"\"";
								ostFinal << "}";
							}
							else
							{

								{
									MESSAGE_DEBUG("", action, "updating DB");
								}

								ostFinal.str("");
								ostFinal << "{";
								ostFinal << "\"result\" : \"error\",";
								ostFinal << "\"description\" : \"ошибка БД\"";
								ostFinal << "}";
							}
					}
					else
					{
						{
							MESSAGE_DEBUG("", action, "user.id(" + user.GetID() + ") is not a event(" + id + ") owner");
						}

						ostFinal.str("");
						ostFinal << "{";
						ostFinal << "\"result\" : \"error\",";
						ostFinal << "\"description\" : \"вы не можете изменить данные события\"";
						ostFinal << "}";
					}
				}
				else
				{
					ostringstream	ost;
					{
						MESSAGE_DEBUG("", action, "id(" + id + ") or value(" + value + ") is empty");
					}

					ostFinal.str("");
					ostFinal << "{";
					ostFinal << "\"result\" : \"error\",";
					ostFinal << "\"description\" : \"пустые параметры id или value\"";
					ostFinal << "}";
				}
			}


			indexPage.RegisterVariableForce("result", ostFinal.str());

			if(!indexPage.SetTemplate("json_response.htmlt"))
			{
				MESSAGE_DEBUG("", action, "template file json_response.htmlt was missing");
				throw CException("Template file was missing");
			}

			{
				MESSAGE_DEBUG("", action, "end");
			}

		}

		if((action == "AJAX_updateStartTime") || (action == "AJAX_updateStartDate"))
		{
			string			value, id;
			ostringstream	ostFinal;

			{
				MESSAGE_DEBUG("", action, "start");
			}

			if(user.GetLogin() == "Guest")
			{
				ostringstream	ost;

				{
					MESSAGE_DEBUG("", action, "re-login required");
				}

				ost.str("");
				ostFinal.str("");
				ostFinal << "{";
				ostFinal << "\"result\" : \"error\",";
				ostFinal << "\"description\" : \"re-login required\"";
				ostFinal << "}";
			}
			else
			{

				value = CheckHTTPParam_Number(indexPage.GetVarsHandler()->Get("value"));
				id = CheckHTTPParam_Number(indexPage.GetVarsHandler()->Get("eventid"));
				ostFinal.str("");

				if(value.length() && id.length()) 
				{
					if(db.Query("SELECT `id` FROM `event_hosts` WHERE `event_id`=\"" + id + "\" AND `user_id`=\"" + user.GetID() + "\";"))
					{
							db.Query("UPDATE `events` SET `startTimestamp`=\"" + value + "\"  WHERE `id`=\"" + id + "\";");

							if(!db.isError())
							{
								ostFinal.str("");
								ostFinal << "{";
								ostFinal << "\"result\" : \"success\",";
								ostFinal << "\"description\" : \"\"";
								ostFinal << "}";
							}
							else
							{

								{
									MESSAGE_DEBUG("", action, "updating DB");
								}

								ostFinal.str("");
								ostFinal << "{";
								ostFinal << "\"result\" : \"error\",";
								ostFinal << "\"description\" : \"ошибка БД\"";
								ostFinal << "}";
							}
					}
					else
					{
						{
							MESSAGE_DEBUG("", action, "user.id(" + user.GetID() + ") is not a event(" + id + ") host");
						}

						ostFinal.str("");
						ostFinal << "{";
						ostFinal << "\"result\" : \"error\",";
						ostFinal << "\"description\" : \"вы не можете изменить данные события\"";
						ostFinal << "}";
					}
				}
				else
				{
					ostringstream	ost;
					{
						MESSAGE_DEBUG("", action, "id(" + id + ") or value(" + value + ") is empty");
					}

					ostFinal.str("");
					ostFinal << "{";
					ostFinal << "\"result\" : \"error\",";
					ostFinal << "\"description\" : \"пустые параметры id или value\"";
					ostFinal << "}";
				}
			}


			indexPage.RegisterVariableForce("result", ostFinal.str());

			if(!indexPage.SetTemplate("json_response.htmlt"))
			{
				MESSAGE_DEBUG("", action, "template file json_response.htmlt was missing");
				throw CException("Template file was missing");
			}

			{
				MESSAGE_DEBUG("", action, "end");
			}

		}


		if(action == "AJAX_updateEventLink")
		{
			string			value, id;
			ostringstream	ostFinal;

			{
				MESSAGE_DEBUG("", action, "start");
			}

			if(user.GetLogin() == "Guest")
			{
				ostringstream	ost;

				{
					MESSAGE_DEBUG("", action, "re-login required");
				}

				ost.str("");
				ostFinal.str("");
				ostFinal << "{";
				ostFinal << "\"result\" : \"error\",";
				ostFinal << "\"description\" : \"re-login required\"";
				ostFinal << "}";
			}
			else
			{

				value = toLower(CheckHTTPParam_Text(indexPage.GetVarsHandler()->Get("value")));
				value = value.substr(0, 64);
				id = CheckHTTPParam_Number(indexPage.GetVarsHandler()->Get("eventid"));
				ostFinal.str("");

				if(value.length() && id.length()) 
				{
					if(db.Query("SELECT `id` FROM `event_hosts` WHERE `event_id`=\"" + id + "\" AND `user_id`=\"" + user.GetID() + "\";"))
					{
						if(db.Query("SELECT `link` FROM `events` WHERE `id`=\"" + id + "\";"))
						{
							string	originalLink = db.Get(0, "link");

							if(value.length() < 10)
							{
								{
									MESSAGE_DEBUG("", action, "link[" + value + "] is too short, must be longer than 10 symbols");
								}

								ostFinal.str("");
								ostFinal << "{";
								ostFinal << "\"result\" : \"error\",";
								ostFinal << "\"link\" : \"" + originalLink + "\",";
								ostFinal << "\"description\" : \"ссылка должна быть минимум 10 символов\"";
								ostFinal << "}";
							}
							else if(value.find_first_not_of("abcdefghijklmnopqrstuvwxyz_-1234567890") != string::npos)
							{
								{
									MESSAGE_DEBUG("", action, "link[" + value + "] contains prohibited symbols");
								}

								ostFinal.str("");
								ostFinal << "{";
								ostFinal << "\"result\" : \"error\",";
								ostFinal << "\"link\" : \"" + originalLink + "\",";
								ostFinal << "\"description\" : \"ссылка можеть содержать только английские буквы или цифры\"";
								ostFinal << "}";
							}
							else if(db.Query("SELECT `id` FROM `events` WHERE `link`=\"" + value + "\";"))
							{
								{
									MESSAGE_DEBUG("", action, "link[" + value + "] contains prohibited symbols");
								}

								ostFinal.str("");
								ostFinal << "{";
								ostFinal << "\"result\" : \"error\",";
								ostFinal << "\"link\" : \"" + originalLink + "\",";
								ostFinal << "\"description\" : \"событие(" + value + ") уже существует\"";
								ostFinal << "}";
							}
							else
							{
								db.Query("UPDATE `events` SET `link`=\"" + value + "\"  WHERE `id`=\"" + id + "\";");

								if(!db.isError())
								{
									ostFinal.str("");
									ostFinal << "{";
									ostFinal << "\"result\" : \"success\",";
									ostFinal << "\"description\" : \"\"";
									ostFinal << "}";
								}
								else
								{

									{
										MESSAGE_DEBUG("", action, "updating DB");
									}

									ostFinal.str("");
									ostFinal << "{";
									ostFinal << "\"result\" : \"error\",";
									ostFinal << "\"link\" : \"" + originalLink + "\",";
									ostFinal << "\"description\" : \"ошибка БД\"";
									ostFinal << "}";
								}
							}
						}
						else
						{
							{
								MESSAGE_DEBUG("", action, "event(" + id + ") not found");
							}

							ostFinal.str("");
							ostFinal << "{";
							ostFinal << "\"result\" : \"error\",";
							ostFinal << "\"description\" : \"событие не найдено\"";
							ostFinal << "}";
						}
					}
					else
					{
						{
							MESSAGE_DEBUG("", action, "user.id(" + user.GetID() + ") is not a event(" + id + ") owner");
						}

						ostFinal.str("");
						ostFinal << "{";
						ostFinal << "\"result\" : \"error\",";
						ostFinal << "\"description\" : \"Вы не можете изменить данные события\"";
						ostFinal << "}";
					}
				}
				else
				{
					string			originalLink = "";
					ostringstream	ost;

					{
						MESSAGE_DEBUG("", action, "id(" + id + ") or value(" + value + ") is empty");
					}


					if(id.length() && db.Query("SELECT `link` FROM `events` WHERE `id`=\"" + id + "\";")) originalLink = db.Get(0, "link");


					ostFinal.str("");
					ostFinal << "{";
					ostFinal << "\"result\" : \"error\",";
					ostFinal << "\"link\" : \"" + originalLink + "\",";
					ostFinal << "\"description\" : \"ссылка не должна быть пустой\"";
					ostFinal << "}";
				}
			}


			indexPage.RegisterVariableForce("result", ostFinal.str());

			if(!indexPage.SetTemplate("json_response.htmlt"))
			{
				MESSAGE_DEBUG("", action, "template file json_response.htmlt was missing");
				throw CException("Template file was missing");
			}

			{
				MESSAGE_DEBUG("", action, "end");
			}

		}

		if(action == "AJAX_createEvent")
		{
			string			id;
			ostringstream	ostFinal;

			{
				MESSAGE_DEBUG("", action, "start");
			}

			if(user.GetLogin() == "Guest")
			{
				{
					MESSAGE_DEBUG("", action, "re-login required");
				}

				ostFinal.str("");
				ostFinal << "{";
				ostFinal << "\"result\" : \"error\",";
				ostFinal << "\"description\" : \"re-login required\"";
				ostFinal << "}";
			}
			else
			{
				string		title, timestamp, accessType, address, description;

				title = CheckHTTPParam_Text(indexPage.GetVarsHandler()->Get("title"));
				timestamp = CheckHTTPParam_Number(indexPage.GetVarsHandler()->Get("timestamp"));
				accessType = CheckHTTPParam_Text(indexPage.GetVarsHandler()->Get("accessType"));
				address = CheckHTTPParam_Text(indexPage.GetVarsHandler()->Get("address"));
				description = CheckHTTPParam_Text(indexPage.GetVarsHandler()->Get("description"));

				ostFinal.str("");
				if(title.length() == 0) 
				{
					ostringstream	ost;
					{
						MESSAGE_DEBUG("", action, "title is empty");
					}

					ostFinal.str("");
					ostFinal << "{";
					ostFinal << "\"result\" : \"error\",";
					ostFinal << "\"description\" : \"Событию необходимо название\"";
					ostFinal << "}";
				}
				else if(timestamp.length() == 0) 
				{
					ostringstream	ost;
					{
						MESSAGE_ERROR("", action, "event timestamp is empty");
					}

					ostFinal.str("");
					ostFinal << "{";
					ostFinal << "\"result\" : \"error\",";
					ostFinal << "\"description\" : \"Нужно указать дату и время начала события\"";
					ostFinal << "}";
				}
				else if((accessType != "invitee") && (accessType != "everyone"))
				{
					ostringstream	ost;
					{
						MESSAGE_ERROR("", action, "accessType(" + accessType + ") is unknown");
					}

					ostFinal.str("");
					ostFinal << "{";
					ostFinal << "\"result\" : \"error\",";
					ostFinal << "\"description\" : \"'по приглашению' или 'открыто для всех' ?\"";
					ostFinal << "}";
				}
				else
				{
					long int		id;
			
					id = db.InsertQuery("INSERT INTO `events` SET "
										" `title`=\"" + title + "\","
										" `description`=\"" + description + "\","
										" `address`=\"" + address + "\","
										" `owner_id`=\"" + user.GetID() + "\","
										" `accessType`=\"" + accessType + "\","
										" `startTimestamp`=\"" + timestamp + "\","
										" `eventTimestampCreation`=UNIX_TIMESTAMP()"
										";");

					if(id)
					{
						if(db.InsertQuery("INSERT INTO `event_hosts` SET `event_id`=\"" + to_string(id) + "\", `user_id`=\"" + user.GetID() + "\", `eventTimestamp`=UNIX_TIMESTAMP();"))
						{
							bool	isDuplicate = true;
							string	eventLink = to_string(id);

							do
							{
								if(!db.Query("SELECT `id` FROM `events` WHERE `link`=\"" + eventLink + "\";")) 
									isDuplicate = false;
								else
									eventLink = GetRandom(32);
							} while(isDuplicate);

							db.Query("UPDATE `events` SET `link`=\"" + eventLink + "\" WHERE `id`=\"" + to_string(id) + "\";");
							if(db.isError())
							{
								MESSAGE_DEBUG("", action, "updating `event` table");
							}

							ostFinal.str("");
							ostFinal << "{";
							ostFinal << "\"result\" : \"success\",";
							ostFinal << "\"events\" : [" << GetEventListInJSONFormat("SELECT * FROM `events` WHERE `id`=\"" + to_string(id) + "\";", &db, &user) << "],";
							ostFinal << "\"description\" : \"\"";
							ostFinal << "}";
						}
						else
						{
							ostFinal.str("");
							ostFinal << "{";
							ostFinal << "\"result\" : \"error\",";
							ostFinal << "\"description\" : \"ошибка БД\"";
							ostFinal << "}";
							
							{
								MESSAGE_ERROR("", action, "inserting into `event_hosts` table, lost event.id(" + to_string(id) + ")");
							}
						}


	/*
						db.InsertQuery("INSERT INTO `users_subscriptions` SET `user_id`=\"" + user.GetID() + "\","
																			"`entity_type`=\"group\","
																			"`entity_id`=\"" + to_string(id) + "\","
																			"`eventTimestamp`=UNIX_TIMESTAMP()"
																			";");

						// --- insert creation event into feed
						if(db.Query("SELECT `id` FROM `feed` WHERE `userId`=\"" + user.GetID() + "\" AND `srcType`=\"user\" AND `actionTypeId`=\"65\" AND `actionId`=\"" + to_string(id) + "\";"))
						{
							MESSAGE_DEBUG("", action, "already subscribed [groupID: " + to_string(id) + ", userID: " + user.GetID() + "]");
						}
						else
						{
							if(db.InsertQuery("INSERT INTO `feed` (`userId`, `actionTypeId`, `actionId`, `eventTimestamp`) values(\"" + user.GetID() + "\", \"65\", \"" + to_string(id) + "\", NOW())"))
							{
							}
							else
							{
								MESSAGE_ERROR("", action, "inserting into `feed` table");
							}
						}
	*/
					}
					else
					{

						{
							MESSAGE_ERROR("", action, "inserting into `event` table");
						}

						ostFinal.str("");
						ostFinal << "{";
						ostFinal << "\"result\" : \"error\",";
						ostFinal << "\"description\" : \"ошибка БД\"";
						ostFinal << "}";
					}
				}
			}

			indexPage.RegisterVariableForce("result", ostFinal.str());

			if(!indexPage.SetTemplate("json_response.htmlt"))
			{
				MESSAGE_DEBUG("", action, "template file json_response.htmlt was missing");
				throw CException("Template file was missing");
			}

			{
				MESSAGE_DEBUG("", action, "end");
			}
		}

		if(action == "AJAX_removeEventHost")
		{
			ostringstream	ostFinal;
			auto			id = ""s;
			auto			error_message = ""s;

			{
				MESSAGE_DEBUG("", action, "start");
			}

			if(user.GetLogin() == "Guest")
			{
				ostringstream	ost;

				{
					MESSAGE_DEBUG("", action, "re-login required");
				}

				ost.str("");
				ostFinal.str("");
				ostFinal << "{";
				ostFinal << "\"result\" : \"error\",";
				ostFinal << "\"description\" : \"re-login required\"";
				ostFinal << "}";
			}
			else
			{
				id = CheckHTTPParam_Number(indexPage.GetVarsHandler()->Get("id"));
				ostFinal.str("");

				if(id.length()) 
				{
					if(db.Query("SELECT * FROM `event_hosts` WHERE `id`=\"" + id + "\";"))
					{
						string	event_id = db.Get(0, "event_id");
						string	user_id = db.Get(0, "user_id");

						if(db.Query("SELECT `owner_id` FROM `events` WHERE `id`=\"" + event_id + "\";"))
						{
							string	owner_id = db.Get(0, "owner_id");

							if(owner_id == user_id)
							{
								// --- speacial case: owner can remove himself
								// --- this is required if you organize someones event and don't want to expose your own gifts
								if(owner_id == user.GetID())
								{
									if(db.Query("SELECT `user_id` FROM `event_hosts` WHERE `event_id`=\"" + event_id + "\" AND `user_id`!=\"" + owner_id + "\";"))
									{
										string	co_host_user_id = db.Get(0, "user_id");

										db.Query("UPDATE `events` SET `owner_id`=\"" + co_host_user_id + "\" WHERE `id`=\"" + event_id + "\";");
										if(db.isError())
										{
											MESSAGE_ERROR("", action, "fail to reassign event.id(" + event_id + ") to user.id(" + co_host_user_id + ")");
										}
										else
										{
											if(RemoveEventHosts(id, db))
											{
												ostFinal.str("");
												ostFinal << "{";
												ostFinal << "\"result\" : \"success\",";
												ostFinal << "\"hosts\" : [" << GetEventHostsListInJSONFormat("SELECT * FROM `event_hosts` WHERE `event_id`=\"" + event_id + "\";", &db, &user) << "],";
												ostFinal << "\"removed_user_id\" : \"" << user_id << "\",";
												ostFinal << "\"description\" : \"\"";
												ostFinal << "}";
											}
											else
											{
												MESSAGE_ERROR("", action, "RemoveEventHosts failed");

												error_message = "ошибка БД";
											}
										}
									}
									else
									{
										MESSAGE_DEBUG("", action, "only event owner can't be removed");

										error_message = "Единственный владелец не может быть удален";
									}
								}
								else
								{
									MESSAGE_DEBUG("", action, "event owner can't be deleted from hosts list");

									error_message = "Владелец не может быть удален";
								}
							}
							else
							{
								if(RemoveEventHosts(id, db))
								{
									ostFinal.str("");
									ostFinal << "{";
									ostFinal << "\"result\" : \"success\",";
									ostFinal << "\"hosts\" : [" << GetEventHostsListInJSONFormat("SELECT * FROM `event_hosts` WHERE `event_id`=\"" + event_id + "\";", &db, &user) << "],";
									ostFinal << "\"removed_user_id\" : \"" << user_id << "\",";
									ostFinal << "\"description\" : \"\"";
									ostFinal << "}";
								}
								else
								{
									MESSAGE_DEBUG("", action, "RemoveEventHosts failed");

									error_message = "ошибка БД";
								}
							}
						}
						else
						{
							MESSAGE_DEBUG("", action, "user.id(" + user.GetID() + ") is not a event(" + id + ") owner");

							error_message = "вы не можете изменить данные события";
						}
					}
					else
					{
						MESSAGE_DEBUG("", action, "event_hosts.id(" + id + ") not found");

						error_message = "организатор не найден";
					}
				}
				else
				{
					MESSAGE_DEBUG("", action, "id(" + id + ")");

					error_message = "пустые параметры id или value";
				}
			}


			if(error_message.empty())
			{
			}
			else
			{
				MESSAGE_DEBUG("", action, "failed");

				ostFinal.str("");
				ostFinal << "{\"result\":\"error\",\"description\":\"" + error_message + "\"}";
			}

			indexPage.RegisterVariableForce("result", ostFinal.str());

			if(!indexPage.SetTemplate("json_response.htmlt"))
			{
				MESSAGE_DEBUG("", action, "template file json_response.htmlt was missing");
				throw CException("Template file was missing");
			}

			{
				MESSAGE_DEBUG("", action, "end");
			}

		}

		if(action == "AJAX_removeEventGuest")
		{
			string			id;
			ostringstream	ostFinal;

			{
				MESSAGE_DEBUG("", action, "start");
			}

			if(user.GetLogin() == "Guest")
			{
				ostringstream	ost;

				{
					MESSAGE_DEBUG("", action, "re-login required");
				}

				ost.str("");
				ostFinal.str("");
				ostFinal << "{";
				ostFinal << "\"result\" : \"error\",";
				ostFinal << "\"description\" : \"re-login required\"";
				ostFinal << "}";
			}
			else
			{
				id = CheckHTTPParam_Number(indexPage.GetVarsHandler()->Get("id"));
				ostFinal.str("");

				if(id.length()) 
				{
					if(db.Query("SELECT * FROM `event_guests` WHERE `id`=\"" + id + "\";"))
					{
						string	event_id = db.Get(0, "event_id");
						string	user_id = db.Get(0, "user_id");

						if(db.Query("SELECT `id` FROM `event_hosts` WHERE `event_id`=\"" + event_id + "\" AND `user_id`=\"" + user.GetID() + "\";"))
						{
							{

								db.Query("DELETE FROM `users_notification` WHERE `actionTypeId`=\"68\" AND (`actionId` IN (SELECT `id` FROM `event_guests` WHERE `id`=\"" + id + "\"));");
								db.Query("DELETE FROM `users_notification` WHERE `actionTypeId`=\"69\" AND (`actionId` IN (SELECT `id` FROM `event_guests` WHERE `id`=\"" + id + "\"));");
								db.Query("DELETE FROM `quick_registration` WHERE `id` IN (SELECT `quick_registration_id` FROM `event_guests` WHERE `id`=\"" + id + "\");");
								db.Query("DELETE FROM `event_guests` WHERE `id`=\"" + id + "\";");

								if(!db.isError())
								{
									ostFinal.str("");
									ostFinal << "{";
									ostFinal << "\"result\" : \"success\",";
									ostFinal << "\"guests\" : [" << GetEventGuestsListInJSONFormat("SELECT * FROM `event_guests` WHERE `event_id`=\"" + event_id + "\";", &db, &user) << "],";
									ostFinal << "\"description\" : \"\"";
									ostFinal << "}";
								}
								else
								{

									{
										MESSAGE_DEBUG("", action, "updating DB");
									}

									ostFinal.str("");
									ostFinal << "{";
									ostFinal << "\"result\" : \"error\",";
									ostFinal << "\"description\" : \"ошибка БД\"";
									ostFinal << "}";
								}
							}
						}
						else
						{
							{
								MESSAGE_DEBUG("", action, "user.id(" + user.GetID() + ") is not a event(" + id + ") owner");
							}

							ostFinal.str("");
							ostFinal << "{";
							ostFinal << "\"result\" : \"error\",";
							ostFinal << "\"description\" : \"вы не можете изменить данные события\"";
							ostFinal << "}";
						}
					}
					else
					{
							{
								MESSAGE_DEBUG("", action, "event_guests.id(" + id + ") not found");
							}

							ostFinal.str("");
							ostFinal << "{";
							ostFinal << "\"result\" : \"error\",";
							ostFinal << "\"description\" : \"гость не найден\"";
							ostFinal << "}";
					}
				}
				else
				{
					ostringstream	ost;
					{
						MESSAGE_DEBUG("", action, "id(" + id + ")");
					}

					ostFinal.str("");
					ostFinal << "{";
					ostFinal << "\"result\" : \"error\",";
					ostFinal << "\"description\" : \"пустые параметры id или value\"";
					ostFinal << "}";
				}
			}


			indexPage.RegisterVariableForce("result", ostFinal.str());

			if(!indexPage.SetTemplate("json_response.htmlt"))
			{
				MESSAGE_DEBUG("", action, "template file json_response.htmlt was missing");
				throw CException("Template file was missing");
			}

			{
				MESSAGE_DEBUG("", action, "end");
			}
		}

		if(action == "AJAX_updateTitle")
		{
			string			value, id;
			ostringstream	ostFinal;

			{
				MESSAGE_DEBUG("", action, "start");
			}

			if(user.GetLogin() == "Guest")
			{
				ostringstream	ost;

				{
					MESSAGE_DEBUG("", action, "re-login required");
				}

				ost.str("");
				ostFinal.str("");
				ostFinal << "{";
				ostFinal << "\"result\" : \"error\",";
				ostFinal << "\"description\" : \"re-login required\"";
				ostFinal << "}";
			}
			else
			{

				value = CheckHTTPParam_Text(indexPage.GetVarsHandler()->Get("value"));
				value = value.substr(0, 512);
				id = CheckHTTPParam_Number(indexPage.GetVarsHandler()->Get("eventid"));
				ostFinal.str("");

				if(value.length() && id.length()) 
				{
					if(db.Query("SELECT `id` FROM `event_hosts` WHERE `event_id`=\"" + id + "\" AND `user_id`=\"" + user.GetID() + "\";"))
					{
							db.Query("UPDATE `events` SET `title`=\"" + value + "\"  WHERE `id`=\"" + id + "\";");

							if(!db.isError())
							{
								ostFinal.str("");
								ostFinal << "{";
								ostFinal << "\"result\" : \"success\",";
								ostFinal << "\"description\" : \"\"";
								ostFinal << "}";
							}
							else
							{
								{
									MESSAGE_DEBUG("", action, "updating DB");
								}

								ostFinal.str("");
								ostFinal << "{";
								ostFinal << "\"result\" : \"error\",";
								ostFinal << "\"description\" : \"ошибка БД\"";
								ostFinal << "}";
							}
					}
					else
					{
						{
							MESSAGE_DEBUG("", action, "user.id(" + user.GetID() + ") is not a event(" + id + ") owner");
						}

						ostFinal.str("");
						ostFinal << "{";
						ostFinal << "\"result\" : \"error\",";
						ostFinal << "\"description\" : \"Вы не можете изменить данные события\"";
						ostFinal << "}";
					}
				}
				else
				{
					ostringstream	ost;
					{
						MESSAGE_DEBUG("", action, "id(" + id + ") or value(" + value + ") is empty");
					}

					ostFinal.str("");
					ostFinal << "{";
					ostFinal << "\"result\" : \"error\",";
					ostFinal << "\"description\" : \"Название не должно быть пустым\"";
					ostFinal << "}";
				}
			}


			indexPage.RegisterVariableForce("result", ostFinal.str());

			if(!indexPage.SetTemplate("json_response.htmlt"))
			{
				MESSAGE_DEBUG("", action, "template file json_response.htmlt was missing");
				throw CException("Template file was missing");
			}

			{
				MESSAGE_DEBUG("", action, "end");
			}
		}

		if(action == "AJAX_updateAddress")
		{
			string			value, id;
			ostringstream	ostFinal;

			{
				MESSAGE_DEBUG("", action, "start");
			}

			if(user.GetLogin() == "Guest")
			{
				ostringstream	ost;

				{
					MESSAGE_DEBUG("", action, "re-login required");
				}

				ost.str("");
				ostFinal.str("");
				ostFinal << "{";
				ostFinal << "\"result\" : \"error\",";
				ostFinal << "\"description\" : \"re-login required\"";
				ostFinal << "}";
			}
			else
			{

				value = CheckHTTPParam_Text(indexPage.GetVarsHandler()->Get("value"));
				value = value.substr(0, 512);
				id = CheckHTTPParam_Number(indexPage.GetVarsHandler()->Get("eventid"));
				ostFinal.str("");

				if(value.length() && id.length()) 
				{
					if(db.Query("SELECT `id` FROM `event_hosts` WHERE `event_id`=\"" + id + "\" AND `user_id`=\"" + user.GetID() + "\";"))
					{
							db.Query("UPDATE `events` SET `address`=\"" + value + "\"  WHERE `id`=\"" + id + "\";");

							if(!db.isError())
							{
								ostFinal.str("");
								ostFinal << "{";
								ostFinal << "\"result\" : \"success\",";
								ostFinal << "\"description\" : \"\"";
								ostFinal << "}";
							}
							else
							{
								{
									MESSAGE_DEBUG("", action, "updating DB");
								}

								ostFinal.str("");
								ostFinal << "{";
								ostFinal << "\"result\" : \"error\",";
								ostFinal << "\"description\" : \"ошибка БД\"";
								ostFinal << "}";
							}
					}
					else
					{
						{
							MESSAGE_DEBUG("", action, "user.id(" + user.GetID() + ") is not a event(" + id + ") owner");
						}

						ostFinal.str("");
						ostFinal << "{";
						ostFinal << "\"result\" : \"error\",";
						ostFinal << "\"description\" : \"вы не можете изменить данные события\"";
						ostFinal << "}";
					}
				}
				else
				{
					ostringstream	ost;
					{
						MESSAGE_DEBUG("", action, "id(" + id + ") or value(" + value + ") is empty");
					}

					ostFinal.str("");
					ostFinal << "{";
					ostFinal << "\"result\" : \"error\",";
					ostFinal << "\"description\" : \"Адрес не должен быть пустым\"";
					ostFinal << "}";
				}
			}


			indexPage.RegisterVariableForce("result", ostFinal.str());

			if(!indexPage.SetTemplate("json_response.htmlt"))
			{
				MESSAGE_DEBUG("", action, "template file json_response.htmlt was missing");
				throw CException("Template file was missing");
			}

			{
				MESSAGE_DEBUG("", action, "end");
			}

		}


		if((action == "AJAX_getEventProfile") || (action == "AJAX_getEventProfileAndUser"))
		{
			ostringstream   ostResult;

			{
				MESSAGE_DEBUG("", action, "start");
			}

			ostResult.str("");
			if(user.GetLogin() == "Guest")
			{
				{
					MESSAGE_DEBUG("", action, "re-login required");
				}

				ostResult << "{\"result\":\"error\",\"description\":\"сессия закончилась, необходимо вновь зайти на сайт\"}";
			}
			else
			{
				string		eventID, eventLink;

				eventID = CheckHTTPParam_Number(indexPage.GetVarsHandler()->Get("id"));
				eventLink = CheckHTTPParam_Number(indexPage.GetVarsHandler()->Get("link"));

				if(eventID.length() || eventLink.length())
				{
					ostringstream		ost;

					ost.str("");
					if(eventID.length())
						ost << "SELECT * FROM `events` WHERE `id`=\"" << eventID << "\" AND (`isBlocked`='N' OR `id` IN (SELECT `event_id` FROM `event_hosts` WHERE `user_id`=\"" << user.GetID() << "\"));";
					else if(eventLink.length())
						ost << "SELECT * FROM `events` WHERE `link`=\"" << eventLink << "\" AND (`isBlocked`='N' OR `id` IN (SELECT `event_id` FROM `event_hosts` WHERE `user_id`=\"" << user.GetID() << "\"));";

					ostResult	<< "{"
								<< "\"result\":\"success\","
								<< "\"events\":[" << GetEventListInJSONFormat(ost.str(), &db, &user) << "]"
								<< (action == "AJAX_getEventProfileAndUser" ? ",\"users\":[" + GetUserListInJSONFormat("SELECT * FROM `users` WHERE `id`=\"" + user.GetID() + "\" AND `isblocked`=\"N\";", &db, &user) + "]" : "")
								<< "}";
				}
				else
				{
					{
						MESSAGE_DEBUG("", action, "in eventID [" + eventID + "]");
					}

					ostResult << "{\"result\":\"error\",\"description\":\"ERROR in eventID\",\"events\":[]}";
				}
			}



			indexPage.RegisterVariableForce("result", ostResult.str());

			if(!indexPage.SetTemplate("json_response.htmlt"))
			{
				{
					MESSAGE_DEBUG("", action, "template file json_response.htmlt was missing");
				}
				throw CException("Template file json_response.htmlt was missing");
			}  // if(!indexPage.SetTemplate("AJAX_precreateNewOpenVacancy.htmlt"))

			{
				MESSAGE_DEBUG("", action, "start");
			}
		}

		if(action == "AJAX_getFriendsNotOnEvent")
		{

			ostringstream   ostResult;
			{
				MESSAGE_DEBUG("", action, "start");
			}

			ostResult.str("");
			if(user.GetLogin() == "Guest")
			{
				ostringstream   ost;

				{
					MESSAGE_DEBUG("", action, "re-login required");
				}

				ostResult << "{\"result\": \"error\", \"description\": \"session lost. Need to relogin\"}";
			}
			else
			{
				string		  event_id = CheckHTTPParam_Number(indexPage.GetVarsHandler()->Get("event_id"));

				if(event_id.length())
				{
					if(db.Query("SELECT * FROM `event_hosts` WHERE `event_id`=\"" + event_id + "\" and `user_id`=\"" + user.GetID() + "\";"))
					{
						ostResult	<< "{"
									<< "\"result\":\"success\","
									<< "\"users\":[" + GetUserListInJSONFormat(	"SELECT * FROM `users` WHERE "
																				"`id` IN ("
																				" SELECT `friendID` FROM `users_friends` WHERE `userID`=\"" + user.GetID() + "\" AND `state`=\"confirmed\"" 
																				"	AND "
																				"	(`friendID` NOT IN (SELECT `user_id` FROM `event_guests` WHERE `event_id`=\"" + event_id + "\"))"
																				"	AND "
																				"	(`friendID` NOT IN (SELECT `user_id` FROM `event_hosts` WHERE `event_id`=\"" + event_id + "\"))"
																				");", &db, &user) + "]"
									<< "}";
					}
					else
					{
						{
							MESSAGE_ERROR("", action, "you are not event host");
						}

						ostResult << "{\"result\": \"error\", \"description\": \"Вы не можете редактировать данные мероприятия\"}";
					}
				}
				else
				{
					{
						MESSAGE_ERROR("", action, "mandatory parameter missed or empty in HTML request");
					}

					ostResult << "{\"result\": \"error\", \"description\": \"некорректные параметры\"}";
				}

			}

			indexPage.RegisterVariableForce("result", ostResult.str());
			if(!indexPage.SetTemplate("json_response.htmlt"))
			{
				MESSAGE_ERROR("", action, "can't find template json_response.htmlt");
				throw CExceptionHTML("user not activated");
			}

			{
				MESSAGE_DEBUG("", action, "finish");
			}
		}

		if(action == "AJAX_addEventHost")
		{

				ostringstream   ostResult;
				{
					MESSAGE_DEBUG("", action, "start");
				}

				ostResult.str("");
				if(user.GetLogin() == "Guest")
				{
					ostringstream   ost;

					{
						MESSAGE_DEBUG("", action, "re-login required");
					}

					ostResult << "{\"result\": \"error\", \"description\": \"session lost. Need to relogin\"}";
				}
				else
				{
					string		  user_id = CheckHTTPParam_Number(indexPage.GetVarsHandler()->Get("user_id"));
					string		  event_id = CheckHTTPParam_Number(indexPage.GetVarsHandler()->Get("event_id"));

					if(event_id.length() && user_id.length())
					{
						if(db.Query("SELECT * FROM `event_hosts` WHERE `event_id`=\"" + event_id + "\" and `user_id`=\"" + user.GetID() + "\";"))
						{
							if(db.Query("SELECT * FROM `event_hosts` WHERE `event_id`=\"" + event_id + "\" and `user_id`=\"" + user_id + "\";"))
							{
								{
									MESSAGE_DEBUG("", action, "event host already exists");
								}

								ostResult << "{\"result\": \"error\", \"description\": \"Уже есть в списке\"}";
							}
							else
							{
								unsigned long		newEventHostID = db.InsertQuery("INSERT INTO `event_hosts` (`event_id`, `user_id`, `eventTimestamp`) VALUES (\"" + event_id + "\", \"" + user_id + "\", UNIX_TIMESTAMP());");

								if(newEventHostID)
								{
									// --- Update live feed
									if(db.InsertQuery("INSERT INTO `users_notification` (`title`, `userId`, `actionTypeId`, `actionId`, `eventTimestamp`) values(\"\",\"" + user_id + "\", \"67\", \"" + to_string(newEventHostID) + "\", UNIX_TIMESTAMP())"))
									{

									}
									else
									{
										{
											MESSAGE_ERROR("", action, "inserting into `feed`");
										}
									}

									ostResult << "{\"result\": \"success\", \"hosts\": [" << GetEventHostsListInJSONFormat("SELECT * FROM `event_hosts` WHERE `event_id`=\"" + event_id + "\";", &db, &user) << "]}";

								}
								else
								{
									{
										MESSAGE_ERROR("", action, "error insertion into event_host");
									}

									ostResult << "{\"result\": \"error\", \"description\": \"ERROR: insertion into event_host\"}";
								}
							}
						}
						else
						{
							{
								MESSAGE_ERROR("", action, "you(" + user.GetID() + ") are not event(" + event_id + ") host");
							}

							ostResult << "{\"result\": \"error\", \"description\": \"Вы не можете редактировать данные мероприятия\"}";
						}
					}
					else
					{
						{
							MESSAGE_ERROR("", action, "mandatory parameter has missed or empty in HTML request");
						}

						ostResult << "{\"result\": \"error\", \"description\": \"некорректные параметры\"}";
					}

				}

				indexPage.RegisterVariableForce("result", ostResult.str());
				if(!indexPage.SetTemplate("json_response.htmlt"))
				{
					MESSAGE_ERROR("", action, "can't find template json_response.htmlt");
					throw CExceptionHTML("user not activated");
				}

				{
					MESSAGE_DEBUG("", action, "finish");
				}
		}

		if(action == "AJAX_addEventGuest")
		{

			ostringstream   ostResult;

			{
				MESSAGE_DEBUG("", action, "start");
			}

			ostResult.str("");
			if(user.GetLogin() == "Guest")
			{
				ostringstream   ost;

				{
					MESSAGE_DEBUG("", action, "re-login required");
				}

				ostResult << "{\"result\": \"error\", \"description\": \"session lost. Need to relogin\"}";
			}
			else
			{
				string		  users_list = CheckHTTPParam_Text(indexPage.GetVarsHandler()->Get("user_id"));
				string		  event_id = CheckHTTPParam_Number(indexPage.GetVarsHandler()->Get("event_id"));
				string		  email = CheckHTTPParam_Email(indexPage.GetVarsHandler()->Get("user_email"));

				if((users_list.length() || email.length()) && event_id.length())
				{
					if(db.Query("SELECT * FROM `event_hosts` WHERE `event_id`=\"" + event_id + "\" and `user_id`=\"" + user.GetID() + "\";"))
					{
						if(users_list.length())
						{
							vector<string>	tempVector;

							qw(users_list, tempVector);
							if(tempVector.size())
							{
								for(auto& user_id: tempVector)
								{
									// --- guest invited by user_id
									if(db.Query("SELECT * FROM `event_hosts` WHERE `event_id`=\"" + event_id + "\" and `user_id`=\"" + user_id + "\";"))
									{
										{
											MESSAGE_DEBUG("", action, "event host can't be guest");
										}

										ostResult << "{\"result\": \"error\", \"description\": \"Устроитель мероприятия уже в списке\"}";
										break;
									}
									else if(db.Query("SELECT * FROM `event_guests` WHERE `event_id`=\"" + event_id + "\" and `user_id`=\"" + user_id + "\";"))
									{
										{
											MESSAGE_DEBUG("", action, "event guest already exists");
										}

										ostResult << "{\"result\": \"error\", \"description\": \"Уже есть в списке\"}";
										break;
									}
									else
									{
										unsigned long		newEventGuestID = db.InsertQuery("INSERT INTO `event_guests` (`event_id`, `user_id`, `status`, `eventTimestamp`) VALUES (\"" + event_id + "\", \"" + user_id + "\", \"invited\", UNIX_TIMESTAMP());");

										if(newEventGuestID)
										{
											// --- Update live feed
											if(db.InsertQuery("INSERT INTO `users_notification` (`title`, `userId`, `actionTypeId`, `actionId`, `eventTimestamp`) values(\"\",\"" + user_id + "\", \"68\", \"" + to_string(newEventGuestID) + "\", UNIX_TIMESTAMP())"))
											{

											}
											else
											{
												{
													MESSAGE_ERROR("", action, "inserting into `users_notification`");
												}
											}
										}
										else
										{
											{
												MESSAGE_ERROR("", action, "error insertion into event_guest");
											}

											ostResult << "{\"result\": \"error\", \"description\": \"ERROR: insertion into event_guest\"}";
											break;
										}
									}
								}
								if(ostResult.str().length())
								{
									// --- error message constructed in previous loop
								}
								else
								{
									ostResult << "{\"result\": \"success\", \"guests\": [" << GetEventGuestsListInJSONFormat("SELECT * FROM `event_guests` WHERE `event_id`=\"" + event_id + "\";", &db, &user) << "]}";
								}
							}
							else
							{
								{
									MESSAGE_DEBUG("", action, "usersList is empty");
								}
								ostResult << "{\"result\": \"error\", \"description\": \"Список пользователей пустой\"}";
							}

						}
						else
						{
							// --- guest invited by email
							// --- quick registration

							if(db.Query("SELECT `id` FROM `event_guests` WHERE `event_id`=\"" + event_id + "\" AND (`quick_registration_id` IN (SELECT `id` FROM `quick_registration` WHERE `email`=\"" + email + "\"))"))
							{
								{
									MESSAGE_DEBUG("", action, "email(" + email + ") already invited to event(" + event_id + ")");
								}
								
								ostResult << "{\"result\": \"error\", \"description\": \"уже в списке\"}";
							}
							else
							{
								string		invite_hash = GetRandom(10);
								long int	quick_registration_id = 0;

								while(db.Query("SELECT `id` FROM `quick_registration` WHERE `invite_hash`=\"" + invite_hash + "\";"))
								{
									invite_hash = GetRandom(10);
								}

								quick_registration_id = db.InsertQuery("INSERT INTO `quick_registration` (`email`, `invite_hash`, `action`, `action_id`, `eventTimestamp`) values(\"" + email + "\",\"" + invite_hash + "\",\"event\", \"" + event_id + "\", UNIX_TIMESTAMP())");
								if(quick_registration_id)
								{
									if(db.Query("SELECT * FROM `events` WHERE `id`=\"" + event_id + "\";"))
									{
										string		startTimestamp = db.Get(0, "startTimestamp");
										string		logo_folder = db.Get(0, "logo_folder");
										string		logo_filename = db.Get(0, "logo_filename");

										indexPage.RegisterVariableForce("invite_title", db.Get(0, "title"));
										indexPage.RegisterVariableForce("invite_description", db.Get(0, "description"));
										indexPage.RegisterVariableForce("invite_address", db.Get(0, "address"));
										indexPage.RegisterVariableForce("invite_hash", invite_hash);
										indexPage.RegisterVariableForce("invite_startTimestamp", startTimestamp);
										indexPage.RegisterVariableForce("invite_logo", (logo_filename.length() ? "<img src=\"https://" + indexPage.GetVarsHandler()->Get("SERVER_NAME") + "/images/events/" + logo_folder + "/" + logo_filename + "\" style=\"max-width:150px;max-height:150px;\">" : ""));
										
										if(db.InsertQuery("INSERT INTO `event_guests` (`event_id`, `quick_registration_id`, `status`, `eventTimestamp`) VALUES (\"" + event_id + "\", \"" + to_string(quick_registration_id) + "\", \"invited\", UNIX_TIMESTAMP());"))
										{
											// Send mail invite
											CMailLocal	mail;

											indexPage.RegisterVariableForce("invite_name", user.GetName() + " " + user.GetNameLast());

											mail.Send("", "quick_registration_invite", indexPage.GetVarsHandler(), &db, email);

											ostResult << "{\"result\": \"success\", \"guests\": [" << GetEventGuestsListInJSONFormat("SELECT * FROM `event_guests` WHERE `event_id`=\"" + event_id + "\";", &db, &user) << "]}";
										}
										else
										{
											MESSAGE_ERROR("", action, "inserting into `quick_registration`");
											
											ostResult << "{\"result\": \"error\", \"description\": \"ошибка БД\"}";
										}
									}
									else
									{
										MESSAGE_ERROR("", action, "event.id(" + event_id + ") doesn't exists");
										
										ostResult << "{\"result\": \"error\", \"description\": \"событие не существует\"}";
									}
								}
								else
								{
									{
										MESSAGE_ERROR("", action, "inserting into `quick_registration`");
									}
									
									ostResult << "{\"result\": \"error\", \"description\": \"ошибка БД\"}";
								}
							}

						}
					}
					else
					{
						{
							MESSAGE_ERROR("", action, "you(" + user.GetID() + ") are not event(" + event_id + ") host");
						}

						ostResult << "{\"result\": \"error\", \"description\": \"Вы не можете редактировать данные мероприятия\"}";
					}
				}
				else
				{
					{
						MESSAGE_ERROR("", action, "mandatory parameter missed or empty in HTML request");
					}

					ostResult << "{\"result\": \"error\", \"description\": \"некорректные параметры\"}";
				}

			}

			indexPage.RegisterVariableForce("result", ostResult.str());
			if(!indexPage.SetTemplate("json_response.htmlt"))
			{
				MESSAGE_ERROR("", action, "can't find template json_response.htmlt");
				throw CExceptionHTML("user not activated");
			}

			{
				MESSAGE_DEBUG("", action, "finish");
			}
		}

		if((action == "AJAX_setNoGift_N") || (action == "AJAX_setNoGift_Y"))
		{
			ostringstream	ostFinal;

			MESSAGE_DEBUG("", action, "start");

			if(user.GetLogin() == "Guest")
			{
				ostringstream	ost;

				{
					MESSAGE_DEBUG("", action, "re-login required");
				}

				ost.str("");
				ostFinal.str("");
				ostFinal << "{";
				ostFinal << "\"result\" : \"error\",";
				ostFinal << "\"description\" : \"re-login required\"";
				ostFinal << "}";
			}
			else
			{
				string	id = CheckHTTPParam_Number(indexPage.GetVarsHandler()->Get("event_id"));

				if(id.length())
				{
					if(db.Query("SELECT `id` FROM `event_hosts` WHERE `event_id`=\"" + id + "\" AND `user_id`=\"" + user.GetID() + "\";"))
					{
						db.Query("UPDATE `events` SET `hideGifts`=\""s + (action == "AJAX_setNoGift_Y" ? "Y" : "N") + "\" WHERE `id`=\"" + id + "\";");
						if(db.isError())
						{
							MESSAGE_ERROR("", action, "fail to update events table");

							ostFinal.str("");
							ostFinal << "{";
							ostFinal << "\"result\" : \"error\",";
							ostFinal << "\"description\" : \"Ошибка БД\"";
							ostFinal << "}";
						}
						else
						{
							ostFinal.str("");
							ostFinal << "{";
							ostFinal << "\"result\" : \"success\"";
							ostFinal << "}";
						}

					}
					else
					{
						{
							MESSAGE_DEBUG("", action, "user.id(" + user.GetID() + ") is not a event(" + id + ") owner");
						}

						ostFinal.str("");
						ostFinal << "{";
						ostFinal << "\"result\" : \"error\",";
						ostFinal << "\"description\" : \"вы не можете изменить данные события\"";
						ostFinal << "}";
					}
				}
				else
				{
					ostringstream	ost;
					{
						MESSAGE_DEBUG("", action, "id(" + id + ") is empty");
					}

					ostFinal.str("");
					ostFinal << "{";
					ostFinal << "\"result\" : \"error\",";
					ostFinal << "\"description\" : \"необходимо выбрать событие\"";
					ostFinal << "}";
				}

			}


			indexPage.RegisterVariableForce("result", ostFinal.str());

			if(!indexPage.SetTemplate("json_response.htmlt"))
			{
				MESSAGE_DEBUG("", action, "template file json_response.htmlt was missing");
				throw CException("Template file was missing");
			}

			{
				MESSAGE_DEBUG("", action, "end");
			}	
		}

		if((action == "AJAX_guestAccept") || (action == "AJAX_guestReject") || (action == "AJAX_updateAdultsCounter") || (action == "AJAX_updateKidsCounter"))
		{
			string			kidsCounter, adultsCounter, id;
			ostringstream	ostFinal;

			{
				MESSAGE_DEBUG("", action, "start");
			}

			if(user.GetLogin() == "Guest")
			{
				ostringstream	ost;

				{
					MESSAGE_DEBUG("", action, "re-login required");
				}

				ost.str("");
				ostFinal.str("");
				ostFinal << "{";
				ostFinal << "\"result\" : \"error\",";
				ostFinal << "\"description\" : \"re-login required\"";
				ostFinal << "}";
			}
			else
			{

				kidsCounter = CheckHTTPParam_Number(indexPage.GetVarsHandler()->Get("kidsCounter"));
				adultsCounter = CheckHTTPParam_Number(indexPage.GetVarsHandler()->Get("adultsCounter"));
				id = CheckHTTPParam_Number(indexPage.GetVarsHandler()->Get("event_id"));
				ostFinal.str("");

				if(id.length())
				{
					if(db.Query("SELECT `id`,`status` FROM `event_guests` WHERE `event_id`=\"" + id + "\" AND `user_id`=\"" + user.GetID() + "\";"))
					{
						string	event_guests_id = db.Get(0, "id");
						string	event_guests_status = db.Get(0, "status");

						
						// --- 1) notify event_hosts about accept
						// --- 2) establish friendship relations
						if((action == "AJAX_guestAccept") || (action == "AJAX_updateAdultsCounter") || (action == "AJAX_updateKidsCounter"))
						{
							if(event_guests_status == "accepted")
							{
								{
									MESSAGE_DEBUG("", action, "invite status 'accepted', not required to send notifications again");
								}
							}
							else
							{
								int		affected = 0;
								string	sqlQuery1Values = "";
								string	sqlQuery2Values = "";
								
								affected = db.Query("SELECT `user_id` FROM `event_hosts` WHERE `event_id`=\"" + id + "\";");
								if(affected)
								{
									vector<string> usersVec;

									// --- 1) notify hosts about accept
									for(int i = 0; i < affected; i++)
									{
										if(sqlQuery1Values.length()) sqlQuery1Values += ",";
										sqlQuery1Values += "(\"" + string(db.Get(i, "user_id")) + "\", \"69\", \"" + event_guests_id + "\", UNIX_TIMESTAMP())";

										usersVec.push_back(db.Get(i, "user_id"));
									}

									if(db.InsertQuery("INSERT INTO `users_notification` (`userId`, `actionTypeId`, `actionId`, `eventTimestamp`) VALUES " + sqlQuery1Values))
									{
									}
									else
									{
										MESSAGE_ERROR("", action, "inserting into `users_notification`");
									}

									// --- 2) establish friendship relations
									for(auto userItem: usersVec)
									{
										if(db.Query("SELECT `id` FROM `users_friends` WHERE `userID`=\"" + user.GetID() + "\" AND `friendID`=\"" + userItem + "\";"))
										{
											// --- friendship already setup
										}
										else
										{
											if(db.InsertQuery("INSERT INTO `users_friends` SET `userID`=\"" + user.GetID() + "\", `friendID`=\"" + userItem + "\", `state`=\"confirmed\";"))
											{
											}
											else
											{
												MESSAGE_ERROR("", action, "inserting into `users_friends`");
											}
											if(db.InsertQuery("INSERT INTO `users_friends` SET `userID`=\"" + userItem + "\", `friendID`=\"" + user.GetID() + "\", `state`=\"confirmed\";"))
											{
											}
											else
											{
												MESSAGE_ERROR("", action, "inserting into `users_friends`");
											}
											
										}
									}

								}
								else
								{
									{
										MESSAGE_DEBUG("", action, "no hosts for event_id(" + id + ")");
									}
								}
							}
						}


						if((action == "AJAX_guestAccept"))
						{
							db.Query("UPDATE `event_guests` SET `status`=\"accepted\"  WHERE `id`=\"" + event_guests_id + "\";");

							if(!db.isError())
							{
								ostFinal.str("");
								ostFinal << "{";
								ostFinal << "\"result\" : \"success\",";
								ostFinal << "\"guests\" : [" << GetEventGuestsListInJSONFormat("SELECT * FROM `event_guests` WHERE `event_id`=\"" + id + "\";", &db, &user) << "],";
								ostFinal << "\"description\" : \"\"";
								ostFinal << "}";
							}
							else
							{
								{
									MESSAGE_ERROR("", action, "updating DB");
								}

								ostFinal.str("");
								ostFinal << "{";
								ostFinal << "\"result\" : \"error\",";
								ostFinal << "\"description\" : \"ошибка БД\"";
								ostFinal << "}";
							}
						}
						else if(action == "AJAX_guestReject")
						{
							if(event_guests_status == "accepted")
							{
								// --- 1) clear event_hosts notifications
								db.Query("DELETE FROM `users_notification` WHERE `actionTypeId`=\"69\" AND `actionId`=\"" + event_guests_id + "\";");
								if(db.isError())
								{
									{
										MESSAGE_ERROR("", action, "removing from table users_notifications");
									}
								}
							}
							else
							{
								{
									MESSAGE_DEBUG("", action, "invite status 'rejected' or 'invited', not required to clean notifications");
								}
							}


							db.Query("UPDATE `event_guests` SET `status`=\"rejected\",`adults`=\"0\",`kids`=\"0\"  WHERE `id`=\"" + event_guests_id + "\";");

							if(!db.isError())
							{
								ostFinal.str("");
								ostFinal << "{";
								ostFinal << "\"result\" : \"success\",";
								ostFinal << "\"guests\" : [" << GetEventGuestsListInJSONFormat("SELECT * FROM `event_guests` WHERE `event_id`=\"" + id + "\";", &db, &user) << "],";
								ostFinal << "\"description\" : \"\"";
								ostFinal << "}";
							}
							else
							{
								{
									MESSAGE_DEBUG("", action, "updating DB");
								}

								ostFinal.str("");
								ostFinal << "{";
								ostFinal << "\"result\" : \"error\",";
								ostFinal << "\"description\" : \"ошибка БД\"";
								ostFinal << "}";
							}
						}
						else if((action == "AJAX_updateAdultsCounter"))
						{
							if(adultsCounter.length())
							{

								db.Query("UPDATE `event_guests` SET `status`=\"accepted\", `adults`=\"" + adultsCounter + "\"  WHERE `id`=\"" + event_guests_id + "\";");

								if(!db.isError())
								{
									ostFinal.str("");
									ostFinal << "{";
									ostFinal << "\"result\" : \"success\",";
									ostFinal << "\"guests\" : [" << GetEventGuestsListInJSONFormat("SELECT * FROM `event_guests` WHERE `event_id`=\"" + id + "\";", &db, &user) << "],";
									ostFinal << "\"description\" : \"\"";
									ostFinal << "}";
								}
								else
								{
									{
										MESSAGE_DEBUG("", action, "updating DB");
									}

									ostFinal.str("");
									ostFinal << "{";
									ostFinal << "\"result\" : \"error\",";
									ostFinal << "\"description\" : \"ошибка БД\"";
									ostFinal << "}";
								}
							}
							else
							{
								{
									MESSAGE_DEBUG("", action, "adultsCounter is empty");
								}

								ostFinal.str("");
								ostFinal << "{";
								ostFinal << "\"result\" : \"error\",";
								ostFinal << "\"description\" : \"необходимо указать количество взрослых\"";
								ostFinal << "}";
							}
						}
						else if((action == "AJAX_updateKidsCounter"))
						{
							if(kidsCounter.length())
							{

								db.Query("UPDATE `event_guests` SET `status`=\"accepted\", `kids`=\"" + kidsCounter + "\"  WHERE `id`=\"" + event_guests_id + "\";");

								if(!db.isError())
								{
									ostFinal.str("");
									ostFinal << "{";
									ostFinal << "\"result\" : \"success\",";
									ostFinal << "\"guests\" : [" << GetEventGuestsListInJSONFormat("SELECT * FROM `event_guests` WHERE `event_id`=\"" + id + "\";", &db, &user) << "],";
									ostFinal << "\"description\" : \"\"";
									ostFinal << "}";
								}
								else
								{
									{
										MESSAGE_DEBUG("", action, "updating DB");
									}

									ostFinal.str("");
									ostFinal << "{";
									ostFinal << "\"result\" : \"error\",";
									ostFinal << "\"description\" : \"ошибка БД\"";
									ostFinal << "}";
								}
							}
							else
							{
								{
									MESSAGE_DEBUG("", action, "kidsCounter is empty");
								}

								ostFinal.str("");
								ostFinal << "{";
								ostFinal << "\"result\" : \"error\",";
								ostFinal << "\"description\" : \"необходимо указать количество детей\"";
								ostFinal << "}";
							}
						}
						else
						{
								{
									MESSAGE_DEBUG("", action, "unknown action");
								}

								ostFinal.str("");
								ostFinal << "{";
								ostFinal << "\"result\" : \"error\",";
								ostFinal << "\"description\" : \"запрос неизвестного типа\"";
								ostFinal << "}";
						}
					}
					else
					{
						{
							MESSAGE_DEBUG("", action, "user.id(" + user.GetID() + ") is not a event(" + id + ") owner");
						}

						ostFinal.str("");
						ostFinal << "{";
						ostFinal << "\"result\" : \"error\",";
						ostFinal << "\"description\" : \"вы не можете изменить данные события\"";
						ostFinal << "}";
					}
				}
				else
				{
					ostringstream	ost;
					{
						MESSAGE_DEBUG("", action, "id(" + id + ") or kidsCounter(" + kidsCounter + ") is empty");
					}

					ostFinal.str("");
					ostFinal << "{";
					ostFinal << "\"result\" : \"error\",";
					ostFinal << "\"description\" : \"необходимо выбрать событие\"";
					ostFinal << "}";
				}
			}


			indexPage.RegisterVariableForce("result", ostFinal.str());

			if(!indexPage.SetTemplate("json_response.htmlt"))
			{
				MESSAGE_DEBUG("", action, "template file json_response.htmlt was missing");
				throw CException("Template file was missing");
			}

			{
				MESSAGE_DEBUG("", action, "end");
			}
		}

		if(action == "getEventWall")
		{
			ostringstream	ost;
			string			id = "", link = "";

			{
				MESSAGE_DEBUG("", action, "start");
			}

	/*
			if(user.GetLogin() == "Guest")
			{
				ostringstream	ost;

				{
					MESSAGE_DEBUG("", action, "re-login required");
				}

				ost.str("");
				ost << "/?rand=" << GetRandom(10);
				indexPage.Redirect(ost.str());
			}
	*/
			id					= CheckHTTPParam_Number(indexPage.GetVarsHandler()->Get("id"));
			link				= CheckHTTPParam_Text(indexPage.GetVarsHandler()->Get("link"));

			if(link.length())
			{
				if(db.Query("SELECT `id` FROM `events` WHERE `link`=\"" + link + "\";"))
				{
					id = db.Get(0, "id");
				}
				else
				{
					{
						MESSAGE_DEBUG("", action, "event.link(" + link + ") not found");
					}
				}
			}
			else if(id.length())
			{
				if(db.Query("SELECT `link` FROM `events` WHERE `id`=\"" + id + "\";"))
				{
					link = db.Get(0, "link");
				}
				else
				{
					{
						MESSAGE_DEBUG("", action, "event.id(" + id + ") not found");
					}
				}
			}
			else
			{
				{
					MESSAGE_DEBUG("", action, "id and link are empty");
				}

			}

			if(id.length()) indexPage.RegisterVariableForce("id", id);
			if(link.length()) indexPage.RegisterVariableForce("link", link);

			if(!indexPage.SetTemplate("view_event_profile.htmlt"))
			{
				MESSAGE_DEBUG("", action, "template file getEventWall.htmlt was missing");
				throw CException("Template file was missing");
			}
		}

		if(action == "AJAX_getEventWall")
		{
			ostringstream	ost;
			string			strPageToGet, strNewsOnSinglePage;
			int				currPage = 0, newsOnSinglePage = 0;
			vector<int>		vectorFriendList;
			string			eventLink = "", eventID = "", result = "";

			strNewsOnSinglePage = CheckHTTPParam_Number(indexPage.GetVarsHandler()->Get("NewsOnSinglePage"));
			strPageToGet		= CheckHTTPParam_Number(indexPage.GetVarsHandler()->Get("page"));
			eventLink			= CheckHTTPParam_Text(indexPage.GetVarsHandler()->Get("link"));
			eventID				= CheckHTTPParam_Number(indexPage.GetVarsHandler()->Get("id"));

			if(strPageToGet.empty()) strPageToGet = "0";

			{
				MESSAGE_DEBUG("", action, "page " + strPageToGet + " requested");
			}

			try{
				currPage = stoi(strPageToGet);
			} catch(...) {
				currPage = 0;
			}

			try{
				newsOnSinglePage = stoi(strNewsOnSinglePage);
			} catch(...) {
				newsOnSinglePage = 30;
			}

	/*
			if(user.GetLogin() == "Guest")
			{
				ostringstream   ost;

				{
					MESSAGE_DEBUG("", action, "re-login required");
				}

				ost.str("");
				ost << "/?rand=" << GetRandom(10);
				indexPage.Redirect(ost.str());
			}
	*/
			
			// --- define eventID by eventLink
			if(eventLink.length() && db.Query("SELECT `id`, `isBlocked` FROM `events` WHERE `link`=\"" + eventLink + "\";")) eventID = db.Get(0, "id");

			if(eventID.length())
			{
				string	extraFilter = " AND `isBlocked`=\"N\"";

				if(db.Query("SELECT `id` FROM `event_hosts` WHERE `user_id`=\"" + user.GetID() + "\" AND `event_id`=\"" + eventID + "\";"))
				{
					extraFilter = "";
				}
				else
				{
					{
						MESSAGE_DEBUG("", action, "user(" + user.GetID() + ") is not event(" + eventID + ") host ");
					}
					
				}

				result = GetNewsFeedInJSONFormat(" `feed`.`dstType`=\"event\" AND `feed`.`dstID` IN (SELECT `id` FROM `events` WHERE `id`=\"" + eventID + "\" " + extraFilter + ") ", currPage, newsOnSinglePage, &user, &db);
			}
			else
			{
				MESSAGE_ERROR("", action, "eventID not found");
			}

			indexPage.RegisterVariableForce("result", "{"
														"\"status\":\"success\","
														"\"feed\":[" + result + "]"
														"}");

			if(!indexPage.SetTemplate("json_response.htmlt"))
			{
				MESSAGE_ERROR("", action, "can't find template json_response.htmlt");
				throw CExceptionHTML("event not activated");
			} // if(!indexPage.SetTemplate("json_response.htmlt"))
		}


		if(action == "createnewevent")
		{
			ostringstream	ost;

			{
				MESSAGE_DEBUG("", action, "start");
			}

			if(user.GetLogin() == "Guest")
			{
				ostringstream	ost;

				{
					MESSAGE_DEBUG("", action, "re-login required");
				}

				ost.str("");
				ost << "/?rand=" << GetRandom(10);
				indexPage.Redirect(ost.str());
			}

			indexPage.RegisterVariableForce("title", "Новое событие");

			if(!indexPage.SetTemplate("create_event.htmlt"))
			{
				MESSAGE_DEBUG("", action, "template file createnewevent.htmlt was missing");
				throw CException("Template file createnewevent.htmlt was missing");
			}  // if(!indexPage.SetTemplate("createnewevent.htmlt"))

			{
				MESSAGE_DEBUG("", action, "end");
			}
		} 	// if(action == "createnewevent")

		if(action == "check_initial_action")
		{
			string		invite_hash;

			{
				MESSAGE_DEBUG("", action, "start");
			}

			invite_hash = CheckHTTPParam_Number(indexPage.GetVarsHandler()->Get("id"));

			if(invite_hash.length())
			{

			if(user.GetLogin() == "Guest")
			{
				ostringstream	ost;

				{
					MESSAGE_DEBUG("", action, "guest workflow");
				}

				indexPage.Cookie_InitialAction_Assign(invite_hash);
				indexPage.RegisterVariableForce("redirect_url", "/login?rand=" + GetRandom(10));
			}
			else
			{
				string		eventLink = "";

				{
					MESSAGE_DEBUG("", action, "userID(" + user.GetID() + ") workflow");
				}

				// --- remove "inviteHash" cookie
				if(!indexPage.Cookie_InitialAction_Expire()) 
				{
					MESSAGE_DEBUG("", action, "issue in session expiration");
				}

				// --- update db
				indexPage.RegisterVariableForce("redirect_url", "/" + GetDefaultActionLoggedinUser() + "?rand=" + GetRandom(10));

				if(db.Query("SELECT * FROM `quick_registration` WHERE `invite_hash`=\"" + invite_hash + "\";"))
				{
					string		quick_registration_id = db.Get(0, "id");
					string		quick_registration_action = db.Get(0, "action");
					string		quick_registration_action_id = db.Get(0, "action_id");

					if(quick_registration_action == "event")
					{
						if(db.Query("SELECT `event_id` FROM `event_guests` WHERE `quick_registration_id`=\"" + quick_registration_id + "\";"))
						{
							string		event_id = db.Get(0, "event_id");

							if(db.Query("SELECT `link` FROM `events` WHERE `id`=\"" + event_id + "\";"))
							{
								string		event_link = db.Get(0, "link");

								db.Query("UPDATE `event_guests` SET `user_id`=\"" + user.GetID() + "\" WHERE `quick_registration_id`=\"" + quick_registration_id + "\";");
								if(db.isError())
								{
									{
										MESSAGE_DEBUG("", action, "issue updating event_guest table");
									}
								}
								else
								{
									indexPage.RegisterVariableForce("redirect_url", "/event/" + event_link + "?rand=" + GetRandom(10));
								}
							}
							else
							{
								{
									MESSAGE_DEBUG("", action, "event.id(" + event_id + ") not found");
								}
							}


							// --- redirect to /event/yyy
						}
						else
						{
							{
								MESSAGE_DEBUG("", action, "quick_registration_id(" + quick_registration_id + ") not found");
							}
						}
					}
					else
					{
						{
							MESSAGE_DEBUG("", action, "unknown action(" + quick_registration_action + ")");
						}
					}
				}
				else
				{
					{
						MESSAGE_DEBUG("", action, "invite_hash(" + invite_hash + ") not found");
					}
				}

			}


			}
			else
			{
				{
					MESSAGE_DEBUG("", action, "invite_hash[" + indexPage.GetVarsHandler()->Get("id") + "] is not a number");
				}

			}

			if(!indexPage.SetTemplate("check_invite.htmlt"))
			{
				MESSAGE_DEBUG("", action, "template file check_invite.htmlt was missing");
				throw CException("Template file check_invite.htmlt was missing");
			}  // if(!indexPage.SetTemplate("check_invite.htmlt"))

			{
				MESSAGE_DEBUG("", action, "end");
			}
		}

		{
			MESSAGE_DEBUG("", "", "post-condition if(action == \"" + action + "\")");
		}

		indexPage.OutTemplate();

	}
/*
	catch(CExceptionRedirect &c) {
		{
			MESSAGE_DEBUG("", "", "catch CRedirectHTML: exception used for redirection");
		}

		c.SetDB(&db);

		if(!indexPage.SetTemplate(c.GetTemplate())) 
		{
			MESSAGE_ERROR("", "", "catch CRedirectHTML: ERROR, template redirect.htmlt not found");
			throw CException("Template file was missing");
		}

		indexPage.RegisterVariableForce("content", "redirect page");
		indexPage.OutTemplate();

	}
*/
	catch(CExceptionHTML &c)
	{

		c.SetLanguage(indexPage.GetLanguage());
		c.SetDB(&db);
		MESSAGE_DEBUG("", action, "catch CExceptionHTML: DEBUG exception reason: [" + c.GetReason() + "]");

		if(!indexPage.SetTemplate(c.GetTemplate()))
		{
			return(-1);
		}
		indexPage.RegisterVariable("content", c.GetReason());
		indexPage.OutTemplate();
		return(0);
	}
	catch(CException &c)
	{
		if(!indexPage.SetTemplateFile("templates/error.htmlt"))
		{
			return(-1);
		}
		{
			MESSAGE_ERROR("", action, "catch CException: exception: ERROR  " + c.GetReason());
		}

		indexPage.RegisterVariable("content", c.GetReason());
		indexPage.OutTemplate();
		return(-1);
	}
	catch(exception& e)
	{
		{
			MESSAGE_ERROR("", action, "catch(exception& e): catch standard exception: ERROR  " + e.what());
		}

		if(!indexPage.SetTemplateFile("templates/error.htmlt"))
		{
			return(-1);
		}
		indexPage.RegisterVariable("content", e.what());
		indexPage.OutTemplate();
		return(-1);
	}

	return(0);
}
