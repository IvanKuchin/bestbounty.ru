#include "account.h"

static auto RemovePhoneConfirmationCodes(string sessid, CMysql *db)
{
	auto	error_message = ""s;

	MESSAGE_DEBUG("", "", "start");

	db->Query("DELETE FROM `phone_confirmation` WHERE `session`=\"" + sessid + "\";");

	MESSAGE_DEBUG("", "", "finish");

	return error_message;
}

int main()
{
	CStatistics		appStat;  // --- CStatistics must be firts statement to measure end2end param's
	CCgi			indexPage(EXTERNAL_TEMPLATE);
	CUser			user;
	string			action, partnerID;
	CMysql			db;
	struct timeval	tv;

	MESSAGE_DEBUG("", "", __FILE__);

	signal(SIGSEGV, crash_handler); 

	gettimeofday(&tv, NULL);
	srand(tv.tv_sec * tv.tv_usec * 100000);

	try
	{

	indexPage.ParseURL();
	indexPage.AddCookie("lng", "ru", "", "", "/");

	if(!indexPage.SetTemplate("index.htmlt"))
	{
		CLog	log;

		log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR: template file was missing");
		throw CException("Template file was missing");
	}

	if(db.Connect(DB_NAME, DB_LOGIN, DB_PASSWORD) < 0)
	{
		CLog	log;

		log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR: Can not connect to mysql database");
		throw CExceptionHTML("MySql connection");
	}

	indexPage.SetDB(&db);

	action = CheckHTTPParam_Text(indexPage.GetVarsHandler()->Get("action"));

	MESSAGE_DEBUG("", "", "action = " + action);

	// --- generate common parts
	{
		// --- it has to be run before session, because session relay upon Apache environment variable
		if(RegisterInitialVariables(&indexPage, &db, &user))
		{
		}
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

	MESSAGE_DEBUG("", "", "pre-condition action = " + action);

	if(action == "AJAX_changeLogin")
	{
		ostringstream   ostResult;
		string		    loginFromUser;

		{
			CLog	log;
			log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]:action == " + action + ": start");
		}

		ostResult.str("");
/*
		if(user.GetLogin() == "Guest")
		{
			{
				CLog	log;
				log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]:action == " + action + ": re-login required");
			}

			ostResult << "{\"result\":\"error\",\"description\":\"re-login required\"}";
		}
		else
*/
		{
			loginFromUser = CheckHTTPParam_Text(indexPage.GetVarsHandler()->Get("login"));

			if(loginFromUser.length() >= 8)
			{

				if(loginFromUser.find_first_of(" \\/%?абвгдеёжзийклмнопрстуфхцчшщьыъэюяАБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЬЫЪЭЮЯ") == string::npos)
				{

					if(db.Query("SELECT `id` FROM `users` WHERE `login`=\"" + loginFromUser + "\" AND `id`!=\"" + user.GetID() + "\";"))
					{
						{
							CLog	log;
							log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]:action == " + action + ": Login already used");
						}

						ostResult << "{" 
								  << "\"result\":\"error\","
								  << "\"description\":\"Имя уже занято\""
								  << "}";
					}
					else
					{

						db.Query("UPDATE `users` SET `login`=\"" + loginFromUser + "\" WHERE `id`=\"" + user.GetID() + "\";");
						if(!db.isError())
						{
							ostResult << "{" 
									  << "\"result\":\"success\","
									  << "\"userLogin\":\"" << loginFromUser << "\""
									  << "}";
						}
						else
						{
							{
								CLog	log;
								log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:action == " + action + ":ERROR: updating users table [" + db.GetErrorMessage() + "]");
							}

							ostResult << "{" 
									  << "\"result\":\"error\","
									  << "\"description\":\"Внутренняя ошибка БД\""
									  << "}";
						}
					}
				}
				else
				{
					
					{
						CLog	log;
						log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:action == " + action + ":ERROR: login containing symbols \\ or /");
					}

					ostResult << "{" 
							  << "\"result\":\"error\","
							  << "\"description\":\"Нельзя использовать: русские буквы, пробел или символы /%?\""
							  << "}";
				}
			}
			else
			{
				
				{
					CLog	log;
					log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:action == " + action + ":ERROR: login too short");
				}

				ostResult << "{" 
						  << "\"result\":\"error\","
						  << "\"description\":\"Должен быть 8 и более символов\""
						  << "}";
			}

		} // --- if(user.GetLogin() == "Guest")


		indexPage.RegisterVariableForce("result", ostResult.str());

		if(!indexPage.SetTemplate("json_response.htmlt"))
		{
			CLog	log;

			log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR: template file json_response.htmlt was missing");
			throw CException("Template file json_response.htmlt was missing");
		}  // if(!indexPage.SetTemplate("AJAX_changeLogin.htmlt"))

		{
			CLog	log;
			log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]:action == " + action + ": end");
		}
	}   // if(action == "AJAX_changeLogin")

	if(action == "AJAX_changeUserSex")
	{
		ostringstream   ostResult;
		string		    userSex;

		{
			CLog	log;
			log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]:action == " + action + ": start");
		}

		ostResult.str("");
