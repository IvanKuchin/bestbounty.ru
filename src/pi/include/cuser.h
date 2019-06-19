#ifndef __CUSER__H__
#define __CUSER__H__

#include <stdio.h>

using namespace std;

#include "clog.h"
#include "cmysql.h"
#include "cvars.h"

extern double GetSecondsSinceY2k();

class CUser
{
	private:
		string		id, login, passwd, passwdConfirm, email, lng, ip, agreement, type, partnerID, phone, cv, name, nameLast, lastOnline;
		string		avatar = "empty";
		string		site_theme_id = "";
		string		site_theme = "";
		string		country, city;
		CMysql		*db;
		CVars		*vars;

		bool		LoadAvatar();
	public:
					CUser();
					CUser(string log, string pas, string pasConfirm, string em, string l, string i, string agr, string t, string pID, string ph);

		void		SetDB(CMysql *param)		{ db = param; }
		CMysql*		GetDB			()	const	{ return db; }

		void		SetVars(CVars *param)		{ vars = param; }
		CVars*		GetVars			()	const	{ return vars; }

		bool		isAllowedLng(string p);
		bool		isLoginExist();
		bool		isPasswdError();
		bool		isEmailCorrect();
		bool		isEmailDuplicate();
		bool		isPhoneCorrect();
		bool		isPhoneDuplicate();
		bool		isAgree();
		bool		isTypeCorrect();
		bool		isBlock();
		bool		isActive();

		string		GetSiteTheme	();
		string		GetID			()	const					{ return id; }
		string		GetLogin		()	const					{ return login; }
		string		GetName			()	const					{ return name; }
		string		GetNameLast		()	const					{ return nameLast; }
		string		GetPasswd		()	const					{ return passwd; }
		string		GetSiteThemeID	()	const					{ return site_theme_id; }
		string		GetEmail		()	const					{ return email; }
		string		GetCountry		()	const					{ return country; }
		string		GetCity			()	const					{ return city; }
		string		GetLng			()	const					{ return lng; }
		string		GetIP			()	const					{ return ip; }
		string		GetPhone		()	const					{ return phone; }
		string		GetType			()	const					{ return type; }
		string		GetPartnerID	()	const					{ return partnerID; }
		string		GetCV			()	const					{ return cv; }
		string		GetAvatar		()	const					{ return avatar; }
		string		GetLastOnline	()	const					{ return lastOnline; }

		void		SetLng			(const string &p);
		void		SetID			(const string &p) 			{ id = p; };
		void		SetLogin		(const string &p) 			{ login = p; };
		void		SetName			(const string &p) 			{ name = p; };
		void		SetNameLast		(const string &p) 			{ nameLast = p; };
		void		SetPasswd		(const string &p) 			{ passwd = p; };
		void		SetSiteThemeID	(const string &p) 			{ site_theme_id = p; };
		void		SetEmail		(const string &p) 			{ email = p; };
		void		SetCountry		(const string &p) 			{ country = p; };
		void		SetCity			(const string &p) 			{ city = p; };
		void		SetIP			(const string &p) 			{ ip = p; };
		void		SetPhone		(const string &p) 			{ phone = p; };
		void		SetType			(const string &p) 			{ type = p; };
		void		SetPartnerID	(const string &p) 			{ partnerID = p; };
		void		SetCV			(const string &p) 			{ cv = p; };
		void		SetAvatar		(const string &p) 			{ avatar = p; };
		void		SetLastOnline	(const string &p) 			{ lastOnline = p; };

		void		SetID			(string &&p) 	noexcept	{ id = move(p); };
		void		SetLogin		(string &&p) 	noexcept	{ login = move(p); };
		void		SetName			(string &&p) 	noexcept	{ name = move(p); };
		void		SetNameLast		(string &&p) 	noexcept	{ nameLast = move(p); };
		void		SetPasswd		(string &&p) 	noexcept	{ passwd = move(p); };
		void		SetSiteThemeID	(string &&p) 	noexcept	{ site_theme_id = move(p); };
		void		SetEmail		(string &&p) 	noexcept	{ email = move(p); };
		void		SetCountry		(string &&p) 	noexcept	{ country = move(p); };
		void		SetCity			(string &&p) 	noexcept	{ city = move(p); };
		void		SetIP			(string &&p) 	noexcept	{ ip = move(p); };
		void		SetPhone		(string &&p) 	noexcept	{ phone = move(p); };
		void		SetType			(string &&p) 	noexcept	{ type = move(p); };
		void		SetPartnerID	(string &&p) 	noexcept	{ partnerID = move(p); };
		void		SetCV			(string &&p) 	noexcept	{ cv = move(p); };
		void		SetAvatar		(string &&p) 	noexcept	{ avatar = move(p); };
		void		SetLastOnline	(string &&p) 	noexcept	{ lastOnline = move(p); };

		void		Create();
		void		Block(string reason);
		void		Email(string message);
		bool		UpdatePresence();

		bool		GetFromDBbyLogin(string log);
		bool		GetFromDBbyID(string id);
		bool		GetFromDBbyEmail(string email);
		bool		GetFromDBbyEmailNoPassword(string email);

					~CUser();
};

#endif

