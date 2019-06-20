#include "csession.h"

CSession::CSession() : db(NULL), cookies(NULL)
{
	struct	timeval	tv;

	MESSAGE_DEBUG("", "", "start");

	gettimeofday(&tv, NULL);
	srand(tv.tv_sec * tv.tv_usec * 100000);

	sessID = GetRandom(SESSION_LEN);

	MMDB_usage = false;

	MESSAGE_DEBUG("", "", "end");
}

string CSession::GetRandom(int len)
{
	string	result;
	int	i;

	for(i = 0; i < len; i++)
	{
		result += (char)('0' + (int)(rand()/(RAND_MAX + 1.0) * 10));
	}

	return result;
}

void CSession::InitMaxMind()
{
	{
		MESSAGE_DEBUG("", "", "start");
	}

	MMDB_usage = false;

#ifndef MAXMIND_DISABLE
	if(getenv("REMOTE_ADDR"))
	{
		int		status;

		{
		    status = MMDB_open(MMDB_fname, MMDB_MODE_MMAP, &mmdb);
		}
	    if (status == MMDB_SUCCESS)
	    {
	    	int gai_error, mmdb_error;

			MMDB_usage = true;
			{
				MESSAGE_DEBUG("", "", "MMDB_open(" + MMDB_fname + ") opened succesfully. ");
			}

		    MMDB_result = MMDB_lookup_string(&mmdb, getenv("REMOTE_ADDR"), &gai_error, &mmdb_error);

		    if (0 != gai_error) {
		    	{
					MESSAGE_ERROR("", "", "ERROR: MMDB_open(" + MMDB_fname + ") Error from getaddrinfo for " + getenv("REMOTE_ADDR") + ", " + gai_strerror(gai_error) + ". ");
				}

				MMDB_usage = false;
				MMDB_close(&mmdb);
		    }

		    if (MMDB_SUCCESS != mmdb_error) {
		    	{
					MESSAGE_ERROR("", "", "ERROR: MMDB_open(" + MMDB_fname + ") Got an error from libmaxminddb: " + MMDB_strerror(mmdb_error));
				}

				MMDB_usage = false;
				MMDB_close(&mmdb);
		    }

		    MMDB_entry_data_list = NULL;
		    if (MMDB_result.found_entry) {
		        int status = MMDB_get_entry_data_list(&MMDB_result.entry, &MMDB_entry_data_list);

		        if (MMDB_SUCCESS != status) {
			    	{
						MESSAGE_ERROR("", "", "ERROR: MMDB_open(" + MMDB_fname + ") : Got an error looking up the entry data -" + MMDB_strerror(status));
					}

					MMDB_usage = false;
					MMDB_close(&mmdb);
		        }

		    }


	    }
	    else
	    {
			MESSAGE_ERROR("", "", "ERROR: in reading GeoInfo DB MMDB_open(" + MMDB_fname + ") error is: " + MMDB_strerror(status) + ". ");

			MMDB_usage = false;

	        if (MMDB_IO_ERROR == status)
	        {
				MESSAGE_ERROR("", "", "ERROR: MMDB_open(" + MMDB_fname + ") returned MMDB_IO_ERROR: " + strerror(status) + ". ");
	        }
	    }

	}
	else
	{
	    {
			MESSAGE_DEBUG("", "", "REMOTE_ADDR is empty, no way to determine remote IP");
		}
	}
#endif

	{
		MESSAGE_DEBUG("", "", "finish (MMDB_usage = " + to_string(MMDB_usage) + ")");
	}
}