/*		if(user.GetLogin() == "Guest")
		{
			{
				CLog	log;
				log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]:action == " + action + ": re-login required");
			}

			ostResult << "{\"result\":\"error\",\"description\":\"re-login required\"}";
		}
		else
*/
		{
			userSex = indexPage.GetVarsHandler()->Get("userSex");

			if((userSex == "male") || (userSex == "female"))
			{

				db.Query("UPDATE `users` SET `sex`=\"" + userSex + "\" WHERE `id`=\"" + user.GetID() + "\";");
				if(!db.isError())
				{
					ostResult << "{" 
							  << "\"result\":\"success\","
							  << "\"userLogin\":\"" << userSex << "\""
							  << "}";
				}
				else
				{
					{
						CLog	log;
						log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:action == " + action + ":ERROR: updating users table [" + db.GetErrorMessage() + "]");
					}

					ostResult << "{" 
							  << "\"result\":\"error\","
							  << "\"description\":\"Внутренняя ошибка БД\""
							  << "}";
				}

			}
			else
			{
				
				{
					CLog	log;
					log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:action == " + action + ":ERROR: wrong userSex[" + userSex + "]");
				}

				ostResult << "{" 
						  << "\"result\":\"error\","
						  << "\"description\":\"некорректный параметер\""
						  << "}";
			}

		} // --- if(user.GetLogin() == "Guest")


		indexPage.RegisterVariableForce("result", ostResult.str());

		if(!indexPage.SetTemplate("json_response.htmlt"))
		{
			CLog	log;

			log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR: template file json_response.htmlt was missing");
			throw CException("Template file json_response.htmlt was missing");
		}  // if(!indexPage.SetTemplate("AJAX_changeUserSex.htmlt"))

		{
			CLog	log;
			log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]:action == " + action + ": end");
		}
	}   // if(action == "AJAX_changeUserSex")

	if(action == "AJAX_changeUserBirthday")
	{
		ostringstream   ostResult;
		string		    userBirthday;

		{
			CLog	log;
			log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]:action == " + action + ": start");
		}

		ostResult.str("");
