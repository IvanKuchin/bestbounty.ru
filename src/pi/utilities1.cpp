#include "utilities.h"

void crash_handler(int sig)
{
	void	   *array[10];
	size_t		nptrs;
	char	   **strings;

	// get void*'s for all entries on the stack
	nptrs = backtrace(array, 10);

	// print out all the frames to stderr
	fprintf(stderr, "Error: signal %d:\n", sig);
	backtrace_symbols_fd(array, nptrs, STDERR_FILENO);

	{
		CLog  log;
		log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR: core dumped");
	}

	// --- try to print out to the CLog	
	strings = backtrace_symbols(array, nptrs);
	if(strings)
	{
		for(unsigned int i = 0; i < nptrs; i++)
		{
			CLog log;
			log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR: " + string(strings[i]));
		}

		free(strings);
	}

	exit(1);
}



std::string rtrim(std::string& str)
{
	str.erase(str.find_last_not_of(' ')+1);		 //suffixing spaces
	return str;
}

std::string ltrim(std::string& str)
{
	str.erase(0, str.find_first_not_of(' '));	   //prefixing spaces
	return str;
}

std::string trim(std::string& str)
{
	rtrim(str);
	ltrim(str);
	return str;
}

string	quoted(string src)
{
	return '"' + src + '"';
}

string  toLower(string src)
{
	using namespace std::regex_constants;

	string  result = src;
	regex	r1("А");
	regex	r2("Б");
	regex	r3("В");
	regex	r4("Г");
	regex	r5("Д");
	regex	r6("Е");
	regex	r7("Ё");
	regex	r8("Ж");
	regex	r9("З");
	regex	r10("И");
	regex	r11("Й");
	regex	r12("К");
	regex	r13("Л");
	regex	r14("М");
	regex	r15("Н");
	regex	r16("О");
	regex	r17("П");
	regex	r18("Р");
	regex	r19("С");
	regex	r20("Т");
	regex	r21("У");
	regex	r22("Ф");
	regex	r23("Х");
	regex	r24("Ц");
	regex	r25("Ч");
	regex	r26("Ш");
	regex	r27("Щ");
	regex	r28("Ь");
	regex	r29("Ы");
	regex	r30("Ъ");
	regex	r31("Э");
	regex	r32("Ю");
	regex	r33("Я");

	src = regex_replace(src, r1, "а");
	src = regex_replace(src, r2, "б");
	src = regex_replace(src, r3, "в");
	src = regex_replace(src, r4, "г");
	src = regex_replace(src, r5, "д");
	src = regex_replace(src, r6, "е");
	src = regex_replace(src, r7, "ё");
	src = regex_replace(src, r8, "ж");
	src = regex_replace(src, r9, "з");
	src = regex_replace(src, r10, "и");
	src = regex_replace(src, r11, "й");
	src = regex_replace(src, r12, "к");
	src = regex_replace(src, r13, "л");
	src = regex_replace(src, r14, "м");
	src = regex_replace(src, r15, "н");
	src = regex_replace(src, r16, "о");
	src = regex_replace(src, r17, "п");
	src = regex_replace(src, r18, "р");
	src = regex_replace(src, r19, "с");
	src = regex_replace(src, r20, "т");
	src = regex_replace(src, r21, "у");
	src = regex_replace(src, r22, "ф");
	src = regex_replace(src, r23, "х");
	src = regex_replace(src, r24, "ц");
	src = regex_replace(src, r25, "ч");
	src = regex_replace(src, r26, "ш");
	src = regex_replace(src, r27, "щ");
	src = regex_replace(src, r28, "ь");
	src = regex_replace(src, r29, "ы");
	src = regex_replace(src, r30, "ъ");
	src = regex_replace(src, r31, "э");
	src = regex_replace(src, r32, "ю");
	src = regex_replace(src, r33, "я");

	transform(src.begin(), src.end(), result.begin(), (int(*)(int))tolower);

	return result;
}

string	GetPasswordNounsList(CMysql *db)
{
	string 	result = "";
	int		affected;

	MESSAGE_DEBUG("", "", "start");

	affected = db->Query("SELECT COUNT(*) as `total` FROM `password_dictionary_nouns`;");
	if(affected)
	{
		// int		total_number_of_words = atoi(db->Get(0, "total")) - 1;
		auto		total_number_of_words = stoi(db->Get(0, "total")) - 1;

		affected = db->Query("SELECT * FROM `password_dictionary_nouns` WHERE `id` in (round(rand()*" + to_string(total_number_of_words) + ") + 1, round(rand()*" + to_string(total_number_of_words) + ") + 1,round(rand()*" + to_string(total_number_of_words) + ") + 1,round(rand()*" + to_string(total_number_of_words) + ") + 1,round(rand()*" + to_string(total_number_of_words) + ") + 1,round(rand()*" + to_string(total_number_of_words) + ") + 1,round(rand()*" + to_string(total_number_of_words) + ") + 1)");
		for(int i = 0; i < affected; ++i)
		{
			if(result.length()) result += ",";
			result += string("\"") + db->Get(i, "word") + "\"";
		}
	}
	else
	{
		MESSAGE_ERROR("", "", "password_dictionary_nouns is empty");
	}

	MESSAGE_DEBUG("", "", "finish (result length is " + to_string(result.length()) + ")");

	return result;
}

string	GetPasswordCharacteristicsList(CMysql *db)
{
	string 	result = "";
	int		affected;

	MESSAGE_DEBUG("", "", "start");

	affected = db->Query("SELECT COUNT(*) as `total` FROM `password_dictionary_characteristics`;");
	if(affected)
	{
		// int		total_number_of_words = atoi(db->Get(0, "total"));
		auto		total_number_of_words = stoi(db->Get(0, "total"));

		affected = db->Query("SELECT * FROM `password_dictionary_characteristics` WHERE `id` in (round(rand()*" + to_string(total_number_of_words) + ") + 1, round(rand()*" + to_string(total_number_of_words) + ") + 1,round(rand()*" + to_string(total_number_of_words) + ") + 1,round(rand()*" + to_string(total_number_of_words) + ") + 1,round(rand()*" + to_string(total_number_of_words) + ") + 1,round(rand()*" + to_string(total_number_of_words) + ") + 1,round(rand()*" + to_string(total_number_of_words) + ") + 1)");
		for(int i = 0; i < affected; ++i)
		{
			if(result.length()) result += ",";
			result += string("\"") + db->Get(i, "word") + "\"";
		}
	}
	else
	{
		MESSAGE_ERROR("", "", "password_dictionary_characteristics is empty");
	}

	MESSAGE_DEBUG("", "", "finish (result length is " + to_string(result.length()) + ")");

	return result;
}

string	GetPasswordAdjectivesList(CMysql *db)
{
	string 	result = "";
	int		affected;

	MESSAGE_DEBUG("", "", "start");

	affected = db->Query("SELECT COUNT(*) as `total` FROM `password_dictionary_adjectives`;");
	if(affected)
	{
		// int		total_number_of_words = atoi(db->Get(0, "total"));
		auto		total_number_of_words = stoi(db->Get(0, "total"));

		affected = db->Query("SELECT * FROM `password_dictionary_adjectives` WHERE `id` in (round(rand()*" + to_string(total_number_of_words) + ") + 1, round(rand()*" + to_string(total_number_of_words) + ") + 1,round(rand()*" + to_string(total_number_of_words) + ") + 1,round(rand()*" + to_string(total_number_of_words) + ") + 1,round(rand()*" + to_string(total_number_of_words) + ") + 1,round(rand()*" + to_string(total_number_of_words) + ") + 1,round(rand()*" + to_string(total_number_of_words) + ") + 1)");
		for(int i = 0; i < affected; ++i)
		{
			if(result.length()) result += ",";
			result += string("\"") + db->Get(i, "word") + "\"";
		}
	}
	else
	{
		MESSAGE_ERROR("", "", "password_dictionary_adjectives is empty");
	}

	MESSAGE_DEBUG("", "", "finish (result length is " + to_string(result.length()) + ")");

	return result;
}

string GetRandom(int len)
{
	string	result;
	int	i;

	for(i = 0; i < len; i++)
	{
		result += (char)('0' + (int)(rand()/(RAND_MAX + 1.0) * 10));
	}

	return result;
}

string GetDefaultActionLoggedinUser(void)
{
	MESSAGE_DEBUG("", "", "start");

	MESSAGE_DEBUG("", "", "finish");

	return LOGGEDIN_USER_DEFAULT_ACTION;
}

string DeleteHTML(string src, bool removeBR /* = true*/)
{
	string			  result;
	string::size_type   firstPos, lastPos;

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: start (src.len = " + to_string(src.length()) + ")");
	}

	result = src;
	firstPos = result.find("<");
	if(firstPos != string::npos)
	{
		lastPos = result.find(">", firstPos);
		if(lastPos == string::npos) lastPos = result.length();

		while(firstPos != string::npos)
		{
			if(removeBR)
				result.erase(firstPos, lastPos - firstPos + 1);
			else
			{
				string  htmlTag; 

				// --- this will run in case "keep BR"
				htmlTag = result.substr(firstPos + 1, lastPos-firstPos-1);
				transform(htmlTag.begin(), htmlTag.end(),htmlTag.begin(), ::tolower);

				if(htmlTag != "br")
				{
					result.erase(firstPos, lastPos - firstPos + 1);
				}
				else
				{
					{
						CLog	log;
						log.Write(DEBUG, string(__func__) + "(" + src + ")" + "[" + to_string(__LINE__) + "]: keep <br> at pos: " + to_string(firstPos));
					}
				}
			}

			firstPos = result.find("<", firstPos + 1);
			if(firstPos == string::npos) break;
			lastPos = result.find(">", firstPos);
			if(lastPos == string::npos) lastPos = result.length();
		}
	} // --- if "<" fount in srcStr

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: finish(result.len = " + to_string(result.length()) + ")");
	}

	return result;
}

/*
	Delete symbol " from string src
*/
string RemoveQuotas(string src)
{
	string		result = src;
	string::size_type	pos = 0;

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: start (src = " + src + ")");
	}

	while((pos = result.find("\"", pos)) != string::npos)
	{
		result.replace(pos, 1, "\\\"");
		pos += 2;
	}

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: finish (result = " + result + ")");
	}

	return result;
}

/*
	Delete special symbols like \t \\ \<
*/
string RemoveSpecialSymbols(string src)
{
	string		result = src;
	string::size_type	pos = 0;

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: start (src = " + src + ")");
	}

	while ((result.length()) && (result.at(result.length() - 1) == '\\')) result.replace(result.length() - 1, 1, "");

	pos = 0;
	while((pos = result.find("\\", pos)) != string::npos)
	{
		result.replace(pos, 2, "");
		pos += 2;
	}


	pos = 0;
	while((pos = result.find("\t", pos)) != string::npos)
	{
		result.replace(pos, 1, " ");
	}

	pos = 0;
	while((pos = result.find("№", pos)) != string::npos)
	{
		result.replace(pos, 1, "N");
		pos += 1;
	}

	pos = 0;
	while((pos = result.find("—", pos)) != string::npos)
	{
		result.replace(pos, 1, "-");
		pos += 1;
	}


	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: finish (result = " + result + ")");
	}

	return result;
}

/*
	Delete special symbols like \t \\ \<
*/
string RemoveSpecialHTMLSymbols(string src)
{
	string		result = src;
	string::size_type	pos = 0;

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: start");
	}

	while ((result.length()) && (result.at(result.length() - 1) == '\\')) result.replace(result.length() - 1, 1, "");

	pos = 0;
	while((pos = result.find("\\", pos)) != string::npos)
	{
		result.replace(pos, 1, "&#92;");
	}


	pos = 0;
	while((pos = result.find("\t", pos)) != string::npos)
	{
		result.replace(pos, 1, " ");
	}

	pos = 0;
	while((pos = result.find("<", pos)) != string::npos)
	{
		result.replace(pos, 1, "&lt;");
	}

	pos = 0;
	while((pos = result.find(">", pos)) != string::npos)
	{
		result.replace(pos, 1, "&gt;");
	}

	pos = 0;
	while((pos = result.find("№", pos)) != string::npos)
	{
		result.replace(pos, 1, "&#35;");
	}

	pos = 0;
	while((pos = result.find("—", pos)) != string::npos)
	{
		result.replace(pos, 1, "-");
	}

	pos = 0;
	while((pos = result.find("\"", pos)) != string::npos)
	{
		result.replace(pos, 1, "&quot;");
	}


	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: finish (result.length = " + to_string(result.length()) + ")");
	}

	return result;
}

/*
	Change " symbol to " from string src
*/
string ReplaceDoubleQuoteToQuote(string src)
{
	string		result = src;
	string::size_type	pos = 0;

	while((pos = result.find("\"", pos)) != string::npos)
	{
		result.replace(pos, 1, "'");
		// pos += 2;
	}

	return result;
}

/*
	Change CR/CRLF symbol to <BR> from string src
*/
string ReplaceCRtoHTML(string src)
{
	string		result = src;
	string::size_type	pos = 0;

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: start");
	}


	pos = 0;
	while((pos = result.find("\r\n", pos)) != string::npos)
	{
		result.replace(pos, 2, "<br>");
		// pos += 1;
	}

	pos = 0;
	while((pos = result.find("\n", pos)) != string::npos)
	{
		result.replace(pos, 1, "<bR>");
	}

	pos = 0;
	while((pos = result.find("\r", pos)) != string::npos)
	{
		result.replace(pos, 1, "<Br>");
	}


	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: finish");
	}

	return result;
}

string CleanUPText(const string messageBody, bool removeBR/* = true*/)
{
	string	  result = messageBody;

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + string("[") + to_string(__LINE__) + string("]: start"));
	}

	result = DeleteHTML(result, removeBR);
	result = ReplaceDoubleQuoteToQuote(result);
	result = ReplaceCRtoHTML(result);
	result = RemoveSpecialSymbols(result);
	trim(result);

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + string("[") + to_string(__LINE__) + string("]: end"));
	}

	return result;
}

