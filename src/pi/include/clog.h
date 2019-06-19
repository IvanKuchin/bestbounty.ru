#ifndef __CLOG__H__
#define __CLOG__H__

#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/file.h>
#include <errno.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <sstream>
#include <chrono>

using namespace std;

#include "localy.h"
// #include "clock.h"

#define DEBUG		0
#define	WARNING		1
#define	ERROR		2
#define	PANIC		3

#define	CURRENT_LOG_LEVEL			DEBUG

#define	MESSAGE_DEBUG(classname_legacy, action, mess) \
{ \
	CLog	obj; \
	string	pretty = __PRETTY_FUNCTION__;  \
    auto    classname = ""s; \
	auto    action_output = ""s; \
    auto    func_name_finish = pretty.find("(");\
    \
    if(func_name_finish != string::npos)\
    {\
        auto    func_name_start = pretty.find_last_of(" :", func_name_finish);\
        auto    class_name_start = pretty.find_last_of(" ", func_name_finish);\
        auto    class_separator = pretty.find("::");\
\
    	if(\
    	    (func_name_start != string::npos) &&\
    	    (class_name_start != string::npos) &&\
    	    (class_name_start < func_name_finish - 3)\
    	    ) \
    	{ classname = pretty.substr(class_name_start + 1, func_name_start - class_name_start); } \
    	else if(\
    	    (class_separator != string::npos) &&\
    	    (class_name_start == string::npos)\
    	    )\
    	{ classname = pretty.substr(0, class_separator + 2); /* --- this case address constructors / destructors "Class1::~Class1()"  */ } \
    }\
	if(string(action).length()) action_output = " "s + action + ":"; \
\
	obj.Write(DEBUG, classname + string(__func__) + "[" + to_string(__LINE__) + "]" + action_output + " " + mess); \
}

#define	MESSAGE_ERROR(classname_legacy, action, mess) \
{ \
	CLog	obj; \
	string	pretty = __PRETTY_FUNCTION__;  \
    auto    classname = ""s; \
	auto    action_output = ""s; \
    auto    func_name_finish = pretty.find("(");\
    \
    if(func_name_finish != string::npos)\
    {\
        auto    func_name_start = pretty.find_last_of(" :", func_name_finish);\
        auto    class_name_start = pretty.find_last_of(" ", func_name_finish);\
        auto    class_separator = pretty.find("::");\
\
    	if(\
    	    (func_name_start != string::npos) &&\
    	    (class_name_start != string::npos) &&\
    	    (class_name_start < func_name_finish - 3)\
    	    ) \
    	{ classname = pretty.substr(class_name_start + 1, func_name_start - class_name_start); } \
    	else if(\
    	    (class_separator != string::npos) &&\
    	    (class_name_start == string::npos)\
    	    )\
    	{ classname = pretty.substr(0, class_separator + 2); } \
    }\
	if(string(action).length()) action_output = " "s + action + ":"; \
\
	obj.Write(ERROR, classname + string(__func__) + "[" + to_string(__LINE__) + "]" + action_output + " " + mess); \
}

using namespace std::chrono;

class CLog
{
    private:
    	string	fileName;
//	CLock	lock;
    	
		string	SpellLogLevel(int level)
		{
			return  (level == 0 ? "DEBUG" :
				    (level == 1 ? "WARNING" :
				    (level == 2 ? "ERROR" :
				    (level == 3 ? "PANIC" :
				    			 "UNKNOWN"))));
		};

    public:
	CLog(): fileName (LOG_FILE_NAME)
	{
//		lock.Get("log");
	};

	CLog(string fName): fileName (fName)
	{
//		lock.Get("log");
	};

	void Write(int level, const string &mess)
	{
	    if(level < CURRENT_LOG_LEVEL)
		return;

	    FILE	*fh;
	    fh = fopen(fileName.c_str(), "a");
	    if(fh == NULL)
	    {
			cout << "CLog::" << __func__ << "[" << __LINE__ << "]: ERROR: opening log file [" << fileName << "] " << strerror(errno) << "\n";

	        return;
	    }

	    time_t			binTime = time((time_t *)NULL);
	    // char			*time = ctime((const time_t *)&binTime);
	    struct tm		*timeInfo = localtime(&binTime);
	    pid_t			processID = getpid();
	    microseconds	ms = duration_cast< microseconds >(system_clock::now().time_since_epoch());

	    if(timeInfo)
	    {
	    	ostringstream	ost;
	    	string			msCount = to_string(ms.count());
	    	char			localtimeBuffer[80];

	    	memset(localtimeBuffer, 0, sizeof(localtimeBuffer));
	    	strftime(localtimeBuffer, 80 - 1, "%b %d %T", timeInfo);

	    	if(msCount.length() > 6) msCount = msCount.substr(msCount.length() - 6, 6);

			ost << localtimeBuffer << "." << msCount << "[" << processID << "] " << (level ? SpellLogLevel(level) + ":" : "") << mess << endl;
			fprintf(fh, "%s", ost.str().c_str());

			// if(time[strlen(time) - 1] == '\n')
			//     time[strlen(time) - 1] = 0;
			// fprintf(fh, "%s:%d: [%d] %s\n", time, ms.count(), processID, mess.c_str());
	    }
	    else
	    {
			fprintf(fh, "[%d] %s\n", processID, mess.c_str());
	    }

	    fflush(fh);
	    fclose(fh);

//	    lock.UnLock();
	}

	void Write(int level, const string &mess, const string &p1)
	{
		Write(level, mess + " " + p1);
	}

	void Write(int level, const string &mess, const string &p1, const string &p2)
	{
		Write(level, mess + " " + p1 + " " + p2);
	}

	void Write(int level, const string &mess, const string &p1, const string &p2, const string &p3)
	{
		Write(level, mess + " " + p1 + " " + p2 + " " + p3);
	}

	inline void Debug(string action, string message)
	{
		Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]:" + action + ": " + message);
	}

	~CLog()
	{
	}

    private:
	key_t	key;
	int	hSem;
	struct	sembuf sop_lock[2];
	struct	sembuf sop_unlock[1];
};

#endif 