string CSession::DetectItem(string MMDB_itemName) {
	string		item = "";

#ifndef MAXMIND_DISABLE
    MESSAGE_DEBUG("", "", "start (detectItem " + MMDB_itemName + " by IP)");

	if(MMDB_usage)
	{
        MMDB_entry_data_s       MMDB_entryDataS;

		if(MMDB_get_value(&MMDB_result.entry, &MMDB_entryDataS, MMDB_itemName.c_str(), "names", "en", NULL) == MMDB_SUCCESS)
		{
			if(MMDB_entryDataS.has_data && (MMDB_entryDataS.type == MMDB_DATA_TYPE_UTF8_STRING))
			{
				char	buffer[1024];

				if(MMDB_entryDataS.data_size < sizeof(buffer))
				{
				    memcpy(buffer, MMDB_entryDataS.utf8_string, MMDB_entryDataS.data_size);
				    buffer[MMDB_entryDataS.data_size] = 0;
				    item = buffer;
				}
				else
				{
				    MESSAGE_ERROR("", "", "mmdb result is too long");
				}
			}
			else
			{
				MESSAGE_DEBUG("", "", "MMDB_entry has no data");
			}
		}
		else
		{
			MESSAGE_ERROR("", "", "MMDB_get_value returned error");
		}
	}

    MESSAGE_DEBUG("", "", "finish (" + item + ")");
#endif
	return	item;
}

auto CSession::DetectCountry() -> string
{
    MESSAGE_DEBUG("", "", "attempt to detect country by IP");

	return DetectItem("country");
}

auto CSession::DetectCity() -> string
{
    MESSAGE_DEBUG("", "", "attempt to detect city by IP");

	return DetectItem("city");
}

bool CSession::Save(string u, string i, string l)
{
	SetUser(u);
	SetIP(i);
	SetLng(l);
	// --- try to detect IP address on new session
	InitMaxMind();
	return Save();
}

bool CSession::Save()
{
	ostringstream	ost;

	if(!db)
	{
		MESSAGE_ERROR("", "", "ERROR: connect to database in CSession module");

		throw CExceptionHTML("error db");
	}

	if(GetUser().empty())
	{
		MESSAGE_ERROR("", "", " user name must be set in session::Save");

		throw CExceptionHTML("session error");
	}
	if(GetID().empty())
	{
		MESSAGE_ERROR("", "", " id must be set in session::Save");

		throw CExceptionHTML("session error");
	}
	if(GetIP().empty())
	{
		MESSAGE_ERROR("", "", " ip must be set in session::Save");

		throw CExceptionHTML("session error");
	}
	if(GetLng().empty())
	{
		MESSAGE_ERROR("", "", " lng must be set in session::Save");

		throw CExceptionHTML("session error");
	}

	if(db->Query("INSERT INTO `sessions` (`id`, `user`, `country_auto`, `city_auto`, `lng`, `ip`, `time`,`expire` ) VALUES ('" + GetID() + "', '" + GetUser() + "', '" + DetectCountry() + "', '" + DetectCity() + "', '" + GetLng() + "', '" + GetIP() + "', NOW(), '" + to_string(SESSION_LEN * 60) + "')") != 0) {
		MESSAGE_ERROR("", "", "ERROR: in insert SQL-query");

		return false;
	}

	return true;
}



bool CSession::Load(string id)
{
	ostringstream	ost;
	string			currIP;
	bool			result = true;

	{
		MESSAGE_DEBUG("", "", "start");
	}

	if(id.find(",") != string::npos)
	{
		string		ip = (getenv("REMOTE_ADDR") ? getenv("REMOTE_ADDR") : "");

		if(isBotIP(ip))
		{
			MESSAGE_DEBUG("", "", "wrong cookie behavior from search engine bot [" + ip + "]");
		}
		else
		{
			MESSAGE_ERROR("", "", "ERROR: \"sessid\" got the wrong value (" + id + "). (probably bot, check IP in whois DB)");
		}
	}

	if(isExist(id))
	{
		// --- DB should not return NULL value
		// --- all fields must have "not null"-attribute
		SetID(id);
		SetUser(db->Get(0, "user"));
		SetIP(db->Get(0, "ip"));
		SetLng(db->Get(0, "lng"));
		SetExpire(stoi(db->Get(0, "expire")));

		{
			MESSAGE_DEBUG("", "", "session loaded for user [" + GetUser() + "]");
		}
	}
	else
	{
		MESSAGE_DEBUG("", "", "there is no session in DB, session hasn't been loaded from DB");

		result = false;
	}


	{
		MESSAGE_DEBUG("", "", "result (" + to_string(result) + ")");
	}

	return result;
}

