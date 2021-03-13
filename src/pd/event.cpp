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
	CStatistics		appStat;  // --- CStatistics must be first statement to measure end2end param's
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

		if(!indexPage.SetTemplate("index.htmlt"))
		{
			MESSAGE_DEBUG("", action, "template file was missing");
			throw CException("Template file was missing");
		}

		if(db.Connect() < 0)
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
			action = LogoutIfGuest(action, &indexPage, &db, &user);
		}
		// ------------ end generate common parts

		MESSAGE_DEBUG("", "", "pre-condition if(action == \"" + action + "\")");


		if((action.length() > 10) && (action.compare(action.length() - 9, 9, "_template") == 0))
		{
			ostringstream	ost;
			string			strPageToGet, strFriendsOnSinglePage;

			MESSAGE_DEBUG("", action, "start");

			{
				string		template_name = action.substr(0, action.length() - 9) + ".htmlt";

				if(!indexPage.SetTemplate(template_name))
				{
					MESSAGE_ERROR("", action, "can't find template " + template_name);
				}
			}

			MESSAGE_DEBUG("", action, "finish");
		}

		if(action == "events_list")
		{
			ostringstream	ost;
			string			strPageToGet, strFriendsOnSinglePage;

			MESSAGE_DEBUG("", action, "start");

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
					MESSAGE_DEBUG("", action, "can't find template");
					throw CExceptionHTML("user not activated");
				}
			}


			MESSAGE_DEBUG("", action, "finish");
		}

		if((action == "AJAX_getFindEventsListAutocomplete") || (action == "AJAX_getFindEventsList") || (action == "AJAX_findEventByID") || (action == "AJAX_getMyEventsList"))
		{
			ostringstream	ost, ostFinal;
			string			sessid, lookForKey, eventsList;
			vector<string>	searchWords;

			MESSAGE_DEBUG("", action, "start");

			// --- Initialization
			ostFinal.str("");

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
			MESSAGE_DEBUG("", action, "start");

			ostringstream   ostResult;
			auto			eventID = CheckHTTPParam_Number(indexPage.GetVarsHandler()->Get("id"));;

			ostResult.str("");

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


			indexPage.RegisterVariableForce("result", ostResult.str());

			if(!indexPage.SetTemplate("json_response.htmlt"))
			{
				MESSAGE_DEBUG("", action, "template file json_response.htmlt was missing");
				throw CException("Template file json_response.htmlt was missing");
			}  // if(!indexPage.SetTemplate("AJAX_precreateNewOpenVacancy.htmlt"))

			MESSAGE_DEBUG("", action, "start");
		}

		if(action == "AJAX_updateEventDescription")
		{
			MESSAGE_DEBUG("", action, "start");


			auto			value = CheckHTTPParam_Text(indexPage.GetVarsHandler()->Get("description"));
			auto			id = CheckHTTPParam_Number(indexPage.GetVarsHandler()->Get("eventid"));
			ostringstream	ostFinal;

			ostFinal.str("");


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

			indexPage.RegisterVariableForce("result", ostFinal.str());

			if(!indexPage.SetTemplate("json_response.htmlt"))
			{
				MESSAGE_DEBUG("", action, "template file json_response.htmlt was missing");
				throw CException("Template file was missing");
			}

			MESSAGE_DEBUG("", action, "finish");

		}

		if(action == "AJAX_removeEvent")
		{
			MESSAGE_DEBUG("", action, "start");

			auto	success_message = ""s;
			auto	error_message = ""s;
			auto	id = CheckHTTPParam_Number(indexPage.GetVarsHandler()->Get("id"));

			if(id.length()) 
			{
				if(db.Query("SELECT `id` FROM `events` WHERE `id`=\"" + id + "\" AND `owner_id`=\"" + user.GetID() + "\";"))
				{
					error_message = RemoveChecklistsByEventID(id, &db, &user);
					if(error_message.empty())
					{
						db.Query("DELETE FROM `users_notification` WHERE `actionTypeId`=\"70\" AND `actionId`=\"" + id + "\";");
						db.Query("DELETE FROM `users_notification` WHERE `actionTypeId`=\"69\" AND (`actionId` IN (SELECT `id` FROM `event_guests` WHERE `event_id`=\"" + id + "\"));");
						db.Query("DELETE FROM `users_notification` WHERE `actionTypeId`=\"68\" AND (`actionId` IN (SELECT `id` FROM `event_guests` WHERE `event_id`=\"" + id + "\"));");
						db.Query("DELETE FROM `users_notification` WHERE `actionTypeId`=\"67\" AND (`actionId` IN (SELECT `id` FROM `event_hosts` WHERE `event_id`=\"" + id + "\"));");

						db.Query("DELETE FROM `event_hosts` WHERE `event_id`=\"" + id + "\";");
						db.Query("DELETE FROM `event_guests` WHERE `event_id`=\"" + id + "\";");

						db.Query("DELETE FROM `event_guests` WHERE `event_id`=\"" + id + "\";");

						db.Query("DELETE FROM `events` WHERE `id`=\"" + id + "\";");

						if(db.isError())
						{
							error_message = gettext("error deleting event");
							MESSAGE_ERROR("", action, "updating DB");
						}
					}
					else
					{
						MESSAGE_ERROR("", action, error_message);
					}
				}
				else
				{
					error_message = gettext("Access prohibited");
					MESSAGE_DEBUG("", action, "user.id(" + user.GetID() + ") is not an event(" + id + ") host");
				}
			}
			else
			{
				error_message = gettext("mandatory parameter missed");
				MESSAGE_DEBUG("", action, error_message);
			}

			AJAX_ResponseTemplate(&indexPage, success_message, error_message);

			MESSAGE_DEBUG("", action, "finish");
		}

		if(action == "edit_event")
		{
			MESSAGE_DEBUG("", action, "start");

			indexPage.RegisterVariableForce("title", "Редактирование события");

			if(!indexPage.SetTemplate("edit_event.htmlt"))
			{
				MESSAGE_DEBUG("", action, "template file edit_event.htmlt was missing");
				throw CException("Template file edit_event.htmlt was missing");
			}

			MESSAGE_DEBUG("", action, "finish");
		}

		if(action == "AJAX_updateAccessType")
		{
			MESSAGE_DEBUG("", action, "start");

			ostringstream	ostFinal;

			{
				auto	value = toLower(CheckHTTPParam_Text(indexPage.GetVarsHandler()->Get("value")));
				auto	id = CheckHTTPParam_Number(indexPage.GetVarsHandler()->Get("eventid"));

				value = value.substr(0, 64);
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

			MESSAGE_DEBUG("", action, "finish");

		}

		if((action == "AJAX_updateStartTime") || (action == "AJAX_updateStartDate"))
		{
			MESSAGE_DEBUG("", action, "start");

			ostringstream	ostFinal;

			{
				auto	value = CheckHTTPParam_Number(indexPage.GetVarsHandler()->Get("value"));
				auto	id = CheckHTTPParam_Number(indexPage.GetVarsHandler()->Get("eventid"));

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

			MESSAGE_DEBUG("", action, "finish");

		}

		if(action == "AJAX_updateEventLink")
		{
			MESSAGE_DEBUG("", action, "start");

			ostringstream	ostFinal;

			{
				auto	value = toLower(CheckHTTPParam_Text(indexPage.GetVarsHandler()->Get("value")));
				auto	id = CheckHTTPParam_Number(indexPage.GetVarsHandler()->Get("eventid"));

				value = value.substr(0, 64);
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

			MESSAGE_DEBUG("", action, "finish");

		}

		if(action == "AJAX_createEvent")
		{
			MESSAGE_DEBUG("", action, "start");

			ostringstream	ostFinal;

			{

				auto	title			= CheckHTTPParam_Text(indexPage.GetVarsHandler()->Get("title"));
				auto	timestamp		= CheckHTTPParam_Number(indexPage.GetVarsHandler()->Get("timestamp"));
				auto	accessType		= CheckHTTPParam_Text(indexPage.GetVarsHandler()->Get("accessType"));
				auto	address			= CheckHTTPParam_Text(indexPage.GetVarsHandler()->Get("address"));
				auto	description		= CheckHTTPParam_Text(indexPage.GetVarsHandler()->Get("description"));

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

			MESSAGE_DEBUG("", action, "finish");
		}

		if(action == "AJAX_removeEventHost")
		{
			MESSAGE_DEBUG("", action, "start");

			ostringstream	ostFinal;
			auto			error_message = ""s;

			{
				auto	id = CheckHTTPParam_Number(indexPage.GetVarsHandler()->Get("id"));
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
								// --- special case: owner can remove himself
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

			MESSAGE_DEBUG("", action, "finish");

		}

		if(action == "AJAX_removeEventGuest")
		{
			MESSAGE_DEBUG("", action, "start");

			ostringstream	ostFinal;

			{
				auto	id = CheckHTTPParam_Number(indexPage.GetVarsHandler()->Get("id"));

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

			MESSAGE_DEBUG("", action, "finish");
		}

		if(action == "AJAX_updateTitle")
		{
			MESSAGE_DEBUG("", action, "start");

			ostringstream	ostFinal;

			{
				auto	value = CheckHTTPParam_Text(indexPage.GetVarsHandler()->Get("value"));
				auto	id = CheckHTTPParam_Number(indexPage.GetVarsHandler()->Get("eventid"));

				value = value.substr(0, 512);
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

			MESSAGE_DEBUG("", action, "finish");
		}

		if(action == "AJAX_updateAddress")
		{
			MESSAGE_DEBUG("", action, "start");

			ostringstream	ostFinal;
			{
				auto	value = CheckHTTPParam_Text(indexPage.GetVarsHandler()->Get("value")).substr(0, 512);
				auto	id = CheckHTTPParam_Number(indexPage.GetVarsHandler()->Get("eventid"));

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

			MESSAGE_DEBUG("", action, "finish");
		}


		if((action == "AJAX_getEventProfile") || (action == "AJAX_getEventProfileAndUser"))
		{
			MESSAGE_DEBUG("", action, "start");

			ostringstream   ostResult;
			ostResult.str("");
			{
				auto	eventID = CheckHTTPParam_Number(indexPage.GetVarsHandler()->Get("id"));
				auto	eventLink = CheckHTTPParam_Number(indexPage.GetVarsHandler()->Get("link"));

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

			MESSAGE_DEBUG("", action, "start");
		}

		if(action == "AJAX_getFriendsNotOnEvent")
		{
			MESSAGE_DEBUG("", action, "start");

			ostringstream   ostResult;

			ostResult.str("");
			{
				auto		  event_id = CheckHTTPParam_Number(indexPage.GetVarsHandler()->Get("event_id"));

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

			MESSAGE_DEBUG("", action, "finish");
		}

		if(action == "AJAX_addEventHost")
		{
				MESSAGE_DEBUG("", action, "start");

				ostringstream   ostResult;

				ostResult.str("");
				{
					auto		  user_id = CheckHTTPParam_Number(indexPage.GetVarsHandler()->Get("user_id"));
					auto		  event_id = CheckHTTPParam_Number(indexPage.GetVarsHandler()->Get("event_id"));

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
			MESSAGE_DEBUG("", action, "start");

			ostringstream   ostResult;
			ostResult.str("");
			{
				auto		  users_list	= CheckHTTPParam_Text(indexPage.GetVarsHandler()->Get("user_id"));
				auto		  event_id		= CheckHTTPParam_Number(indexPage.GetVarsHandler()->Get("event_id"));
				auto		  email			= CheckHTTPParam_Email(indexPage.GetVarsHandler()->Get("user_email"));

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

											mail.SendToEmail(email, user.GetLogin(), "quick_registration_invite", indexPage.GetVarsHandler(), &db);

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

			MESSAGE_DEBUG("", action, "finish");
		}

		if((action == "AJAX_setNoGift_N") || (action == "AJAX_setNoGift_Y"))
		{
			MESSAGE_DEBUG("", action, "start");

			ostringstream	ostFinal;

			{
				auto	id = CheckHTTPParam_Number(indexPage.GetVarsHandler()->Get("event_id"));

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
			MESSAGE_DEBUG("", action, "start");

			ostringstream	ostFinal;
			{

				auto	kidsCounter = CheckHTTPParam_Number(indexPage.GetVarsHandler()->Get("kidsCounter"));
				auto	adultsCounter = CheckHTTPParam_Number(indexPage.GetVarsHandler()->Get("adultsCounter"));
				auto	id = CheckHTTPParam_Number(indexPage.GetVarsHandler()->Get("event_id"));
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

			MESSAGE_DEBUG("", action, "finish");
		}

		if(action == "createnewevent")
		{
			MESSAGE_DEBUG("", action, "start");

			indexPage.RegisterVariableForce("title", "Новое событие");

			if(!indexPage.SetTemplate("create_event.htmlt"))
			{
				MESSAGE_DEBUG("", action, "template file createnewevent.htmlt was missing");
				throw CException("Template file createnewevent.htmlt was missing");
			} 

			MESSAGE_DEBUG("", action, "finish");
		}

		if((action == "AJAX_switchChecklistItem") || (action == "AJAX_updateChecklistItemPrice") || (action == "AJAX_updateChecklistItemComment"))
		{
			MESSAGE_DEBUG("", action, "start");

			auto	success_message	= ""s;
			auto	error_message	= ""s;
			auto	id				= CheckHTTPParam_Number(indexPage.GetVarsHandler()->Get("id"));
			auto	value			= CheckHTTPParam_Text(indexPage.GetVarsHandler()->Get("value"));

			if(id.length() && value.length())
			{
				if((error_message = amIAllowedToChangeEvent(GetValueFromDB(Get_EventIDByChecklistItemID(id), &db), &db, &user)).empty())
				{
					if(action == "AJAX_switchChecklistItem")
						db.Query("UPDATE `checklist_items` SET `state`=" + quoted(value) + " WHERE `id`=" + quoted(id) + ";");
					if(action == "AJAX_updateChecklistItemPrice")
						db.Query("UPDATE `checklist_items` SET `price`=" + quoted(value) + " WHERE `id`=" + quoted(id) + ";");
					if(action == "AJAX_updateChecklistItemComment")
						db.Query("UPDATE `checklist_items` SET `comment`=" + quoted(value) + " WHERE `id`=" + quoted(id) + ";");
				}
				else
				{
					MESSAGE_ERROR("", action, error_message);
				}
			}
			else
			{
				if(value.empty())
				{
					error_message = gettext("cost not found");
					MESSAGE_DEBUG("", action, error_message);
				}
				else
				{
					error_message = gettext("mandatory parameter missed");
					MESSAGE_ERROR("", action, error_message);
				}
			}

			AJAX_ResponseTemplate(&indexPage, success_message, error_message);

			MESSAGE_DEBUG("", action, "finish");
		}

		if((action == "AJAX_getFavoriteChecklistsCategories"))
		{
			MESSAGE_DEBUG("", action, "start");

			auto	success_message	= ""s;
			auto	error_message	= ""s;

			success_message = "\"checklists\":[" + GetFavoriteChecklistCategoriesInJSONFormat("SELECT * FROM `event_checklists` WHERE `favorite`=\"Y\";", &db, &user) + "]";

			AJAX_ResponseTemplate(&indexPage, success_message, error_message);

			MESSAGE_DEBUG("", action, "finish");
		}

		if((action == "AJAX_getFavoriteChecklistItems"))
		{
			MESSAGE_DEBUG("", action, "start");

			auto	success_message	= ""s;
			auto	error_message	= ""s;
			auto	id				= CheckHTTPParam_Number(indexPage.GetVarsHandler()->Get("id"));

			if(id.length())
			{
				success_message = "\"checklists\":[" + GetEventChecklistInJSONFormat("SELECT * FROM `event_checklists` WHERE `id`=" + quoted(id) + ";", &db, &user) + "]";
			}
			else
			{
				error_message = gettext("mandatory parameter missed");
				MESSAGE_ERROR("", action, error_message);
			}

			AJAX_ResponseTemplate(&indexPage, success_message, error_message);

			MESSAGE_DEBUG("", action, "finish");
		}

		if((action == "AJAX_addFavoriteChecklistItems"))
		{
			MESSAGE_DEBUG("", action, "start");

			auto	success_message		= ""s;
			auto	error_message		= ""s;
			auto	event_id			= CheckHTTPParam_Number(indexPage.GetVarsHandler()->Get("event_id"));
			auto	from_checklist_id	= CheckHTTPParam_Number(indexPage.GetVarsHandler()->Get("from_checklist_id"));

			if(event_id.length() && from_checklist_id.length())
			{
				if((error_message = amIAllowedToChangeEvent(event_id, &db, &user)).empty())
				{
					auto	to_checklist_id = GetChecklistIDByEventID_CreateIfMissed(event_id, &db, &user);

					if(to_checklist_id.length())
					{
						if((error_message = addMissedChecklistItems(from_checklist_id, to_checklist_id, &db)).empty())
						{

						}
						else
						{
							MESSAGE_ERROR("", action, error_message);
						}
					}
					else
					{
						error_message = gettext("checklist not found");
						MESSAGE_ERROR("", action, error_message);
					}
				}
				else
				{
					MESSAGE_ERROR("", action, error_message);
				}
			}
			else
			{
				error_message = gettext("mandatory parameter missed");
				MESSAGE_ERROR("", action, error_message);
			}

			AJAX_ResponseTemplate(&indexPage, success_message, error_message);

			MESSAGE_DEBUG("", action, "finish");
		}

		if(action == "AJAX_addChecklistItem")
		{
			MESSAGE_DEBUG("", action, "start");

			auto	success_message		= ""s;
			auto	error_message		= ""s;
			auto	event_id			= CheckHTTPParam_Number(indexPage.GetVarsHandler()->Get("event_id"));
			auto	category			= CheckHTTPParam_Text(indexPage.GetVarsHandler()->Get("category"));
			auto	title				= CheckHTTPParam_Text(indexPage.GetVarsHandler()->Get("title"));

			if(event_id.length() && title.length())
			{
				if((error_message = amIAllowedToChangeEvent(event_id, &db, &user)).empty())
				{
					auto	to_checklist_id = GetChecklistIDByEventID_CreateIfMissed(event_id, &db, &user);

					if(to_checklist_id.length())
					{
						if((error_message = addChecklistItem(to_checklist_id, title, category, &db, &user)).empty())
						{

						}
						else
						{
							MESSAGE_ERROR("", action, error_message);
						}
					}
					else
					{
						error_message = gettext("checklist not found");
						MESSAGE_ERROR("", action, error_message);
					}
				}
				else
				{
					MESSAGE_ERROR("", action, error_message);
				}
			}
			else
			{
				error_message = gettext("mandatory parameter missed");
				MESSAGE_ERROR("", action, error_message);
			}

			AJAX_ResponseTemplate(&indexPage, success_message, error_message);

			MESSAGE_DEBUG("", action, "finish");
		}

		if((action == "AJAX_deleteChecklistItem"))
		{
			MESSAGE_DEBUG("", action, "start");

			auto	success_message		= ""s;
			auto	error_message		= ""s;
			auto	id					= CheckHTTPParam_Number(indexPage.GetVarsHandler()->Get("id"));

			if(id.length())
			{
				auto	event_id	= GetValueFromDB(Get_EventIDByChecklistItemID(id), &db);

				if(event_id.length())
				{
					if((error_message = amIAllowedToChangeEvent(event_id, &db, &user)).empty())
					{
						db.Query("DELETE FROM `checklist_items` WHERE `id`=" + quoted(id) + ";");
					}
					else
					{
						MESSAGE_ERROR("", action, error_message);
					}
				}
				else
				{
					error_message = gettext("mandatory parameter missed");
					MESSAGE_ERROR("", action, error_message);
				}
			}
			else
			{
				error_message = gettext("mandatory parameter missed");
				MESSAGE_ERROR("", action, error_message);
			}

			AJAX_ResponseTemplate(&indexPage, success_message, error_message);

			MESSAGE_DEBUG("", action, "finish");
		}

		if(action == "AJAX_getChecklistCategoryAutocompleteList")
		{
			auto	term			= CheckHTTPParam_Text(indexPage.GetVarsHandler()->Get("term"));
			auto	template_name	= "json_response.htmlt"s;
			auto	error_message	= ""s;
			auto	success_message	= ""s;
			auto	result			= ""s;

			if(term.length())
			{
				auto	affected = db.Query("SELECT DISTINCT(`category`) FROM `checklist_predefined` WHERE `category` LIKE \"%" + term + "%\" LIMIT 0, 20;");
				if(affected)
				{
					for(int i = 0; i < affected; ++i)
					{
						if(i) success_message += ",";
						success_message += "{\"id\":\"0\",\"label\":\"" + db.Get(i, "category") + "\"}";
					}
				}
				else
				{
					error_message = gettext("category not found");
					MESSAGE_DEBUG("", "", error_message);
				}	
			}
			else
			{
				MESSAGE_DEBUG("", "", "term is empty");
			}

			if(error_message.empty())
			{
				result = "[" + success_message + "]";
			}
			else
			{
				MESSAGE_DEBUG("", action, "failed");
				result = "[]";
			}

			indexPage.RegisterVariableForce("result", result);

			if(!indexPage.SetTemplate(template_name)) MESSAGE_ERROR("", action, "can't find template " + template_name);
		}
		if(action == "AJAX_getChecklistTitleAutocompleteList")
		{
			auto	term			= CheckHTTPParam_Text(indexPage.GetVarsHandler()->Get("term"));
			auto	template_name	= "json_response.htmlt"s;
			auto	error_message	= ""s;
			auto	success_message	= ""s;
			auto	result			= ""s;

			if(term.length())
			{
				auto	affected = db.Query("SELECT DISTINCT(`title`) FROM `checklist_predefined` WHERE `title` LIKE \"%" + term + "%\" LIMIT 0, 20;");
				if(affected)
				{
					for(int i = 0; i < affected; ++i)
					{
						if(i) success_message += ",";
						success_message += "{\"id\":\"0\",\"label\":\"" + db.Get(i, "title") + "\"}";
					}
				}
				else
				{
					error_message = gettext("title not found");
					MESSAGE_DEBUG("", "", error_message);
				}	
			}
			else
			{
				MESSAGE_DEBUG("", "", "term is empty");
			}

			if(error_message.empty())
			{
				result = "[" + success_message + "]";
			}
			else
			{
				MESSAGE_DEBUG("", action, "failed");
				result = "[]";
			}

			indexPage.RegisterVariableForce("result", result);

			if(!indexPage.SetTemplate(template_name)) MESSAGE_ERROR("", action, "can't find template " + template_name);
		}

		MESSAGE_DEBUG("", "", "post-condition if(action == \"" + action + "\")");

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