/*
	Delete any special symbols
	Used only for matching duplicates
*/
string RemoveAllNonAlphabetSymbols(string src)
{
	string		result = src;
	string::size_type	pos = 0;

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + string("[") + to_string(__LINE__) + string("]: start"));
	}

	while ((result.length()) && (result.at(result.length() - 1) == '\\')) result.replace(result.length() - 1, 1, "");

	pos = 0;
	while((pos = result.find("&lt;", pos)) != string::npos)
	{
		result.replace(pos, 4, "");
	}
	pos = 0;
	while((pos = result.find("&gt;", pos)) != string::npos)
	{
		result.replace(pos, 4, "");
	}
	pos = 0;
	while((pos = result.find("&quot;", pos)) != string::npos)
	{
		result.replace(pos, 6, "");
	}
	pos = 0;
	while((pos = result.find("&#92;", pos)) != string::npos)
	{
		result.replace(pos, 5, "");
	}
	pos = 0;
	while((pos = result.find(" ", pos)) != string::npos)
	{
		result.replace(pos, 1, "");
	}
	pos = 0;
	while((pos = result.find("\\", pos)) != string::npos)
	{
		result.replace(pos, 1, "");
	}
	pos = 0;
	while((pos = result.find("/", pos)) != string::npos)
	{
		result.replace(pos, 1, "");
	}
	pos = 0;
	while((pos = result.find("\t", pos)) != string::npos)
	{
		result.replace(pos, 1, "");
	}

	pos = 0;
	while((pos = result.find("<", pos)) != string::npos)
	{
		result.replace(pos, 1, "");
	}

	pos = 0;
	while((pos = result.find(">", pos)) != string::npos)
	{
		result.replace(pos, 1, "");
	}

	pos = 0;
	while((pos = result.find("№", pos)) != string::npos)
	{
		result.replace(pos, 1, "");
	}

	pos = 0;
	while((pos = result.find("—", pos)) != string::npos)
	{
		result.replace(pos, 1, "");
	}

	pos = 0;
	while((pos = result.find("\"", pos)) != string::npos)
	{
		result.replace(pos, 1, "");
	}

	pos = 0;
	while((pos = result.find("'", pos)) != string::npos)
	{
		result.replace(pos, 1, "");
	}
	pos = 0;
	while((pos = result.find(";", pos)) != string::npos)
	{
		result.replace(pos, 1, "");
	}
	pos = 0;
	while((pos = result.find(":", pos)) != string::npos)
	{
		result.replace(pos, 1, "");
	}


	pos = 0;
	while((pos = result.find("`", pos)) != string::npos)
	{
		result.replace(pos, 1, "");
	}

	pos = 0;
	while((pos = result.find(".", pos)) != string::npos)
	{
		result.replace(pos, 1, "");
	}

	pos = 0;
	while((pos = result.find(",", pos)) != string::npos)
	{
		result.replace(pos, 1, "");
	}

	pos = 0;
	while((pos = result.find("%", pos)) != string::npos)
	{
		result.replace(pos, 1, "");
	}

	pos = 0;
	while((pos = result.find("-", pos)) != string::npos)
	{
		result.replace(pos, 1, "");
	}
	pos = 0;
	while((pos = result.find("N", pos)) != string::npos)
	{
		result.replace(pos, 1, "");
	}


	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + string("[") + to_string(__LINE__) + string("]: end ( result length = " + to_string(result.length()) + ")"));
	}

	return result;
}


string ConvertTextToHTML(const string messageBody)
{
	string 		result = messageBody;

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + string("[") + to_string(__LINE__) + string("]: start"));
	}

	result = RemoveSpecialHTMLSymbols(result);
	result = DeleteHTML(result);
	result = ReplaceCRtoHTML(result);
	trim(result);

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + string("[") + to_string(__LINE__) + string("]: end ( result length = " + to_string(result.length()) + ")"));
	}

	return result;
}

string CheckHTTPParam_Text(string srcText)
{
	char	convertBuffer[16384];
	string	result = "";

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + string("[") + to_string(__LINE__) + string("]: start param(" + srcText + ")"));
	}

	memset(convertBuffer, 0, sizeof(convertBuffer));
	convert_utf8_to_windows1251(srcText.c_str(), convertBuffer, sizeof(convertBuffer) - 1);
	result = ConvertTextToHTML(convertBuffer);

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + string("[") + to_string(__LINE__) + string("]: end ( result length = " + to_string(result.length()) + ")"));
	}
	
	return	result;
}

string CheckHTTPParam_Number(string srcText)
{
	string	result = "";

	{
		MESSAGE_DEBUG("", "", "start param(" + srcText + ")");
	}

	if(srcText.length())
	{
		smatch	m;
		regex	r("[^0-9]");

		if(regex_search(srcText, m, r))
		{
			MESSAGE_ERROR("", "", "can't convert(" + srcText + ") to number");
		}
		else
		{
			result = srcText;
		}
	}

	{
		MESSAGE_DEBUG("", "", "end ( result length = " + to_string(result.length()) + ")");
	}

	return	result;
}

string CheckHTTPParam_Float(const string &srcText)
{
	string	result = "";

	{
		MESSAGE_DEBUG("", "", "start param(" + srcText + ")");
	}

	if(srcText.length())
	{
		regex	r("^[0-9]+\\.?[0-9]+$");

		if(regex_match(srcText, r))
		{
			result = srcText;
		}
		else
		{
			MESSAGE_ERROR("", "", "can't convert(" + srcText + ") to float");
		}
	}

	{
		MESSAGE_DEBUG("", "", "finish ( result length = " + to_string(result.length()) + ")");
	}

	return	result;
}

string CheckHTTPParam_Date(const string &srcText)
{
	string	result = "";

	{
		MESSAGE_DEBUG("", "", "start param(" + srcText + ")");
	}

	if(srcText.length())
	{
		regex	r("^([[:digit:]]{1,2})\\/([[:digit:]]{1,2})\\/([[:digit:]]{2,4})$");
		smatch	sm;

		if(regex_match(srcText, sm, r))
		{
			auto	date	= stoi(sm[1]);
			auto	month	= stoi(sm[2]);
			auto	year	= stoi(sm[3]);

			if(year < 100) year += 2000;

			if((1 <= date) && (date <= 31))
			{
				if((1 <= month) && (month <= 12))
				{
					if((1900 <= year) && (year <= 2100))
					{
						auto tm_obj = GetTMObject(srcText);

						mktime(&tm_obj);

						if((date == tm_obj.tm_mday) && (month == (tm_obj.tm_mon + 1)) && (year == (tm_obj.tm_year + 1900)))
						{
							result = srcText;
						}
						else
						{
							MESSAGE_ERROR("", "", "wrong date (" + srcText + " -> " + to_string(date) + "/" + to_string(month) + "/" + to_string(year) + " vs " + to_string(tm_obj.tm_mday) + "/" + to_string(tm_obj.tm_mon + 1) + "/" + to_string(tm_obj.tm_year + 1900) + ")");
						}
					}
					else
					{
						MESSAGE_ERROR("", "", "year (" + to_string(year) + ") is out of range");
					}
				}
				else
				{
					MESSAGE_ERROR("", "", "month (" + to_string(month) + ") is out of range");
				}
			}
			else
			{
				MESSAGE_ERROR("", "", "date (" + to_string(date) + ") is out of range");
			}
		}
		else
		{
			MESSAGE_ERROR("", "", "(" + srcText + ") doesn't match date regex");
		}
	}

	{
		MESSAGE_DEBUG("", "", "finish (result length = " + to_string(result.length()) + ")");
	}

	return	result;
}

string CheckHTTPParam_Email(string srcText)
{
	char		convertBuffer[16384];
	string		result = "";

    regex       positionRegex(".*([+-][[:digit:]]+\\.[[:digit:]]+)([+-][[:digit:]]+\\.[[:digit:]]+)([+-][[:digit:]]+\\.[[:digit:]]+).*");
    smatch      matchResult;

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + string("[") + to_string(__LINE__) + string("]: start param(" + srcText + ")"));
	}

	memset(convertBuffer, 0, sizeof(convertBuffer));
	convert_utf8_to_windows1251(srcText.c_str(), convertBuffer, sizeof(convertBuffer) - 1);
	result = ConvertTextToHTML(convertBuffer);

	if(regex_match(srcText, regex("^[._!&#*[:alnum:]]+@[.[:alnum:]]+.[[:alnum:]]{2,5}$") ))
    {
    	// --- regex matched
    }
    else
    {
		{
			CLog	log;
			log.Write(DEBUG, string(__func__) + string("[") + to_string(__LINE__) + string("]: email doesn't match regex " + result));
		}

    	result = "";
    }

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + string("[") + to_string(__LINE__) + string("]: end ( result length = " + to_string(result.length()) + ")"));
	}
	
	return	result;
}

string CheckIfFurtherThanNow(string occupationStart_cp1251) 
{
	time_t	  now_t, checked_t;
	// char		utc_str[100];
	struct tm   *local_tm, check_tm;
	ostringstream	ost;

	{
		CLog	log;
		ostringstream	ost;

		ost.str("");
		ost << "CheckIfFurtherThanNow(" << occupationStart_cp1251 << "): start";
		log.Write(DEBUG, ost.str());
	}


	now_t = time(NULL);
	local_tm = localtime(&now_t);
	if(local_tm == NULL) 
	{
		CLog	log;
		ostringstream	ost;

		ost.str("");
		ost << "CheckIfFurtherThanNow(now): ERROR in running localtime(&t)";
		log.Write(ERROR, ost.str());
	}

	{
		CLog	log;
		ostringstream	ost;

		ost.str("");
		ost << "CheckIfFurtherThanNow(now): now_t = " << now_t;
		log.Write(DEBUG, ost.str());
	}


	// now2_t = time(NULL);
	// check_tm = localtime(&now2_t);
	sscanf(occupationStart_cp1251.c_str(), "%4d-%2d-%2d", &check_tm.tm_year, &check_tm.tm_mon, &check_tm.tm_mday);
	check_tm.tm_year -= 1900;
	check_tm.tm_mon -= 1;
	check_tm.tm_hour = 23;
	check_tm.tm_min = 59;
	check_tm.tm_isdst = 0;	// --- Summer time is OFF. Be carefull with it.

	{
		CLog	log;
		ostringstream	ost;

		ost.str("");
		ost << "CheckIfFurtherThanNow(" << occupationStart_cp1251 << "): checked year = " << check_tm.tm_year << " checked month = " << check_tm.tm_mon << " checked day = " << check_tm.tm_mday << "";
		log.Write(DEBUG, ost.str());
	}

	checked_t = mktime(&check_tm);

	{
		CLog	log;
		ostringstream	ost;
		char	buffer[80];

		ost.str("");
		strftime(buffer,80,"check_tm: date regenerated: %02d-%b-%Y %T %Z  %I:%M%p.", &check_tm);
		ost << "CheckIfFurtherThanNow(" << occupationStart_cp1251 << "): " << buffer << "";
		log.Write(DEBUG, ost.str());

		memset(buffer, 0, 80);
		strftime(buffer,80,"local_tm: date regenerated: %02d-%b-%Y %T %Z  %I:%M%p.", local_tm);
		ost.str("");
		ost << "CheckIfFurtherThanNow(" << occupationStart_cp1251 << "): " << buffer << "";
		log.Write(DEBUG, ost.str());

		ost.str("");
		ost << "CheckIfFurtherThanNow(" << occupationStart_cp1251 << "): difftime( now_t=" << now_t << ", checked_t=" << checked_t << ")";
		log.Write(DEBUG, ost.str());

		ost.str("");
		ost << "CheckIfFurtherThanNow(" << occupationStart_cp1251 << "): difference = " << difftime(now_t, checked_t);
		log.Write(DEBUG, ost.str());
	}

	if(difftime(now_t, checked_t) <= 0)
	{
		CLog	log;
		ostringstream	ost;

		ost.str("");
		ost << "CheckIfFurtherThanNow(" << occupationStart_cp1251 << "): clicked date further in futer than now, therefore considered as a 0000-00-00";
		log.Write(DEBUG, ost.str());

		return "0000-00-00";
	}

	return occupationStart_cp1251;
}

string	GetDefaultActionFromUserType(string role, CMysql *db)
{
	string	result = GUEST_USER_DEFAULT_ACTION;

	MESSAGE_DEBUG("", "", "start");

	if(role == "user")			result = LOGGEDIN_USER_DEFAULT_ACTION;
	if(role == "subcontractor")	result = LOGGEDIN_SUBCONTRACTOR_DEFAULT_ACTION;
	if(role == "agency")		result = LOGGEDIN_AGENCY_DEFAULT_ACTION;
	if(role == "approver")		result = LOGGEDIN_APPROVER_DEFAULT_ACTION;
	if(role == "no role")		result = LOGGEDIN_NOROLE_DEFAULT_ACTION;

	MESSAGE_DEBUG("", "", "finish (result = " + result + ")");

	return result;
}

double GetSecondsSinceY2k()
{
	time_t	  timer;
	// struct tm   y2k = {0};
	double		seconds;
	double		secondsY2kUTC = 946684800;

	// y2k.tm_hour = 0;   y2k.tm_min = 0; y2k.tm_sec = 0;
	// y2k.tm_year = 100; y2k.tm_mon = 0; y2k.tm_mday = 1;

	timer = time(NULL);  /* get current time; same as: timer = time(NULL)  */
	// seconds = difftime(timer,mktime(&y2k));
	seconds = timer - secondsY2kUTC;

	{
		CLog	log;
		ostringstream	ost;

		ost.str("");
		ost << "GetSecondsSinceY2k(): end (seconds since Y2k" << std::to_string(seconds) << ")";
		log.Write(DEBUG, ost.str());
	}

	return seconds;
}

