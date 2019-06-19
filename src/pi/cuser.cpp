#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <iomanip>
#include <iostream>

#include <sstream>

#include "cactivator.h"
#include "cmail.h"
#include "cuser.h"
#include "cexception.h"
//#include "GeoIP.h"

CUser::CUser() : login("Guest"), db(NULL), vars(NULL)
{
}

CUser::CUser(string log, string pas, string pasConfirm, string em, string l, string i, string agr, string t, string pID, string ph) : db(NULL), vars(NULL)
{
	SetLogin(log);
	SetPasswd(pas);
	passwdConfirm = pasConfirm;
	SetEmail(em);
	SetLng(l);
	SetIP(i);
	agreement = agr;
	SetType(t);
	SetPartnerID(pID);
	SetPhone(ph);
}

bool CUser::isBlock()
{
	ostringstream	ost;

	ost << "select * from users_block where `userid`='" << GetLogin() << "'";
	if(!db)
	{
		MESSAGE_ERROR("CUser", "", "connecting db in CUser::Block");
		throw CExceptionHTML("error db");
	}
	if(db->Query(ost.str()) > 0)
		return true;
	else
		return false;
}

bool CUser::isActive()
{
	ostringstream	ost;
	string		tmp;

	ost << "select * from users where `login`='" << GetLogin() << "'";
	if(!db)
	{
		MESSAGE_ERROR("CUser", "", "connecting db in CUser::Block");
		throw CExceptionHTML("error db");
	}
	if(db->Query(ost.str()) > 0)
	{
		tmp = db->Get(0, "isactivated");
		if(tmp == "Y") return true;
	}
	return false;
}

void CUser::Block(string reason)
{
	ostringstream	ost;

	if(GetLogin().empty())
	{
		MESSAGE_ERROR("CUser", "", "it is require to set user_login before block in CUser::Block");
		throw CExceptionHTML("no user");
	}
	ost << "INSERT INTO users_block(`userid`,`date`,`notes`) VALUE('" << GetLogin() << "',now(),'" << reason << "')";
	if(!db)
	{
		MESSAGE_ERROR("CUser", "", "connecting db in CUser::Block");
		throw CExceptionHTML("error db");
	}
	db->Query(ost.str());
}

bool CUser::isLoginExist()
{
	ostringstream	ost;

	ost << "select type from users where login='" << login << "'";
	if(!db)
	{
		MESSAGE_ERROR("CUser", "", "connecting db");
		throw CExceptionHTML("error db");
	}
	if(db->Query(ost.str()) <= 0)
		return false;

	return true;
}

bool CUser::isPasswdError()
{
	if(passwd.empty()) return true;
	if(passwd != passwdConfirm)
		return true;
	else
		return false;
}

bool CUser::isEmailCorrect()
{
	string::size_type	atPos, dotPos;
	ostringstream	ost;

	if(email.empty()) return false;

	atPos = email.find("@");
	if((atPos == 0) || (atPos == string::npos)) return false;

	dotPos = email.rfind(".");
	if((dotPos == 0) || (dotPos == string::npos)) return false;
	if(dotPos < email.rfind("@")) return false;
	return true;
}

bool CUser::isEmailDuplicate()
{
	ostringstream	ost;

	ost << "select type from users where email='" << email << "'";
	if(!db) throw CExceptionHTML("error db");
	if(db->Query(ost.str()) > 0) return true;

	return false;
}

bool CUser::isPhoneCorrect()
{
	string		ph;
	ostringstream	ost;

	ph = GetPhone();
	if(ph.find_first_not_of("01234567890-()") != string::npos) return false;
	while(ph.find_first_not_of("0123456789") != string::npos)
	{
		ph.replace(ph.find_first_not_of("0123456789"), 1, "");
	}
	if(ph.length() != 10) return false;
	return true;
}

bool CUser::isPhoneDuplicate()
{
	string		ph;
	ostringstream	ost;

	ph = GetPhone();
	ost.str("");
	ost << "select * from `users` where `phone`='" << ph << "'";
	if(!db)
	{
		MESSAGE_ERROR("CUser", "", "connecting db in CUser::isPhoneCorrect");
		throw CExceptionHTML("error db");
	}
	if(db->Query(ost.str()) > 0) return true;

	return false;
}