bool CSession::CheckConsistency() {
	bool		result = true;

	{
		MESSAGE_DEBUG("", "", "start");
	}
	// --- Check IP address changing
	if(getenv("REMOTE_ADDR"))
	{
		string	currIP = getenv("REMOTE_ADDR");
		string	oldIP = GetIP();

		if(currIP != oldIP)
		{
			if(UpdateIP(currIP))
			{
				MESSAGE_DEBUG("", "", "user (" + GetUser() + ") IP was changed (" + oldIP + " -> " + currIP + ") during the session.");
			}
			else
			{
				MESSAGE_ERROR("", "", "ERROR: fail to update IP (" + oldIP + " -> " + currIP + ")");
			}
		}
	}
	else
	{
		MESSAGE_DEBUG("", "", "environment variable 'REMOTE_ADDR' doesn't defined");
	}

	{
		MESSAGE_DEBUG("", "", "end (result: " + (result ? "true" : "false") + ")");
	}
	return result;
}

bool CSession::Update() {

	if(GetUser() == "")
	{
		MESSAGE_ERROR("", "", "ERROR: session [" + GetID() + "], user is empty");

		return false;
	}

	if(GetID() == "")
	{
		MESSAGE_ERROR("", "", "ERROR: sessionID is empty");

		return false;
	}

	if(db->Query("UPDATE `sessions` SET `time`=NOW(),`http_user_agent`=\""s + (getenv("HTTP_USER_AGENT") ? getenv("HTTP_USER_AGENT") : "") + "\" WHERE `id`=\"" + GetID() + "\";")) 
	{
		MESSAGE_ERROR("", "", "ERROR: session [" + GetID() + "] there are more than one session with the same session ID.");

		return false;
	}

	return true;
}

bool CSession::UpdateIP(string newIP) {
	ostringstream	ost;

	if(GetUser() == "")
	{
		MESSAGE_ERROR("", "", "ERROR: session [" + GetID() + "], user is empty");

		return false;
	}

	if(GetID() == "")
	{
		MESSAGE_ERROR("", "", "ERROR: session ID is empty");

		return false;
	}

	// --- try to detect IP address on session with changed IP-address
	// --- call stack here
	// --- CCgi::SessID_CheckConsistency
	// --- CSession::CheckConsistency
	// --- CSession::UpdateDB (here)
	InitMaxMind();

	if(db->Query("UPDATE `sessions` SET `ip`=\"" + newIP + "\", `country_auto`=\"" + DetectCountry() + "\", `city_auto`=\"" + DetectCity() + "\" WHERE `id`=\"" + GetID() + "\";"))
	{
		MESSAGE_ERROR("", "", "ERROR: session [" + GetID() + "] there are more than one session with the same session ID.");

		return false;
	}
	SetIP(newIP);

	return true;
}

bool CSession::isExist(string id)
{
	ostringstream	ost;
	string		currIP;

	{
		MESSAGE_DEBUG("", "", "start");
	}

	if(!db)
	{
		MESSAGE_ERROR("", "", "db not initialized");

		throw CExceptionHTML("error db");
	}
	if(id.empty())
	{
		MESSAGE_ERROR("", "", "id is empty");

		throw CExceptionHTML("activator error");
	}

	db->Query("delete from `sessions` where `time`<=(NOW()-interval `expire` second) and `expire`>'0'");
	if(db->Query("select * from `sessions` where `id`=\"" + id + "\";"))
	{
	}
	else
	{
		MESSAGE_DEBUG("", "", "finish (false)");

		return false;
	}

	{
		MESSAGE_DEBUG("", "", "finish (true)");
	}

	return true;
}

CSession::~CSession()
{
	{
		MESSAGE_DEBUG("", "", "start");
	}

#ifndef MAXMIND_DISABLE

	if(MMDB_usage) {
		MMDB_close(&mmdb);
	}

#endif

	{
		MESSAGE_DEBUG("", "", "finish");
	}
}




 