string GetLocalFormattedTimestamp()
{
	time_t	  now_t;
	struct tm   *local_tm;
	char		buffer[80];
	string		result = "";

	now_t = time(NULL);
	local_tm = localtime(&now_t);
	if(local_tm == NULL) 
	{
		CLog	log;
		ostringstream	ost;

		ost.str("");
		ost << "GetLocalFormatedTimestamp(): ERROR in running localtime(&t)";
		log.Write(ERROR, ost.str());
	}
	
	memset(buffer, 0, 80);
	strftime(buffer,80,"%Y-%m-%02d %T", local_tm);
	result = buffer;


	{
		CLog	log;
		ostringstream	ost;

		ost.str("");
		ost << "GetLocalFormatedTimestamp(): end (" << result << ")";
		log.Write(DEBUG, ost.str());
	}

	return result;

}

// --- input format: "2015-06-10 00:00:00"
// --- return: number of second difference from now 
double GetTimeDifferenceFromNow(const string timeAgo)
{
	time_t	  now_t, checked_t;
	// char		utc_str[100];
	struct tm   *local_tm, check_tm;
	ostringstream	ost;
/*
	{
		CLog	log;
		ostringstream	ost;

		ost.str("");
		ost << "GetTimeDifferenceFromNow(" << timeAgo << "): start";
		log.Write(DEBUG, ost.str());
	}
*/

	now_t = time(NULL);
	local_tm = localtime(&now_t);
	if(local_tm == NULL) 
	{
		CLog	log;
		ostringstream	ost;

		ost.str("");
		ost << "GetTimeDifferenceFromNow(now): ERROR in running localtime(&t)";
		log.Write(ERROR, ost.str());
	}
/*
	{
		CLog	log;
		ostringstream	ost;

		ost.str("");
		ost << "GetTimeDifferenceFromNow(now): now_t = " << now_t;
		log.Write(DEBUG, ost.str());
	}
*/

	// now2_t = time(NULL);
	// check_tm = localtime(&now2_t);
	sscanf(timeAgo.c_str(), "%4d-%2d-%2d %2d:%2d:%2d", &check_tm.tm_year, &check_tm.tm_mon, &check_tm.tm_mday, &check_tm.tm_hour, &check_tm.tm_min, &check_tm.tm_sec);
	check_tm.tm_year -= 1900;
	check_tm.tm_mon -= 1;
	check_tm.tm_isdst = 0;	// --- "Summer time" is OFF. Be carefull with it.
							// --- Russia is not using Daylight saving
							// --- USA is

	// --- For testing purposes try to use the same Daylight saving as in local clock
	// --- Test starts on 9/11. If there is no side effect, comment-out previous line.
	if(local_tm) check_tm.tm_isdst = local_tm->tm_isdst;	// --- "Summer time" is the same as locak clock.

/*
	{
		CLog	log;
		ostringstream	ost;

		ost.str("");
		ost << "GetTimeDifferenceFromNow(" << timeAgo << "): checked year = " << check_tm.tm_year << " checked month = " << check_tm.tm_mon << " checked day = " << check_tm.tm_mday << " checked hour = " << check_tm.tm_hour << " checked min = " << check_tm.tm_min << " checked sec = " << check_tm.tm_sec;
		log.Write(DEBUG, ost.str());
	}
*/
	checked_t = mktime(&check_tm);

	{
		CLog	log;
		ostringstream	ost;
		char	buffer[80];

		ost.str("");
		strftime(buffer,80,"check_tm: date regenerated: %02d-%b-%Y %T %Z  %I:%M%p.", &check_tm);
		ost << "GetTimeDifferenceFromNow(" << timeAgo << "): " << buffer << "";
		log.Write(DEBUG, ost.str());

		memset(buffer, 0, 80);
		strftime(buffer,80,"local_tm: date regenerated: %02d-%b-%Y %T %Z  %I:%M%p.", local_tm);
		ost.str("");
		ost << "GetTimeDifferenceFromNow(" << timeAgo << "): " << buffer << "";
		log.Write(DEBUG, ost.str());

		ost.str("");
		ost << "GetTimeDifferenceFromNow(" << timeAgo << "): difftime( now_t=" << now_t << ", checked_t=" << checked_t << ")";
		log.Write(DEBUG, ost.str());

		ost.str("");
		ost << "GetTimeDifferenceFromNow(" << timeAgo << "): end (difference = " << difftime(now_t, checked_t) << ")";
		log.Write(DEBUG, ost.str());
	}

	return difftime(now_t, checked_t);
}

string GetMinutesDeclension(const int value)
{
	map<int, string> 	mapDeclension = {
		{1, "минуту"},
		{2, "минуты"},
		{3, "минут"}
	};

	string				result;

	if(value % 10 == 0) 						{ result = mapDeclension.at(3); };
	if(value % 10 == 1) 						{ result = mapDeclension.at(1); };
	if((value % 10 >= 2) and (value % 10 <= 4))	{ result = mapDeclension.at(2); };
	if((value % 10 >= 5) and (value % 10 <= 9))	{ result = mapDeclension.at(3); };
	if((value >= 5) and (value <= 20))			{ result = mapDeclension.at(3); };
	return result;
}


string GetHoursDeclension(const int value)
{
	map<int, string> 	mapDeclension = {
		{1, "час"},
		{2, "часа"},
		{3, "часов"}
	};
	string				result;

	if(value % 10 == 0) 						{ result = mapDeclension.at(3); };
	if(value % 10 == 1) 						{ result = mapDeclension.at(1); };
	if((value % 10 >= 2) and (value % 10 <= 4))	{ result = mapDeclension.at(2); };
	if((value % 10 >= 5) and (value % 10 <= 9))	{ result = mapDeclension.at(3); };
	if((value >= 5) and (value <= 20))			{ result = mapDeclension.at(3); };
	return result;
}

string GetDaysDeclension(const int value)
{
	map<int, string> 	mapDeclension = {
		{1, "день"},
		{2, "дня"},
		{3, "дней"}
	};
	string				result;

	if(value % 10 == 0) 						{ result = mapDeclension.at(3); };
	if(value % 10 == 1) 						{ result = mapDeclension.at(1); };
	if((value % 10 >= 2) and (value % 10 <= 4))	{ result = mapDeclension.at(2); };
	if((value % 10 >= 5) and (value % 10 <= 9))	{ result = mapDeclension.at(3); };
	if((value >= 5) and (value <= 20))			{ result = mapDeclension.at(3); };
	return result;
}

string GetMonthsDeclension(const int value)
{
	map<int, string> 	mapDeclension = {
		{1, "месяц"},
		{2, "месяца"},
		{3, "месяцев"}
	};
	string				result;

	if(value % 10 == 0) 						{ result = mapDeclension.at(3); };
	if(value % 10 == 1) 						{ result = mapDeclension.at(1); };
	if((value % 10 >= 2) and (value % 10 <= 4))	{ result = mapDeclension.at(2); };
	if((value % 10 >= 5) and (value % 10 <= 9))	{ result = mapDeclension.at(3); };
	if((value >= 5) and (value <= 20))			{ result = mapDeclension.at(3); };
	return result;
}

string GetYearsDeclension(const int value)
{
	map<int, string> 	mapDeclension = {
		{1, "год"},
		{2, "года"},
		{3, "лет"}
	};
	string				result;

	if(value % 10 == 0) 						{ result = mapDeclension.at(3); };
	if(value % 10 == 1) 						{ result = mapDeclension.at(1); };
	if((value % 10 >= 2) and (value % 10 <= 4))	{ result = mapDeclension.at(2); };
	if((value % 10 >= 5) and (value % 10 <= 9))	{ result = mapDeclension.at(3); };
	if((value >= 5) and (value <= 20))			{ result = mapDeclension.at(3); };
	return result;
}

// --- input format: "2015-06-10 00:00:00"
// --- return: human readable format
string GetHumanReadableTimeDifferenceFromNow (const string timeAgo)
{
	double			seconds = GetTimeDifferenceFromNow(timeAgo);
	double			minutes = seconds / 60;
	double			hours = minutes / 60;
	double			days = hours / 24;
	double			months = days / 30;
	double			years = months / 12;
	ostringstream	ost;

	ost.str("");
	if(years >= 1)
	{
		ost << (int)years << " " << GetYearsDeclension(years);
	} 
	else if(months >= 1)
	{
		ost << (int)months << " " << GetMonthsDeclension(months);
	}
	else if(days >= 1)
	{
		ost << (int)days << " " << GetDaysDeclension(days);
	}
	else if(hours >= 1)
	{
		ost << (int)hours << " " << GetHoursDeclension(hours);
	}
	else
	{
		ost << (int)minutes << " " << GetMinutesDeclension(minutes);
	}

	ost << " назад.";


	// --- commented to reduce logs flooding
	{
		CLog	log;
		ostringstream	ost1;

		ost1.str("");
		ost1 << "string GetHumanReadableTimeDifferenceFromNow (" << timeAgo << "): sec difference (" << seconds << ") human format (" << ost.str() << ")" ;
		log.Write(DEBUG, ost1.str());
	}


	return ost.str();
}