/*		if(user.GetLogin() == "Guest")
		{
			{
				CLog	log;
				log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]:action == " + action + ": re-login required");
			}

			ostResult << "{\"result\":\"error\",\"description\":\"re-login required\"}";
		}
		else
*/
		{
			userBirthday = CheckHTTPParam_Text(indexPage.GetVarsHandler()->Get("value"));

			if(std::regex_match(userBirthday, std::regex("^[[:digit:]]{1,2}/[[:digit:]]{1,2}/[[:digit:]]{2,4}$")))
			{

				db.Query("UPDATE `users` SET `birthday`=\"" + userBirthday + "\" WHERE `id`=\"" + user.GetID() + "\";");
				if(!db.isError())
				{
					ostResult << "{" 
							  << "\"result\":\"success\","
							  << "\"birthday\":\"" << userBirthday << "\""
							  << "}";
				}
				else
				{
					{
						CLog	log;
						log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:action == " + action + ":ERROR: updating users table [" + db.GetErrorMessage() + "]");
					}

					ostResult << "{" 
							  << "\"result\":\"error\","
							  << "\"description\":\"Внутренняя ошибка БД\""
							  << "}";
				}

			}
			else
			{
				
				{
					CLog	log;
					log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:action == " + action + ":ERROR: wrong date format dd/mm/yy userBirthday[" + userBirthday + "]");
				}

				ostResult << "{" 
						  << "\"result\":\"error\","
						  << "\"description\":\"некорректный формат даты\""
						  << "}";
			}

		} // --- if(user.GetLogin() == "Guest")


		indexPage.RegisterVariableForce("result", ostResult.str());

		if(!indexPage.SetTemplate("json_response.htmlt"))
		{
			CLog	log;

			log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR: template file json_response.htmlt was missing");
			throw CException("Template file json_response.htmlt was missing");
		}  // if(!indexPage.SetTemplate("AJAX_changeUserBirthday.htmlt"))

		{
			CLog	log;
			log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]:action == " + action + ": end");
		}
	}   // if(action == "AJAX_changeUserBirthday")

	if(action == "AJAX_updateCity")
	{
		ostringstream   ostResult;
		string		    city;

		{
			CLog	log;
			log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]:action == " + action + ": start");
		}

		ostResult.str("");
/*		if(user.GetLogin() == "Guest")
		{
			{
				CLog	log;
				log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]:action == " + action + ": re-login required");
			}

			ostResult << "{\"result\":\"error\",\"description\":\"re-login required\"}";
		}
		else
*/
		{
			string::size_type 	openBrace;
			string				cityID = "";

			city = CheckHTTPParam_Text(indexPage.GetVarsHandler()->Get("value"));

			openBrace = city.find("(");
			if(openBrace != string::npos)
			{
				city = city.substr(0, openBrace);
			}
			trim(city);

			cityID = GetGeoLocalityIDByCityAndRegion("", city, &db);
			if(cityID.empty()) cityID = "0";

			if(cityID.length())
			{

				db.Query("UPDATE `users` SET `geo_locality_id`=\"" + cityID + "\" WHERE `id`=\"" + user.GetID() + "\";");
				if(!db.isError())
				{
					ostResult << "{" 
							  << "\"result\":\"success\","
							  << "\"city\":\"" << city << "\""
							  << "}";
				}
				else
				{
					{
						CLog	log;
						log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:action == " + action + ":ERROR: updating users table [" + db.GetErrorMessage() + "]");
					}

					ostResult << "{" 
							  << "\"result\":\"error\","
							  << "\"description\":\"Внутренняя ошибка БД\""
							  << "}";
				}

			}
			else
			{
				
				{
					CLog	log;
					log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]:action == " + action + ": city is empty");
				}

				ostResult << "{" 
						  << "\"result\":\"error\","
						  << "\"description\":\"город не указан\""
						  << "}";
			}

		} // --- if(user.GetLogin() == "Guest")


		indexPage.RegisterVariableForce("result", ostResult.str());

		if(!indexPage.SetTemplate("json_response.htmlt"))
		{
			CLog	log;

			log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR: template file json_response.htmlt was missing");
			throw CException("Template file json_response.htmlt was missing");
		}  // if(!indexPage.SetTemplate("AJAX_updateCity.htmlt"))

		{
			CLog	log;
			log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]:action == " + action + ": end");
		}
	}   // if(action == "AJAX_updateCity")

	if((action == "AJAX_editProfile_setBirthdayPrivate") || (action == "AJAX_editProfile_setBirthdayPublic"))
	{
		ostringstream   ostResult;
		string		    birthdayAccess = (action == "AJAX_editProfile_setBirthdayPrivate" ? "private" : "public");

		{
			CLog	log;
			log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]:action == " + action + ": start");
		}

		ostResult.str("");