bool CUser::isTypeCorrect()
{
	if(GetType().empty()) return false;

	if(GetType() == "user") return true;
	if(GetType() == "partner") return true;
	if(GetType() == "investor") return true;
	if(GetType() == "no role") return true;
	if(GetType() == "approver") return true;
	if(GetType() == "agency") return true;
	if(GetType() == "subcontractor") return true;

	return false;
}

bool CUser::isAgree()
{
	if((agreement[0] == 'n') || (agreement[0] == 'N')) return false;

	return true;
}

string CUser::GetSiteTheme()
{
	string	result = DEFAULT_SITE_THEME;

	if(site_theme.length()) result = site_theme;
	else
	{
		if(db)
		{
			if(db->Query("SELECT `path` FROM `site_themes` WHERE `id`=\"" + GetSiteThemeID() + "\";"))
			{
				site_theme = db->Get(0, "path");
				result = site_theme;
			}
		}
		else
		{
			MESSAGE_ERROR("CUser", "", "db not initialized");
		}
	}
	return result;
}

bool CUser::isAllowedLng(string p)
{
	struct	stat	buf;
	string			directory;
	bool			result = false;

	directory = TEMPLATE_PATH + p + "/";

	if(stat(directory.c_str(), &buf) == 0)
		result = true;
	else
	{
		MESSAGE_ERROR("CUser", "", "directory [" + directory + "] doesn't exists");

		result = false;
	}

	return result;
}

void CUser::SetLng(const string &p)
{
	if(isAllowedLng(p))
		lng = p;
	else
	{
		MESSAGE_ERROR("CUser", "", "language for user " + GetLogin() + " changed to default, because [" + p + "] is not supported yet.");
		lng = DEFAULT_LANGUAGE;
	}
}

void CUser::Create()
{
	ostringstream		ost;
	string				countryName;
	unsigned long		newUserID;

	if(db == NULL)
	{
		MESSAGE_ERROR("CUser", "", "db is null");
		throw CExceptionHTML("error db in User Create");
	}

	ost.str("");
	ost << "insert into `users` (`login`, `email`, `type`, `isactivated`, `lng`, `regdate`, `partnerid`, `country`, `ip`, `phone`, `activator_sent`, `cv`) values (\"" << GetLogin() << "\", \"" << GetEmail() << "\", \"" << GetType() << "\", 'N', \"" << GetLng() << "\", now(), \"" << (GetPartnerID().length() ? GetPartnerID() : "0") << "\", \"0\", \"" << GetIP() << "\", \"" << GetPhone() << "\", NOW(), \"" << GetCV() << "\")";
	newUserID = db->InsertQuery(ost.str());

	if(newUserID)
	{
		int		passwdCount;

		SetID(to_string(newUserID));

		// --- safety check
		// --- `users_passwd` should not contain any passwords for a new user before the user actually created
		passwdCount = db->Query("SELECT `passwd` FROM `users_passwd` WHERE `userID`=\"" + to_string(newUserID) + "\";");
		if(passwdCount)
		{
			MESSAGE_ERROR("CUser", "", "new user(" + to_string(newUserID) + ") having " + to_string(passwdCount) + " password(s) set in `users_passwd` table, before password actually created. Here is one of passwords(" + db->Get(0, "passwd") + ")")

			db->Query("DELETE FROM `users_passwd` WHERE `userID`=\"" + to_string(newUserID) + "\";");
		}

		if(db->InsertQuery(
				"INSERT INTO `users_passwd` (`userID`, `passwd`, `isActive`, `eventTimestamp`)"
				"VALUES (\"" + GetID() + "\", \"" + GetPasswd() + "\", \"true\", NOW());"
				)
		)
		{
			// --- update user data with city/country, if detected
			string	country = GetCountry();
			string	city = GetCity();
			string	countryID = "", cityID = "";

			if(country.length())
			{
				if(db->Query("SELECT `id` FROM `geo_country` WHERE `title`=\"" + country + "\";"))
				{
					countryID = db->Get(0, "id");
				}
				else
				{
					unsigned long	temp;

					temp = db->InsertQuery("INSERT INTO `geo_country` SET `title`=\"" + country + "\";");
					if(temp)
						countryID = to_string(temp);
					else
					{
						MESSAGE_ERROR("CUser", "", "fail to insert to table `geo_country`")
					}
				}
			}

			if(city.length())
			{
				if(db->Query("SELECT `id` FROM `geo_locality` WHERE `title`=\"" + city + "\";"))
				{
					cityID = db->Get(0, "id");
				}
				else
				{
					unsigned long	temp;

					temp = db->InsertQuery("INSERT INTO `geo_locality` SET `title`=\"" + city + "\";");
					if(temp)
						cityID = to_string(temp);
					else
					{
						MESSAGE_ERROR("CUser", "", "fail to insert to table `geo_city`")
					}
				}

				if(cityID.length())
				{
					db->Query("UPDATE `users` SET `geo_locality_id`=\"" + cityID + "\" WHERE `id`=\"" + GetID() + "\";");
					if(db->isError())
					{
						MESSAGE_ERROR("CUser", "", "fail to update table `users` (" + db->GetErrorMessage() + ")")
					}
				}
				else
				{
					MESSAGE_ERROR("CUser", "", "cityID is empty, although cityName defined (" + city + ").")
				}
			}
		}
		else
		{
			MESSAGE_ERROR("CUser", "", "fail to insert to table users_passwd");
		}
	}
	else
	{
		MESSAGE_ERROR("CUser", "", "fail to insert to table `users`");
		throw CExceptionHTML("error db");
	}

	// Email("registered");
}

