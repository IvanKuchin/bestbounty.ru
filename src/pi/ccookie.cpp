#include "ccookie.h"

/*
Header mismatch:
	Expected: Set-Cookie: name=value; expires=Thu, 24-Apr-2014 22:36:43 GMT; Max-Age=5
	Received: Set-Cookie: name=value; expires=Thu, 24-Apr-2014 22:36:43 GMT
*/

void	CCookie::SetNew(bool s) 
{
	MESSAGE_DEBUG("", "", "name = " + GetName());

	isNew = s;
}

CCookies::CCookies()
{
	cookies.reserve(8);
}

void CCookies::Add(string name, string value, string expiration, string domain, string path, string secure, bool newCookie)
{
	CCookie		*c;

	MESSAGE_DEBUG("", "", "start (add " + name + "=" + value + ";expiration=" + expiration + ";newCookie=" + to_string(newCookie) + ")");

	if((cookies.capacity() - cookies.size()) <= 0)
	{
		MESSAGE_ERROR("", "", "Size of cookie vector is overflow. May be reallocating memory.");
	}

	c = new(CCookie);
	if(!c)
	{
		MESSAGE_ERROR("", "", "error allocating memory (cookie module)");
		throw CException("error allocating memory (cookie module)");
	}

	c->SetName(name);
	c->SetValue(value);
	c->SetExpiration(expiration);
	// c->SetMaxAge(0);
	c->SetDomain(domain);
	c->SetPath(path);
	c->SetSecure(secure);
	c->SetNew(newCookie);

	cookies.push_back(c);

	MESSAGE_DEBUG("", "", "finish (size of cookies=" + to_string(cookies.size()) + " object)");
}

void CCookies::Add(string name, string value, string expiration, int maxAge, string domain, string path, string secure, bool newCookie)
{
	CCookie		*c;

	MESSAGE_DEBUG("", "", "start (" + name + "=" + value + ";exp=" + expiration + ";maxAge=" + to_string(maxAge) + ";domain=" + domain + ";path=" + path + ";secure=" + secure + ";newCookie=" + to_string(newCookie) + ")");

	if((cookies.capacity() - cookies.size()) <= 0)
	{
		MESSAGE_ERROR("", "", "Size of cookie vector is overflowed. Memory reallocating possible. (cookies.capacity()=" + to_string(cookies.capacity()) + " cookies.size()=" + to_string(cookies.size()) + " )");
	}

	c = new(CCookie);
	if(!c)
	{
		MESSAGE_ERROR("", "", "error allocating memory (cookie module)");
		throw CException("error allocating memory (cookie module)");
	}

	c->SetName(name);
	c->SetValue(value);
	c->SetExpiration((expiration == "") ? GetTimestampShifted(maxAge) : expiration);
	c->SetMaxAge(maxAge);
	c->SetDomain(domain);
	c->SetPath(path);
	c->SetSecure(secure);
	c->SetNew(newCookie);

	cookies.push_back(c);

	MESSAGE_DEBUG("", "", "finish (size of cookies=" + to_string(cookies.size()) + " object)");
}

bool CCookies::IsExist(string name)
{
	bool	result;
	vector<CCookie *>::iterator	im;

	result = false;
	for(im = cookies.begin(); im < cookies.end(); ++im)
	{
		if((*im)->GetName() == name)
		{
			result = true;
			break;
		}
	}

	return result;
}

// --- input: deltaTimeStamp in seconds
// --- return: string with shifted timestamp
string CCookies::GetTimestampShifted(int deltaTimeStamp)
{
	time_t		t;
	char		utc_str[100];
	struct tm	*utc_tm;
	string		result = "";

	MESSAGE_DEBUG("", "", " start (deltaTimeStamp = " + to_string(deltaTimeStamp) + ")");

	t = time(NULL) + deltaTimeStamp;
	utc_tm = gmtime(&t);
	if(utc_tm)
	{
		if(strftime(utc_str, sizeof(utc_str), "%a, %02d %b %Y %T %Z", utc_tm))
		{
			result = utc_str;
		} 
		else
		{
			MESSAGE_ERROR("", "", "ERROR: in running strftime)(" + utc_str + ", " + to_string(sizeof(utc_str)) + ", '', utc_tm) buffer size is not enough to convert date to GMT");
		}
	}
	else
	{
		MESSAGE_ERROR("", "", "ERROR: in running gmtime(&t)  (possible problem with cookie expiration in IE)");
	}


	MESSAGE_DEBUG("", "", " return (result = " + result + ")");

	return result;
}

// --- shift timestamp and update cookies
// --- name - cookie name
// --- deltaTimeStamp - delta time in minutes
bool CCookies::UpdateTS(string name, int deltaTimeStamp)
{
	string						value, path, timestamp, domain, secure;
	vector<CCookie *>::iterator	im;
	bool						result = true;

	MESSAGE_DEBUG("", "", " start (name = " + name + ", deltaTimeStamp = " + to_string(deltaTimeStamp) + ")");

	if(IsExist(name)) {
		value = Get(name);
		timestamp = GetTimestampShifted(deltaTimeStamp * 60);
		path = "/";
		domain = "";
		secure = "";

		for(im = cookies.begin(); im < cookies.end(); ++im)
		{
			if((*im)->GetName() == name)
			{
				(*im)->SetValue(value);
				(*im)->SetDomain(domain);
				(*im)->SetPath(path);
				(*im)->SetSecure(secure);
				(*im)->SetNew(true);
				if(deltaTimeStamp)
				{
					(*im)->SetExpiration(timestamp);
					(*im)->SetMaxAge(deltaTimeStamp * 60);
				}
				else
				{
					(*im)->SetExpiration("");
				}
				break;
			}
		}

	}
	else 
	{
		{
			MESSAGE_ERROR("", "", "unable to find cookie (name = " + name + ")");
		}

		result = false;
	} // if(IsExist(name)) 

	MESSAGE_DEBUG("", "", "finish (" + to_string(result) + ")");

	return result;
}