/*		if(user.GetLogin() == "Guest")
		{
			{
				CLog	log;
				log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]:action == " + action + ": re-login required");
			}

			ostResult << "{\"result\":\"error\",\"description\":\"re-login required\"}";
		}
		else
*/
		{
			db.Query("UPDATE `users` SET `birthdayAccess`=\"" + birthdayAccess + "\" WHERE `id`=\"" + user.GetID() + "\";");
			if(!db.isError())
			{
				ostResult << "{" 
						  << "\"result\":\"success\","
						  << "\"birthday\":\"" << birthdayAccess << "\""
						  << "}";
			}
			else
			{
				{
					CLog	log;
					log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:action == " + action + ":ERROR: updating users table [" + db.GetErrorMessage() + "]");
				}

				ostResult << "{" 
						  << "\"result\":\"error\","
						  << "\"description\":\"Внутренняя ошибка БД\""
						  << "}";
			}
		} // --- if(user.GetLogin() == "Guest")


		indexPage.RegisterVariableForce("result", ostResult.str());

		if(!indexPage.SetTemplate("json_response.htmlt"))
		{
			CLog	log;

			log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR: template file json_response.htmlt was missing");
			throw CException("Template file json_response.htmlt was missing");
		}  // if(!indexPage.SetTemplate("AJAX_editProfile_setBirthdayPrivate.htmlt"))

		{
			CLog	log;
			log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]:action == " + action + ": end");
		}
	}   // if(action == "AJAX_editProfile_setBirthdayPrivate")

	if(action == "AJAX_sendPhoneConfirmationSMS")
	{
		auto			template_file = "json_response.htmlt"s;
		auto			error_message = ""s;
		ostringstream   ostResult;

/*		if(user.GetLogin() == "Guest")
		{
			MESSAGE_DEBUG("", "", "re-login required");
			error_message = "re-login required";
		}
		else
*/
		{
			auto			country_code = CheckHTTPParam_Number(indexPage.GetVarsHandler()->Get("country_code"));
			auto			phone_number = CheckHTTPParam_Number(indexPage.GetVarsHandler()->Get("phone_number"));
			auto			confirmation_code = GetRandom(4);
			c_smsc			smsc(&db);

			MESSAGE_DEBUG("", action, "start");

			if(country_code.length() && phone_number.length())
			{
				auto	phone_confirmation_id = db.InsertQuery("INSERT INTO `phone_confirmation` (`session`, `confirmation_code`, `country_code`, `phone_number`, `eventTimestamp`)"
								" VALUES ("
								"\"" + indexPage.SessID_Get_FromHTTP() + "\","
								"\"" + confirmation_code + "\","
								"\"" + country_code + "\","
								"\"" + phone_number + "\","
								"UNIX_TIMESTAMP()"
								")"
								);

				if(phone_confirmation_id)
				{
					auto	ret = smsc.send_sms(country_code + phone_number, "Код для привязки телефона " + confirmation_code, 0, "", 0, 0, DOMAIN_NAME, "", "");
				}
				else
				{
					MESSAGE_ERROR("", action, "fail to insert to db");
					error_message = gettext("SQL syntax issue");
				}
			}
			else
			{
				error_message = gettext("phone number is incorrect");
				MESSAGE_ERROR("", action, "country_code(" + country_code + ") or phone_number(" + phone_number + ") is empty");
			}
		}

		ostResult.str("");
		if(error_message.empty())
		{
			ostResult << "{" 
					  << "\"result\":\"success\""
					  << "}";
		}
		else
		{
			MESSAGE_ERROR("", action, "fail to send phone confirmation sms");

			ostResult << "{" 
					  << "\"result\":\"error\","
					  << "\"description\":\"" + error_message + "\""
					  << "}";
		}

		indexPage.RegisterVariableForce("result", ostResult.str());

		if(!indexPage.SetTemplate(template_file))
		{
			MESSAGE_ERROR("", action, "template file " + template_file + " was missing");
			throw CException("Template file " + template_file + " was missing");
		}

		MESSAGE_DEBUG("", action, "finish");
	}

	if(action == "AJAX_checkPhoneConfirmationCode")
	{
		auto			template_file = "json_response.htmlt"s;
		auto			error_message = ""s;
		auto			extra_fields = ""s;
		ostringstream   ostResult;

		{
			auto			confirmation_code = CheckHTTPParam_Number(indexPage.GetVarsHandler()->Get("confirmation_code"));

			MESSAGE_DEBUG("", action, "start");

			if(db.Query("SELECT * FROM `phone_confirmation` WHERE "
							"`confirmation_code`=\"" + confirmation_code + "\" AND "
							"`session`=\"" + indexPage.SessID_Get_FromHTTP() + "\" AND "
							"`attempt`<=\"3\" AND "
							"`eventTimestamp`>=(UNIX_TIMESTAMP() - " + to_string(SMSC_EXPIRATION) + ")"
							";"))
			{
				auto	country_code = db.Get(0, "country_code");
				auto	phone_number = db.Get(0, "phone_number");

				db.Query("UPDATE `users` SET `country_code`=\""s + country_code + "\", `phone`=\"" + phone_number + "\" , `is_phone_confirmed`=\"Y\" WHERE `id`=\"" + user.GetID() + "\";");
				if(db.isError())
				{
					MESSAGE_ERROR("", action, "fail to update db");
					error_message = gettext("SQL syntax issue");
				}
				else
				{
					RemovePhoneConfirmationCodes(indexPage.SessID_Get_FromHTTP(), &db);
				}
			}
			else
			{
				db.Query("UPDATE `phone_confirmation` SET `attempt`=`attempt` + 1 WHERE"
							"`session`=\"" + indexPage.SessID_Get_FromHTTP() + "\" AND "
							"`eventTimestamp`>=(UNIX_TIMESTAMP() - " + to_string(SMSC_EXPIRATION) + ")"
							);

				if(db.Query("SELECT `attempt` FROM `phone_confirmation` WHERE "
								"`session`=\"" + indexPage.SessID_Get_FromHTTP() + "\" AND "
								"`eventTimestamp`>=(UNIX_TIMESTAMP() - " + to_string(SMSC_EXPIRATION) + ")"
								";"))
				{
					auto	attempts = db.Get(0, "attempt");

					if(stoi(attempts) >= 3) RemovePhoneConfirmationCodes(indexPage.SessID_Get_FromHTTP(), &db);
					extra_fields = "\"attempt\":\""s + attempts + "\",";
				}
				else
				{
					MESSAGE_ERROR("", "", "fail to select data");
				}
				error_message = gettext("incorrect confirmation code");
				MESSAGE_ERROR("", action, "incorrect confirmation code");
			}
		}

		ostResult.str("");
		if(error_message.empty())
		{
			ostResult << "{" 
					  << "\"result\":\"success\""
					  << "}";
		}
		else
		{
			MESSAGE_ERROR("", action, "fail to send phone confirmation sms");

			ostResult << "{" 
					  << "\"result\":\"error\","
					  << extra_fields
					  << "\"description\":\"" + error_message + "\""
					  << "}";
		}

		indexPage.RegisterVariableForce("result", ostResult.str());

		if(!indexPage.SetTemplate(template_file))
		{
			MESSAGE_ERROR("", action, "template file " + template_file + " was missing");
			throw CException("Template file " + template_file + " was missing");
		}

		MESSAGE_DEBUG("", action, "finish");
	}


	MESSAGE_DEBUG("", action, "finish condition")

	indexPage.OutTemplate();

	}
	catch(CExceptionHTML &c)
	{
		CLog	log;

		c.SetLanguage(indexPage.GetLanguage());
		c.SetDB(&db);

		log.Write(DEBUG, string(__func__) + ": catch CExceptionHTML: DEBUG exception reason: [", c.GetReason(), "]");

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
		CLog 	log;

		if(!indexPage.SetTemplateFile("templates/error.htmlt"))
		{
			return(-1);
		}

		log.Write(ERROR, string(__func__) + ": catch CException: exception: ERROR  ", c.GetReason());

		indexPage.RegisterVariable("content", c.GetReason());
		indexPage.OutTemplate();
		return(-1);
	}
	catch(exception& e)
	{
		CLog 	log;
		log.Write(ERROR, string(__func__) + ": catch(exception& e): catch standard exception: ERROR  ", e.what());

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