void CUser::Email(string messageID)
{
	CMailLocal	mail;
	CActivator	activator;

	if(GetVars() == NULL)
	{
		MESSAGE_ERROR("CUser", "", "Vars array must be send to CUser()");
		throw CExceptionHTML("vars array");
	}

	if(GetDB() == NULL)
	{
		MESSAGE_ERROR("CUser", "", "DB-connection must be send to CUser()");
		throw CExceptionHTML("db error parameter");
	}

	if(GetLogin().empty())
	{
		MESSAGE_ERROR("CUser", "", "Login must be set in CUser()");
		throw CExceptionHTML("recipient before template");
	}

	if(messageID.empty())
	{
		MESSAGE_ERROR("CUser", "", "Type of mail message is unknown");
		throw CExceptionHTML("mail template");
	}

	activator.SetDB(GetDB());
	activator.SetUser(GetLogin());
	activator.SetType("reg_user");
	GetVars()->Set("actid", activator.GetID());
	GetVars()->Set("host", getenv("HTTP_HOST"));
	activator.Save();
	mail.Send(GetLogin(), messageID, GetVars(), GetDB());
}

bool CUser::GetFromDBbyLogin(string log)
{
	ostringstream		ost;
	bool				result;

	if(db == NULL) throw CExceptionHTML("error db");

	ost << "SELECT * FROM `users`  \
			INNER JOIN `users_passwd` ON `users`.`id`=`users_passwd`.`userID` \
			WHERE `users`.`login`=\"" << log << "\" AND `users_passwd`.`isActive`='true';";
	if(db->Query(ost.str()) == 0) {
		result = false;
	}
	else{
		SetID(db->Get(0, "id"));
		SetLogin(db->Get(0, "login"));
		SetPasswd(db->Get(0, "passwd"));
		SetEmail(db->Get(0, "email"));
		SetSiteThemeID(db->Get(0, "site_theme_id"));
		SetLng(db->Get(0, "lng"));
		SetIP(db->Get(0, "ip"));
		SetType(db->Get(0, "type"));
		// SetPartnerID(db->Get(0, "partnerid"));
		SetCV(db->Get(0, "cv"));
		SetName(db->Get(0, "name"));
		SetNameLast(db->Get(0, "nameLast"));
		if(LoadAvatar()) {}
		else { MESSAGE_DEBUG("CUser", "", "no avatar for user.id(" + GetID() + ")"); }

		result = true;
	}

	return result;
}

bool CUser::GetFromDBbyID(string id)
{
	ostringstream		ost;
	bool				result;

	if(db == NULL) throw CExceptionHTML("error db");

	ost << "SELECT * FROM `users`  \
			INNER JOIN `users_passwd` ON `users`.`id`=`users_passwd`.`userID` \
			WHERE `users`.`id`=\"" << id << "\" AND `users_passwd`.`isActive`='true';";
	if(db->Query(ost.str()) == 0) {
		result =  false;
	}
	else {
		SetID(db->Get(0, "id"));
		SetLogin(db->Get(0, "login"));
		SetPasswd(db->Get(0, "passwd"));
		SetEmail(db->Get(0, "email"));
		SetSiteThemeID(db->Get(0, "site_theme_id"));
		SetLng(db->Get(0, "lng"));
		SetIP(db->Get(0, "ip"));
		SetType(db->Get(0, "type"));
		SetCV(db->Get(0, "cv"));
		SetName(db->Get(0, "name"));
		SetNameLast(db->Get(0, "nameLast"));
		if(LoadAvatar()) {}
		else { MESSAGE_DEBUG("CUser", "", "no avatar for user.id(" + GetID() + ")"); }

		result = true;
	}

	return result;
}