void CCookies::Modify(string name, string value, string expiration, string domain, string path, string secure)
{
	vector<CCookie *>::iterator	im;

	for(im = cookies.begin(); im < cookies.end(); ++im)
	{
		if((*im)->GetName() == name)
		{
			(*im)->SetValue(value);
			(*im)->SetExpiration(expiration);
			(*im)->SetDomain(domain);
			(*im)->SetPath(path);
			(*im)->SetSecure(secure);
			(*im)->SetNew(true);
			break;
		}
	}
}

void CCookies::Delete(string name, string domain, string path, string secure)
{
// TODO delete Jun 1
/*
	time_t		t = time(NULL) - 3600 * 24 * 30 * 12;
	char		date[100];

	memset(date, 0, sizeof(date));
	strftime(date, sizeof(date) - 2, "%a, %d %b %Y %X %Z", localtime(&t));
*/
	for(auto im = cookies.begin(); im < cookies.end(); ++im)
	{
		if((*im)->GetName() == name)
		{
			(*im)->SetValue((*im)->GetValue());
			(*im)->SetExpiration(GetTimestampShifted(-3600 * 24));
			(*im)->SetPath(path);
			(*im)->SetMaxAge(0);
			(*im)->SetNew(TRUE);
//			break;
		}
	}
}

string CCookies::Get(string name)
{
	string result;
	vector<CCookie *>::iterator	im;

	result = "";
	for(im = cookies.begin(); im < cookies.end(); ++im)
	{
		if((*im)->GetName() == name)
		{
			result = (*im)->GetValue();
			break;
		}
	}


	return result;
}

string CCookies::GetAll()
{
	string 						result;
	vector<CCookie *>::iterator	im;
	ostringstream				ost;

	MESSAGE_DEBUG("", "", " start");

	result = "";
	for(im = cookies.begin(); im < cookies.end(); ++im)
	{
		if((*im)->GetNew())// does this cookie needs to be sendback to server ?
		{
			string	temp;

			temp = "Set-Cookie: ";
			temp += (*im)->GetName();
			temp += "=";
			temp += (*im)->GetValue();
			if((*im)->GetExpiration().length() > 0)
			{
				temp += "; ";
				temp += "expires=";
				temp += (*im)->GetExpiration();
			}
			if((*im)->GetMaxAge() >= 0)
			{
				ost.str("");
				ost << "; max-age=" << (*im)->GetMaxAge();
				temp += ost.str();
			}
			if((*im)->GetPath().length() > 0)
			{
				temp += "; ";
				temp += "path=";
				temp += (*im)->GetPath();
			}
			temp += "\n";

			result += temp;

			{
				MESSAGE_DEBUG("", "", " add cookie: " + temp);
			}
		}
	}
	
	if(result.length() > 1)	result.erase(result.length() - 1);

	MESSAGE_DEBUG("", "", " finish (result len = " + to_string(result.length()) + ")");

	return result;
}

void CCookies::RegisterCookieVariables(CVars *v)
{
	vector<CCookie *>::iterator	im;

	MESSAGE_DEBUG("", "", " start ");

	for(im = cookies.begin(); im < cookies.end(); ++im)
		if((*im)->GetName().length())
			v->Add((*im)->GetName(), (*im)->GetValue());
		else
		{
			{
				MESSAGE_ERROR("", "", "ERROR: cookie name is empty (" + (*im)->GetName() + " = " + (*im)->GetValue() + ")");
			}
		}

	MESSAGE_DEBUG("", "", " finish ");
}

void CCookies::ParseString(string str)
{
	string				result, expr, cookName, cookValue;
	string::size_type	exprPos, eqPos;

	MESSAGE_DEBUG("", "", "start ");

	result = str;
	exprPos = result.find(";");

	while((exprPos != string::npos) || (result.length() > 2))
	{
		if(exprPos != string::npos)
		{
			expr = result.substr(0, exprPos);
			result = result.erase(0, exprPos + 1);
		}
		else
		{
			expr = result;
			result.erase(0, result.length());
		}

		eqPos = expr.find("=");
		if(eqPos == string::npos)
		{
			exprPos = result.find(";");
			continue;
		}

		cookName = expr.substr(0, eqPos);
		cookValue = expr.substr(eqPos + 1, expr.length() - eqPos - 1);
		trim(cookName);
		trim(cookValue);

		{
			MESSAGE_DEBUG("", "", "adding cookie: " + cookName + " = " + cookValue);
		}

		if(cookName.length())
			Add(cookName, cookValue, "", "", "", "", FALSE);
		else
		{
			{
				MESSAGE_ERROR("", "", "cookie parsing end-up with empty name() and value(" + cookValue + ")");
			}

		}

		exprPos = result.find(";");
	}

	MESSAGE_DEBUG("", "", "finish");
}

CCookies::~CCookies()
{
	vector<CCookie *>::iterator	iv;

	MESSAGE_DEBUG("", "", " start");

	for(iv = cookies.begin(); iv < cookies.end(); ++iv)
	{
		delete(*iv);
	}

	MESSAGE_DEBUG("", "", " finish");
}
