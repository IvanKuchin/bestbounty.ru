#ifndef __CCOOKIE__H__
#define __CCOOKIE__H__

#include <string>
#include <vector>
#include "utilities.h"
#include "cvars.h"

using namespace std;

class CCookie
{
	private:
		string	name, value, domain, path, expiration, secure;
		int		maxAge = -1;
		bool	isNew;		// this parameter control sentback coockie to server
	public:
				CCookie() {};
				CCookie(string n, string v) : name(n), value(v) {};

		void	SetName(const string &s)				{ name = s; }
		void	SetName(string &&s) 		noexcept	{ name = move(s); }
		void	SetValue(const string &s)				{ value = s; }
		void	SetValue(string &&s) 		noexcept	{ value = move(s); }
		void	SetDomain(const string &s)				{ domain = s; }
		void	SetDomain(string &&s) 		noexcept	{ domain = move(s); }
		void	SetPath(const string &s)				{ path = s; }
		void	SetPath(string &&s) 		noexcept	{ path = move(s); }
		void	SetExpiration(const string &s)			{ expiration = s; }
		void	SetExpiration(string &&s) 	noexcept	{ expiration = move(s); }
		void	SetSecure(const string &s)				{ secure = s; }
		void	SetSecure(string &&s) 		noexcept	{ secure = move(s); }
		void	SetMaxAge(const int s)					{ maxAge = s; }
		void	SetNew(bool s);

		string	GetName()					const		{ return name; }
		string	GetValue()					const		{ return value; }
		string	GetDomain()					const		{ return domain; }
		string	GetPath()					const		{ return path; }
		string	GetExpiration()				const		{ return expiration; }
		int		GetMaxAge()					const		{ return maxAge; }
		string	GetSecure()					const		{ return secure; }
		bool	GetNew()					const		{ return isNew; }
};

class CCookies
{
	private:
		vector<CCookie *>	cookies;
		
	public:
				CCookies();

		void	Add(string name, string value, string expiration = "", string domain = "", string path = "", string secure = "", bool newCookie = TRUE);
		void	Add(string name, string value, string expiration = "", int maxage = 0, string domain = "", string path = "", string secure = "", bool newCookie = TRUE);
		string	GetTimestampShifted(int deltaTimeStamp);
		bool	UpdateTS(string name, int deltaTimeStamp);
		void	Modify(string name, string value, string expiration = "", string domain = "", string path = "", string secure = "");
		void	Delete(string name, string domain = "", string path = "", string secure = "");
		string	Get(string name);
		bool	IsExist(string name);
		string	GetAll();
		void	ParseString(string str);
		void	RegisterCookieVariables(CVars *v);

				~CCookies();
};

#endif