// --- Get `user`.`id` from DB using `sessions`.`user`
// --- basically checking if users still exists in DB if session has been stored on client device
// --- This function differ from FullVersion that did not get the password from `users_passwd`
bool CUser::GetFromDBbyEmailNoPassword(string email)
{
	ostringstream		ost;
	bool				result = false;

	MESSAGE_DEBUG("CUser", "", "start");

	if(db == NULL)
	{
		MESSAGE_ERROR("CUser", "", "db must be initialized");

		throw CExceptionHTML("error db");
	}

	if(db->Query("SELECT * FROM `users` WHERE `email`=\"" + email + "\";") == 0)
	{
		MESSAGE_DEBUG("CUser", "", "user.email [" + email + "] not found");
	}
	else
	{
		SetID(db->Get(0, "id"));
		SetLogin(db->Get(0, "login"));
		SetEmail(db->Get(0, "email"));
		SetSiteThemeID(db->Get(0, "site_theme_id"));
		SetLng(db->Get(0, "lng"));
		SetIP(db->Get(0, "ip"));
		SetType(db->Get(0, "type"));
		SetCV(db->Get(0, "cv"));
		SetName(db->Get(0, "name"));
		SetNameLast(db->Get(0, "nameLast"));
		if(LoadAvatar()) {}
		else { MESSAGE_DEBUG("CUser", "", "no avatar for user.id(" + GetID() + ")"); }
		result = true;
	}

	MESSAGE_DEBUG("CUser", "", "finish (return " + (result ? string("true") : string("false")) + ")");
	return result;
}

// --- Get `user`.`id` from DB using `sessions`.`user`
// --- basically checking if users still exists in DB if session has been stored on client device
bool CUser::GetFromDBbyEmail(string email)
{
	ostringstream		ost;
	bool				result = false;

	MESSAGE_DEBUG("CUser", "", "start");

	if(db == NULL)
	{
		MESSAGE_ERROR("CUser", "", "db must be initialized");

		throw CExceptionHTML("error db");
	}

	if(GetFromDBbyEmailNoPassword(email))
	{
		if(db->Query("SELECT * FROM `users_passwd` WHERE `userID`=\"" + GetID() + "\" AND `users_passwd`.`isActive`='true';") == 0)
		{
			MESSAGE_ERROR("CUser", "", "no active password found for user.id(" + GetID() + ")");
		}
		else
		{
			SetPasswd(db->Get(0, "passwd"));
			result = true;
		}
	}
	else
	{
		MESSAGE_DEBUG("CUser", "", "user not found in GetFromDBbyEmailNoPassword(" + email + ")")
	}

	MESSAGE_DEBUG("CUser", "", "finish (" + (result ? string("true") : string("false")) + ")");

	return result;
}

bool CUser::LoadAvatar()
{
	ostringstream	ost1;
	bool			result = false;
	string			avatarPath = "empty";

	MESSAGE_DEBUG("CUser", "", "start(user.id = " + GetID() + ")");

	// --- Get user avatars
	if(db->Query("select * from `users_avatars` where `userid`=\"" + GetID() + "\" and `isActive`='1';"))
	{
		avatarPath = "/images/avatars/avatars" + string(db->Get(0, "folder")) + "/" + db->Get(0, "filename");
		result = true;
	}
	SetAvatar(avatarPath);

	MESSAGE_DEBUG("CUser", "", "finish");

	return result;
}

bool CUser::UpdatePresence()
{
	bool 			result = true;
	ostringstream	ost;

	{
		MESSAGE_DEBUG("CUser", "", "start(user.id = " + GetID() + ")");
	}

	if(db->Query("update `users` set `last_online`=now(), `last_onlineSecondsSinceY2k`=\"" + to_string(GetSecondsSinceY2k()) + "\" where `id`=\"" + GetID() + "\";"))
	{
		MESSAGE_ERROR("CUser", "", "update presense SQL-query must return 0. It is not zero, means userID [" + GetID() + "] having more than 1 users.");
		result = false;
	}

	{
		MESSAGE_DEBUG("CUser", "", "finish");
	}

	return result;
}

CUser::~CUser()
{
}