/*
Copyright (c) <YEAR>, <OWNER>
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions a
re met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in th
e documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from t
his software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT L
IMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIG
HT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AN
D ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
int convert_utf8_to_windows1251(const char* utf8, char* windows1251, size_t n)
{
		unsigned int i = 0;
		unsigned int j = 0;
		unsigned int k = 0;

		typedef struct ConvLetter {
				char	win1251;
				int	 unicode;
		} Letter;

		Letter  g_letters[] = {
		{(char)0x82, 0x201A}, // SINGLE LOW-9 QUOTATION MARK
		{(char)0x83, 0x0453}, // CYRILLIC SMALL LETTER GJE
		{(char)0x84, 0x201E}, // DOUBLE LOW-9 QUOTATION MARK
		{(char)0x85, 0x2026}, // HORIZONTAL ELLIPSIS
		{(char)0x86, 0x2020}, // DAGGER
		{(char)0x87, 0x2021}, // DOUBLE DAGGER
		{(char)0x88, 0x20AC}, // EURO SIGN
		{(char)0x89, 0x2030}, // PER MILLE SIGN
		{(char)0x8A, 0x0409}, // CYRILLIC CAPITAL LETTER LJE
		{(char)0x8B, 0x2039}, // SINGLE LEFT-POINTING ANGLE QUOTATION MARK
		{(char)0x8C, 0x040A}, // CYRILLIC CAPITAL LETTER NJE
		{(char)0x8D, 0x040C}, // CYRILLIC CAPITAL LETTER KJE
		{(char)0x8E, 0x040B}, // CYRILLIC CAPITAL LETTER TSHE
		{(char)0x8F, 0x040F}, // CYRILLIC CAPITAL LETTER DZHE
		{(char)0x90, 0x0452}, // CYRILLIC SMALL LETTER DJE
		{(char)0x91, 0x2018}, // LEFT SINGLE QUOTATION MARK
		{(char)0x92, 0x2019}, // RIGHT SINGLE QUOTATION MARK
		{(char)0x93, 0x201C}, // LEFT DOUBLE QUOTATION MARK
		{(char)0x94, 0x201D}, // RIGHT DOUBLE QUOTATION MARK
		{(char)0x95, 0x2022}, // BULLET
		{(char)0x96, 0x2013}, // EN DASH
		{(char)0x97, 0x2014}, // EM DASH
		{(char)0x99, 0x2122}, // TRADE MARK SIGN
		{(char)0x9A, 0x0459}, // CYRILLIC SMALL LETTER LJE
		{(char)0x9B, 0x203A}, // SINGLE RIGHT-POINTING ANGLE QUOTATION MARK
		{(char)0x9C, 0x045A}, // CYRILLIC SMALL LETTER NJE
		{(char)0x9D, 0x045C}, // CYRILLIC SMALL LETTER KJE
		{(char)0x9E, 0x045B}, // CYRILLIC SMALL LETTER TSHE
		{(char)0x9F, 0x045F}, // CYRILLIC SMALL LETTER DZHE
		{(char)0xA0, 0x00A0}, // NO-BREAK SPACE
		{(char)0xA1, 0x040E}, // CYRILLIC CAPITAL LETTER SHORT U
		{(char)0xA2, 0x045E}, // CYRILLIC SMALL LETTER SHORT U
		{(char)0xA3, 0x0408}, // CYRILLIC CAPITAL LETTER JE
		{(char)0xA4, 0x00A4}, // CURRENCY SIGN
		{(char)0xA5, 0x0490}, // CYRILLIC CAPITAL LETTER GHE WITH UPTURN
		{(char)0xA6, 0x00A6}, // BROKEN BAR
		{(char)0xA7, 0x00A7}, // SECTION SIGN
		{(char)0xA8, 0x0401}, // CYRILLIC CAPITAL LETTER IO
		{(char)0xA9, 0x00A9}, // COPYRIGHT SIGN
		{(char)0xAA, 0x0404}, // CYRILLIC CAPITAL LETTER UKRAINIAN IE
		{(char)0xAB, 0x00AB}, // LEFT-POINTING DOUBLE ANGLE QUOTATION MARK
		{(char)0xAC, 0x00AC}, // NOT SIGN
		{(char)0xAD, 0x00AD}, // SOFT HYPHEN
		{(char)0xAE, 0x00AE}, // REGISTERED SIGN
		{(char)0xAF, 0x0407}, // CYRILLIC CAPITAL LETTER YI
		{(char)0xB0, 0x00B0}, // DEGREE SIGN
		{(char)0xB1, 0x00B1}, // PLUS-MINUS SIGN
		{(char)0xB2, 0x0406}, // CYRILLIC CAPITAL LETTER BYELORUSSIAN-UKRAINIAN I
		{(char)0xB3, 0x0456}, // CYRILLIC SMALL LETTER BYELORUSSIAN-UKRAINIAN I
		{(char)0xB4, 0x0491}, // CYRILLIC SMALL LETTER GHE WITH UPTURN
		{(char)0xB5, 0x00B5}, // MICRO SIGN
		{(char)0xB6, 0x00B6}, // PILCROW SIGN
		{(char)0xB7, 0x00B7}, // MIDDLE DOT
		{(char)0xB8, 0x0451}, // CYRILLIC SMALL LETTER IO
		{(char)0xB9, 0x2116}, // NUMERO SIGN
		{(char)0xBA, 0x0454}, // CYRILLIC SMALL LETTER UKRAINIAN IE
		{(char)0xBB, 0x00BB}, // RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK
		{(char)0xBC, 0x0458}, // CYRILLIC SMALL LETTER JE
		{(char)0xBD, 0x0405}, // CYRILLIC CAPITAL LETTER DZE
		{(char)0xBE, 0x0455}, // CYRILLIC SMALL LETTER DZE
		{(char)0xBF, 0x0457} // CYRILLIC SMALL LETTER YI
	};

		typedef struct ConvLetter3Bytes {
				char	win1251;
				char	unicode1;
				char	unicode2;
				char	unicode3;
		} Letter3Bytes;
		Letter3Bytes g_lettersGeneralPunctuation[] = {
			{'-', (char)0xe2, (char)0x80, (char)0x94}, // EM DASH
			{'-', (char)0xe2, (char)0x80, (char)0x93}, // EN DASH
			{'-', (char)0xe2, (char)0x80, (char)0x90}, // HYPHEN
			{'-', (char)0xe2, (char)0x80, (char)0x91}, // NON-BREAKING HYPHEN
			{'-', (char)0xe2, (char)0x80, (char)0x92}, // FIGURE DASH 
			{'-', (char)0xe2, (char)0x80, (char)0x93}, // EN DASH
			{'-', (char)0xe2, (char)0x80, (char)0x94}, // EM DASH
			{'-', (char)0xe2, (char)0x80, (char)0x95}, // HORIZONTAL BAR
			{'|', (char)0xe2, (char)0x80, (char)0x96}, // DOUBLE VERTICAL LINE
			{'_', (char)0xe2, (char)0x80, (char)0x97}, // DOUBLE LOW LINE
			{',', (char)0xe2, (char)0x80, (char)0x9a}, // SINGLE LOW QUOTATION MARK
			{'.', (char)0xe2, (char)0x80, (char)0xa4}, // ONE DOT LEADER
			{'.', (char)0xe2, (char)0x80, (char)0x5}, // TWO DOT LEADER
			{'.', (char)0xe2, (char)0x80, (char)0x6}, // HORIZONTAL ELLIPSIS
			{'<', (char)0xe2, (char)0x80, (char)0xb9}, // SINGLE LEFT-POINTING ANGLE QUOTATION MARK
			{'>', (char)0xe2, (char)0x80, (char)0xba}, // SINGLE RIGHT-POINTING ANGLE QUOTATION MARK
			{'!', (char)0xe2, (char)0x80, (char)0xbc}, // DOUBLE EXCLAMNATION MARK
			{'-', (char)0xe2, (char)0x81, (char)0x83}, // HYPHEN BULLET
			{'/', (char)0xe2, (char)0x81, (char)0x84}, // FRACTION SLASH
			{'?', (char)0xe2, (char)0x81, (char)0x87}, // DOUBLE QUESTION MARK
			{'*', (char)0xe2, (char)0x81, (char)0x8e}, // LOW ASTERISK
			{'|', (char)0xe2, (char)0x83, (char)0x92}, // COMBINING LONG VERTICAL LINE OVERLAY
			{'\\', (char)0xe2, (char)0x83, (char)0xa5}, //COMBINING REVERSE SOLIDUS OVERLAY
			{'|', (char)0xe2, (char)0x83, (char)0xa6}, // COMBINING DOUBLE VERTICAL STROKE OVERLAY
			{'/', (char)0xe2, (char)0x83, (char)0xab}, // COMBINING LONG DOUBLE SOLIDUS OVERLAY
			{'\'', (char)0xe2, (char)0x80, (char)0x9b}, // SINGLE HIGN-REVERSED-9 QUATATION MARK
			{'\\', (char)0xe2, (char)0x80, (char)0xb2}, // PRIME
			{'\\', (char)0xe2, (char)0x80, (char)0xb5}, // REVERSED PRIME
			{'"', (char)0xe2, (char)0x80, (char)0x9c}, // LEFT DOUBLE QUATATION MARK
			{'"', (char)0xe2, (char)0x80, (char)0xb3}, // DOUBLE PRIME
			{'"', (char)0xe2, (char)0x80, (char)0xb6}, // REVERSED DOUBLE PRIME
			{'"', (char)0xe2, (char)0x80, (char)0x9c}, // RIGHT DOUBLE QUATATION MARK
			{'"', (char)0xe2, (char)0x80, (char)0x9e}, // DOUBLE LOW-9 QUATATION MARK
			{'"', (char)0xe2, (char)0x80, (char)0x9f}, // DOUBLE HIGH-REVERSED-9 QUATATION MARK
			{'*', (char)0xe2, (char)0x80, (char)0xA2}, //  BULLET
			{'%', (char)0xe2, (char)0x80, (char)0xb0}, // PER MILLE SIGN
			{'%', (char)0xe2, (char)0x80, (char)0xb1}, // PER TEN THOUSAND SIGN
			{'*', (char)0xe2, (char)0x80, (char)0xa7}, // HYPHENATION POINT
			{'~', (char)0xe2, (char)0x81, (char)0x93}, // SWUNG DASH
			{'*', (char)0xe2, (char)0x81, (char)0x95}, // FLOWER PUNCTION MARK
			{':', (char)0xe2, (char)0x81, (char)0x9a}, // TWO DOT PUNCTION		};
			{'|', (char)0xe2, (char)0x83, (char)0x92}, // COMBINING LONG VERTICAL LINE OVERLAY
			{'\\', (char)0xe2, (char)0x83, (char)0xa5}, // COMBINING REVERSE SOLIDUS OVERLAY
			{'|', (char)0xe2, (char)0x83, (char)0xa6}, // COMBINING DOUBLE VERTICAL STROKE OVERLAY
			{'/', (char)0xe2, (char)0x83, (char)0xab}, // COMBINING LONG DOUBLE SOLIDUS OVERLAY
			{'*', (char)0xe2, (char)0x83, (char)0xb0}, // COMBINING ASTERISK ABOVE
			{'#', (char)0xe2, (char)0x84, (char)0x96}, // number sign
			{'1', (char)0xe2, (char)0x85, (char)0xa0}, // ROMAN NUMERAL ONE 
			{'2', (char)0xe2, (char)0x85, (char)0xa1}, // ROMAN NUMERAL TWO
			{'3', (char)0xe2, (char)0x85, (char)0xa2}, // ROMAN NUMERAL THREE
			{'4', (char)0xe2, (char)0x85, (char)0xa3}, // ROMAN NUMERAL FOUR
			{'5', (char)0xe2, (char)0x85, (char)0xa4}, // ROMAN NUMERAL FIVE
			{'6', (char)0xe2, (char)0x85, (char)0xa5}, // ROMAN NUMERAL SIX
			{'7', (char)0xe2, (char)0x85, (char)0xa6}, // ROMAN NUMERAL SEVEN
			{'8', (char)0xe2, (char)0x85, (char)0xa7}, // ROMAN NUMERAL EIGHT
			{'9', (char)0xe2, (char)0x85, (char)0xa8}, // ROMAN NUMERAL NINE
			{'X', (char)0xe2, (char)0x85, (char)0xa9}, // ROMAN NUMERAL TEN
			{'L', (char)0xe2, (char)0x85, (char)0xac}, // ROMAN NUMERAL FIFTY
			{'C', (char)0xe2, (char)0x85, (char)0xad}, // ROMAN NUMERAL ONE HUNDRED
			{'D', (char)0xe2, (char)0x85, (char)0xae}, // ROMAN NUMERAL FIVE HUNDRED
			{'M', (char)0xe2, (char)0x85, (char)0xaf} // ROMAN NUMERAL ONE THOUSAND
		};
	{
		CLog	log;
		log.Write(DEBUG, __func__ + string("[") + to_string(__LINE__) + string("]:") + string(": start"));
	}


		for(; i < (unsigned int)n && utf8[i] != 0; ++i) {
				char prefix = utf8[i];
				char suffix = utf8[i+1];
				if ((prefix & 0x80) == 0) 
				{
						windows1251[j] = (char)prefix;
						++j;
				} 
				else if ((~prefix) & 0x20) 
				{
						int first5bit = prefix & 0x1F;
						first5bit <<= 6;
						int sec6bit = suffix & 0x3F;
						int unicode_char = first5bit + sec6bit;


						if ( unicode_char >= 0x410 && unicode_char <= 0x44F ) {
								windows1251[j] = (char)(unicode_char - 0x350);
						} else if (unicode_char >= 0x80 && unicode_char <= 0xFF) {
								windows1251[j] = (char)(unicode_char);
						} else if (unicode_char >= 0x402 && unicode_char <= 0x403) {
								windows1251[j] = (char)(unicode_char - 0x382);
						} else {
								unsigned int count = sizeof(g_letters) / sizeof(Letter);
								for (k = 0; k < count; ++k) {
										if (unicode_char == g_letters[k].unicode) {
												windows1251[j] = g_letters[k].win1251;
												goto NEXT_LETTER;
										}
								}
								// can't convert this char
								{
									CLog	log;
									ostringstream	ost;

									ost.str("");
									ost << __func__ << "[" << __LINE__ << "]:" << ": symbol at position " << i << " [" << hex << (prefix & 0xFF) << " " << (suffix & 0xFF) << "] doesn't belongs to UTF-8 cyrillic range (U+0400 ... U+04FF)";
									log.Write(DEBUG, ost.str());
								}
								return 0;
						}
NEXT_LETTER:
						++i;
						++j;
				} 
				else if((prefix == static_cast<char>(0xf0)) and (suffix == static_cast<char>(0x9f)))
				{
					// --- emoji part (f0 9f 98 80 - f0 9f 9b b3) or (U+1F600 - U+1F6FF)
					// --- 4 bytes long
					if((i + 3) < n)
					{
						{
							CLog	log;
							ostringstream   ost;

							ost.str("");
							ost << __func__ << "[" << __LINE__ << "]:" << ": emojy detected [" << hex << (utf8[i] & 0xFF) << " " << (utf8[i+1] & 0xFF) << " " << (utf8[i+2] & 0xFF) << " " << (utf8[i+3] & 0xFF) << "]";
							log.Write(DEBUG, ost.str());
						}
						i += 3;
					}
					else
					{
						{
							CLog	log;
							ostringstream   ost;

							ost.str("");
							ost << __func__ << "[" << __LINE__ << "]:" << ": ERROR: emojy detected but dst string not lenghty enough, dst buffer size(" << n << ") < current position (" << i << "+3)";
							log.Write(ERROR, ost.str());
						}
						return 0;
					}
				}
				else if((prefix == static_cast<char>(0xe2)) && ((suffix & 0x80) == 0x80))
				{
					// --- general punctuation (e2 80 80 - e2 82 bf) or (U+2000 - U+20FF)
					// --- 3 bytes long
					if((i + 2) < n)
					{
						unsigned int	count = sizeof(g_lettersGeneralPunctuation) / sizeof(Letter3Bytes);
						bool			isFound = false;

						for (k = 0; ((k < count) && !isFound); ++k) 
						{
								char currentSymb1 = prefix;
								char currentSymb2 = suffix;
								char currentSymb3 = utf8[i+2];

								if(
									(currentSymb1 == g_lettersGeneralPunctuation[k].unicode1) &&
									(currentSymb2 == g_lettersGeneralPunctuation[k].unicode2) &&
									(currentSymb3 == g_lettersGeneralPunctuation[k].unicode3)
								  )
								{
										windows1251[j] = g_lettersGeneralPunctuation[k].win1251;
										isFound = true;
										i += 2;
										j++;
								}
						}
						if(!isFound)
						{
							// --- unknown symbol
							// --- replace it to space
							windows1251[j] = ' ';
							// --- this is 3 bytes length symbol
							i += 2;
							j++;

							// can't convert this char
							CLog	log;
							ostringstream   ost;

							ost.str("");
							ost << __func__ << "[" << __LINE__ << "]:" << ": symbol at position " << (i - 2) << " [" << hex << (prefix & 0xFF) << " " << (suffix & 0xFF) << " " << (utf8[i+2 - 2] & 0xFF) << "] not found in mapping table Punctuation range (U+0400 ... U+04FF)";
							log.Write(DEBUG, ost.str());
						}
						else
						{
							CLog	log;
							ostringstream   ost;

							ost.str("");
							ost << __func__ << "[" << __LINE__ << "]:" << ": general punctuation detected [" << hex << (prefix & 0xFF) << " " << (suffix & 0xFF) << " " << (utf8[i+2 - 2] & 0xFF) << "] at position " << (i - 2);
							log.Write(DEBUG, ost.str());
						}
					}
					else
					{
						{
							CLog	log;
							ostringstream   ost;

							ost.str("");
							ost << __func__ << "[" << __LINE__ << "]:ERROR: general punctuation detected but dst string not lenghty enough, dst buffer size(" << n << ") < current position (" << i << "+2)";
							log.Write(ERROR, ost.str());
						}
						return 0;
					}

				}
				else
				{
					// can't convert this chars
					{
						CLog	log;
						ostringstream   ost;

						ost.str("");
						ost << __func__ << "[" << __LINE__ << "]:ERROR: can't convert this " << i << "-rd/th char " << hex << (+prefix & 0xFF) << " " << (+suffix & 0xFF);
						log.Write(ERROR, ost.str());
					}
					return 0;
				}
		}
		windows1251[j] = 0;

	{
		CLog	log;
		log.Write(DEBUG, __func__ + string("[") + to_string(__LINE__) + string("]:") + string(": end"));
	}

		return 1;
}

bool convert_cp1251_to_utf8(const char *in, char *out, int size) 
{
	const char table[129*3] = {				 
		"\320\202 \320\203 \342\200\232\321\223 \342\200\236\342\200\246\342\200\240\342\200\241"
		"\342\202\254\342\200\260\320\211 \342\200\271\320\212 \320\214 \320\213 \320\217 "	  
		"\321\222 \342\200\230\342\200\231\342\200\234\342\200\235\342\200\242\342\200\223\342\200\224"
		"   \342\204\242\321\231 \342\200\272\321\232 \321\234 \321\233 \321\237 "					 
		"\302\240 \320\216 \321\236 \320\210 \302\244 \322\220 \302\246 \302\247 "					 
		"\320\201 \302\251 \320\204 \302\253 \302\254 \302\255 \302\256 \320\207 "					 
		"\302\260 \302\261 \320\206 \321\226 \322\221 \302\265 \302\266 \302\267 "
		"\321\221 \342\204\226\321\224 \302\273 \321\230 \320\205 \321\225 \321\227 "
		"\320\220 \320\221 \320\222 \320\223 \320\224 \320\225 \320\226 \320\227 "
		"\320\230 \320\231 \320\232 \320\233 \320\234 \320\235 \320\236 \320\237 "
		"\320\240 \320\241 \320\242 \320\243 \320\244 \320\245 \320\246 \320\247 "
		"\320\250 \320\251 \320\252 \320\253 \320\254 \320\255 \320\256 \320\257 "
		"\320\260 \320\261 \320\262 \320\263 \320\264 \320\265 \320\266 \320\267 "
		"\320\270 \320\271 \320\272 \320\273 \320\274 \320\275 \320\276 \320\277 "
		"\321\200 \321\201 \321\202 \321\203 \321\204 \321\205 \321\206 \321\207 "
		"\321\210 \321\211 \321\212 \321\213 \321\214 \321\215 \321\216 \321\217 "
	};
	int	counter = 0;
	bool result = true;

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: start");
	}

	while (*in)
		if (*in & 0x80) {
			const char *p = &table[3 * (0x7f & *in++)];
			if (*p == ' ')
				continue;
			counter += 2;
			if(counter > size) 
			{
				result = false;
				break;
			}
			*out++ = *p++;
			*out++ = *p++;

			if (*p == ' ')
				continue;

			counter++;
			if(counter > size) 
			{
				result = false;
				break;
			}
			*out++ = *p++;
		}
		else
		{
			counter++;
			if(counter > size) 
			{
				result = false;
				break;
			}
			*out++ = *in++;
		}
	*out = 0;

	if(!result)
	{
		CLog	log;
		log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]: ERROR: buffer size is not enough");
	}

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: end");
	}
	return result;
}

bool isFileExists(const std::string& name) 
{
	struct stat buffer;
	return (stat (name.c_str(), &buffer) == 0);
}

off_t getFileSize(const std::string& name) 
{
	off_t			result = -1;
	struct stat		buffer;

	if(stat(name.c_str(), &buffer) == -1)
	{
		MESSAGE_ERROR("", "", "fail to get file size (" + name + "), probably file doesn't exists");
	}
	else
	{
		result = buffer.st_size;
	}

	return result;
}

string SymbolReplace(const string where, const string src, const string dst)
{
	string				  result;
	string::size_type	   pos;
		
	result = where;
		
	pos = result.find(src);
	while(pos != string::npos)
	{
		result.replace(pos, src.length(), dst);
		pos = result.find(src, pos + 1);
	}
	return result;
}

string SymbolReplace_KeepDigitsOnly(const string where)
{
	string				  result = where;
	unsigned int			i = 0;

	while(i < result.length())
	{
		char	currSymb = result.at(i);

		if((currSymb >= static_cast<char>('0')) && (currSymb <= static_cast<char>('9')))
			i++;
		else
			result.replace(i, 1, "");
	}

	return result;
}

bool CheckUserEmailExisting(string userNameToCheck, CMysql *db) {
	CUser		user;

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: start");
	}

	user.SetDB(db);
	user.SetLogin(userNameToCheck);
	user.SetEmail(userNameToCheck);

	if(user.isLoginExist() or user.isEmailDuplicate()) {
		{
			CLog	log;
			log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: login or email already registered");
		}
		return true;
	}
	else {
		{
			CLog	log;
			log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: login or email not yet exists");
		}
		return false;
	}
}

// --- Quote Words: split string into vector<string>
// --- input: string, reference to vector
// --- output: success status
//				1 - success
//				0 - fail
int	qw(const string src, vector<string> &dst)
{
	std::size_t	prevPointer = 0, nextPointer;
	string		trimmedStr = src;
	int			wordCounter = 0;

	trim(trimmedStr);

	prevPointer = 0, wordCounter = 0;
	do
	{
		nextPointer = trimmedStr.find(" ", prevPointer);
		if(nextPointer == string::npos)
		{
			dst.push_back(trimmedStr.substr(prevPointer));
		}
		else
		{
			dst.push_back(trimmedStr.substr(prevPointer, nextPointer - prevPointer));
		}
		prevPointer = nextPointer + 1;
		wordCounter++;
	} while( (nextPointer != string::npos) );

	return 1;
}

vector<string> split(const string& s, const char& c)
{
	string buff{""};
	vector<string> v;

	for(auto n:s)
	{
		if(n != c) buff+=n;
		else if(n == c && buff != "")
		{
			v.push_back(buff);
			buff = "";
		}
	}
	if(buff.length()) v.push_back(buff);

	return v;
}

string UniqueUserIDInUserIDLine(string userIDLine) //-> decltype(static_cast<string>("123"))
{
	list<long int>	listUserID;
	string			result {""};
	std::size_t		prevPointer {0}, nextPointer;

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: start (", userIDLine, ")");
	}

	do
	{
		nextPointer = userIDLine.find(",", prevPointer);
		if(nextPointer == string::npos)
		{
			listUserID.push_back(atol(userIDLine.substr(prevPointer).c_str()));
		}
		else
		{
			listUserID.push_back(atol(userIDLine.substr(prevPointer, nextPointer - prevPointer).c_str()));
		}
		prevPointer = nextPointer + 1;
	} while( (nextPointer != string::npos) );

	listUserID.sort();
	listUserID.unique();
	for(auto it: listUserID)
	{
		result += (result.length() ? "," : "") + to_string(it);
	}

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: end (result ", result, ")");
	}

	return result;	
}

string GetChatMessagesInJSONFormat(string dbQuery, CMysql *db)
{
	ostringstream	result, ost;
	int				affected;

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: start");
	}
	result.str("");

	affected = db->Query(dbQuery);
	if(affected)
	{
		for(int i = 0; i < affected; i++)
		{
			result << (i ? "," : "") << "{\
				\"id\": \""						<< db->Get(i, "id") << "\", \
				\"message\": \"" 				<< ReplaceDoubleQuoteToQuote(db->Get(i, "message")) << "\", \
				\"fromType\": \"" 				<< db->Get(i, "fromType") << "\",\
				\"fromID\": \""					<< db->Get(i, "fromID") << "\",\
				\"toType\": \""			 		<< db->Get(i, "toType") << "\",\
				\"toID\": \""	 				<< db->Get(i, "toID") << "\",\
				\"messageStatus\": \""		  << db->Get(i, "messageStatus") << "\",\
				\"messageType\": \""			<< db->Get(i, "messageType") << "\",\
				\"eventTimestampDelta\": \""	<< GetHumanReadableTimeDifferenceFromNow(db->Get(i, "eventTimestamp")) << "\",\
				\"secondsSinceY2k\": \""		<< db->Get(i, "secondsSinceY2k") << "\",\
				\"eventTimestamp\": \""			<< db->Get(i, "eventTimestamp") << "\"\
			}";
		}
	}
	
	{
		CLog	log;
		log.Write(DEBUG, __func__ + string("[") + to_string(__LINE__) + string("]: end"));
	}

	return  result.str();
}

bool	isFilenameImage(string filename)
{
	bool	result = false;
	regex   e1("[.](gif|jpg|jpeg|png)$", regex_constants::icase);
	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "(" + filename + ")[" + to_string(__LINE__) + string("]: start" ));
	}

	result = regex_search(filename, e1);

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + string("[") + to_string(__LINE__) + string("]: end (result: ") + (result ? "true" : "false") + ")" );
	}
	return  result;
}

bool	isFilenameVideo(string filename)
{
	bool	result = false;
	regex   e1("[.](mov|avi|mp4|webm)$", regex_constants::icase);
	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "(" + filename + ")[" + to_string(__LINE__) + string("]: start" ));
	}

	result = regex_search(filename, e1);

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + string("[") + to_string(__LINE__) + string("]: end (result: ") + (result ? "true" : "false") + ")" );
	}
	return  result;
}

// --- extrasct all @[[:digit:]] patterns form srcMessage
vector<string> GetUserTagsFromText(string srcMessage)
{
	vector<string>  result;
	regex		   exp1("@([[:digit:]]+)");

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + string("[") + to_string(__LINE__) + string("]: start"));
	}

	regex_token_iterator<string::iterator>   rItr(srcMessage.begin(), srcMessage.end(), exp1, 1);
	regex_token_iterator<string::iterator>   rItrEnd;
	while(rItr != rItrEnd)
	{
		result.push_back(rItr->str());
		++rItr;
	}

	{
		CLog	log;
		log.Write(DEBUG, __func__ + string("[") + to_string(__LINE__) + string("]: end (result length: ") + to_string(result.size()) + string(")"));
	}

	return result;
}

string	AutodetectSexByName(string name, CMysql *db)
{
	string		result = "";

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: start");
	}

	if(db->Query("SELECT * FROM `name_sex` WHERE `name`=\"" + name + "\";"))
		result = db->Get(0, "sex");

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: finish (result = " + result + ")");
	}

	return result;
}


string GetUnreadChatMessagesInJSONFormat(CUser *user, CMysql *db)
{
	ostringstream	result, ost;
	int				affected;

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: start");
	}

	result.str("");

	ost.str("");
	ost << "select * from `chat_messages` where `toID`='" << user->GetID() << "' and (`messageStatus`='unread' or `messageStatus`='delivered' or `messageStatus`='sent');";
	affected = db->Query(ost.str());
	if(affected)
	{
		for(int i = 0; i < affected; i++)
		{
			result << (i ? "," : "") << "{\
				\"id\": \""					<< db->Get(i, "id") << "\", \
				\"message\": \"" 			<< ReplaceDoubleQuoteToQuote(db->Get(i, "message")) << "\", \
				\"fromType\": \"" 			<< db->Get(i, "fromType") << "\",\
				\"fromID\": \""				<< db->Get(i, "fromID") << "\",\
				\"toType\": \""			 	<< db->Get(i, "toType") << "\",\
				\"toID\": \""	 			<< db->Get(i, "toID") << "\",\
				\"messageType\": \""		<< db->Get(i, "messageType") << "\",\
				\"messageStatus\": \""		<< db->Get(i, "messageStatus") << "\",\
				\"eventTimestamp\": \""		<< db->Get(i, "eventTimestamp") << "\"\
			}";
		}
	}
	
	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: end");
	}

	return	result.str();
}


// --- Function returns list of images belongs to imageSet
// --- input: imageSetID, db
// --- output: list of image objects
string GetMessageImageList(string imageSetID, CMysql *db)
{
	ostringstream	ost;
	string		  result = "";

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: start");
	}

	if(imageSetID != "0")
	{
		int				affected;
		
		ost.str("");
		ost << "select * from `feed_images` where `setID`='" << imageSetID << "';";
		affected = db->Query(ost.str());
		if(affected > 0) 
		{
			ost.str("");
			for(int i = 0; i < affected; i++)
			{
				if(i > 0) ost << "," << std::endl;
				ost << "{";
				ost << "\"id\":\"" << db->Get(i, "id") << "\",";
				ost << "\"folder\":\"" << db->Get(i, "folder") << "\",";
				ost << "\"filename\":\"" << db->Get(i, "filename") << "\",";
				ost << "\"mediaType\":\"" << db->Get(i, "mediaType") << "\",";
				ost << "\"isActive\":\"" << db->Get(i, "isActive") << "\"";
				ost << "}";
			}

			result = ost.str();
		}
	}

	{
		CLog			log;
		ostringstream	ost;

		ost.str();
		ost <<  "GetMessageImageList: end. returning string length " << result.length();
		log.Write(DEBUG, ost.str());
	}

	return result;
}


string GetGeoLocalityIDByCityAndRegion(string regionName, string cityName, CMysql *db)
{
	ostringstream   ost;
	string		 	result = "";
	string			regionID = "", cityID = "";

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: start");
	}

	if(regionName.length())
	{
		if(db->Query("SELECT `id` FROM `geo_region` WHERE `title`=\"" + regionName + "\";"))
		{
			regionID = db->Get(0, "id");
		}
		else
		{
			long int 	tmp;
			{
				CLog			log;
				log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: region[" + regionName + "] not found. Creating new one.");
			}

			tmp = db->InsertQuery("INSERT INTO `geo_region` SET `geo_country_id`=\"0\", `title`=\"" + regionName + "\";");
			if(tmp) 
				regionID = to_string(tmp);
			else
			{
				CLog			log;
				log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR: insert into geo_region_db");
			}
		}
	}
	else
	{
		{
			CLog			log;
			log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: regionName is empty");
		}
	}

	if(cityName.length())
	{
		if(db->Query("SELECT `id` FROM `geo_locality` WHERE `title`=\"" + cityName + "\" " + (regionID.length() ? " AND `geo_region_id`=\"" + regionID + "\" " : "") + ";"))
		{
			cityID = db->Get(0, "id");
		}
		else
		{
			long int 	tmp;
			{
				CLog			log;
				log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: region[" + cityName + "] not found. Creating new one.");
			}

			tmp = db->InsertQuery("INSERT INTO `geo_locality` SET " + (regionID.length() ? "`geo_region_id`=\"" + regionID + "\"," : "") + " `title`=\"" + cityName + "\";");
			if(tmp) 
				cityID = to_string(tmp);
			else
			{
				CLog			log;
				log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR: insert into geo_region_db");
			}
		}
		result = cityID;
	}
	else
	{
		{
			CLog			log;
			log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: cityName is empty");
		}
	}

	{
		CLog			log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: end (returning string length " + to_string(result.length()) + ")");
	}

	return result;
}

string GetMessageCommentsCount(string messageID, CMysql *db)
{
	ostringstream   ost;
	int			 affected;
	string		  result = "0";

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: start");
	}

	ost.str("");
	ost << "select count(*) as `counter` from `feed_message_comment` where `type`=\"message\" and `messageID`='" << messageID << "';";
	affected = db->Query(ost.str());
	if(affected > 0) 
	{
		result = db->Get(0, "counter");
	}

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: end");
	}

	return result;
}

string GetMessageSpam(string messageID, CMysql *db)
{
	ostringstream	ost;
	int				affected;
	string			result = "0";

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: start");
	}

	ost.str("");
	ost << "select count(*) as `counter` from `feed_message_params` where `parameter`='spam' and messageID='" << messageID << "';";
	affected = db->Query(ost.str());


	if(affected > 0) 
	{
		result = db->Get(0, "counter");
	}

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: end");
	}

	return result;
}

// --- Function returns true or false depends on userID "spamed" it or not
// --- input: messageID, userID
// --- output: was this message "spamed" by particular user or not
string GetMessageSpamUser(string messageID, string userID, CMysql *db)
{
	ostringstream	ost;
	int				affected;
	string			result = "0";

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: start");
	}

	ost.str("");
	ost << "select count(*) as `counter` from `feed_message_params` where `parameter`='spam' and `messageID`='" << messageID << "' and `userID`='" << userID << "' ;";
	affected = db->Query(ost.str());
	if(affected > 0) 
	{
		result = db->Get(0, "counter");
	}

	
	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: end");
	}

	return result;
}

bool AllowMessageInNewsFeed(CUser *me, const string messageOwnerID, const string messageAccessRights, vector<string> *messageFriendList)
{

	{
		CLog			log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]:parameters (user [" + me->GetID() + "], messageOwnerID [" + messageOwnerID + "], messageAccessRights [" + messageAccessRights + "]): start");
	}

	// --- messages belons to yourself must be shown unconditionally
	// --- must be checked before message access private
	if(me->GetID() == messageOwnerID) return true;

	if(messageAccessRights == "public") return true;
	if(messageAccessRights == "private") return false;

	// --- require to check friendship list;
	if(messageAccessRights == "friends")
	{
		for(auto it = messageFriendList->begin(); it != messageFriendList->end(); ++it)
		{
			if(*it == messageOwnerID) return true;
		}

		return false;
	}

	return true;
}

// --- rate-limit on sessid persistence		
// --- input: REMOTE_ADDR
// --- output: true, if rate-limit required
// ---		 false, if rate-limit not required
bool isPersistenceRateLimited(string REMOTE_ADDR, CMysql *db)
{
	auto 			maxTime = BRUTEFORCE_TIME_INTERVAL, maxAttempts = BRUTEFORCE_ATTEMPTS;
	auto			rateLimitID = ""s;
	auto			affected = 0, attempts = 0;
	auto			result = false;

	MESSAGE_DEBUG("", "", "start (REMOTE_ADDR " + REMOTE_ADDR + ")")

	// --- cleanup rate-limit table
	db->Query("delete from `sessions_persistence_ratelimit` where `eventTimestamp` < (NOW() - interval "s + to_string(maxTime) + " second);");

	affected = db->Query("select `id`, `attempts` from `sessions_persistence_ratelimit` where `ip`='"s + REMOTE_ADDR + "';");
	if(affected)
	{
		MESSAGE_DEBUG("", "", "REMOTE_ADDR is in rate-limit table");

		rateLimitID = db->Get(0, "id");
		// attempts = atoi(db->Get(0, "attempts"));
		attempts = stoi(db->Get(0, "attempts"));

		db->Query("update `sessions_persistence_ratelimit` set `attempts`=`attempts`+1 where `id`='" + rateLimitID + "';");

		if(attempts > maxAttempts)
		{
			MESSAGE_ERROR("", "", "REMOTE_ADDR has tryed " + to_string(attempts) + " times during the last " + to_string(maxTime) + "sec. Needed to be rate-limited.")

			result = true;
		}
		else
		{
			MESSAGE_DEBUG("", "", "REMOTE_ADDR has tryed " + to_string(attempts) + " times during the last " + to_string(maxTime) + "sec. No need to rate-limit.")
			
			result = false;
		}
	}
	else
	{
		MESSAGE_DEBUG("", "", "REMOTE_ADDR not in rate-limit table.")

		db->Query("insert into `sessions_persistence_ratelimit` set `attempts`='1', `ip`='" + REMOTE_ADDR + "', `eventTimestamp`=NOW();");

		result = false;
	}

	MESSAGE_DEBUG("", "", "end. "s + (result ? "rate-limit" : "no need rate-limit"))

	return result;
}

void CopyFile(const string src, const string dst) 
{
	clock_t start, end;

	MESSAGE_DEBUG("", "", "start (" + src + ", " + dst + ")");

	start = clock();

	ifstream source(src.c_str(), ios::binary);
	ofstream dest(dst.c_str(), ios::binary);

	dest << source.rdbuf();

	source.close();
	dest.close();


	end = clock();

	MESSAGE_DEBUG("", "", "finish (time of file copying is " + to_string((end - start) / CLOCKS_PER_SEC) + " second)");
}

// --- admin function
string GetCompanyDuplicates(CMysql *db)
{
	// --- map<companyName, companyID>
	map<string, string>		companyMap;

	// --- map<companyID, companyName>
	map<string, string>		duplicatesMap;
	ostringstream			ost, ostResult;

	{
		CLog	log;
		ostringstream	ostTemp;

		ostTemp.str("");
		ostTemp << "GetCompanyDuplicates: start ";
		log.Write(DEBUG, ostTemp.str());
	}

	ostResult.str("");
	ost.str("");
	ost << "select * from `company`;";
	for(int i = 0; i < db->Query(ost.str()); i++)
	{
		string		companyToTestName, companyToTestID;

		companyToTestID = db->Get(i, "id");
		companyToTestName = RemoveAllNonAlphabetSymbols(db->Get(i, "name"));

		auto it = companyMap.find(companyToTestName);
		if(it != companyMap.end())
		{
			// --- duplicate found
			duplicatesMap[companyToTestID] = companyToTestName;
			duplicatesMap[it->second] = it->first;
		}
		companyMap[companyToTestName] = companyToTestID;
	}

	for (const auto& it : duplicatesMap) 
	{
		if(ostResult.str().length()) ostResult << ",";
		ost.str("");
		ost << "select * from `company` where `id`='" << it.first << "';";
		db->Query(ost.str());
		ostResult << "{\"id\":\"" << it.first << "\", \"companyName\":\"" << db->Get(0, "name") << "\"";

		ost.str("");
		ost << "select count(*) as `number_of_users` from `users_company` where `company_id`='" << it.first << "';";
		db->Query(ost.str());
		ostResult << ", \"usersInCompany\":\"" << db->Get(0, "number_of_users") << "\"}";
	}

	{
		CLog	log;
		ostringstream	ostTemp;

		ostTemp.str("");
		ostTemp << "GetCompanyDuplicates: end (number of duplicates: " << duplicatesMap.size() << ")";
		log.Write(DEBUG, ostTemp.str());
	}

	return ostResult.str();
}

// --- admin function
string GetPicturesWithEmptySet(CMysql *db)
{
	ostringstream		   ost, ostResult;
	int					 affected;

	{
		CLog	log;
		ostringstream   ostTemp;

		ostTemp.str("");
		ostTemp << "GetPicturesWithEmptySet: start ";
		log.Write(DEBUG, ostTemp.str());
	}

	ostResult.str("");
	ost.str("");
	ost << "SELECT * FROM `feed_images` where `setID`='0';";
	affected = db->Query(ost.str());
	for(int i = 0; i < affected; i++)
	{
		if(ostResult.str().length()) ostResult << ",";
		ostResult << "{\"id\":\"" << db->Get(i, "id") << "\",\"srcType\":\"" << db->Get(i, "srcType") << "\",\"userID\":\"" << db->Get(i, "userID") << "\",\"folder\":\"" << db->Get(i, "folder") << "\",\"filename\":\"" << db->Get(i, "filename") << "\"}";
	}

	{
		CLog	log;
		ostringstream   ostTemp;

		ostTemp.str("");
		ostTemp << "GetPicturesWithEmptySet: end (# of lost pictures: " << affected << ")";
		log.Write(DEBUG, ostTemp.str());
	}

	return ostResult.str();
}

// --- admin function
string GetPicturesWithUnknownMessage(CMysql *db)
{
	ostringstream					   ost, ostResult;
	int								 affected, lostCount = 0;
	unordered_set<unsigned long>		allImageSets;
	unordered_set<unsigned long>		lostImageSets;

	{
		CLog	log;
		ostringstream   ostTemp;

		ostTemp.str("");
		ostTemp << "GetPicturesWithUnknownMessage: start ";
		log.Write(DEBUG, ostTemp.str());
	}

	ostResult.str("");
	ost.str("");
	ost << "SELECT `setID` FROM `feed_images`;";
	affected = db->Query(ost.str());
	for(int i = 0; i < affected; i++)
	{
		// allImageSets.insert(atol(db->Get(i, "setID")));
		allImageSets.insert(stol(db->Get(i, "setID")));
	}

	for(const unsigned long& id: allImageSets)
	{
		ost.str("");
		ost << "select count(*) as count from `feed_message` where `imageSetID`=\"" << id << "\";";
		db->Query(ost.str());
		// if(!atoi(db->Get(0, "count")))
		if(!stoi(db->Get(0, "count")))
		{
			lostImageSets.insert(id);
		}
	}

	ostResult.str("");
	for(const unsigned long& id: lostImageSets)
	{
		ost.str("");
		ost << "select * from `feed_images` where `setID`=\"" << id << "\";";
		for(int i = 0; i < db->Query(ost.str()); i++, lostCount++)
		{
			if(ostResult.str().length()) ostResult << ",";
			ostResult << "{\"id\":\"" << db->Get(i, "id") << "\",\"srcType\":\"" << db->Get(i, "srcType") << "\",\"userID\":\"" << db->Get(i, "userID") << "\",\"setID\":\"" << db->Get(i, "setID") << "\",\"folder\":\"" << db->Get(i, "folder") << "\",\"filename\":\"" << db->Get(i, "filename") << "\"}";
		}
	}

	{
		CLog	log;
		ostringstream   ostTemp;

		ostTemp.str("");
		ostTemp << "GetPicturesWithUnknownMessage: end (# of lost pictures: " << affected << ")";
		log.Write(DEBUG, ostTemp.str());
	}

	return ostResult.str();
}

// --- admin function
string GetPicturesWithUnknownUser(CMysql *db)
{
	ostringstream					   		ost, ostResult;
	int								 		affected, lostCount = 0;
	unordered_set<string>					allImageOwners;
	unordered_set<unsigned long>			lostImages;

	{
		CLog	log;
		ostringstream   ostTemp;

		ostTemp.str("");
		ostTemp << "GetPicturesWithUnknownUser: start ";
		log.Write(DEBUG, ostTemp.str());
	}

	ostResult.str("");
	ost.str("");
	ost << "SELECT `srcType`,`userID` FROM `feed_images`;";
	affected = db->Query(ost.str());
	for(int i = 0; i < affected; i++)
		allImageOwners.insert(string(db->Get(i, "srcType")) + string(db->Get(i, "userID")));

	for(auto &item: allImageOwners)
	{
		string				tableName = "";
		string				id;
		size_t				pos;

		if((pos = item.find("company")) != string::npos)
		{
			tableName = "company";
			id = item.substr(string("company").length(), item.length() - string("company").length());
		}
		else if((pos = item.find("user")) != string::npos)
		{
			tableName = "users";
			id = item.substr(string("user").length(), item.length() - string("user").length());
		}

		if(tableName.length() && !db->Query("select `id` from `" + tableName + "` where `id`=\"" + id + "\";"))
			lostImages.insert(stol(id));
	}

	ostResult.str("");
	for(const unsigned long& id: lostImages)
	{
		ost.str("");
		ost << "select * from `feed_images` where `userID`=\"" << id << "\";";
		for(int i = 0; i < db->Query(ost.str()); i++, lostCount++)
		{
			if(ostResult.str().length()) ostResult << ",";
			ostResult << "{\"id\":\"" << db->Get(i, "id") << "\",\"srcType\":\"" << db->Get(i, "srcType") << "\",\"userID\":\"" << db->Get(i, "userID") << "\",\"setID\":\"" << db->Get(i, "setID") << "\",\"folder\":\"" << db->Get(i, "folder") << "\",\"filename\":\"" << db->Get(i, "filename") << "\"}";
		}
	}

	{
		CLog	log;
		ostringstream   ostTemp;

		ostTemp.str("");
		ostTemp << "GetPicturesWithUnknownUser: end (# of lost pictures: " << affected << ")";
		log.Write(DEBUG, ostTemp.str());
	}

	return ostResult.str();
}

// --- admin function
string GetRecommendationAdverse(CMysql *db)
{
	ostringstream					   ost, ostResult, dictionaryStatement;
	int								 affected;

	dictionaryStatement.str("");
	ostResult.str("");
	ost.str("");
	ost << "SELECT * FROM `dictionary_adverse`;";
	affected = db->Query(ost.str());
	if(affected)
	{
		for(int i = 0; i < affected; i++)
		{
			if(i) dictionaryStatement << " or ";
			dictionaryStatement << "(`title` like \"%" << db->Get(i, "word") << "%\")";
		}
	}

	ost.str("");
	ost << "select * from `users_recommendation` where `state` = 'unknown' and (" << dictionaryStatement.str() << ");";
	affected = db->Query(ost.str());
	if(affected)
	{
		for(int i = 0; i < affected; i++)
		{
			if(i) ostResult << ",";
			ostResult << "{";
			ostResult << "\"recommendationId\":\"" << db->Get(i, "id") <<"\",";
			ostResult << "\"recommendationRecommended_userID\":\"" << db->Get(i, "recommended_userID") <<"\",";
			ostResult << "\"recommendationRecommending_userID\":\"" << db->Get(i, "recommending_userID") <<"\",";
			ostResult << "\"recommendationTitle\":\"" << db->Get(i, "title") <<"\",";
			ostResult << "\"recommendationEventTimestamp\":\"" << db->Get(i, "eventTimestamp") <<"\",";
			ostResult << "\"recommendationState\":\"" << db->Get(i, "state") <<"\"";
			ostResult << "}";
		}
	}

	return ostResult.str();
}

// --- User notification part
string GetUserAvatarByUserID(string userID, CMysql *db)
{
	ostringstream   ost;
	string		  userAvatar = "";

	ost.str("");
	ost << "select * from `users_avatars` where `userid`='" << userID << "' and `isActive`='1';";
	if(db->Query(ost.str()))
	{
		ost.str("");
		ost << "/images/avatars/avatars" << db->Get(0, "folder") << "/" << db->Get(0, "filename");
		userAvatar = ost.str();
	}

	return userAvatar;
}

// --- function removes message image from FileSystems and cleanup DB
// --- as input require SWL WHERE clause (because of using SELECT and DELETE statements)
// --- input params:
// --- 1) SQL WHERE statement
// --- 2) db reference  
void	RemoveMessageImages(string sqlWhereStatement, CMysql *db)
{
	int			 affected;
	ostringstream   ost;

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) +  "]: start (sqlWhereStatement: " + sqlWhereStatement + ")");
	}

	ost.str("");
	ost << "select * from `feed_images` where " << sqlWhereStatement;
	affected = db->Query(ost.str());
	if(affected)
	{
		for(int i = 0; i < affected; i++)
		{
			string  filename = "";
			string  mediaType = db->Get(i, "mediaType");

			if(mediaType == "image" || mediaType == "video")
			{
				{
					CLog	log;
					log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) +  "]: file must be deleted [" + filename + "]");
				}
				
				if(mediaType == "image") filename = IMAGE_FEED_DIRECTORY;
				if(mediaType == "video") filename = VIDEO_FEED_DIRECTORY;

				filename +=  "/";
				filename +=  db->Get(i, "folder");
				filename +=  "/";
				filename +=  db->Get(i, "filename");


				if(isFileExists(filename))
				{
					unlink(filename.c_str());
				}
				else
				{
					CLog	log;
					log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) +  "]:ERROR: file is not exists  [filename=" + filename + "]");
				}
			}
			else
			{
				{
					CLog	log;
					log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) +  "]: mediaType[" + mediaType + "] doesn't have local file");
				}
				
			}

		}
		// --- cleanup DB with images pre-populated for posted message
		ost.str("");
		ost << "delete from `feed_images` where " << sqlWhereStatement;
		db->Query(ost.str());
	}

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) +  "]: finish");
	}
}

// --- function removes message image from FileSystems and cleanup DB
// --- as input require SWL WHERE clause (because of using SELECT and DELETE statements)
// --- input params:
// --- 1) SQL WHERE statement
// --- 2) db reference  
void	RemoveBookCover(string sqlWhereStatement, CMysql *db)
{
	int			 affected;
	ostringstream   ost;

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) +  "]: start (sqlWhereStatement: " + sqlWhereStatement + ")");
	}

	ost.str("");
	ost << "select * from `book` where " << sqlWhereStatement;
	affected = db->Query(ost.str());
	if(affected)
	{
		for(int i = 0; i < affected; i++)
		{
			string  filename = "";
			string  mediaType = "image";

			if(mediaType == "image" || mediaType == "video")
			{
				
				if(mediaType == "image") filename = IMAGE_BOOKS_DIRECTORY;

				filename +=  "/";
				filename +=  db->Get(i, "coverPhotoFolder");
				filename +=  "/";
				filename +=  db->Get(i, "coverPhotoFilename");

				{
					CLog	log;
					log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) +  "]: file must be deleted [" + filename + "]");
				}

				if(isFileExists(filename))
				{
					unlink(filename.c_str());
				}
				else
				{
					CLog	log;
					log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) +  "]:ERROR: file doesn't exists  [filename=" + filename + "]");
				}
			}
			else
			{
				{
					CLog	log;
					log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) +  "]: mediaType[" + mediaType + "] doesn't have local file");
				}
				
			}

		}
		// --- cleanup DB with images pre-populated for posted message
		ost.str("");
		ost << "UPDATE `book` SET `coverPhotoFolder`=\"\",`coverPhotoFilename`=\"\" where " << sqlWhereStatement;
		db->Query(ost.str());
	}

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) +  "]: finish");
	}
}

// --- function removes specified image from FileSystems and cleanup DB
// --- input params:
// --- 1) id
// --- 2) type  
// --- 3) db reference  
bool	RemoveSpecifiedCover(string itemID, string itemType, CMysql *db)
{
	int			 affected;
	bool			result = true;

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) +  "]: start (itemID = " + itemID + ", itemType = " + itemType + ")");
	}

	affected = db->Query(GetSpecificData_SelectQueryItemByID(itemID, itemType));
	if(affected)
	{
		for(int i = 0; i < affected; i++)
		{
			string  filename = "";
			string  mediaType = "image";

			if(mediaType == "image" || mediaType == "video")
			{
				
				if(mediaType == "image") filename = GetSpecificData_GetBaseDirectory(itemType);

				filename +=  "/";
				filename +=  db->Get(i, GetSpecificData_GetDBCoverPhotoFolderString(itemType).c_str());
				filename +=  "/";
				filename +=  db->Get(i, GetSpecificData_GetDBCoverPhotoFilenameString(itemType).c_str());

				{
					CLog	log;
					log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) +  "]: file must be deleted [" + filename + "]");
				}

				if(isFileExists(filename))
				{
					unlink(filename.c_str());
				}
				else
				{
					result = false;
					{
						CLog	log;
						log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) +  "]:ERROR: file doesn't exists  [filename=" + filename + "]");
					}					
				}
			}
			else
			{
				{
					CLog	log;
					log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) +  "]: mediaType[" + mediaType + "] doesn't have local file");
				}
				
			}

		}
		// --- cleanup DB with images pre-populated for posted message
		db->Query(GetSpecificData_UpdateQueryItemByID(itemID, itemType, "", ""));
	}

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) +  "]: finish (result = " + (result ? "true" : "false") + ")");
	}

	return result;
}


bool is_base64(BYTE c) {
  return (isalnum(c) || (c == '+') || (c == '/'));
}

string base64_encode(BYTE const* buf, unsigned int bufLen) 
{
  std::string ret;
  int i = 0;
  int j = 0;
  BYTE char_array_3[3];
  BYTE char_array_4[4];

  while (bufLen--) {
	char_array_3[i++] = *(buf++);
	if (i == 3) {
	  char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
	  char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
	  char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
	  char_array_4[3] = char_array_3[2] & 0x3f;

	  for(i = 0; (i <4) ; i++)
		ret += base64_chars[char_array_4[i]];
	  i = 0;
	}
  }

  if (i)
  {
	for(j = i; j < 3; j++)
	  char_array_3[j] = '\0';

	char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
	char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
	char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
	char_array_4[3] = char_array_3[2] & 0x3f;

	for (j = 0; (j < i + 1); j++)
	  ret += base64_chars[char_array_4[j]];

	while((i++ < 3))
	  ret += '=';
  }

  return ret;
}

string base64_decode(std::string const& encoded_string) 
{
  size_t in_len = encoded_string.size();
  size_t i = 0;
  size_t j = 0;
  int in_ = 0;
  unsigned char char_array_4[4], char_array_3[3];
  std::string ret;

  while (in_len-- && ( encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
	char_array_4[i++] = encoded_string[in_]; in_++;
	if (i ==4) {
	  for (i = 0; i <4; i++)
		char_array_4[i] = static_cast<unsigned char>(base64_chars.find(char_array_4[i]));

	  char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
	  char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
	  char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

	  for (i = 0; (i < 3); i++)
		ret += char_array_3[i];
	  i = 0;
	}
  }

  if (i) {
	for (j = i; j <4; j++)
	  char_array_4[j] = 0;

	for (j = 0; j <4; j++)
	  char_array_4[j] = static_cast<unsigned char>(base64_chars.find(char_array_4[j]));

	char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
	char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
	char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

	for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
  }

  return ret;
}

bool RedirStdoutToFile(string fname)
{
	bool		result = true;
	FILE		*fRes;

	fRes = freopen(fname.c_str(), "w", stdout);
	if(!fRes)
	{
		result = false;
		{
			CLog	log;
			log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR: redirect stderr to " + fname);
		}
	}

	return  result;
}

bool RedirStderrToFile(string fname)
{
	bool		result = true;
	FILE		*fRes;

	fRes = freopen(fname.c_str(), "w", stderr);
	if(!fRes)
	{
		result = false;
		{
			CLog	log;
			log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR: redirect stderr to " + fname);
		}
	}

	return  result;
}








int GetSpecificData_GetNumberOfFolders(string itemType)
{
	int	  result = 0;

	{
		CLog	log;

		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: start");
	}

	if(itemType == "certification")		result = CERTIFICATIONSLOGO_NUMBER_OF_FOLDERS;
	else if(itemType == "course")		result = CERTIFICATIONSLOGO_NUMBER_OF_FOLDERS;
	else if(itemType == "university")	result = UNIVERSITYLOGO_NUMBER_OF_FOLDERS;
	else if(itemType == "school")		result = SCHOOLLOGO_NUMBER_OF_FOLDERS;
	else if(itemType == "language")		result = FLAG_NUMBER_OF_FOLDERS;
	else if(itemType == "book")			result = BOOKCOVER_NUMBER_OF_FOLDERS;
	else if(itemType == "company")		result = COMPANYLOGO_NUMBER_OF_FOLDERS;
	else if(itemType == "gift")			result = GIFTIMAGE_NUMBER_OF_FOLDERS;
	else if(itemType == "event")		result = EVENTIMAGE_NUMBER_OF_FOLDERS;
	else
	{
		CLog	log;
		log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR: itemType [" + itemType + "] unknown");
	}

	{
		CLog	log;

		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: finish (result: " + to_string(result) + ")");
	}
	
	return result;
}

int GetSpecificData_GetMaxFileSize(string itemType)
{
	int	  result = 0;

	{
		CLog	log;

		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: start");
	}

	if(itemType == "certification")		result = CERTIFICATIONSLOGO_MAX_FILE_SIZE;
	else if(itemType == "course")		result = CERTIFICATIONSLOGO_MAX_FILE_SIZE;
	else if(itemType == "university")	result = UNIVERSITYLOGO_MAX_FILE_SIZE;
	else if(itemType == "school")		result = SCHOOLLOGO_MAX_FILE_SIZE;
	else if(itemType == "language")		result = FLAG_MAX_FILE_SIZE;
	else if(itemType == "book")			result = BOOKCOVER_MAX_FILE_SIZE;
	else if(itemType == "company")		result = COMPANYLOGO_MAX_FILE_SIZE;
	else if(itemType == "gift")			result = GIFTIMAGE_MAX_FILE_SIZE;
	else if(itemType == "event")		result = EVENTIMAGE_MAX_FILE_SIZE;
	else
	{
		CLog	log;
		log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR: itemType [" + itemType + "] unknown");
	}

	{
		CLog	log;

		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: finish (result: " + to_string(result) + ")");
	}
	
	return result;
}

unsigned int GetSpecificData_GetMaxWidth(string itemType)
{
	int	  result = 0;

	{
		CLog	log;

		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: start");
	}

	if(itemType == "certification")		result = CERTIFICATIONSLOGO_MAX_WIDTH;
	else if(itemType == "course")		result = CERTIFICATIONSLOGO_MAX_WIDTH;
	else if(itemType == "university")	result = UNIVERSITYLOGO_MAX_WIDTH;
	else if(itemType == "school")		result = SCHOOLLOGO_MAX_WIDTH;
	else if(itemType == "language")		result = FLAG_MAX_WIDTH;
	else if(itemType == "book")			result = BOOKCOVER_MAX_WIDTH;
	else if(itemType == "company")		result = COMPANYLOGO_MAX_WIDTH;
	else if(itemType == "gift")			result = GIFTIMAGE_MAX_WIDTH;
	else if(itemType == "event")		result = EVENTIMAGE_MAX_WIDTH;
	else
	{
		CLog	log;
		log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR: itemType [" + itemType + "] unknown");
	}

	{
		CLog	log;

		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: finish (result: " + to_string(result) + ")");
	}
	
	return result;
}

unsigned int GetSpecificData_GetMaxHeight(string itemType)
{
	int	  result = 0;

	{
		CLog	log;

		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: start");
	}

	if(itemType == "certification")		result = CERTIFICATIONSLOGO_MAX_HEIGHT;
	else if(itemType == "course")		result = CERTIFICATIONSLOGO_MAX_HEIGHT;
	else if(itemType == "university")	result = UNIVERSITYLOGO_MAX_HEIGHT;
	else if(itemType == "school")		result = SCHOOLLOGO_MAX_HEIGHT;
	else if(itemType == "language")		result = FLAG_MAX_HEIGHT;
	else if(itemType == "book")			result = BOOKCOVER_MAX_HEIGHT;
	else if(itemType == "company")		result = COMPANYLOGO_MAX_HEIGHT;
	else if(itemType == "gift")	  		result = GIFTIMAGE_MAX_HEIGHT;
	else if(itemType == "event")	  	result = EVENTIMAGE_MAX_HEIGHT;
	else
	{
		CLog	log;
		log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR: itemType [" + itemType + "] unknown");
	}

	{
		CLog	log;

		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: finish (result: " + to_string(result) + ")");
	}
	
	return result;
}

string GetSpecificData_GetBaseDirectory(string itemType)
{
	string	  result = "";

	{
		CLog	log;

		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: start");
	}

	if(itemType == "certification")		result = IMAGE_CERTIFICATIONS_DIRECTORY;
	else if(itemType == "course")		result = IMAGE_CERTIFICATIONS_DIRECTORY;
	else if(itemType == "university")	result = IMAGE_UNIVERSITIES_DIRECTORY;
	else if(itemType == "school")		result = IMAGE_SCHOOLS_DIRECTORY;
	else if(itemType == "language")		result = IMAGE_FLAGS_DIRECTORY;
	else if(itemType == "book")			result = IMAGE_BOOKS_DIRECTORY;
	else if(itemType == "company")		result = IMAGE_COMPANIES_DIRECTORY;
	else if(itemType == "gift")			result = IMAGE_GIFTS_DIRECTORY;
	else if(itemType == "event")		result = IMAGE_EVENTS_DIRECTORY;
	else
	{
		CLog	log;
		log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR: itemType [" + itemType + "] unknown");
	}

	{
		CLog	log;

		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: finish (result: " + result + ")");
	}
	
	return result;
}

string GetSpecificData_SelectQueryItemByID(string itemID, string itemType)
{
	string	  result = "";

	{
		CLog	log;

		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: start");
	}

	if(itemType == "certification")		result = "select * from `certification_tracks` where `id`=\"" + itemID + "\";";
	else if(itemType == "course")		result = "select * from `certification_tracks` where `id`=\"" + itemID + "\";";
	else if(itemType == "university")	result = "select * from `university` where `id`=\"" + itemID + "\";";
	else if(itemType == "school")		result = "select * from `school` where `id`=\"" + itemID + "\";";
	else if(itemType == "language")		result = "select * from `language` where `id`=\"" + itemID + "\";";
	else if(itemType == "book")			result = "select * from `book` where `id`=\"" + itemID + "\";";
	else if(itemType == "company")		result = "select * from `company` where `id`=\"" + itemID + "\";";
	else if(itemType == "gift")			result = "select * from `gifts` where `id`=\"" + itemID + "\";";
	else if(itemType == "event")		result = "select * from `events` where `id`=\"" + itemID + "\";";
	else
	{
		CLog	log;
		log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR: itemType [" + itemType + "] unknown");
	}

	{
		CLog	log;

		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: finish (result: " + result + ")");
	}
	
	return result;
}

string GetSpecificData_UpdateQueryItemByID(string itemID, string itemType, string folderID, string fileName)
{
	string		result = "";
	string		logo_folder = "";
	string		logo_filename = "";

	{
		CLog	log;

		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: start");
	}

	logo_folder = GetSpecificData_GetDBCoverPhotoFolderString(itemType);
	logo_filename = GetSpecificData_GetDBCoverPhotoFilenameString(itemType);

	if(logo_folder.length() && logo_filename.length())
	{
		if(itemType == "certification")		result = "update `certification_tracks` set	`" + logo_folder + "`='" + folderID + "', `" + logo_filename + "`='" + fileName + "' where `id`=\"" + itemID + "\";";
		else if(itemType == "course")		result = "update `certification_tracks` set `" + logo_folder + "`='" + folderID + "', `" + logo_filename + "`='" + fileName + "' where `id`=\"" + itemID + "\";";
		else if(itemType == "university")	result = "update `university` set 			`" + logo_folder + "`='" + folderID + "', `" + logo_filename + "`='" + fileName + "' where `id`=\"" + itemID + "\";";
		else if(itemType == "school")		result = "update `school` set 				`" + logo_folder + "`='" + folderID + "', `" + logo_filename + "`='" + fileName + "' where `id`=\"" + itemID + "\";";
		else if(itemType == "language")		result = "update `language` set 			`" + logo_folder + "`='" + folderID + "', `" + logo_filename + "`='" + fileName + "' where `id`=\"" + itemID + "\";";
		else if(itemType == "book")			result = "update `book` set 				`" + logo_folder + "`='" + folderID + "', `" + logo_filename + "`='" + fileName + "' where `id`=\"" + itemID + "\";";
		else if(itemType == "company")		result = "update `company` set 				`" + logo_folder + "`='" + folderID + "', `" + logo_filename + "`='" + fileName + "' where `id`=\"" + itemID + "\";";
		else if(itemType == "gift")			result = "update `gifts` set 				`" + logo_folder + "`='" + folderID + "', `" + logo_filename + "`='" + fileName + "' where `id`=\"" + itemID + "\";";
		else if(itemType == "event")		result = "update `events` set 				`" + logo_folder + "`='" + folderID + "', `" + logo_filename + "`='" + fileName + "' where `id`=\"" + itemID + "\";";
		else
		{
			CLog	log;
			log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR: itemType [" + itemType + "] unknown");
		}
	}
	else
	{
		{
			CLog	log;
			log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR: logo_folder or logo_filename not found for itemType [" + itemType + "]");
		}
	}


	{
		CLog	log;

		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: finish (result: " + result + ")");
	}
	
	return result;
}

string GetSpecificData_GetDBCoverPhotoFolderString(string itemType)
{
	string	  result = "";

	{
		CLog	log;

		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: start");
	}

	if(itemType == "certification")	 	result = "logo_folder";
	else if(itemType == "course")	   	result = "logo_folder";
	else if(itemType == "university")   result = "logo_folder";
	else if(itemType == "school")	   	result = "logo_folder";
	else if(itemType == "language")	 	result = "logo_folder";
	else if(itemType == "book")		 	result = "coverPhotoFolder";
	else if(itemType == "company")		result = "logo_folder";
	else if(itemType == "gift")	  		result = "logo_folder";
	else if(itemType == "event")	  	result = "logo_folder";
	else
	{
		CLog	log;
		log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR: itemType [" + itemType + "] unknown");
	}

	{
		CLog	log;

		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: finish (result: " + result + ")");
	}
	
	return result;
}

string GetSpecificData_GetDBCoverPhotoFilenameString(string itemType)
{
	string	  result = "";

	{
		CLog	log;

		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: start");
	}

	if(itemType == "certification")		result = "logo_filename";
	else if(itemType == "course")		result = "logo_filename";
	else if(itemType == "university")	result = "logo_filename";
	else if(itemType == "school")		result = "logo_filename";
	else if(itemType == "language")		result = "logo_filename";
	else if(itemType == "book")			result = "coverPhotoFilename";
	else if(itemType == "company")		result = "logo_filename";
	else if(itemType == "gift")			result = "logo_filename";
	else if(itemType == "event")		result = "logo_filename";
	else
	{
		CLog	log;
		log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR: itemType [" + itemType + "] unknown");
	}

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: finish (result: " + result + ")");
	}
	
	return result;
}

// --- Does the owner user allowed to change it ?
// --- For example:
// ---	*) university or school logo can be changed by administartor only.
// ---	*) gift image could be changed by owner
bool GetSpecificData_AllowedToChange(string itemID, string itemType, CMysql *db, CUser *user)
{
	bool	  result = false;

	{
		CLog	log;

		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: start");
	}

	if(db->Query(GetSpecificData_SelectQueryItemByID(itemID, itemType))) // --- item itemID exists ?
	{
		if((itemType == "course") || (itemType == "university") || (itemType == "school") || (itemType == "language") || (itemType == "book") || (itemType == "company") || (itemType == "certification"))
		{
			string	  coverPhotoFolder = db->Get(0, GetSpecificData_GetDBCoverPhotoFolderString(itemType).c_str());
			string	  coverPhotoFilename = db->Get(0, GetSpecificData_GetDBCoverPhotoFilenameString(itemType).c_str());

			if(coverPhotoFolder.empty() && coverPhotoFilename.empty())
				result = true;
			else
			{
				result = false;
				{
					CLog	log;
					log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: access to " + itemType + "(" + itemID + ") denied, because logo already uploaded");
				}
			}
		}
		else if(itemType == "event")
		{
			if(user)
			{
				if(db->Query("SELECT `id` FROM `event_hosts` WHERE `event_id`=\"" + itemID + "\" AND `user_id`=\"" + user->GetID() + "\";"))
					result = true;
				else
				{
					result = false;
					{
						CLog	log;
						log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: access to " + itemType + "(" + itemID + ") denied, you are not the event host");
					}
				}
			}
			else
			{
				result = false;
				{
					CLog	log;
					log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR: user object is NULL");
				}
			}
		}
		else if(itemType == "gift")
		{
			string		user_id = db->Get(0, "user_id");

			if(user)
			{
				if(user_id == user->GetID())
					result = true;
				else
				{
					result = false;
					{
						CLog	log;
						log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: access to " + itemType + "(" + itemID + ") denied, you are not the gift owner");
					}
				}
			}
			else
			{
				result = false;
				{
					CLog	log;
					log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR: user object is NULL");
				}
			}
		}
		else
		{
			CLog	log;
			log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR: itemType [" + itemType + "] unknown");
		}
	}
	else
	{
		result = false;
		{
			CLog	log;
			// --- it could be DEBUG level
			log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]: ERROR: " + itemType + "(" + itemID + ") not found");
		}

	}

	{
		CLog	log;

		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: finish (result: " + (result ? "true" : "false") + ")");
	}
	
	return result;
}

auto isAllowed_NoSession_Action(string action) -> bool
{
	auto			result = false;
	vector<string>	allowed_nosession_actions = ALLOWED_NO_SESSION_ACTION;

	MESSAGE_DEBUG("", "", "start (action " + action + ")");

	for(auto &allowed_nosession_action : allowed_nosession_actions)
	{
		if(action == allowed_nosession_action)
		{
			result = true;
			break;
		}
	}

	MESSAGE_DEBUG("", "", "finish");

	return result;
}

struct tm GetTMObject(string date)
{
	struct tm	result;
	smatch		sm;

	MESSAGE_DEBUG("", "", "start");
	
	result.tm_sec	= 0;   // seconds of minutes from 0 to 61
	result.tm_min	= 0;   // minutes of hour from 0 to 59
	result.tm_hour	= 0;  // hours of day from 0 to 24
	result.tm_mday	= 1;  // day of month from 1 to 31
	result.tm_mon	= 1;   // month of year from 0 to 11
	result.tm_year	= 70;  // year since 1900
	result.tm_wday	= 0;  // days since sunday
	result.tm_yday	= 0;  // days since January 1st
	result.tm_isdst	= 0; // hours of daylight savings time

	// --- normalize start, end format
	if(regex_match(date, sm, regex("^([[:digit:]]{1,2})\\/([[:digit:]]{1,2})\\/([[:digit:]]{2,4})$")))
	{
		result.tm_mday = stoi(sm[1]);
		result.tm_mon = stoi(sm[2]) - 1;
		if(stoi(sm[3]) < 100)
			result.tm_year = 2000 + stoi(sm[3]) - 1900;
		else
			result.tm_year = stoi(sm[3]) - 1900;
	}
	else if(regex_match(date, sm, regex("^([[:digit:]]{2,4})\\-([[:digit:]]{1,2})\\-([[:digit:]]{1,2})$")))
	{
		result.tm_mday = stoi(sm[3]);
		result.tm_mon = stoi(sm[2]) - 1;
		if(stoi(sm[1]) < 100)
			result.tm_year = 2000 + stoi(sm[1]) - 1900;
		else
			result.tm_year = stoi(sm[1]) - 1900;
	}
	else
	{
		MESSAGE_ERROR("", "", "incorrect date(" + date + ") format");
	}

	MESSAGE_DEBUG("", "", "finish");

	return result;
}

bool operator <(const struct tm &param_1, const struct tm &param_2)
{
	bool		result = false;
	struct tm	tm_1 = param_1;
	struct tm	tm_2 = param_2;

	MESSAGE_DEBUG("", "", "start");

	if(difftime(mktime(&tm_2), mktime(&tm_1)) > 0) result = true;

	MESSAGE_DEBUG("", "", "finish (result = " + (result ? "true" : "false") + ")");

	return result;
}

bool operator <=(const struct tm &param_1, const struct tm &param_2)
{
	bool		result = false;
	struct tm	tm_1 = param_1;
	struct tm	tm_2 = param_2;

	MESSAGE_DEBUG("", "", "start");

	if(difftime(mktime(&tm_2), mktime(&tm_1)) >= 0) result = true;

	MESSAGE_DEBUG("", "", "finish (result = " + (result ? "true" : "false") + ")");

	return result;
}

bool operator >(const struct tm &param_1, const struct tm &param_2)
{
	bool		result = false;
	struct tm	tm_1 = param_1;
	struct tm	tm_2 = param_2;

	MESSAGE_DEBUG("", "", "start");

	if(difftime(mktime(&tm_2), mktime(&tm_1)) < 0) result = true;

	MESSAGE_DEBUG("", "", "finish (result = " + (result ? "true" : "false") + ")");

	return result;
}

bool operator >=(const struct tm &param_1, const struct tm &param_2)
{
	bool		result = false;
	struct tm	tm_1 = param_1;
	struct tm	tm_2 = param_2;

	MESSAGE_DEBUG("", "", "start");

	if(difftime(mktime(&tm_2), mktime(&tm_1)) <= 0) result = true;

	MESSAGE_DEBUG("", "", "finish (result = " + (result ? "true" : "false") + ")");

	return result;
}

bool operator ==(const struct tm &param_1, const struct tm &param_2)
{
	bool		result = false;
	struct tm	tm_1 = param_1;
	struct tm	tm_2 = param_2;

	MESSAGE_DEBUG("", "", "start");

	if(difftime(mktime(&tm_2), mktime(&tm_1)) == 0) result = true;

	MESSAGE_DEBUG("", "", "finish (result = " + (result ? "true" : "false") + ")");

	return result;
}

string PrintDate(const struct tm &_tm)
{
	string	result = "";

	result = to_string(_tm.tm_mday) + "/" + to_string(_tm.tm_mon + 1) + "/" + to_string(_tm.tm_year + 1900);

	return result;
}

string PrintSQLDate(const struct tm &_tm)
{
	string	result = "";

	result = to_string(_tm.tm_year + 1900) + "-" + to_string(_tm.tm_mon + 1) + "-" + to_string(_tm.tm_mday);

	return result;
}

string PrintDateTime(const struct tm &_tm)
{
	return PrintTime(_tm, "%T");
}

string PrintTime(const struct tm &_tm, string format)
{
	char		buffer[256];
	string		result = "";
	struct tm	temp_tm = _tm;

	mktime(&temp_tm);
	memset(buffer, 0, sizeof(buffer));
	strftime(buffer, 255, format.c_str(), &temp_tm);
	result = buffer;

	return result;
}

