#include "utilities_bestbounty.h"

string GetEventGuestsListInJSONFormat(string dbQuery, CMysql *db, CUser *user)
{
	struct ItemClass 
	{
		string	id;
		string	event_id;
		string	user_id;
		string	quick_registration_id;
		string	status;
		string	adults;
		string	kids;
		string	eventTimestamp;
	};

	ostringstream			ost, ostFinal;
	string					sessid, lookForKey;
	int						affected;
	vector<ItemClass>		eventGuestsList;

	{
		MESSAGE_DEBUG("", "", "start");
	}

	ostFinal.str("");

	if((affected = db->Query(dbQuery)) > 0)
	{
		int		eventGuestsCounter = affected;

		eventGuestsList.reserve(eventGuestsCounter);  // --- reserving allows avoid moving vector in memory
											// --- to fit vector into continuous memory piece

		for(int i = 0; i < eventGuestsCounter; i++)
		{
			ItemClass	event_guest;

			event_guest.id = db->Get(i, "id");
			event_guest.event_id = db->Get(i, "event_id");
			event_guest.user_id = db->Get(i, "user_id");
			event_guest.quick_registration_id = db->Get(i, "quick_registration_id");
			event_guest.status = db->Get(i, "status");
			event_guest.adults = db->Get(i, "adults");
			event_guest.kids = db->Get(i, "kids");
			event_guest.eventTimestamp = db->Get(i, "eventTimestamp");

			eventGuestsList.push_back(event_guest);
		}

		for(int i = 0; i < eventGuestsCounter; i++)
		{
				if(eventGuestsList[i].user_id != "0")
				{
					if(db->Query("SELECT `id`,`name`,`nameLast` FROM `users` WHERE `id`=\"" + eventGuestsList[i].user_id + "\";"))
					{
						string		userName = db->Get(0, "name");
						string		userNameLast = db->Get(0, "nameLast");
						string		avatar = "";

						if(db->Query("SELECT `folder`, `filename` FROM `users_avatars` WHERE `isActive`=\"1\" AND `userid`=\"" + eventGuestsList[i].user_id + "\";"))
							avatar = string("/images/avatars/avatars") + db->Get(0, "folder") + "/" + db->Get(0, "filename");

						if(ostFinal.str().length()) ostFinal << ", ";

						ostFinal << "{";
						ostFinal << "\"id\": \""				  	<< eventGuestsList[i].id << "\",";
						ostFinal << "\"user_id\": \""				<< eventGuestsList[i].user_id << "\",";
						ostFinal << "\"status\": \""				<< eventGuestsList[i].status << "\",";
						ostFinal << "\"name\": \""					<< userName << "\",";
						ostFinal << "\"nameLast\": \""				<< userNameLast << "\",";
						ostFinal << "\"avatar\": \""				<< avatar << "\",";
						ostFinal << "\"adults\": \""				<< eventGuestsList[i].adults << "\",";
						ostFinal << "\"kids\": \""					<< eventGuestsList[i].kids << "\",";
						ostFinal << "\"gifts\": ["					<< GetGiftListInJSONFormat("SELECT * FROM `gifts` WHERE `user_id`=\"" + eventGuestsList[i].user_id + "\";", db, user) << "],";
						ostFinal << "\"gifts_to_give\": [" 			<< GetGiftToGiveListInJSONFormat("SELECT * FROM `gifts_to_give` WHERE `gift_id` in (SELECT `id` FROM `gifts` WHERE `user_id`=\"" + eventGuestsList[i].user_id + "\");", db, user) << "],";
						ostFinal << "\"eventTimestamp\": \""		<< eventGuestsList[i].eventTimestamp << "\"";
						ostFinal << "}";
					}
					else
					{
						MESSAGE_DEBUG("", "", " user_id(" + eventGuestsList[i].user_id + ") not found in `users` table)");
					}

				}
				else if(eventGuestsList[i].quick_registration_id.length())
				{
					if(user)
					{
						if(db->Query("SELECT `id` FROM `event_hosts` WHERE `event_id`=\"" + eventGuestsList[i].event_id + "\" AND `user_id`=\"" + user->GetID() + "\";"))
						{
							// --- email can be exposed to "event host"

							if(db->Query("SELECT `email` FROM `quick_registration` WHERE `id`=\"" + eventGuestsList[i].quick_registration_id + "\";"))
							{
								if(ostFinal.str().length()) ostFinal << ", ";

								ostFinal << "{";
								ostFinal << "\"id\": \""				  	<< eventGuestsList[i].id << "\",";
								ostFinal << "\"user_id\": \""				<< eventGuestsList[i].user_id << "\",";
								ostFinal << "\"status\": \""				<< eventGuestsList[i].status << "\",";
								ostFinal << "\"name\": \""					<< "" << "\",";
								ostFinal << "\"nameLast\": \""				<< "" << "\",";
								ostFinal << "\"avatar\": \""				<< "" << "\",";
								ostFinal << "\"email\": \""					<< db->Get(0, "email") << "\",";
								ostFinal << "\"adults\": \""				<< eventGuestsList[i].adults << "\",";
								ostFinal << "\"kids\": \""					<< eventGuestsList[i].kids << "\",";
								ostFinal << "\"gifts\": ["					<< "" << "],";
								ostFinal << "\"gifts_to_give\": [" 			<< "" << "],";
								ostFinal << "\"eventTimestamp\": \""		<< eventGuestsList[i].eventTimestamp << "\"";
								ostFinal << "}";
							}
							else
							{
								MESSAGE_ERROR("", "", " quick_registration_id(" + eventGuestsList[i].quick_registration_id + ") not found");
							}

						}
						else
						{
							MESSAGE_DEBUG("", "", "email could be exposed to event host only. You are not event host. (event_id(" + eventGuestsList[i].event_id + ") user_id(" + user->GetID() + "))");
						}
					}
					else
					{
						MESSAGE_DEBUG("", "", "user is not an event host, therefore he have no access to email information");
					}
				}
				else
				{
					MESSAGE_ERROR("", "", "user_id and email are empty in event_guests table.");
				}
		}
	} // --- if sql-query on event selection success
	else
	{
		MESSAGE_DEBUG("", "", "there are no events returned by request [" + dbQuery + "]");
	}

	{
		MESSAGE_DEBUG("", "", "end (result length = " + to_string(ostFinal.str().length()) + ")");
	}

	return ostFinal.str();
}


// --- input: 
//		dbQuery - example "SELECT * FROM `open_vacancies`;"
//		isAdminUser:
//			true - candidates count will be included
//			false - candidates count will NOT be included
//		db - pointer to CMysql object
//		user - pointer to user (if admin_user - return correct answers)
string GetOpenVacanciesInJSONFormat(string dbQuery, CMysql *db, CUser *user/* = NULL*/)
{
	struct ItemClass 
	{
		string id;
		string company_id;
		string company_position_id;
		string geo_locality_id;
		string salary_min;
		string salary_max;
		string start_month;
		string work_format;
		string description;
		string question1;
		string question2;
		string question3;
		string correct_answer1;
		string correct_answer2;
		string correct_answer3;
		string answer11;
		string answer12;
		string answer13;
		string answer21;
		string answer22;
		string answer23;
		string answer31;
		string answer32;
		string answer33;
		string language1_id;
		string language2_id;
		string language3_id;
		string skill1_id;
		string skill2_id;
		string skill3_id;
		string publish_finish;
		string publish_period;
	};

	vector<ItemClass>		itemsList;
	ostringstream   		ostResult;
	string		 			result = "";
	int						itemsCount = 0;

	{
		MESSAGE_DEBUG("", "", "start");
	}

	ostResult.str("");

	itemsCount = db->Query(dbQuery);
	for(int i = 0; i < itemsCount; ++i)
	{
		ItemClass   item;

		item.id = db->Get(i, "id");
		item.company_id = db->Get(i, "company_id");
		item.company_position_id = db->Get(i, "company_position_id");
		item.geo_locality_id = db->Get(i, "geo_locality_id");
		item.salary_min = db->Get(i, "salary_min");
		item.salary_max = db->Get(i, "salary_max");
		item.start_month = db->Get(i, "start_month");
		item.work_format = db->Get(i, "work_format");
		item.description = db->Get(i, "description");
		item.language1_id = db->Get(i, "language1_id");
		item.language2_id = db->Get(i, "language2_id");
		item.language3_id = db->Get(i, "language3_id");
		item.skill1_id = db->Get(i, "skill1_id");
		item.skill2_id = db->Get(i, "skill2_id");
		item.skill3_id = db->Get(i, "skill3_id");
		item.publish_finish = db->Get(i, "publish_finish");
		item.publish_period = db->Get(i, "publish_period");
		item.question1 = db->Get(i, "question1");
		item.question2 = db->Get(i, "question2");
		item.question3 = db->Get(i, "question3");
		item.correct_answer1 = db->Get(i, "correct_answer1");
		item.correct_answer2 = db->Get(i, "correct_answer2");
		item.correct_answer3 = db->Get(i, "correct_answer3");
		item.answer11 = db->Get(i, "answer11");
		item.answer12 = db->Get(i, "answer12");
		item.answer13 = db->Get(i, "answer13");
		item.answer21 = db->Get(i, "answer21");
		item.answer22 = db->Get(i, "answer22");
		item.answer23 = db->Get(i, "answer23");
		item.answer31 = db->Get(i, "answer31");
		item.answer32 = db->Get(i, "answer32");
		item.answer33 = db->Get(i, "answer33");

		itemsList.push_back(item);
	}

	for(int i = 0; i < itemsCount; ++i)
	{
		string		company_position_title = "";
		string		geo_locality_title = "";
		string		geo_region_id = "";
		string		geo_region_title = "";
		string		language1_title = "";
		string		language2_title = "";
		string		language3_title = "";
		string		skill1_title = "";
		string		skill2_title = "";
		string		skill3_title = "";
		string		number_of_applied_candidates = "";
		string		admin_user_id = "";

		if(db->Query("SELECT * FROM `company_position` WHERE `id`=\"" + itemsList[i].company_position_id + "\";"))
			company_position_title = db->Get(0, "title");
		else
		{
			MESSAGE_ERROR("", "", "fail getting company_position by id");
		}

		if(db->Query("SELECT * FROM `geo_locality` WHERE `id`=\"" + itemsList[i].geo_locality_id + "\";"))
		{
			geo_locality_title = db->Get(0, "title");
			geo_region_id = db->Get(0, "geo_region_id");
			if(db->Query("SELECT * FROM `geo_region` WHERE `id`=\"" + geo_region_id + "\";"))
			{
				geo_region_title = db->Get(0, "title");
			}
			else
			{
				MESSAGE_ERROR("", "", "fail getting geo_region by id");
			}
		}
		else
		{
			MESSAGE_ERROR("", "", "fail getting geo_locality by id");
		}

		if(itemsList[i].language1_id != "0")
		{
			if(db->Query("SELECT * FROM `language` WHERE `id`=\"" + itemsList[i].language1_id + "\";"))
				language1_title = db->Get(0, "title");
			else
			{
				MESSAGE_ERROR("", "", "fail getting language1 by id");
			}
		}

		if(itemsList[i].language2_id != "0")
		{
			if(db->Query("SELECT * FROM `language` WHERE `id`=\"" + itemsList[i].language2_id + "\";"))
				language2_title = db->Get(0, "title");
			else
			{
				MESSAGE_ERROR("", "", "fail getting language2 by id");
			}
		}

		if(itemsList[i].language3_id != "0")
		{
			if(db->Query("SELECT * FROM `language` WHERE `id`=\"" + itemsList[i].language3_id + "\";"))
				language3_title = db->Get(0, "title");
			else
			{
				MESSAGE_ERROR("", "", "fail getting language3 by id");
			}
		}

		if(itemsList[i].skill1_id != "0")
		{
			if(db->Query("SELECT * FROM `skill` WHERE `id`=\"" + itemsList[i].skill1_id + "\";"))
				skill1_title = db->Get(0, "title");
			else
			{
				MESSAGE_ERROR("", "", "fail getting skill1 by id");
			}
		}

		if(itemsList[i].skill2_id != "0")
		{
			if(db->Query("SELECT * FROM `skill` WHERE `id`=\"" + itemsList[i].skill2_id + "\";"))
				skill2_title = db->Get(0, "title");
			else
			{
				MESSAGE_ERROR("", "", "fail getting skill2 by id");
			}
		}

		if(itemsList[i].skill3_id != "0")
		{
			if(db->Query("SELECT * FROM `skill` WHERE `id`=\"" + itemsList[i].skill3_id + "\";"))
				skill3_title = db->Get(0, "title");
			else
			{
				MESSAGE_ERROR("", "", "fail getting skill3 by id");
			}
		}

		if(db->Query("SELECT COUNT(*) as `count` FROM `company_candidates` WHERE vacancy_id=\"" + itemsList[i].id + "\" AND `status`=\"applied\";"))
			number_of_applied_candidates = db->Get(0, "count");
		else
			number_of_applied_candidates = "0";

		if(db->Query("SELECT `admin_userID` FROM `company` WHERE `id`=\"" + itemsList[i].company_id + "\";"))
			admin_user_id = db->Get(0, "admin_userID");

		if(ostResult.str().length()) ostResult << ", ";

		ostResult	<< "{"
					<< "\"id\":\"" << itemsList[i].id << "\","
					<< "\"company_id\":\"" << itemsList[i].company_id << "\","
					// << "\"company\":" << GetCompanyListInJSONFormat("SELECT * FROM `company` WHERE `id`=\"" + itemsList[i].company_id + "\";", db, NULL) << ","
					<< "\"company_position_id\":\"" << itemsList[i].company_position_id << "\","
					<< "\"company_position_title\":\"" << company_position_title << "\","
					<< "\"geo_locality_id\":\"" << itemsList[i].geo_locality_id << "\","
					<< "\"geo_locality_title\":\"" << geo_locality_title << "\","
					<< "\"geo_region_id\":\"" << geo_region_id << "\","
					<< "\"geo_region_title\":\"" << geo_region_title << "\","
					<< "\"salary_min\":\"" << itemsList[i].salary_min << "\","
					<< "\"salary_max\":\"" << itemsList[i].salary_max << "\","
					<< "\"start_month\":\"" << itemsList[i].start_month << "\","
					<< "\"work_format\":\"" << itemsList[i].work_format << "\","
					<< "\"description\":\"" << itemsList[i].description << "\","
					<< "\"question1\":\"" << itemsList[i].question1 << "\","
					<< "\"question2\":\"" << itemsList[i].question2 << "\","
					<< "\"question3\":\"" << itemsList[i].question3 << "\","
					<< "\"correct_answer1\":\"" << (user && (admin_user_id == user->GetID()) ? itemsList[i].correct_answer1 : "") << "\","
					<< "\"correct_answer2\":\"" << (user && (admin_user_id == user->GetID()) ? itemsList[i].correct_answer2 : "") << "\","
					<< "\"correct_answer3\":\"" << (user && (admin_user_id == user->GetID()) ? itemsList[i].correct_answer3 : "") << "\","
					<< "\"answer11\":\"" << itemsList[i].answer11 << "\","
					<< "\"answer12\":\"" << itemsList[i].answer12 << "\","
					<< "\"answer13\":\"" << itemsList[i].answer13 << "\","
					<< "\"answer21\":\"" << itemsList[i].answer21 << "\","
					<< "\"answer22\":\"" << itemsList[i].answer22 << "\","
					<< "\"answer23\":\"" << itemsList[i].answer23 << "\","
					<< "\"answer31\":\"" << itemsList[i].answer31 << "\","
					<< "\"answer32\":\"" << itemsList[i].answer32 << "\","
					<< "\"answer33\":\"" << itemsList[i].answer33 << "\","
					<< "\"language1_id\":\"" << itemsList[i].language1_id << "\","
					<< "\"language1_title\":\"" << language1_title << "\","
					<< "\"language2_id\":\"" << itemsList[i].language2_id << "\","
					<< "\"language2_title\":\"" << language2_title << "\","
					<< "\"language3_id\":\"" << itemsList[i].language3_id << "\","
					<< "\"language3_title\":\"" << language3_title << "\","
					<< "\"skill1_id\":\"" << itemsList[i].skill1_id << "\","
					<< "\"skill1_title\":\"" << skill1_title << "\","
					<< "\"skill2_id\":\"" << itemsList[i].skill2_id << "\","
					<< "\"skill2_title\":\"" << skill2_title << "\","
					<< "\"skill3_id\":\"" << itemsList[i].skill3_id << "\","
					<< "\"skill3_title\":\"" << skill3_title << "\","
					<< "\"publish_finish\":\"" << itemsList[i].publish_finish << "\","
					<< "\"publish_period\":\"" << itemsList[i].publish_period << "\","
					<< "\"number_of_applied_candidates\":\"" << (user && (admin_user_id == user->GetID()) ? number_of_applied_candidates : "") << "\""
					<<"}";
	}

	result = ostResult.str();

	{
		MESSAGE_DEBUG("", "", "end (returning string length " + to_string(result.length()) + ")");
	}

	return result;
}

string GetGroupListInJSONFormat(string dbQuery, CMysql *db, CUser *user)
{
	struct ItemClass 
	{
		string	id;
		string	link;
		string	title;
		string	description;
		string	logo_folder;
		string	logo_filename;
		string	owner_id;
		string	isBlocked;
		string	eventTimestampCreation;
		string	eventTimestampLastPost;
	};

	ostringstream			ost, ostFinal;
	string					sessid, lookForKey;
	int						affected;
	vector<ItemClass>		groupsList;

	{
		MESSAGE_DEBUG("", "", "start");
	}

	ostFinal.str("");

	if((affected = db->Query(dbQuery)) > 0)
	{
		int						groupCounter = affected;

		groupsList.reserve(groupCounter);  // --- reserving allows avoid moving vector in memory
											// --- to fit vector into continuous memory piece

		for(int i = 0; i < affected; i++)
		{
			ItemClass	group;

			group.id = db->Get(i, "id");
			group.link = db->Get(i, "link");
			group.title = db->Get(i, "title");
			group.description = db->Get(i, "description");
			group.logo_folder = db->Get(i, "logo_folder");
			group.logo_filename = db->Get(i, "logo_filename");
			group.owner_id = db->Get(i, "owner_id");
			group.isBlocked = db->Get(i, "isBlocked");
			group.eventTimestampCreation = db->Get(i, "eventTimestampCreation");
			group.eventTimestampLastPost = db->Get(i, "eventTimestampLastPost");

			groupsList.push_back(group);
		}

		for(int i = 0; i < groupCounter; i++)
		{
				string		numberOfMembers = "0";

				if(ostFinal.str().length()) ostFinal << ", ";

				if(db->Query("SELECT COUNT(*) as numberOfMembers FROM `users_subscriptions` WHERE `entity_type`=\"group\" AND `entity_id`=\"" + groupsList[i].id + "\";"))
					numberOfMembers = db->Get(0, "numberOfMembers");

				ostFinal << "{";
				ostFinal << "\"id\": \""				  	<< groupsList[i].id << "\",";
				ostFinal << "\"link\": \""					<< groupsList[i].link << "\",";
				ostFinal << "\"title\": \""					<< groupsList[i].title << "\",";
				ostFinal << "\"description\": \""			<< groupsList[i].description << "\",";
				ostFinal << "\"logo_folder\": \""			<< groupsList[i].logo_folder << "\",";
				ostFinal << "\"logo_filename\": \""			<< groupsList[i].logo_filename << "\",";
				ostFinal << "\"isMine\": \""				<< (user ? groupsList[i].owner_id == user->GetID() : false) << "\",";
				ostFinal << "\"numberOfMembers\": \""		<< numberOfMembers << "\",";
				ostFinal << "\"isBlocked\": \""				<< groupsList[i].isBlocked << "\",";
				ostFinal << "\"eventTimestampCreation\": \""<< groupsList[i].eventTimestampCreation << "\",";
				ostFinal << "\"eventTimestampLastPost\": \""<< groupsList[i].eventTimestampLastPost << "\"";
				ostFinal << "}";
		} // --- for loop through group list
	} // --- if sql-query on group selection success
	else
	{
		MESSAGE_DEBUG("", "", "there are no groups returned by request [" + dbQuery + "]");
	}

	{
		MESSAGE_DEBUG("", "", "end (result length = " + to_string(ostFinal.str().length()) + ")");
	}

	return ostFinal.str();
}

// input: ....
//		  includeReaders will add readers counter
string GetBookListInJSONFormat(string dbQuery, CMysql *db, bool includeReaders/* = false*/)
{
	struct BookClass {
		string	id, title, authorID, authorName, isbn10, isbn13, photoCoverFolder, PhotoCoverFilename, readersUserID;
	};

	ostringstream				   ostResult;
	int							 booksCount;
	vector<BookClass>			   bookList;


	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: start");
	}

	ostResult.str("");
	booksCount = db->Query(dbQuery);
	if(booksCount)
	{
		for(int i = 0; i < booksCount; i++) 
		{
			BookClass   bookListItem;
			bookListItem.id				 = db->Get(i, "id");
			bookListItem.title			  = db->Get(i, "title");
			bookListItem.authorID		   = db->Get(i, "authorID");
			bookListItem.isbn10			 = db->Get(i, "isbn10");
			bookListItem.isbn13			 = db->Get(i, "isbn13");
			bookListItem.photoCoverFolder   = db->Get(i, "coverPhotoFolder");
			bookListItem.PhotoCoverFilename = db->Get(i, "coverPhotoFilename");
			bookListItem.readersUserID	  = "";

			bookList.push_back(bookListItem);						
		}
		
		for(int i = 0; i < booksCount; i++) 
		{
				if(db->Query("select * from `book_author` where `id`=\"" + bookList.at(i).authorID + "\";"))
					bookList.at(i).authorName = db->Get(0, "name");

				if(includeReaders)
				{
					string temp = "";

					for(int j = 0; j < db->Query("SELECT `userID` from `users_books` WHERE `bookID`=\"" + bookList.at(i).id + "\";"); ++j)
					{
						if(temp.length()) temp += ",";
						temp += db->Get(j, "userID");
					}

					bookList.at(i).readersUserID = temp;
				}

				if(ostResult.str().length()) ostResult << ", ";

				ostResult << "{"
						  << "\"bookID\": \""				 << bookList.at(i).id << "\", "
						  << "\"bookTitle\": \""			  << bookList.at(i).title << "\", "
						  << "\"bookAuthorID\": \""		   << bookList.at(i).authorID << "\","
						  << "\"bookAuthorName\": \""		 << bookList.at(i).authorName << "\","
						  << "\"bookISBN10\": \""			 << bookList.at(i).isbn10 << "\","
						  << "\"bookISBN13\": \""			 << bookList.at(i).isbn13 << "\","
						  << "\"bookPhotoCoverFolder\": \""   << bookList.at(i).photoCoverFolder << "\","
						  << "\"bookPhotoCoverFilename\": \"" << bookList.at(i).PhotoCoverFilename << "\","
						  << "\"bookReadersUserID\": ["	   << bookList.at(i).readersUserID << "]"
						  << "}";
		} // --- for loop through user list
	} // --- if sql-query on user selection success
	else
	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: there are no books returned by the request [", dbQuery, "]");
	}

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: finish (returning string length = " + to_string(ostResult.str().length()) + ")");
	}

	return ostResult.str();
}

// input: ....
//		  includeReaders will add readers counter
string GetComplainListInJSONFormat(string dbQuery, CMysql *db, bool includeReaders/* = false*/)
{
	struct ComplainClass {
		string	  id;
		string	  userID;
		string	  entityID;
		string	  type;
		string	  subtype;
		string	  complainComment;
		string	  resolveComment;
		string	  state;
		string	  openEventTimestamp;
		string	  closeEventTimestamp;
	};

	ostringstream				   ostResult;
	int							 complainsCount;
	vector<ComplainClass>		   complainList;


	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: start");
	}

	ostResult.str("");
	complainsCount = db->Query(dbQuery);
	if(complainsCount)
	{
		for(int i = 0; i < complainsCount; i++) 
		{
			ComplainClass   complainListItem;
			complainListItem.id				 = db->Get(i, "id");
			complainListItem.userID			 = db->Get(i, "userID");
			complainListItem.entityID		   = db->Get(i, "entityID");
			complainListItem.type			   = db->Get(i, "type");
			complainListItem.subtype			= db->Get(i, "subtype");
			complainListItem.complainComment	= db->Get(i, "complainComment");
			complainListItem.resolveComment	 = db->Get(i, "resolveComment");
			complainListItem.state			  = db->Get(i, "state");
			complainListItem.openEventTimestamp = db->Get(i, "openEventTimestamp");
			complainListItem.closeEventTimestamp= db->Get(i, "closeEventTimestamp");

			complainList.push_back(complainListItem);						
		}
		
		for(int i = 0; i < complainsCount; i++) 
		{
				if(ostResult.str().length()) ostResult << ", ";

				ostResult << "{"
						  << "\"id\": \""			  << complainList.at(i).id << "\", "
						  << "\"userID\": \""		  << complainList.at(i).userID << "\", "
						  << "\"entityID\": \""		<< complainList.at(i).entityID << "\","
						  << "\"type\": \""			<< complainList.at(i).type << "\","
						  << "\"subtype\": \""		 << complainList.at(i).subtype << "\","
						  << "\"complainComment\": \"" << complainList.at(i).complainComment << "\","
						  << "\"resolveComment\": \""  << complainList.at(i).resolveComment << "\","
						  << "\"state\": \""		   << complainList.at(i).state << "\","
						  << "\"openEventTimestamp\": \""  << complainList.at(i).openEventTimestamp << "\","
						  << "\"closeEventTimestamp\": \"" << complainList.at(i).closeEventTimestamp << "\""
						  << "}";
		} // --- for loop through user list
	} // --- if sql-query on user selection success
	else
	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: there are no complains returned by the request [", dbQuery, "]");
	}

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: finish (returning string length = " + to_string(ostResult.str().length()) + ")");
	}

	return ostResult.str();
}

// input: ....
//		  includeDevoted will add student counter
string GetCertificationListInJSONFormat(string dbQuery, CMysql *db, bool includeDevoted/* = false*/)
{
	struct ItemClass {
		string	  id, title, photoFolder, photoFilename, devotedUserList;
		string	  vendorID, vendorName;
		string	  isComplained, complainedUserList;
	};

	ostringstream	   ostResult;
	int				 itemsCount;
	vector<ItemClass>   itemsList;

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: start");
	}

	ostResult.str("");
	itemsCount = db->Query(dbQuery);
	if(itemsCount)
	{
		for(int i = 0; i < itemsCount; i++) 
		{
			ItemClass   item;

			item.id				 = db->Get(i, "id");
			item.title			  = db->Get(i, "title");
			item.vendorID		   = db->Get(i, "vendor_id");
			item.vendorName		 = "";
			item.photoFolder		= db->Get(i, "logo_folder");
			item.photoFilename	  = db->Get(i, "logo_filename");
			item.devotedUserList	= "";

			itemsList.push_back(item);						
		}
		
		for(int i = 0; i < itemsCount; i++) 
		{
				if(db->Query("SELECT `name` FROM `company` WHERE `id`=\"" + itemsList.at(i).vendorID + "\";"))
					itemsList.at(i).vendorName = db->Get(0, "name");

				if(includeDevoted)
				{
					string temp = "";

					for(int j = 0; j < db->Query("SELECT * from `users_certifications` WHERE `track_id`=\"" + itemsList.at(i).id + "\";"); ++j)
					{
						if(temp.length()) temp += ",";
						temp += db->Get(j, "user_id");
					}

					itemsList.at(i).devotedUserList = temp;
				}

				if(ostResult.str().length()) ostResult << ", ";

				ostResult << "{"
						  << "\"certificationID\": \""				 << itemsList.at(i).id << "\", "
						  << "\"certificationTitle\": \""			  << itemsList.at(i).title << "\", "
						  << "\"certificationVendorID\": \""		   << itemsList.at(i).vendorID << "\", "
						  << "\"certificationVendorName\": \""		 << itemsList.at(i).vendorName << "\", "
						  << "\"certificationPhotoCoverFolder\": \""   << itemsList.at(i).photoFolder << "\","
						  << "\"certificationPhotoCoverFilename\": \"" << itemsList.at(i).photoFilename << "\","
						  << "\"certificationReadersUserID\": ["	   << itemsList.at(i).devotedUserList << "]"
						  << "}";
		} // --- for loop through user list
	} // --- if sql-query on user selection success
	else
	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: there are no certifications returned by the request [", dbQuery, "]");
	}

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: finish (returning string length = " + to_string(ostResult.str().length()) + ")");
	}

	return ostResult.str();
}

// input: ....
//		  includeStudents will add student counter
string GetCourseListInJSONFormat(string dbQuery, CMysql *db, bool includeStudents/* = false*/)
{
	struct ItemClass {
		string	  id, title, photoFolder, photoFilename, studentUserList;
		string	  vendorID, vendorName;
		string	  isComplained, complainedUserList;
	};

	ostringstream	   ostResult;
	int				 itemsCount;
	vector<ItemClass>   itemsList;

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: start");
	}

	ostResult.str("");
	itemsCount = db->Query(dbQuery);
	if(itemsCount)
	{
		for(int i = 0; i < itemsCount; i++) 
		{
			ItemClass   item;

			item.id				 = db->Get(i, "id");
			item.title			  = db->Get(i, "title");
			item.vendorID		   = db->Get(i, "vendor_id");
			item.vendorName		 = "";
			item.photoFolder		= db->Get(i, "logo_folder");
			item.photoFilename	  = db->Get(i, "logo_filename");
			item.studentUserList	= "";

			itemsList.push_back(item);						
		}
		
		for(int i = 0; i < itemsCount; i++) 
		{
				if(db->Query("SELECT `name` FROM `company` WHERE `id`=\"" + itemsList.at(i).vendorID + "\";"))
					itemsList.at(i).vendorName = db->Get(0, "name");

				if(includeStudents)
				{
					string temp = "";

					for(int j = 0; j < db->Query("SELECT * from `users_courses` WHERE `track_id`=\"" + itemsList.at(i).id + "\";"); ++j)
					{
						if(temp.length()) temp += ",";
						temp += db->Get(j, "user_id");
					}

					itemsList.at(i).studentUserList = temp;
				}

				if(ostResult.str().length()) ostResult << ", ";

				ostResult << "{"
						  << "\"courseID\": \""				 << itemsList.at(i).id << "\", "
						  << "\"courseTitle\": \""			  << itemsList.at(i).title << "\", "
						  << "\"courseVendorID\": \""		   << itemsList.at(i).vendorID << "\", "
						  << "\"courseVendorName\": \""		 << itemsList.at(i).vendorName << "\", "
						  << "\"coursePhotoCoverFolder\": \""   << itemsList.at(i).photoFolder << "\","
						  << "\"coursePhotoCoverFilename\": \"" << itemsList.at(i).photoFilename << "\","
						  << "\"courseStudentsUserID\": ["	   << itemsList.at(i).studentUserList << "]"
						  << "}";
		} // --- for loop through user list
	} // --- if sql-query on user selection success
	else
	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: there are no courses returned by the request [", dbQuery, "]");
	}

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: finish (returning string length = " + to_string(ostResult.str().length()) + ")");
	}

	return ostResult.str();
}

// input: ....
//		  includeStudents will add student counter
string GetUniversityListInJSONFormat(string dbQuery, CMysql *db, bool includeStudents/* = false*/)
{
	struct ItemClass {
		string	  id, title, photoFolder, photoFilename, studentUserList;
		string	  regionID, regionTitle;
		string	  isComplained, complainedUserList;
	};

	ostringstream	   ostResult;
	int				 itemsCount;
	vector<ItemClass>   itemsList;

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: start");
	}

	ostResult.str("");
	itemsCount = db->Query(dbQuery);
	if(itemsCount)
	{
		for(int i = 0; i < itemsCount; i++) 
		{
			ItemClass   item;

			item.id				 = db->Get(i, "id");
			item.title			  = db->Get(i, "title");
			item.regionID		   = db->Get(i, "geo_region_id");
			item.regionTitle		= "";
			item.photoFolder		= db->Get(i, "logo_folder");
			item.photoFilename	  = db->Get(i, "logo_filename");
			item.studentUserList	= "";

			itemsList.push_back(item);						
		}
		
		for(int i = 0; i < itemsCount; i++) 
		{
				if(db->Query("SELECT `title` FROM `geo_region` WHERE `id`=\"" + itemsList.at(i).regionID + "\";"))
					itemsList.at(i).regionTitle = db->Get(0, "title");

				if(includeStudents)
				{
					string temp = "";

					for(int j = 0; j < db->Query("SELECT * from `users_university` WHERE `university_id`=\"" + itemsList.at(i).id + "\";"); ++j)
					{
						if(temp.length()) temp += ",";
						temp += db->Get(j, "user_id");
					}

					itemsList.at(i).studentUserList = temp;
				}

				if(ostResult.str().length()) ostResult << ", ";

				ostResult << "{"
						  << "\"universityID\": \""				 << itemsList.at(i).id << "\", "
						  << "\"universityTitle\": \""			  << itemsList.at(i).title << "\", "
						  << "\"universityRegionID\": \""		   << itemsList.at(i).regionID << "\", "
						  << "\"universityRegionName\": \""		 << itemsList.at(i).regionTitle << "\", "
						  << "\"universityPhotoCoverFolder\": \""   << itemsList.at(i).photoFolder << "\","
						  << "\"universityPhotoCoverFilename\": \"" << itemsList.at(i).photoFilename << "\","
						  << "\"universityStudentsUserID\": ["	   << itemsList.at(i).studentUserList << "]"
						  << "}";
		} // --- for loop through user list
	} // --- if sql-query on user selection success
	else
	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: there are no university's returned by the request [", dbQuery, "]");
	}

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: finish (returning string length = " + to_string(ostResult.str().length()) + ")");
	}

	return ostResult.str();
}

// input: ....
//		  includeStudents will add student counter
string GetSchoolListInJSONFormat(string dbQuery, CMysql *db, bool includeStudents/* = false*/)
{
	struct ItemClass {
		string	  id, title, photoFolder, photoFilename, studentUserList;
		string	  regionID, regionTitle;
		string	  isComplained, complainedUserList;
	};

	ostringstream	   ostResult;
	int				 itemsCount;
	vector<ItemClass>   itemsList;

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: start");
	}

	ostResult.str("");
	itemsCount = db->Query(dbQuery);
	if(itemsCount)
	{
		for(int i = 0; i < itemsCount; i++) 
		{
			ItemClass   item;

			item.id				 = db->Get(i, "id");
			item.title			  = db->Get(i, "title");
			item.regionID		   = db->Get(i, "geo_locality_id");
			item.regionTitle		= "";
			item.photoFolder		= db->Get(i, "logo_folder");
			item.photoFilename	  = db->Get(i, "logo_filename");
			item.studentUserList	= "";

			itemsList.push_back(item);						
		}
		
		for(int i = 0; i < itemsCount; i++) 
		{
				if(db->Query("SELECT `title` FROM `geo_locality` WHERE `id`=\"" + itemsList.at(i).regionID + "\";"))
					itemsList.at(i).regionTitle = db->Get(0, "title");

				if(includeStudents)
				{
					string temp = "";

					for(int j = 0; j < db->Query("SELECT * from `users_school` WHERE `school_id`=\"" + itemsList.at(i).id + "\";"); ++j)
					{
						if(temp.length()) temp += ",";
						temp += db->Get(j, "user_id");
					}

					itemsList.at(i).studentUserList = temp;
				}

				if(ostResult.str().length()) ostResult << ", ";

				ostResult << "{"
						  << "\"schoolID\": \""				 << itemsList.at(i).id << "\", "
						  << "\"schoolTitle\": \""			  << itemsList.at(i).title << "\", "
						  << "\"schoolLocalityID\": \""		 << itemsList.at(i).regionID << "\", "
						  << "\"schoolLocalityTitle\": \""	   << itemsList.at(i).regionTitle << "\", "
						  << "\"schoolPhotoCoverFolder\": \""   << itemsList.at(i).photoFolder << "\","
						  << "\"schoolPhotoCoverFilename\": \"" << itemsList.at(i).photoFilename << "\","
						  << "\"schoolStudentsUserID\": ["	  << itemsList.at(i).studentUserList << "]"
						  << "}";
		} // --- for loop through user list
	} // --- if sql-query on user selection success
	else
	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: there are no school's returned by the request [", dbQuery, "]");
	}

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: finish (returning string length = " + to_string(ostResult.str().length()) + ")");
	}

	return ostResult.str();
}

string GetGiftToGiveListInJSONFormat(string dbQuery, CMysql *db, CUser *user)
{
	struct ItemClass 
	{
		string	id;
		string	user_id;
		string	gift_id;
		string	reserve_period;
		string	visibility;
		string	eventTimestamp;
	};

	ostringstream			ost, ostFinal;
	string					sessid, lookForKey;
	int						affected;
	vector<ItemClass>		gifts_to_give_list;

	{
		MESSAGE_DEBUG("", "", "start");
	}

	ostFinal.str("");

	if((affected = db->Query(dbQuery)) > 0)
	{
		int		groupCounter = affected;

		gifts_to_give_list.reserve(groupCounter);	// --- reserving allows avoid moving vector in memory
											// --- to fit vector into continuous memory piece

		for(int i = 0; i < affected; i++)
		{
			ItemClass	gift;

			gift.id = db->Get(i, "id");
			gift.user_id = db->Get(i, "user_id");
			gift.gift_id = db->Get(i, "gift_id");
			gift.reserve_period = db->Get(i, "reserve_period");
			gift.visibility = db->Get(i, "visibility");
			gift.eventTimestamp = db->Get(i, "eventTimestamp");

			gifts_to_give_list.push_back(gift);
		}

		for(int i = 0; i < groupCounter; i++)
		{
				if(ostFinal.str().length()) ostFinal << ", ";

				ostFinal << "{";
				ostFinal << "\"id\": \""				<< gifts_to_give_list[i].id << "\",";
				ostFinal << "\"user_id\": \""			<< (user && (user->GetID() == gifts_to_give_list[i].user_id) ? user->GetID() : "") << "\",";
				ostFinal << "\"gift_id\": \""			<< gifts_to_give_list[i].gift_id << "\",";
				ostFinal << "\"reserve_period\": \""	<< gifts_to_give_list[i].reserve_period << "\",";
				ostFinal << "\"visibility\": \""		<< gifts_to_give_list[i].visibility << "\",";
				ostFinal << "\"eventTimestamp\": \""	<< gifts_to_give_list[i].eventTimestamp << "\"";
				ostFinal << "}";
		} // --- for loop through gift list

	} // --- if sql-query on gift selection success
	else
	{
		MESSAGE_DEBUG("", "", "there are no gifts returned by request [" + dbQuery + "]");
	}

	{
		MESSAGE_DEBUG("", "", "end (result length = " + to_string(ostFinal.str().length()) + ")");
	}

	return ostFinal.str();
}

string GetEventListInJSONFormat(string dbQuery, CMysql *db, CUser *user)
{
	struct ItemClass 
	{
		string	id;
		string	link;
		string	title;
		string	address;
		string	accessType;
		string	startTimestamp;
		string	description;
		string	logo_folder;
		string	logo_filename;
		string	owner_id;
		string	hideGifts;
		string	isBlocked;
		string	eventTimestampCreation;
		string	eventTimestampLastPost;
	};

	ostringstream			ost, ostFinal;
	string					sessid, lookForKey;
	int						affected;
	vector<ItemClass>		eventsList;

	{
		MESSAGE_DEBUG("", "", "start");
	}

	ostFinal.str("");

	if((affected = db->Query(dbQuery)) > 0)
	{
		int						eventCounter = affected;

		eventsList.reserve(eventCounter);  // --- reserving allows avoid moving vector in memory
											// --- to fit vector into continuous memory piece

		for(int i = 0; i < affected; i++)
		{
			ItemClass	event;

			event.id = db->Get(i, "id");
			event.link = db->Get(i, "link");
			event.title = db->Get(i, "title");
			event.address = db->Get(i, "address");
			event.accessType = db->Get(i, "accessType");
			event.startTimestamp = db->Get(i, "startTimestamp");
			event.description = db->Get(i, "description");
			event.logo_folder = db->Get(i, "logo_folder");
			event.logo_filename = db->Get(i, "logo_filename");
			event.owner_id = db->Get(i, "owner_id");
			event.hideGifts = db->Get(i, "hideGifts");
			event.isBlocked = db->Get(i, "isBlocked");
			event.eventTimestampCreation = db->Get(i, "eventTimestampCreation");
			event.eventTimestampLastPost = db->Get(i, "eventTimestampLastPost");

			eventsList.push_back(event);
		}

		for(int i = 0; i < eventCounter; i++)
		{
				if(ostFinal.str().length()) ostFinal << ", ";

				ostFinal << "{";
				ostFinal << "\"id\": \""				  	<< eventsList[i].id << "\",";
				ostFinal << "\"link\": \""					<< eventsList[i].link << "\",";
				ostFinal << "\"title\": \""					<< eventsList[i].title << "\",";
				ostFinal << "\"address\": \""				<< eventsList[i].address << "\",";
				ostFinal << "\"accessType\": \""			<< eventsList[i].accessType << "\",";
				ostFinal << "\"startTimestamp\": \""		<< eventsList[i].startTimestamp << "\",";
				ostFinal << "\"description\": \""			<< eventsList[i].description << "\",";
				ostFinal << "\"logo_folder\": \""			<< eventsList[i].logo_folder << "\",";
				ostFinal << "\"logo_filename\": \""			<< eventsList[i].logo_filename << "\",";
				ostFinal << "\"isMine\": \""				<< (user ? eventsList[i].owner_id == user->GetID() : false) << "\",";
				ostFinal << "\"hosts\": ["					<< GetEventHostsListInJSONFormat("SELECT * FROM `event_hosts` WHERE `event_id`=\"" + eventsList[i].id + "\";", db, user) << "],";
				ostFinal << "\"guests\": ["					<< GetEventGuestsListInJSONFormat("SELECT * FROM `event_guests` WHERE `event_id`=\"" + eventsList[i].id + "\";", db, user) << "],";
				ostFinal << "\"checklists\": ["				<< GetEventChecklistInJSONFormat("SELECT * FROM `event_checklists` WHERE `event_id`=\"" + eventsList[i].id + "\";", db, user) << "],";
				ostFinal << "\"hideGifts\": \""				<< eventsList[i].hideGifts << "\",";
				ostFinal << "\"isBlocked\": \""				<< eventsList[i].isBlocked << "\",";
				ostFinal << "\"eventTimestampCreation\": \""<< eventsList[i].eventTimestampCreation << "\",";
				ostFinal << "\"eventTimestampLastPost\": \""<< eventsList[i].eventTimestampLastPost << "\"";
				ostFinal << "}";
		} // --- for loop through event list
	} // --- if sql-query on event selection success
	else
	{
		MESSAGE_DEBUG("", "", "there are no events returned by request [" + dbQuery + "]");
	}

	{
		MESSAGE_DEBUG("", "", "end (result length = " + to_string(ostFinal.str().length()) + ")");
	}

	return ostFinal.str();
}

string GetEventHostsListInJSONFormat(string dbQuery, CMysql *db, CUser *user)
{
	struct ItemClass 
	{
		string	id;
		string	event_id;
		string	user_id;
		string	eventTimestamp;
	};

	ostringstream			ost, ostFinal;
	string					sessid, lookForKey;
	int						affected;
	vector<ItemClass>		eventHostsList;

	{
		MESSAGE_DEBUG("", "", "start");
	}

	ostFinal.str("");

	if((affected = db->Query(dbQuery)) > 0)
	{
		int		eventHostsCounter = affected;

		eventHostsList.reserve(eventHostsCounter);  // --- reserving allows avoid moving vector in memory
											// --- to fit vector into continuous memory piece

		for(int i = 0; i < eventHostsCounter; i++)
		{
			ItemClass	event_host;

			event_host.id = db->Get(i, "id");
			event_host.event_id = db->Get(i, "event_id");
			event_host.user_id = db->Get(i, "user_id");
			event_host.eventTimestamp = db->Get(i, "eventTimestamp");

			eventHostsList.push_back(event_host);
		}

		for(int i = 0; i < eventHostsCounter; i++)
		{
				if(db->Query("SELECT `id`,`name`,`nameLast` FROM `users` WHERE `id`=\"" + eventHostsList[i].user_id + "\";"))
				{
					string		userName = db->Get(0, "name");
					string		userNameLast = db->Get(0, "nameLast");
					string		avatar = "";

					if(db->Query("SELECT `folder`, `filename` FROM `users_avatars` WHERE `isActive`=\"1\" AND `userid`=\"" + eventHostsList[i].user_id + "\";"))
						avatar = string("/images/avatars/avatars") + db->Get(0, "folder") + "/" + db->Get(0, "filename");

					if(ostFinal.str().length()) ostFinal << ", ";

					ostFinal << "{";
					ostFinal << "\"id\": \""				<< eventHostsList[i].id << "\",";
					ostFinal << "\"user_id\": \""			<< eventHostsList[i].user_id << "\",";
					ostFinal << "\"name\": \""				<< userName << "\",";
					ostFinal << "\"nameLast\": \""			<< userNameLast << "\",";
					ostFinal << "\"avatar\": \""			<< avatar << "\",";
					ostFinal << "\"status\": \""			<< "accepted\",";
					ostFinal << "\"gifts\": ["				<< GetGiftListInJSONFormat("SELECT * FROM `gifts` WHERE `user_id`=\"" + eventHostsList[i].user_id + "\";", db, user) << "],";
					ostFinal << "\"gifts_to_give\": [" 		<< GetGiftToGiveListInJSONFormat("SELECT * FROM `gifts_to_give` WHERE `gift_id` in (SELECT `id` FROM `gifts` WHERE `user_id`=\"" + eventHostsList[i].user_id + "\");", db, user) << "],";
					ostFinal << "\"eventTimestamp\": \""	<< eventHostsList[i].eventTimestamp << "\"";
					ostFinal << "}";

				}
				else
				{
					MESSAGE_ERROR("", "", "event_id(" + eventHostsList[i].event_id + ") user_id(" + eventHostsList[i].user_id + "), host_user_id not found");
				}
		}
	} // --- if sql-query on event selection success
	else
	{
		MESSAGE_DEBUG("", "", "there are no events returned by request [" + dbQuery + "]");
	}

	{
		MESSAGE_DEBUG("", "", "end (result length = " + to_string(ostFinal.str().length()) + ")");
	}

	return ostFinal.str();
}

// --- Function returns list of users "liked" usersCertificationID in JSON-format
// --- input: usersCertificationID, user, db
//			user - used for friendship status definition
// --- output: was this message "liked" by particular user or not
string GetCertificationLikesUsersList(string usersCertificationID, CUser *user, CMysql *db)
{
	ostringstream   ost;
	int			 affected;
	string		  result = "";

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: start");
	}

	affected = db->Query("select * from `feed_message_params` where `parameter`='likeCertification' and `messageID`='" + usersCertificationID + "';");
	if(affected > 0) 
	{
		ost.str("");
		ost << "select * from `users` where id in (";
		for(int i = 0; i < affected; i++)
		{
			if(i > 0) ost << ",";
			ost << db->Get(i, "userID");
		}
		ost << ") and `isactivated`='Y' and `isblocked`='N';";
		result = GetUserListInJSONFormat(ost.str(), db, user);
	}

	{
		CLog			log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: end (returning string length " + to_string(result.length()) + ")");
	}

	return result;
}

// --- Function returns list of users "liked" usersCourseID in JSON-format
// --- input: usersCourseID, user, db
//			user - used for friendship status definition
// --- output: was this message "liked" by particular user or not
string GetCourseLikesUsersList(string usersCourseID, CUser *user, CMysql *db)
{
	ostringstream   ost;
	int			 affected;
	string		  result = "";

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: start");
	}

	affected = db->Query("select * from `feed_message_params` where `parameter`='likeCourse' and `messageID`='" + usersCourseID + "';");
	if(affected > 0) 
	{
		ost.str("");
		ost << "select * from `users` where id in (";
		for(int i = 0; i < affected; i++)
		{
			if(i > 0) ost << ",";
			ost << db->Get(i, "userID");
		}
		ost << ") and `isactivated`='Y' and `isblocked`='N';";
		result = GetUserListInJSONFormat(ost.str(), db, user);
	}

	{
		CLog			log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: end (returning string length " + to_string(result.length()) + ")");
	}

	return result;
}

// --- Function returns list of users "liked" universityDegreeID in JSON-format
// --- input: universityDegreeID, user, db
//			user - used for friendship status definition
// --- output: was this message "liked" by particular user or not
string GetUniversityDegreeLikesUsersList(string universityDegreeID, CUser *user, CMysql *db)
{
	ostringstream   ost;
	int			 affected;
	string		  result = "";

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: start");
	}

	affected = db->Query("select * from `feed_message_params` where `parameter`='likeUniversityDegree' and `messageID`='" + universityDegreeID + "';");
	if(affected > 0) 
	{
		ost.str("");
		ost << "select * from `users` where id in (";
		for(int i = 0; i < affected; i++)
		{
			if(i > 0) ost << ",";
			ost << db->Get(i, "userID");
		}
		ost << ") and `isactivated`='Y' and `isblocked`='N';";
		result = GetUserListInJSONFormat(ost.str(), db, user);
	}

	{
		CLog			log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: end (returning string length " + to_string(result.length()) + ")");
	}

	return result;
}

// --- Function returns list of users "liked" bookID in JSON-format
// --- input: bookID, user, db
//			user - used for friendship status definition
// --- output: was this message "liked" by particular user or not
string GetBookRatingUsersList(string bookID, CUser *user, CMysql *db)
{
	ostringstream   ost;
	int			 affected;
	string		  result = "";

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: start");
	}

	affected = db->Query("select * from `users_books` where `bookID`=\"" + bookID + "\";");
	if(affected > 0) 
	{
		ost.str("");
		ost << "select * from users where id in (";
		for(int i = 0; i < affected; ++i)
		{
			if(i) ost << ",";
			ost << db->Get(i, "userID");
		}
		ost << ") and `isactivated`='Y' and `isblocked`='N';";
		result = GetUserListInJSONFormat(ost.str(), db, user);
	}

	{
		CLog			log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: end (returning string length " + to_string(result.length()) + ")");
	}

	return result;
}

// --- Function returns array of book rating
// --- input: bookID, db
// --- output: book rating array
string GetBookRatingList(string bookID, CMysql *db)
{
	int			 affected;
	string		  result = "";

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: start");
	}

	affected = db->Query("select * from `users_books` where `bookID`=\"" + bookID + "\";");
	if(affected > 0) 
	{
		for(int i = 0; i < affected; ++i)
		{
			if(i) result += ",";
			result += db->Get(i, "rating");
		}
	}

	{
		CLog			log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: end (returning string length " + to_string(result.length()) + ")");
	}

	return result;
}

// --- Function returns array of course rating
// --- input: courseID, db
// --- output: course rating array
string GetCourseRatingList(string courseID, CMysql *db)
{
	int				affected;
	string			result = "";

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: start");
	}

	affected = db->Query("select * from `users_courses` where `track_id`=\"" + courseID + "\";");
	if(affected > 0) 
	{
		for(int i = 0; i < affected; ++i)
		{
			if(i) result += ",";
			result += db->Get(i, "rating");
		}
	}

	{
		CLog			log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: end (returning string length " + to_string(result.length()) + ")");
	}

	return result;
}

// --- Function returns list of users "liked" usersCompanyID in JSON-format
// --- input: usersCompanyID, user, db
//			user - used for friendship status definition
// --- output: was this message "liked" by particular user or not
string GetCompanyLikesUsersList(string usersCompanyID, CUser *user, CMysql *db)
{
	ostringstream   ost;
	int			 affected;
	string		  result = "";

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: start");
	}

	affected = db->Query("select * from `feed_message_params` where `parameter`='likeCompany' and `messageID`='" + usersCompanyID + "';");
	if(affected > 0) 
	{
		ost.str("");
		ost << "select * from `users` where id in (";
		for(int i = 0; i < affected; i++)
		{
			if(i > 0) ost << ",";
			ost << db->Get(i, "userID");
		}
		ost << ") and `isactivated`='Y' and `isblocked`='N';";
		result = GetUserListInJSONFormat(ost.str(), db, user);
	}

	{
		CLog			log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: end (returning string length " + to_string(result.length()) + ")");
	}

	return result;
}

// --- Function returns list of users "liked" usersLanguageID in JSON-format
// --- input: usersLanguageID, user, db
//			user - used for friendship status definition
// --- output: was this message "liked" by particular user or not
string GetLanguageLikesUsersList(string usersLanguageID, CUser *user, CMysql *db)
{
	ostringstream   ost;
	int			 affected;
	string		  result = "";

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: start");
	}

	affected = db->Query("select * from `feed_message_params` where `parameter`='likeLanguage' and `messageID`='" + usersLanguageID + "';");
	if(affected > 0) 
	{
		ost.str("");
		ost << "select * from `users` where id in (";
		for(int i = 0; i < affected; i++)
		{
			if(i > 0) ost << ",";
			ost << db->Get(i, "userID");
		}
		ost << ") and `isactivated`='Y' and `isblocked`='N';";
		result = GetUserListInJSONFormat(ost.str(), db, user);
	}

	{
		CLog			log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: end (returning string length " + to_string(result.length()) + ")");
	}

	return result;
}

// --- Function returns list of users "liked" usersBookID in JSON-format
// --- input: usersBookID, user, db
//			user - used for friendship status definition
// --- output: was this message "liked" by particular user or not
string GetBookLikesUsersList(string usersBookID, CUser *user, CMysql *db)
{
	ostringstream   ost;
	int			 affected;
	string		  result = "";

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: start");
	}

	affected = db->Query("select * from `feed_message_params` where `parameter`='likeBook' and `messageID`='" + usersBookID + "';");
	if(affected > 0) 
	{
		ost.str("");
		ost << "select * from `users` where id in (";
		for(int i = 0; i < affected; i++)
		{
			if(i > 0) ost << ",";
			ost << db->Get(i, "userID");
		}
		ost << ") and `isactivated`='Y' and `isblocked`='N';";
		result = GetUserListInJSONFormat(ost.str(), db, user);
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

	MESSAGE_DEBUG("", "", "start");

	ost.str("");
	ost << "select count(*) as `counter` from `feed_message_comment` where `type`=\"message\" and `messageID`='" << messageID << "';";
	affected = db->Query(ost.str());
	if(affected > 0)
	{
		result = db->Get(0, "counter");
	}

	MESSAGE_DEBUG("", "", "finish");


	return result;
}

string GetCompanyCommentsCount(string messageID, CMysql *db)
{
	ostringstream   ost;
	int			 affected;
	string		  result = "0";

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: start");
	}

	ost.str("");
	ost << "select count(*) as `counter` from `feed_message_comment` where `type`=\"company\" and `messageID`='" << messageID << "';";
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

string GetLanguageCommentsCount(string messageID, CMysql *db)
{
	ostringstream   ost;
	int			 affected;
	string		  result = "0";

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: start");
	}

	ost.str("");
	ost << "select count(*) as `counter` from `feed_message_comment` where `type`=\"language\" and `messageID`='" << messageID << "';";
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

string GetBookCommentsCount(string messageID, CMysql *db)
{
	ostringstream   ost;
	int			 affected;
	string		  result = "0";

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: start");
	}

	ost.str("");
	ost << "select count(*) as `counter` from `feed_message_comment` where `type`=\"book\" and `messageID`='" << messageID << "';";
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

string GetCertificateCommentsCount(string messageID, CMysql *db)
{
	ostringstream   ost;
	int			 affected;
	string		  result = "0";

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: start");
	}

	ost.str("");
	ost << "select count(*) as `counter` from `feed_message_comment` where `type` in (\"certification\", \"course\") and `messageID`='" << messageID << "';";
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

string GetUniversityDegreeCommentsCount(string messageID, CMysql *db)
{
	ostringstream   ost;
	int			 affected;
	string		  result = "0";

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: start");
	}

	ost.str("");
	ost << "select count(*) as `counter` from `feed_message_comment` where `type`=\"university\" and `messageID`='" << messageID << "';";
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

string GetLanguageIDByTitle(string languageTitle, CMysql *db)
{
	ostringstream   ost;
	string		  	result = "";
	string			languageID = "0";

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: start");
	}

	if(languageTitle.length())
	{
		if(db->Query("SELECT `id` FROM `language` WHERE `title`=\"" + languageTitle + "\";"))
		{
			languageID = db->Get(0, "id");
		}
		else
		{
			long int 	tmp;
			{
				CLog			log;
				log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: languageTitle [" + languageTitle + "] not found. Creating new one.");
			}

			tmp = db->InsertQuery("INSERT INTO `language` SET `title`=\"" + languageTitle + "\";");
			if(tmp) 
				languageID = to_string(tmp);
			else
			{
				CLog			log;
				log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR: insert into language");
			}
		}
	}
	else
	{
		{
			CLog			log;
			log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: languageTitle is empty");
		}
	}

	result = languageID;

	{
		CLog			log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: end (returning string length " + to_string(result.length()) + ")");
	}


	return result;
}

string GetSkillIDByTitle(string skillTitle, CMysql *db)
{
	ostringstream   ost;
	string		  	result = "";
	string			languageID = "0";

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: start");
	}

	if(skillTitle.length())
	{
		if(db->Query("SELECT `id` FROM `skill` WHERE `title`=\"" + skillTitle + "\";"))
		{
			languageID = db->Get(0, "id");
		}
		else
		{
			long int 	tmp;
			{
				CLog			log;
				log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: skillTitle [" + skillTitle + "] not found. Creating new one.");
			}

			tmp = db->InsertQuery("INSERT INTO `skill` SET `title`=\"" + skillTitle + "\";");
			if(tmp) 
				languageID = to_string(tmp);
			else
			{
				CLog			log;
				log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR: insert into skill");
			}
		}
	}
	else
	{
		{
			CLog			log;
			log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: skillTitle is empty");
		}
	}

	result = languageID;

	{
		CLog			log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: end (returning string length " + to_string(result.length()) + ")");
	}

	return result;
}

string GetCompanyPositionIDByTitle(string positionTitle, CMysql *db)
{
	ostringstream   ost;
	string		  	result = "";
	string			positionID = "";

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: start");
	}

	if(positionTitle.length())
	{
		if(db->Query("SELECT `id` FROM `company_position` WHERE `title`=\"" + positionTitle + "\";"))
		{
			positionID = db->Get(0, "id");
		}
		else
		{
			long int 	tmp;
			{
				CLog			log;
				log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: position[" + positionTitle + "] not found. Creating new one.");
			}

			tmp = db->InsertQuery("INSERT INTO `company_position` SET `area`=\"\", `title`=\"" + positionTitle + "\";");
			if(tmp) 
				positionID = to_string(tmp);
			else
			{
				CLog			log;
				log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR: insert into company_position");
			}
		}
	}
	else
	{
		{
			CLog			log;
			log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: positionTitle is empty");
		}
	}

	result = positionID;

	{
		CLog			log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: end (returning string length " + to_string(result.length()) + ")");
	}


	return result;
}

string GetCandidatesListAppliedToVacancyInJSONFormat(string dbQuery, CMysql *db)
{
	string			result = "";
	int				itemsCount;
	ostringstream	ostResult;

	struct ItemClass 
	{
		string	id;
		string	vacancy_id;
		string	user_id;
		string	answer1;
		string	answer2;
		string	answer3;
		string	language1;
		string	language2;
		string	language3;
		string	skill1;
		string	skill2;
		string	skill3;
		string	description;
		string	status;
		string	eventTimestamp;
	};
	vector<ItemClass>		itemsList;

	{
		MESSAGE_DEBUG("", "", "start");
	}

	ostResult.str("");

	itemsCount = db->Query(dbQuery);
	for(int i = 0; i < itemsCount; ++i)
	{
		ItemClass	item;

		item.id = db->Get(i, "id");
		item.vacancy_id = db->Get(i, "vacancy_id");
		item.user_id = db->Get(i, "user_id");
		item.answer1 = db->Get(i, "answer1");
		item.answer2 = db->Get(i, "answer2");
		item.answer3 = db->Get(i, "answer3");
		item.language1 = db->Get(i, "language1");
		item.language2 = db->Get(i, "language2");
		item.language3 = db->Get(i, "language3");
		item.skill1 = db->Get(i, "skill1");
		item.skill2 = db->Get(i, "skill2");
		item.skill3 = db->Get(i, "skill3");
		item.description = db->Get(i, "description");
		item.status = db->Get(i, "status");
		item.eventTimestamp = db->Get(i, "eventTimestamp");

		itemsList.push_back(item);
	}

	for(int i = 0; i < itemsCount; ++i)
	{
		string	name = "", nameLast = "";

		if(ostResult.str().length()) ostResult << ", ";

		ostResult << "{"
					  "\"id\":\"" << itemsList[i].id << "\","
					  "\"vacancy_id\":\"" << itemsList[i].vacancy_id << "\","
					  "\"user_id\":\"" << itemsList[i].user_id << "\","
					  "\"user\":" << GetUserListInJSONFormat("SELECT * FROM `users` WHERE `id`=\"" + itemsList[i].user_id + "\" AND `isblocked`=\"N\";", db, NULL) << ","
					  "\"answer1\":\"" << itemsList[i].answer1 << "\","
					  "\"answer2\":\"" << itemsList[i].answer2 << "\","
					  "\"answer3\":\"" << itemsList[i].answer3 << "\","
					  "\"language1\":\"" << itemsList[i].language1 << "\","
					  "\"language2\":\"" << itemsList[i].language2 << "\","
					  "\"language3\":\"" << itemsList[i].language3 << "\","
					  "\"skill1\":\"" << itemsList[i].skill1 << "\","
					  "\"skill2\":\"" << itemsList[i].skill2 << "\","
					  "\"skill3\":\"" << itemsList[i].skill3 << "\","
					  "\"description\":\"" << itemsList[i].description << "\","
					  "\"status\":\"" << itemsList[i].status << "\","
					  "\"eventTimestamp\":\"" << itemsList[i].eventTimestamp << "\""
					  "}";
	}

	result = ostResult.str();

	{
		MESSAGE_DEBUG("", "", "finish (result.length() = " + to_string(result.length()) + ")");
	}
	return result;
}

// --- Returns company list in JSON format grabbed from DB
// --- Input:   dbQuery - SQL format returns users
//			  db	  - DB connection
//			  user	- current user object, used to define company admin
//			  quickSearch	- owners, founders, openVacancies are not included into result
//			  includeEmployedUsersList	- used in admin _ONLY_ to get company list + # of users in each company
string GetCompanyListInJSONFormat(string dbQuery, CMysql *db, CUser *user, bool quickSearch/* = true*/, bool includeEmployedUsersList /*= false*/)
{
	struct CompanyClass {
		string	id, name, link, foundationDate, numberOfEmployee, admin_userID, webSite, description;
		string	type, logo_folder, logo_filename;
		string	employedUsersList;
	};

	ostringstream				   ost, ostFinal;
	string						  sessid, lookForKey;
	int							 affected;
	vector<CompanyClass>			companiesList;
	int							 companyCounter = 0;

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: start");
	}

	ostFinal.str("");

	if((affected = db->Query(dbQuery)) > 0)
	{
		companyCounter = affected;
		companiesList.reserve(companyCounter);  // --- reserving allows avoid moving vector in memory
												// --- to fit vector into continuous memory piece

		for(int i = 0; i < affected; i++)
		{
			CompanyClass	company;

			company.id = db->Get(i, "id");
			company.name = db->Get(i, "name");
			company.link = db->Get(i, "link");
			company.admin_userID = db->Get(i, "admin_userID");
			company.foundationDate = db->Get(i, "foundationDate");
			company.numberOfEmployee = db->Get(i, "numberOfEmployee");
			company.webSite = db->Get(i, "webSite");
			company.description = db->Get(i, "description");
			company.type = db->Get(i, "type");
			company.logo_folder = db->Get(i, "logo_folder");
			company.logo_filename = db->Get(i, "logo_filename");
			company.employedUsersList = "";
			companiesList.push_back(company);
		}

		for(int i = 0; i < companyCounter; i++)
		{
				string			  companyOwners = "";
				string			  companyFounders = "";
				string			  companyIndustry = "";

				if(includeEmployedUsersList)
				{
					// --- VERY VERY VERY !!! slow implementation 
					// --- do NOT use it in production
					string	  temp = "";

					for(int j = 0; j < db->Query("select `user_id` from `users_company` where `company_id`=\"" + companiesList[i].id + "\";"); ++j)
					{
						if(temp.length()) temp += ",";
						temp += db->Get(j, "user_id");
					}
					companiesList[i].employedUsersList = temp;
				}

				if(!quickSearch)
				{
					// --- company industry list
					ost.str("");
					ost << "select `company_industry_ref`.`id` as `company_industry_ref_id`, `company_industry`.`name` as `company_industry_name` from `company_industry_ref` \
	right join `company_industry` on `company_industry_ref`.`profile_id`=`company_industry`.`id`\
	where `company_industry_ref`.`company_id`=\"" << companiesList[i].id << "\"";
					affected = db->Query(ost.str());
					if(affected)
						for(int i = 0; i < affected; i++) 
						{
							if(i) companyIndustry += ",";
							companyIndustry += "{\"company_industry_ref_id\":\"";
							companyIndustry += db->Get(i, "company_industry_ref_id");
							companyIndustry += "\",\"name\":\"";
							companyIndustry += db->Get(i, "company_industry_name");
							companyIndustry += "\"}";
						}

					// --- company owners list
					ost.str("");
					ost << "select * from `company_owner`  where `companyID`=\"" << companiesList[i].id << "\"";
					affected = db->Query(ost.str());
					if(affected)
					{
						struct  CompanyOwnerType 
						{
							string  id, name, userID;
						};
						vector<CompanyOwnerType>	tempVector;
						int						   vectorSize = affected;

						tempVector.reserve(vectorSize);

						for(int i = 0; i < vectorSize; i++) 
						{
							CompanyOwnerType  tempObj;

							tempObj.id = db->Get(i, "id");
							tempObj.name = db->Get(i, "owner_name");
							tempObj.userID = db->Get(i, "owner_userID");

							tempVector.push_back(tempObj);
						}

						for(int i = 0; i < affected; i++) 
						{
							if(i) companyOwners += ",";
							companyOwners += "{\"id\":\"";
							companyOwners += tempVector.at(i).id;
							companyOwners += "\",\"name\":\"";
							if(tempVector.at(i).userID != "0")
							{
								ost.str("");
								ost << "select * from `users` where `id`=\"" << tempVector.at(i).userID << "\";";
								if(db->Query(ost.str()))
								{
									companyOwners += db->Get(0, "name");
									companyOwners += " ";
									companyOwners += db->Get(0, "nameLast");
								}
								else
								{
									CLog	log;
									log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: there are no user with ID [", tempVector.at(i).userID, "]");
								}
							}
							else
								companyOwners += tempVector.at(i).name;
							companyOwners += "\",\"userid\":\"";
							companyOwners += tempVector.at(i).userID;
							companyOwners += "\"}";
						}
					}

					// --- company founders list
					ost.str("");
					ost << "select * from `company_founder`  where `companyID`=\"" << companiesList[i].id << "\"";
					affected = db->Query(ost.str());
					if(affected)
					{
						struct  CompanyFounderType 
						{
							string  id, name, userID;
						};
						vector<CompanyFounderType>	tempVector;
						int						   vectorSize = affected;

						tempVector.reserve(vectorSize);

						for(int i = 0; i < vectorSize; i++) 
						{
							CompanyFounderType  tempObj;

							tempObj.id = db->Get(i, "id");
							tempObj.name = db->Get(i, "founder_name");
							tempObj.userID = db->Get(i, "founder_userID");

							tempVector.push_back(tempObj);
						}

						for(int i = 0; i < vectorSize; i++) 
						{
							if(i) companyFounders += ",";
							companyFounders += "{\"id\":\"";
							companyFounders += tempVector.at(i).id;
							companyFounders += "\",\"name\":\"";
							if(tempVector.at(i).userID != "0")
							{
								ost.str("");
								ost << "select * from `users` where `id`=\"" << tempVector.at(i).userID << "\";";
								if(db->Query(ost.str()))
								{
									companyFounders += db->Get(0, "name");
									companyFounders += " ";
									companyFounders += db->Get(0, "nameLast");
								}
								else
								{
									CLog	log;
									log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: there are no user with ID [", tempVector.at(i).userID, "]");
								}
							}
							else
								companyFounders += tempVector.at(i).name;
							companyFounders += "\",\"userid\":\"";
							companyFounders += tempVector.at(i).userID;
							companyFounders += "\"}";
						}
					}
				}

				if(ostFinal.str().length()) ostFinal << ", ";

				ostFinal << "{ \"id\": \""				  	<< companiesList[i].id << "\",";
				ostFinal <<   "\"name\": \""				<< companiesList[i].name << "\", ";
				ostFinal <<   "\"link\": \""				<< companiesList[i].link << "\", ";
				ostFinal <<   "\"foundationDate\": \""	  	<< companiesList[i].foundationDate << "\",";
				ostFinal <<   "\"numberOfEmployee\": \""	<< companiesList[i].numberOfEmployee << "\",";
				ostFinal <<   "\"webSite\": \""			 	<< companiesList[i].webSite << "\",";
				ostFinal <<   "\"description\": \""		 	<< companiesList[i].description << "\",";
				ostFinal <<   "\"type\": \""				<< companiesList[i].type << "\",";
				ostFinal <<   "\"logo_folder\": \""		 	<< companiesList[i].logo_folder << "\",";
				ostFinal <<   "\"logo_filename\": \""	   	<< companiesList[i].logo_filename << "\",";
				ostFinal <<   "\"industries\": ["		   	<< companyIndustry << "],";
				ostFinal <<   "\"owners\": ["			   	<< companyOwners << "],";
				ostFinal <<   "\"founders\": ["			 	<< companyFounders << "],";
				ostFinal <<   "\"employedUsersList\": ["	<< companiesList[i].employedUsersList << "],";
				ostFinal <<   "\"open_vacancies\": ["		<< (quickSearch ? "" : GetOpenVacanciesInJSONFormat("SELECT * FROM `company_vacancy` WHERE `company_id`=\"" + companiesList[i].id + "\";", db, user)) << "],";
				ostFinal <<   "\"isMine\": \""				<< (user ? companiesList[i].admin_userID == user->GetID() : false) << "\",";
				ostFinal <<   "\"isFree\": \""			  	<< (companiesList[i].admin_userID == "0") << "\"";
				ostFinal << "}";
		} // --- for loop through company list
	} // --- if sql-query on company selection success
	else
	{
		CLog	log;

		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: there are no companies returned by request [", dbQuery, "]");
	}

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: end (number of companies returned is " + to_string(companyCounter) + ")");
	}

	return ostFinal.str();
}

string GetNewsFeedInJSONFormat(string whereStatement, int currPage, int newsOnSinglePage, CUser *user, CMysql *db)
{
		struct  ItemClass
		{
			string  action_category_title;
			string  action_category_title_female;
			string  action_category_title_male;
			string  action_types_title;
			string  action_types_title_female;
			string  action_types_title_male;
			string  feed_actionId;
			string  feed_actionTypeId;
			string  feed_eventTimestamp;
			string  feed_id;
			string  src_id;
			string  src_type;
			string  src_name;
			string  src_nameLast;
			string  src_sex;
			string  dst_id;
			string  dst_type;
			string  dst_name;
			string  dst_nameLast;
			string  dst_sex;
		};
		vector<ItemClass>   itemsList;

		ostringstream	  ostResult;
		int			  affected;
		vector<string>	vectorFriendList;

		ostResult.str("");
		ostResult << "SELECT `users_friends`.`friendID` \
				from `users_friends` \
				left join `users` on `users`.`id`=`users_friends`.`friendID` \
				where `users_friends`.`userID`='" << user->GetID() << "' and `users_friends`.`state`='confirmed' and `users`.`isactivated`='Y' and `users`.`isblocked`='N';";

		affected = db->Query(ostResult.str());
		for(int i = 0; i < affected; i++)
		{
			vectorFriendList.push_back(db->Get(i, "friendID"));
		}

		ostResult.str("");
		ostResult << "SELECT "
			<< "`feed`.`id` as `feed_id`, `feed`.`eventTimestamp` as `feed_eventTimestamp`, `feed`.`actionId` as `feed_actionId` , `feed`.`actionTypeId` as `feed_actionTypeId`, `feed`.`srcType` as `feed_srcType`, `feed`.`userId` as `feed_srcID`, `feed`.`dstType` as `feed_dstType`, `feed`.`dstID` as `feed_dstID`, "
			<< "`action_types`.`title` as `action_types_title`, "
			<< "`action_types`.`title_male` as `action_types_title_male`, "
			<< "`action_types`.`title_female` as `action_types_title_female`, "
			<< "`action_category`.`title` as `action_category_title`, "
			<< "`action_category`.`title_male` as `action_category_title_male`, "
			<< "`action_category`.`title_female` as `action_category_title_female` "
			<< "FROM `feed` "
			<< "INNER JOIN  `action_types`  ON `feed`.`actionTypeId`=`action_types`.`id` "
			<< "INNER JOIN  `action_category`   ON `action_types`.`categoryID`=`action_category`.`id` "
			<< "WHERE (" << whereStatement << ") and `action_types`.`isShowFeed`='1' "
			<< "ORDER BY  `feed`.`eventTimestamp` DESC LIMIT " << currPage * newsOnSinglePage << " , " << newsOnSinglePage;

		affected = db->Query(ostResult.str());
		for(int i = 0; i < affected; i++)
		{
			ItemClass   item;

			item.action_category_title = db->Get(i, "action_category_title");
			item.action_category_title_female = db->Get(i, "action_category_title_female");
			item.action_category_title_male = db->Get(i, "action_category_title_male");
			item.action_types_title = db->Get(i, "action_types_title");
			item.action_types_title_female = db->Get(i, "action_types_title_female");
			item.action_types_title_male = db->Get(i, "action_types_title_male");
			item.feed_actionId = db->Get(i, "feed_actionId");
			item.feed_actionTypeId = db->Get(i, "feed_actionTypeId");
			item.feed_eventTimestamp = db->Get(i, "feed_eventTimestamp");
			item.feed_id = db->Get(i, "feed_id");
			item.src_id = db->Get(i, "feed_srcID");
			item.src_type = db->Get(i, "feed_srcType");
			item.dst_id = db->Get(i, "feed_dstID");
			item.dst_type = db->Get(i, "feed_dstType");

			itemsList.push_back(item);
		}

		ostResult.str("");
		for(int i = 0; i < affected; i++)
		{
			ostringstream   ost1;
			string		  srcAvatarPath;
			string		  dstAvatarPath;
			string		  feedID = itemsList[i].feed_id;
			string		  feedActionTypeId = itemsList[i].feed_actionTypeId;
			string		  feedActionId = itemsList[i].feed_actionId;
			string		  feedMessageOwner = itemsList[i].src_id;
			string		  feedMessageTimestamp = itemsList[i].feed_eventTimestamp;
			string		  messageSrcObject = "";
			string		  messageDstObject = "";

			// --- avatar for srcObj
			if(itemsList[i].src_type == "user")
			{
				if(db->Query("SELECT `id`, `name`, `nameLast`, `sex` FROM `users` WHERE `id`=\"" + itemsList[i].src_id + "\";"))
				{
					messageSrcObject = "\"type\":\"" + itemsList[i].src_type + "\",\"id\":\"" + itemsList[i].src_id + "\",\"name\":\"" + db->Get(0, "name") + "\",\"nameLast\":\"" + db->Get(0, "nameLast") + "\",\"sex\":\"" + db->Get(0, "sex") + "\",\"link\":\"\"";

					if(db->Query("SELECT * FROM `users_avatars` WHERE `userid`='" + itemsList[i].src_id + "' and `isActive`='1';"))
						srcAvatarPath = "/images/avatars/avatars" + string(db->Get(0, "folder")) + "/" + string(db->Get(0, "filename"));
					else
						srcAvatarPath = "empty";

					messageSrcObject = messageSrcObject + ",\"avatar\":\"" + srcAvatarPath + "\"";
				}
				else
				{
					CLog	log;
					log.Write(ERROR, string(__func__) + string("[") + to_string(__LINE__) + "]:ERROR: user.id [" + itemsList[i].src_id + "] not found");
				}
			}
			else if(itemsList[i].src_type == "company")
			{
				if(db->Query("SELECT `id`, `type`, `name`, `link`, `logo_folder`, `logo_filename` FROM `company` WHERE `id`=\"" + itemsList[i].src_id + "\";"))
				{
					string  logo_filename = db->Get(0, "logo_filename");

					messageSrcObject = "\"type\":\"" + itemsList[i].src_type + "\",\"id\":\"" + itemsList[i].src_id + "\",\"companyType\":\"" + string(db->Get(0, "type")) + "\",\"name\":\"" + string(db->Get(0, "name")) + "\",\"nameLast\":\"\",\"sex\":\"F\",\"link\":\"" + db->Get(0, "link") + "\"";

					if(logo_filename.length())
						srcAvatarPath = "/images/companies/" + string(db->Get(0, "logo_folder")) + "/" + string(db->Get(0, "logo_filename"));
					else
						srcAvatarPath = "empty";

					messageSrcObject = messageSrcObject + ",\"avatar\":\"" + srcAvatarPath + "\"";
				}
				else
				{
					CLog	log;
					log.Write(ERROR, string(__func__) + string("[") + to_string(__LINE__) + "]:ERROR: company.id [" + itemsList[i].src_id + "] not found");
				}
			}
			else if(itemsList[i].src_type == "group")
			{
				if(db->Query("SELECT * FROM `groups` WHERE `id`=\"" + itemsList[i].src_id + "\";"))
				{
					string  logo_filename = db->Get(0, "logo_filename");

					messageSrcObject = "\"type\":\"" + itemsList[i].src_type + "\",\"id\":\"" + itemsList[i].src_id + "\",\"name\":\"" + db->Get(0, "title") + "\",\"nameLast\":\"\",\"sex\":\"F\",\"link\":\"" + db->Get(0, "link") + "\"";

					if(logo_filename.length())
						srcAvatarPath = "/images/groups/" + string(db->Get(0, "logo_folder")) + "/" + string(db->Get(0, "logo_filename"));
					else
						srcAvatarPath = "empty";

					messageSrcObject = messageSrcObject + ",\"avatar\":\"" + srcAvatarPath + "\"";
				}
				else
				{
					CLog	log;
					log.Write(ERROR, string(__func__) + string("[") + to_string(__LINE__) + "]:ERROR: company.id [" + itemsList[i].src_id + "] not found");
				}
			}
			else
			{
				CLog	log;
				log.Write(ERROR, string(__func__) + string("[") + to_string(__LINE__) + "]:ERROR: unknown srcType [" + itemsList[i].src_type + "]");
			}

			// --- avatar for dstObj
			if(itemsList[i].dst_type == "group")
			{
				if(db->Query("SELECT * FROM `groups` WHERE `id`=\"" + itemsList[i].dst_id + "\";"))
				{
					string  logo_filename = db->Get(0, "logo_filename");

					messageDstObject = "\"type\":\"" + itemsList[i].dst_type + "\",\"id\":\"" + itemsList[i].dst_id + "\",\"name\":\"" + db->Get(0, "title") + "\",\"nameLast\":\"\",\"sex\":\"F\",\"link\":\"" + db->Get(0, "link") + "\"";

					if(logo_filename.length())
						dstAvatarPath = "/images/groups/" + string(db->Get(0, "logo_folder")) + "/" + string(db->Get(0, "logo_filename"));
					else
						dstAvatarPath = "empty";

					messageDstObject = messageDstObject + ",\"avatar\":\"" + dstAvatarPath + "\"";
				}
				else
				{
					CLog	log;
					log.Write(ERROR, string(__func__) + string("[") + to_string(__LINE__) + "]:ERROR: company.id [" + itemsList[i].dst_id + "] not found");
				}
			}
			// --- avatar for dstObj
			if(itemsList[i].dst_type == "event")
			{
				if(db->Query("SELECT * FROM `events` WHERE `id`=\"" + itemsList[i].dst_id + "\";"))
				{
					string  logo_filename = db->Get(0, "logo_filename");

					messageDstObject = "\"type\":\"" + itemsList[i].dst_type + "\",\"id\":\"" + itemsList[i].dst_id + "\",\"name\":\"" + db->Get(0, "title") + "\",\"nameLast\":\"\",\"sex\":\"\",\"link\":\"" + db->Get(0, "link") + "\"";

					if(logo_filename.length())
						dstAvatarPath = "/images/events/" + string(db->Get(0, "logo_folder")) + "/" + string(db->Get(0, "logo_filename"));
					else
						dstAvatarPath = "empty";

					messageDstObject = messageDstObject + ",\"avatar\":\"" + dstAvatarPath + "\"";
				}
				else
				{
					CLog	log;
					log.Write(ERROR, string(__func__) + string("[") + to_string(__LINE__) + "]:ERROR: company.id [" + itemsList[i].dst_id + "] not found");
				}
			}

			if(feedActionTypeId == "11") 
			{
				// --- 11 - message
				
				if(db->Query("SELECT * FROM `feed_message` WHERE `id`='" + feedActionId + "';"))
				{
					string  messageId = db->Get(0, "id");
					string  messageTitle = db->Get(0, "title");
					string  messageLink = db->Get(0, "link");
					string  messageMessage = db->Get(0, "message");
					string  messageImageSetID = db->Get(0, "imageSetID");
					string  messageAccessRights = db->Get(0, "access");
					string  messageImageList =			GetMessageImageList(messageImageSetID, db);
					string  messageParamLikesUserList =  GetMessageLikesUsersList(messageId, user, db);
					string  messageParamCommentsCount =  GetMessageCommentsCount(messageId, db);
					string  messageParamSpam =			GetMessageSpam(messageId, db);
					string  messageParamSpamMe =			GetMessageSpamUser(messageId, user->GetID(), db);

					if(AllowMessageInNewsFeed(user, feedMessageOwner, messageAccessRights, &vectorFriendList))
					{
						if(ostResult.str().length() > 10) ostResult << ",";

						ostResult << "{";
						ostResult << "\"srcObj\":{"			 << messageSrcObject << "},";
						ostResult << "\"dstObj\":{"			 << messageDstObject << "},";
						ostResult << "\"actionCategoryTitle\":\"" << itemsList[i].action_category_title		   << "\",";
						ostResult << "\"actionCategoryTitleMale\":\"" << itemsList[i].action_category_title_male	<< "\",";
						ostResult << "\"actionCategoryTitleFemale\":\""<< itemsList[i].action_category_title_female   << "\",";
						ostResult << "\"actionTypesTitle\":\""  << itemsList[i].action_types_title			  << "\",";
						ostResult << "\"actionTypesTitleMale\":\""<< itemsList[i].action_types_title_male		 << "\",";
						ostResult << "\"actionTypesTitleFemale\":\""<< itemsList[i].action_types_title_female		<< "\",";
						ostResult << "\"actionTypesId\":\""	<< itemsList[i].feed_actionTypeId				   << "\",";

						ostResult << "\"messageId\":\""		<< messageId							 << "\",";
						ostResult << "\"messageTitle\":\""	  << messageTitle						  << "\",";
						ostResult << "\"messageLink\":\""		<< messageLink						   << "\",";
						ostResult << "\"messageMessage\":\""	  << messageMessage						<< "\",";
						ostResult << "\"messageImageSetID\":\""   << messageImageSetID					  << "\",";
						ostResult << "\"messageImageList\":["	<< messageImageList						 << "],";
						ostResult << "\"messageLikesUserList\":[" << messageParamLikesUserList			  << "],";
						ostResult << "\"messageCommentsCount\":\""<< messageParamCommentsCount			  << "\",";
						ostResult << "\"messageSpam\":\""		<< messageParamSpam						 << "\",";
						ostResult << "\"messageSpamMe\":\""	<< messageParamSpamMe					   << "\",";
						ostResult << "\"messageAccessRights\":\"" << messageAccessRights					  << "\",";

						ostResult << "\"eventTimestamp\":\""	  << itemsList[i].feed_eventTimestamp										<< "\",";
						ostResult << "\"eventTimestampDelta\":\"" << to_string(GetTimeDifferenceFromNow(itemsList[i].feed_eventTimestamp))  << "\"";

						ostResult << "}";

						// if(i < (affected - 1)) ostResult << ",";
					}
				}
				else
				{
					CLog	log;
					log.Write(ERROR, string(__func__) + string("[") + to_string(__LINE__) + "]:ERROR: can't get message [" + feedActionId + "] FROM feed_message");
				} // --- Message in news feed not found
			}
			else if((feedActionTypeId == "14") || (feedActionTypeId == "15") || (feedActionTypeId == "16")) 
			{
				// --- 14 friendship established
				// --- 15 friendship broken
				// --- 16 friendship request sent

				string  friendID = feedActionId;

				ost1.str("");
				ost1 << "SELECT `users`.`name` as `users_name`, `users`.`nameLast` as `users_nameLast` FROM `users` WHERE `id`=\"" << friendID << "\" and `isblocked`='N';";
				if(db->Query(ost1.str()))
				{
					string  friendAvatar = "empty";
					string  friendName;
					string  friendNameLast;
					string  friendCompanyName;
					string  friendCompanyID;
					string  friendUsersCompanyPositionTitle;

					friendName = db->Get(0, "users_name");
					friendNameLast = db->Get(0, "users_nameLast");

					ost1.str("");
					ost1 << "SELECT * FROM `users_avatars` WHERE `userid`='" << friendID << "' and `isActive`='1';";
					if(db->Query(ost1.str()))
					{
						ost1.str("");
						ost1 << "/images/avatars/avatars" << db->Get(0, "folder") << "/" << db->Get(0, "filename");
						friendAvatar = ost1.str();
					}

					ost1.str("");
					ost1 << "SELECT `company_position`.`title` as `users_company_position_title`,  \
							`company`.`id` as `company_id`, `company`.`name` as `company_name` \
							FROM `users_company` \
							LEFT JOIN  `company_position` ON `company_position`.`id`=`users_company`.`position_title_id` \
							LEFT JOIN  `company`				ON `company`.`id`=`users_company`.`company_id` \
							WHERE `users_company`.`user_id`=\"" << friendID << "\" and `users_company`.`current_company`='1' \
							ORDER BY  `users_company`.`occupation_start` DESC ";
					if(db->Query(ost1.str()))
					{
						friendCompanyName = db->Get(0, "company_name");
						friendCompanyID = db->Get(0, "company_id");
						friendUsersCompanyPositionTitle = db->Get(0, "users_company_position_title");
					}
					else
					{
						CLog	log;
						log.Write(DEBUG, string(__func__) + string("[") + to_string(__LINE__) + string("]:") + ": can't get information [", itemsList[i].feed_actionId, "] about his/her employment");
					} // --- Message in news feed not found

					{
						if(ostResult.str().length() > 10) ostResult << ",";

						ostResult << "{";
						ostResult << "\"avatar\":\""			  << srcAvatarPath									<< "\",";
						ostResult << "\"srcObj\":{"			 << messageSrcObject << "},";
						ostResult << "\"messageOwner\":{" << messageSrcObject << "},";
						ostResult << "\"actionCategoryTitle\":\"" << itemsList[i].action_category_title		   << "\",";
						ostResult << "\"actionCategoryTitleMale\":\"" << itemsList[i].action_category_title_male	<< "\",";
						ostResult << "\"actionCategoryTitleFemale\":\""<< itemsList[i].action_category_title_female   << "\",";
						ostResult << "\"actionTypesTitle\":\""  << itemsList[i].action_types_title			  << "\",";
						ostResult << "\"actionTypesTitleMale\":\""<< itemsList[i].action_types_title_male		 << "\",";
						ostResult << "\"actionTypesTitleFemale\":\""<< itemsList[i].action_types_title_female	 << "\",";
						ostResult << "\"actionTypesId\":\""	<< itemsList[i].feed_actionTypeId				   << "\",";

						ostResult << "\"friendAvatar\":\""	  << friendAvatar								  << "\",";
						ostResult << "\"friendID\":\""		  << friendID									  << "\",";
						ostResult << "\"friendName\":\""		  << friendName									<< "\",";
						ostResult << "\"friendNameLast\":\""	  << friendNameLast								<< "\",";
						ostResult << "\"friendCompanyID\":\""	<< friendCompanyID							   << "\",";
						ostResult << "\"friendCompanyName\":\""   << friendCompanyName							  << "\",";
						ostResult << "\"friendUsersCompanyPositionTitle\":\"" << friendUsersCompanyPositionTitle	  << "\",";

						ostResult << "\"eventTimestamp\":\""	  << itemsList[i].feed_eventTimestamp										<< "\",";
						// ostResult << "\"eventTimestampDelta\":\""  << GetHumanReadableTimeDifferenceFromNow(itemsList[i].feed_eventTimestamp)   << "\"";
						ostResult << "\"eventTimestampDelta\":\"" << to_string(GetTimeDifferenceFromNow(itemsList[i].feed_eventTimestamp))  << "\"";
						ostResult << "}";

						// if(i < (affected - 1)) ostResult << ",";
					}
				}
				else
				{
					CLog	log;
					log.Write(ERROR, string(__func__) + string("[") + to_string(__LINE__) + "]:ERROR: user [", friendID, "] not found or blocked");
				}
			}
			else if((feedActionTypeId == "41")) 
			{
				// --- 41 skill added

				string  users_skillID = feedActionId;

				ost1.str("");
				ost1 << "SELECT * FROM `users_skill` WHERE `id`=\"" << users_skillID << "\";";
				if(db->Query(ost1.str()))
				{
					string  skillID = db->Get(0, "skill_id");

					ost1.str("");
					ost1 << "SELECT * FROM `skill` WHERE `id`=\"" << skillID << "\";";
					if(db->Query(ost1.str()))
					{
						string	skillTitle = db->Get(0, "title");
						
						if(ostResult.str().length() > 10) ostResult << ",";

						ostResult << "{";
						ostResult << "\"avatar\":\""			  << srcAvatarPath									<< "\",";
						ostResult << "\"srcObj\":{"			 << messageSrcObject << "},";
						ostResult << "\"messageOwner\":{" << messageSrcObject << "},";
						ostResult << "\"actionCategoryTitle\":\"" << itemsList[i].action_category_title		   << "\",";
						ostResult << "\"actionCategoryTitleMale\":\"" << itemsList[i].action_category_title_male	<< "\",";
						ostResult << "\"actionCategoryTitleFemale\":\""<< itemsList[i].action_category_title_female   << "\",";
						ostResult << "\"actionTypesTitle\":\""  << itemsList[i].action_types_title			  << "\",";
						ostResult << "\"actionTypesTitleMale\":\""<< itemsList[i].action_types_title_male		 << "\",";
						ostResult << "\"actionTypesTitleFemale\":\""<< itemsList[i].action_types_title_female	 << "\",";
						ostResult << "\"actionTypesId\":\""	<< itemsList[i].feed_actionTypeId				   << "\",";

						ostResult << "\"skillID\":\""			<< skillID									   << "\",";
						ostResult << "\"skillTitle\":\""		  << skillTitle									<< "\",";

						ostResult << "\"eventTimestamp\":\""	  << itemsList[i].feed_eventTimestamp							<< "\",";
						// ostResult << "\"eventTimestampDelta\":\""  << GetHumanReadableTimeDifferenceFromNow(itemsList[i].feed_eventTimestamp)   << "\"";
						ostResult << "\"eventTimestampDelta\":\"" << to_string(GetTimeDifferenceFromNow(itemsList[i].feed_eventTimestamp))  << "\"";
						ostResult << "}";

					}
					else
					{
						CLog	log;
						log.Write(ERROR, string(__func__) + string("[") + to_string(__LINE__) + "]:ERROR: skill_id [", skillID, "] not found");
					}

				}
				else
				{
					CLog	log;
					log.Write(ERROR, string(__func__) + string("[") + to_string(__LINE__) + "]:ERROR: users_skill_id [", users_skillID, "] not found");
				}
			}
			else if((feedActionTypeId == "54") || (feedActionTypeId == "53"))
			{
				// --- book read
				string  usersBooksID = itemsList[i].feed_actionId;
				string  bookID = "";
				string  readerUserID = "";
				string  bookReadTimestamp = "";

				if(db->Query("SELECT * FROM `users_books` WHERE `id`=\"" + feedActionId + "\";"))
				{
					bookID = db->Get(0, "bookID");
					readerUserID = db->Get(0, "userID");
					bookReadTimestamp = db->Get(0, "bookReadTimestamp");


					if(bookID.length() && db->Query("SELECT * FROM `book` WHERE `id`=\"" + bookID + "\";"))
					{
						string  bookTitle = db->Get(0, "title");
						string  bookAuthorID = db->Get(0, "authorID");
						string  bookISBN10 = db->Get(0, "isbn10");
						string  bookISBN13 = db->Get(0, "isbn13");
						string  bookCoverPhotoFolder = db->Get(0, "coverPhotoFolder");
						string  bookCoverPhotoFilename = db->Get(0, "coverPhotoFilename");
						
						if(bookAuthorID.length() && db->Query("SELECT * FROM `book_author` WHERE `id`=\"" + bookAuthorID + "\";"))
						{
							string  bookAuthorName = db->Get(0, "name");
							string  bookReadersRatingList =	 GetBookRatingList(bookID, db);
							string  messageParamLikesUserList = GetBookLikesUsersList(usersBooksID, user, db);
							string  messageParamCommentsCount = GetBookCommentsCount(bookID, db);
							string  bookMyRating = "0";

							if(db->Query("SELECT * FROM `users_books` WHERE `userID`=\"" + user->GetID() + "\" AND `bookID`=\"" + bookID + "\";"))
								bookMyRating = db->Get(0, "rating");

							if(ostResult.str().length() > 10) ostResult << ",";

							ostResult << "{";

							ostResult << "\"bookID\":\""			<< bookID << "\",";
							ostResult << "\"bookTitle\":\""	   << bookTitle << "\",";
							ostResult << "\"bookAuthorID\":\""	  << bookAuthorID << "\",";
							ostResult << "\"bookAuthorName\":\""	<< bookAuthorName << "\",";
							ostResult << "\"bookISBN10\":\""		<< bookISBN10 << "\",";
							ostResult << "\"bookISBN13\":\""		<< bookISBN13 << "\",";
							ostResult << "\"bookMyRating\":\""		<< bookMyRating << "\",";
							ostResult << "\"bookCoverPhotoFolder\":\""<< bookCoverPhotoFolder << "\",";
							ostResult << "\"bookCoverPhotoFilename\":\""<< bookCoverPhotoFilename << "\",";
							ostResult << "\"bookReadTimestamp\":\""   << bookReadTimestamp << "\",";
							ostResult << "\"bookReadersRatingList\":["<< bookReadersRatingList << "],";
							ostResult << "\"bookCommentsCount\":\""   << messageParamCommentsCount << "\",";

							ostResult << "\"usersBooksID\":\""	  << usersBooksID << "\",";
							ostResult << "\"messageLikesUserList\":[" << messageParamLikesUserList << "],";

							ostResult << "\"avatar\":\""			<< srcAvatarPath << "\",";
							ostResult << "\"srcObj\":{"			 << messageSrcObject << "},";
							ostResult << "\"messageOwner\":{" << messageSrcObject << "},";
							ostResult << "\"actionCategoryTitle\":\"" << itemsList[i].action_category_title		   << "\",";
							ostResult << "\"actionCategoryTitleMale\":\"" << itemsList[i].action_category_title_male	<< "\",";
							ostResult << "\"actionCategoryTitleFemale\":\""<< itemsList[i].action_category_title_female   << "\",";
							ostResult << "\"actionTypesTitle\":\""  << itemsList[i].action_types_title			  << "\",";
							ostResult << "\"actionTypesTitleMale\":\""<< itemsList[i].action_types_title_male		 << "\",";
							ostResult << "\"actionTypesTitleFemale\":\""<< itemsList[i].action_types_title_female	 << "\",";
							ostResult << "\"actionTypesId\":\""   << itemsList[i].feed_actionTypeId << "\",";
							ostResult << "\"actionID\":\""		  << itemsList[i].feed_actionId << "\",";
							ostResult << "\"eventTimestamp\":\""	<< itemsList[i].feed_eventTimestamp << "\",";
							ostResult << "\"eventTimestampDelta\":\"" << to_string(GetTimeDifferenceFromNow(itemsList[i].feed_eventTimestamp)) << "\"";
							ostResult << "}";
						}
						else
						{
							CLog	log;
							log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR: books_author_id [" + bookAuthorID + "] not found");
						}

					}
					else
					{
						CLog	log;
						log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR: books_id [" + bookID + "] not found");
					}
					
				}
				else
				{
					CLog	log;
					log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR: users_books_id [" + feedActionId + "] not found");
				}
			}
			else if((feedActionTypeId == "64") || (feedActionTypeId == "65"))
			{
				// --- group created / subscribed
				string  groupID = feedActionId;

				if(groupID.length())
				{
					if(db->Query("SELECT * FROM `groups` WHERE `id`=\"" + groupID + "\""))
					{
						string		isBlocked = db->Get(0, "isBlocked");

						if(isBlocked == "N")
						{
							if(ostResult.str().length() > 10) ostResult << ",";

							ostResult << "{";
							ostResult << "\"groups\":["			<< GetGroupListInJSONFormat("SELECT * FROM `groups` WHERE `id`=\"" + groupID + "\" AND `isBlocked`=\"N\";", db, user) << "],";
							ostResult << "\"avatar\":\""		<< srcAvatarPath << "\",";
							ostResult << "\"srcObj\":{"			<< messageSrcObject << "},";
							ostResult << "\"messageOwner\":{"	<< messageSrcObject << "},";
							ostResult << "\"actionCategoryTitle\":\"" << itemsList[i].action_category_title		   << "\",";
							ostResult << "\"actionCategoryTitleMale\":\"" << itemsList[i].action_category_title_male	<< "\",";
							ostResult << "\"actionCategoryTitleFemale\":\""<< itemsList[i].action_category_title_female   << "\",";
							ostResult << "\"actionTypesTitle\":\""  << itemsList[i].action_types_title			  << "\",";
							ostResult << "\"actionTypesTitleMale\":\""<< itemsList[i].action_types_title_male		 << "\",";
							ostResult << "\"actionTypesTitleFemale\":\""<< itemsList[i].action_types_title_female	 << "\",";
							ostResult << "\"actionTypesId\":\""   << itemsList[i].feed_actionTypeId << "\",";
							ostResult << "\"actionID\":\""		  << itemsList[i].feed_actionId << "\",";
							ostResult << "\"eventTimestamp\":\""	<< itemsList[i].feed_eventTimestamp << "\",";
							ostResult << "\"eventTimestampDelta\":\"" << to_string(GetTimeDifferenceFromNow(itemsList[i].feed_eventTimestamp)) << "\"";
							ostResult << "}";
						}
						else
						{
							CLog	log;
							log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: `group`.`id` [" + groupID + "] blocked");
						}
					}
					else
					{
						CLog	log;
						log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR: `group`.`id` [" + groupID + "] not found");
					}
				}
				else
				{
					CLog	log;
					log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR: `group`.`id` [" + groupID + "] is empty");
				}
			}
			else if(feedActionTypeId == "63")
			{
				// --- group created / subscribed
				string  companyID = feedActionId;

				if(companyID.length() && db->Query("SELECT * FROM `company` WHERE `id`=\"" + companyID + "\" AND `isBlocked`=\"N\";"))
				{
					if(ostResult.str().length() > 10) ostResult << ",";

					ostResult << "{";
					ostResult << "\"companies\":["		<< GetCompanyListInJSONFormat("SELECT * FROM `company` WHERE `id`=\"" + companyID + "\" AND `isBlocked`=\"N\";", db, user) << "],";
					ostResult << "\"avatar\":\""		<< srcAvatarPath << "\",";
					ostResult << "\"srcObj\":{"			<< messageSrcObject << "},";
					ostResult << "\"messageOwner\":{"	<< messageSrcObject << "},";
					ostResult << "\"actionCategoryTitle\":\"" << itemsList[i].action_category_title		   << "\",";
					ostResult << "\"actionCategoryTitleMale\":\"" << itemsList[i].action_category_title_male	<< "\",";
					ostResult << "\"actionCategoryTitleFemale\":\""<< itemsList[i].action_category_title_female   << "\",";
					ostResult << "\"actionTypesTitle\":\""  << itemsList[i].action_types_title			  << "\",";
					ostResult << "\"actionTypesTitleMale\":\""<< itemsList[i].action_types_title_male		 << "\",";
					ostResult << "\"actionTypesTitleFemale\":\""<< itemsList[i].action_types_title_female	 << "\",";
					ostResult << "\"actionTypesId\":\""   << itemsList[i].feed_actionTypeId << "\",";
					ostResult << "\"actionID\":\""		  << itemsList[i].feed_actionId << "\",";
					ostResult << "\"eventTimestamp\":\""	<< itemsList[i].feed_eventTimestamp << "\",";
					ostResult << "\"eventTimestampDelta\":\"" << to_string(GetTimeDifferenceFromNow(itemsList[i].feed_eventTimestamp)) << "\"";
					ostResult << "}";
				}
				else
				{
					CLog	log;
					log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR: books_id [" + companyID + "] not found");
				}
			}
			else if(feedActionTypeId == "22")
			{
				// --- certificate received

				if(db->Query("SELECT * FROM `users_certifications` WHERE `id`=\"" + feedActionId + "\";"))
				{
					string  certificationNumber = "";
					string  certificationTrackID = "";
					string  certifiedUserID = "";
					string  usersCertificationID = "";

					usersCertificationID = db->Get(0, "id");
					certificationNumber = db->Get(0, "certification_number");
					certificationTrackID = db->Get(0, "track_id");
					certifiedUserID = db->Get(0, "user_id");

					if(certificationTrackID.length() && db->Query("SELECT * FROM `certification_tracks` WHERE `id`=\"" + certificationTrackID + "\";"))
					{
						string  certificationVendorID = db->Get(0, "vendor_id");
						string  certificationVendorName = "";
						string  certificationTrackTitle = db->Get(0, "title");
						string  certificationVendorLogoFolder = db->Get(0, "logo_folder");
						string  certificationVendorLogoFilename = db->Get(0, "logo_filename");
						string  messageParamLikesUserList = GetCertificationLikesUsersList(usersCertificationID, user, db);
						string  messageParamCommentsCount = GetCertificateCommentsCount(certificationTrackID, db);

						if(certificationVendorID.length() && db->Query("SELECT * FROM `company` WHERE `id`=\"" + certificationVendorID + "\";"))
							certificationVendorName = db->Get(0, "name");
						else
						{
							CLog	log;
							log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR: certification vendor [" + certificationVendorID + "] not found");
						}
							
						if(ostResult.str().length() > 10) ostResult << ",";

						ostResult << "{";

						ostResult << "\"certificationID\":\"" << usersCertificationID << "\",";
						ostResult << "\"certificationTrackID\":\"" << certificationTrackID << "\",";
						ostResult << "\"certificationNumber\":\"" << certificationNumber << "\",";
						ostResult << "\"certificationVendorID\":\"" << certificationVendorID << "\",";
						ostResult << "\"certificationVendorName\":\"" << certificationVendorName << "\",";
						ostResult << "\"certificationTrackTitle\":\"" << certificationTrackTitle << "\",";
						ostResult << "\"certificationVendorLogoFolder\":\""<< certificationVendorLogoFolder << "\",";
						ostResult << "\"certificationVendorLogoFilename\":\""<< certificationVendorLogoFilename << "\",";

						ostResult << "\"certificationCommentsCount\":\"" << messageParamCommentsCount << "\",";
						ostResult << "\"messageLikesUserList\":[" << messageParamLikesUserList << "],";

						ostResult << "\"avatar\":\""			<< srcAvatarPath << "\",";
						ostResult << "\"srcObj\":{"			 << messageSrcObject << "},";
						ostResult << "\"messageOwner\":{" << messageSrcObject << "},";
						ostResult << "\"actionCategoryTitle\":\"" << itemsList[i].action_category_title		   << "\",";
						ostResult << "\"actionCategoryTitleMale\":\"" << itemsList[i].action_category_title_male	<< "\",";
						ostResult << "\"actionCategoryTitleFemale\":\""<< itemsList[i].action_category_title_female   << "\",";
						ostResult << "\"actionTypesTitle\":\""  << itemsList[i].action_types_title			  << "\",";
						ostResult << "\"actionTypesTitleMale\":\""<< itemsList[i].action_types_title_male		 << "\",";
						ostResult << "\"actionTypesTitleFemale\":\""<< itemsList[i].action_types_title_female	 << "\",";
						ostResult << "\"actionTypesId\":\""   << itemsList[i].feed_actionTypeId << "\",";
						ostResult << "\"eventTimestamp\":\""	<< itemsList[i].feed_eventTimestamp << "\",";
						ostResult << "\"eventTimestampDelta\":\"" << to_string(GetTimeDifferenceFromNow(itemsList[i].feed_eventTimestamp)) << "\"";
						ostResult << "}";
					}
					else
					{
						CLog	log;
						log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR: certification track [" + certificationTrackID + "] not found");
					}

					
				}
				else
				{
					CLog	log;
					log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR: users_certification_id [" + feedActionId + "] not found");
				}
				
			}
			else if(feedActionTypeId == "23")
			{
				// --- course received

				if(db->Query("SELECT * FROM `users_courses` WHERE `id`=\"" + feedActionId + "\";"))
				{
					string  courseTrackID = "";
					string  courseUserID = "";
					string  courseMyRating = "0";
					string  usersCourseID = "";
					string  eventTimestamp = "";

					usersCourseID = db->Get(0, "id");
					courseTrackID = db->Get(0, "track_id");
					courseUserID = db->Get(0, "user_id");
					eventTimestamp = db->Get(0, "eventTimestamp");

					if(courseTrackID.length() && db->Query("SELECT * FROM `certification_tracks` WHERE `id`=\"" + courseTrackID + "\";"))
					{
						string  courseVendorID = db->Get(0, "vendor_id");
						string  courseVendorName = "";
						string  courseTrackTitle = db->Get(0, "title");
						string  courseVendorLogoFolder = db->Get(0, "logo_folder");
						string  courseVendorLogoFilename = db->Get(0, "logo_filename");
						string  courseRatingList =			GetCourseRatingList(courseTrackID, db);
						string  messageParamLikesUserList = GetCourseLikesUsersList(usersCourseID, user, db);
						string  messageParamCommentsCount = GetCertificateCommentsCount(courseTrackID, db);

						if(courseVendorID.length() && db->Query("SELECT * FROM `company` WHERE `id`=\"" + courseVendorID + "\";"))
							courseVendorName = db->Get(0, "name");
						else
						{
							CLog	log;
							log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR: course vendor [" + courseVendorID + "] not found");
						}

						if(db->Query("SELECT * FROM `users_courses` WHERE `user_id`=\"" + user->GetID() + "\" AND `track_id`=\"" + courseTrackID + "\";"))
							courseMyRating = db->Get(0, "rating");

						if(ostResult.str().length() > 10) ostResult << ",";

						ostResult << "{";

						ostResult << "\"courseID\":\""		  << courseTrackID << "\",";
						ostResult << "\"usersCourseID\":\""   << usersCourseID << "\",";
						ostResult << "\"courseTrackID\":\""   << courseTrackID << "\",";
						ostResult << "\"courseVendorID\":\""	<< courseVendorID << "\",";
						ostResult << "\"courseVendorName\":\""  << courseVendorName << "\",";
						ostResult << "\"courseTrackTitle\":\""  << courseTrackTitle << "\",";
						ostResult << "\"courseVendorLogoFolder\":\""<< courseVendorLogoFolder << "\",";
						ostResult << "\"courseVendorLogoFilename\":\""<< courseVendorLogoFilename << "\",";
						ostResult << "\"courseMyRating\":\""	<< courseMyRating << "\",";
						ostResult << "\"courseRatingList\":["  << courseRatingList << "],";
						ostResult << "\"courseEventTimestamp\":\""<< eventTimestamp << "\",";

						ostResult << "\"courseCommentsCount\":\"" << messageParamCommentsCount << "\",";
						ostResult << "\"messageLikesUserList\":[" << messageParamLikesUserList << "],";

						ostResult << "\"avatar\":\""			<< srcAvatarPath << "\",";
						ostResult << "\"srcObj\":{"			 << messageSrcObject << "},";
						ostResult << "\"messageOwner\":{" << messageSrcObject << "},";
						ostResult << "\"actionCategoryTitle\":\"" << itemsList[i].action_category_title		   << "\",";
						ostResult << "\"actionCategoryTitleMale\":\"" << itemsList[i].action_category_title_male	<< "\",";
						ostResult << "\"actionCategoryTitleFemale\":\""<< itemsList[i].action_category_title_female   << "\",";
						ostResult << "\"actionTypesTitle\":\""  << itemsList[i].action_types_title			  << "\",";
						ostResult << "\"actionTypesTitleMale\":\""<< itemsList[i].action_types_title_male		 << "\",";
						ostResult << "\"actionTypesTitleFemale\":\""<< itemsList[i].action_types_title_female	 << "\",";
						ostResult << "\"actionTypesId\":\""   << itemsList[i].feed_actionTypeId << "\",";
						ostResult << "\"eventTimestamp\":\""	<< itemsList[i].feed_eventTimestamp << "\",";
						ostResult << "\"eventTimestampDelta\":\"" << to_string(GetTimeDifferenceFromNow(itemsList[i].feed_eventTimestamp)) << "\"";
						ostResult << "}";
					}
					else
					{
						CLog	log;
						log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR: course track [" + courseTrackID + "] not found");
					}

					
				}
				else
				{
					CLog	log;
					log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR: users_course_id [" + feedActionId + "] not found");
				}
				
			}
			else if(feedActionTypeId == "39")
			{
				// --- university degree received

				if(db->Query("SELECT * FROM `users_university` WHERE `id`=\"" + feedActionId + "\";"))
				{
					string  scienceDegreeTitle = "";
					string  scienceDegreeUniversityID = "";
					string  scienceDegreeStart = "";
					string  scienceDegreeFinish = "";
					string  degreedUserID = "";
					string  usersUniversityDegreeID = "";

					usersUniversityDegreeID = db->Get(0, "id");
					degreedUserID = db->Get(0, "user_id");
					scienceDegreeUniversityID = db->Get(0, "university_id");
					scienceDegreeTitle = db->Get(0, "degree");
					scienceDegreeStart = db->Get(0, "occupation_start");
					scienceDegreeFinish = db->Get(0, "occupation_finish");

					if(scienceDegreeUniversityID.length() && db->Query("SELECT * FROM `university` WHERE `id`=\"" + scienceDegreeUniversityID + "\";"))
					{
						string  scienceDegreeUniversityID = db->Get(0, "id");
						string  scienceDegreeUniversityTitle = db->Get(0, "title");
						string  scienceDegreeUniversityRegionID = db->Get(0, "geo_region_id");
						string  scienceDegreeUniversityRegionTitle = "";
						string  scienceDegreeUniversityCountryID = "";
						string  scienceDegreeUniversityCountryTitle = "";
						string  scienceDegreeUniversityLogoFolder = db->Get(0, "logo_folder");
						string  scienceDegreeUniversityLogoFilename = db->Get(0, "logo_filename");
						
						if(scienceDegreeUniversityRegionID.length() && db->Query("SELECT * FROM `geo_region` WHERE `id`=\"" + scienceDegreeUniversityRegionID + "\";"))
						{
							scienceDegreeUniversityRegionTitle = db->Get(0, "title");
							scienceDegreeUniversityCountryID = db->Get(0, "geo_country_id");
							if(scienceDegreeUniversityCountryID.length() && db->Query("SELECT * FROM `geo_country` WHERE `id`=\"" + scienceDegreeUniversityCountryID + "\";"))
							{
								scienceDegreeUniversityCountryTitle = db->Get(0, "title");
							}
							else
							{
								CLog	log;
								log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]: university [" + scienceDegreeUniversityID + "] geo_region [" + scienceDegreeUniversityRegionID + "] geo_country [" + scienceDegreeUniversityCountryID + "] not found");
							}

						}
						else
						{
							CLog	log;
							log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]: university [" + scienceDegreeUniversityID + "] geo_region [" + scienceDegreeUniversityRegionID + "] not found");
						}

						{
							string  messageParamLikesUserList = GetUniversityDegreeLikesUsersList(usersUniversityDegreeID, user, db);
							string  messageParamCommentsCount = GetUniversityDegreeCommentsCount(scienceDegreeUniversityID, db);

							if(ostResult.str().length() > 10) ostResult << ",";

							ostResult << "{";

							ostResult << "\"scienceDegreeID\":\"" << usersUniversityDegreeID << "\",";
							ostResult << "\"scienceDegreeTitle\":\"" << scienceDegreeTitle << "\",";
							ostResult << "\"scienceDegreeUniversityTitle\":\"" << scienceDegreeUniversityTitle << "\",";
							ostResult << "\"scienceDegreeUniversityID\":\"" << scienceDegreeUniversityID << "\",";
							ostResult << "\"scienceDegreeUniversityRegionID\":\"" << scienceDegreeUniversityRegionID << "\",";
							ostResult << "\"scienceDegreeUniversityRegionTitle\":\"" << scienceDegreeUniversityRegionTitle << "\",";
							ostResult << "\"scienceDegreeUniversityCountryID\":\"" << scienceDegreeUniversityCountryID << "\",";
							ostResult << "\"scienceDegreeUniversityCountryTitle\":\"" << scienceDegreeUniversityCountryTitle << "\",";
							ostResult << "\"scienceDegreeUniversityLogoFolder\":\""<< scienceDegreeUniversityLogoFolder << "\",";
							ostResult << "\"scienceDegreeUniversityLogoFilename\":\""<< scienceDegreeUniversityLogoFilename << "\",";
							ostResult << "\"scienceDegreeStart\":\""<< scienceDegreeStart << "\",";
							ostResult << "\"scienceDegreeFinish\":\""<< scienceDegreeFinish << "\",";

							ostResult << "\"scienceDegreeCommentsCount\":\"" << messageParamCommentsCount << "\",";
							ostResult << "\"messageLikesUserList\":[" << messageParamLikesUserList << "],";

							ostResult << "\"avatar\":\""			<< srcAvatarPath << "\",";
							ostResult << "\"srcObj\":{"			 << messageSrcObject << "},";
							ostResult << "\"messageOwner\":{" << messageSrcObject << "},";
							ostResult << "\"actionCategoryTitle\":\"" << itemsList[i].action_category_title		   << "\",";
							ostResult << "\"actionCategoryTitleMale\":\"" << itemsList[i].action_category_title_male	<< "\",";
							ostResult << "\"actionCategoryTitleFemale\":\""<< itemsList[i].action_category_title_female   << "\",";
							ostResult << "\"actionTypesTitle\":\""  << itemsList[i].action_types_title			  << "\",";
							ostResult << "\"actionTypesTitleMale\":\""<< itemsList[i].action_types_title_male		 << "\",";
							ostResult << "\"actionTypesTitleFemale\":\""<< itemsList[i].action_types_title_female	 << "\",";
							ostResult << "\"actionTypesId\":\""   << itemsList[i].feed_actionTypeId << "\",";
							ostResult << "\"eventTimestamp\":\""	<< itemsList[i].feed_eventTimestamp << "\",";
							ostResult << "\"eventTimestampDelta\":\"" << to_string(GetTimeDifferenceFromNow(itemsList[i].feed_eventTimestamp)) << "\"";
							ostResult << "}";
						}

					}
					else
					{
						CLog	log;
						log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR: university ID [" + scienceDegreeUniversityID + "] not found");
					}
					
				}
				else
				{
					CLog	log;
					log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR: users_university_id [" + feedActionId + "] not found");
				}
			}
			else if(feedActionTypeId == "40")
			{
				// --- language improved

				if(db->Query("SELECT * FROM `users_language` WHERE `id`=\"" + feedActionId + "\";"))
				{
					string  usersLanguageID = "";
					string  languageID = "";
					string  languageUserID = "";
					string  languageLevel = "";

					usersLanguageID = db->Get(0, "id");
					languageID = db->Get(0, "language_id");
					languageUserID = db->Get(0, "user_id");
					languageLevel = db->Get(0, "level");

					if(languageID.length() && db->Query("SELECT * FROM `language` WHERE `id`=\"" + languageID + "\";"))
					{
						string  languageTitle = db->Get(0, "title");
						string  languageLogoFolder = db->Get(0, "logo_folder");
						string  languageLogoFilename = db->Get(0, "logo_filename");
						string  messageParamLikesUserList = GetLanguageLikesUsersList(usersLanguageID, user, db);
						string  messageParamCommentsCount = GetLanguageCommentsCount(languageID, db);

						if(ostResult.str().length() > 10) ostResult << ",";

						ostResult << "{";

						ostResult << "\"languageID\":\""		  << languageID << "\",";
						ostResult << "\"usersLanguageID\":\""   << usersLanguageID << "\",";
						ostResult << "\"languageTitle\":\""   << languageTitle << "\",";
						ostResult << "\"languageLogoFolder\":\""<< languageLogoFolder << "\",";
						ostResult << "\"languageLogoFilename\":\""<< languageLogoFilename << "\",";
						ostResult << "\"languageLevel\":\""  << languageLevel << "\",";

						ostResult << "\"languageCommentsCount\":\"" << messageParamCommentsCount << "\",";
						ostResult << "\"messageLikesUserList\":[" << messageParamLikesUserList << "],";

						ostResult << "\"avatar\":\""			<< srcAvatarPath << "\",";
						ostResult << "\"srcObj\":{"			 << messageSrcObject << "},";
						ostResult << "\"messageOwner\":{" << messageSrcObject << "},";
						ostResult << "\"actionCategoryTitle\":\"" << itemsList[i].action_category_title		   << "\",";
						ostResult << "\"actionCategoryTitleMale\":\"" << itemsList[i].action_category_title_male	<< "\",";
						ostResult << "\"actionCategoryTitleFemale\":\""<< itemsList[i].action_category_title_female   << "\",";
						ostResult << "\"actionTypesTitle\":\""  << itemsList[i].action_types_title			  << "\",";
						ostResult << "\"actionTypesTitleMale\":\""<< itemsList[i].action_types_title_male		 << "\",";
						ostResult << "\"actionTypesTitleFemale\":\""<< itemsList[i].action_types_title_female	 << "\",";
						ostResult << "\"actionTypesId\":\""   << itemsList[i].feed_actionTypeId << "\",";
						ostResult << "\"eventTimestamp\":\""	<< itemsList[i].feed_eventTimestamp << "\",";
						ostResult << "\"eventTimestampDelta\":\"" << to_string(GetTimeDifferenceFromNow(itemsList[i].feed_eventTimestamp)) << "\"";
						ostResult << "}";
					}
					else
					{
						CLog	log;
						log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR: language[" + languageID + "] not found");
					}

					
				}
				else
				{
					CLog	log;
					log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR: users_language_id [" + feedActionId + "] not found");
				}
				
			}
			else if(feedActionTypeId == "1")
			{
				// --- change employment

				if(db->Query("SELECT * FROM `users_company` WHERE `id`=\"" + feedActionId + "\";"))
				{
					string  usersCompanyID = "";
					string  companyID = "";
					string  companyUserID = "";
					string  companyPositionTitleID = "";

					usersCompanyID = db->Get(0, "id");
					companyID = db->Get(0, "company_id");
					companyUserID = db->Get(0, "user_id");
					companyPositionTitleID = db->Get(0, "position_title_id");

					if(companyID.length() && db->Query("SELECT * FROM `company` WHERE `id`=\"" + companyID + "\";"))
					{
						string  companyTitle = db->Get(0, "name");
						string  companyLogoFolder = db->Get(0, "logo_folder");
						string  companyLogoFilename = db->Get(0, "logo_filename");
						string  companyPositionTitle = "";
						string  messageParamLikesUserList = GetCompanyLikesUsersList(usersCompanyID, user, db);
						string  messageParamCommentsCount = GetCompanyCommentsCount(companyID, db);

						if(companyPositionTitleID.length() && db->Query("SELECT * FROM `company_position` WHERE `id`=\"" + companyPositionTitleID + "\";"))
							companyPositionTitle = db->Get(0, "title");

						if(ostResult.str().length() > 10) ostResult << ",";

						ostResult << "{";

						ostResult << "\"companyID\":\""		<< companyID << "\",";
						ostResult << "\"usersCompanyID\":\""	 << usersCompanyID << "\",";
						ostResult << "\"companyTitle\":\""  << companyTitle << "\",";
						ostResult << "\"companyLogoFolder\":\""<< companyLogoFolder << "\",";
						ostResult << "\"companyLogoFilename\":\""<< companyLogoFilename << "\",";
						ostResult << "\"companyPositionTitleID\":\""	<< companyPositionTitleID << "\",";
						ostResult << "\"companyPositionTitle\":\""	<< companyPositionTitle << "\",";

						ostResult << "\"companyCommentsCount\":\"" << messageParamCommentsCount << "\",";
						ostResult << "\"messageLikesUserList\":[" << messageParamLikesUserList << "],";

						ostResult << "\"avatar\":\""			<< srcAvatarPath << "\",";
						ostResult << "\"srcObj\":{"			 << messageSrcObject << "},";
						ostResult << "\"messageOwner\":{" << messageSrcObject << "},";
						ostResult << "\"actionCategoryTitle\":\"" << itemsList[i].action_category_title		   << "\",";
						ostResult << "\"actionCategoryTitleMale\":\"" << itemsList[i].action_category_title_male	<< "\",";
						ostResult << "\"actionCategoryTitleFemale\":\""<< itemsList[i].action_category_title_female   << "\",";
						ostResult << "\"actionTypesTitle\":\""  << itemsList[i].action_types_title			  << "\",";
						ostResult << "\"actionTypesTitleMale\":\""<< itemsList[i].action_types_title_male		 << "\",";
						ostResult << "\"actionTypesTitleFemale\":\""<< itemsList[i].action_types_title_female	 << "\",";
						ostResult << "\"actionTypesId\":\""   << itemsList[i].feed_actionTypeId << "\",";
						ostResult << "\"eventTimestamp\":\""	<< itemsList[i].feed_eventTimestamp << "\",";
						ostResult << "\"eventTimestampDelta\":\"" << to_string(GetTimeDifferenceFromNow(itemsList[i].feed_eventTimestamp)) << "\"";
						ostResult << "}";
					}
					else
					{
						CLog	log;
						log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR: company[" + companyID + "] not found");
					}

					
				}
				else
				{
					CLog	log;
					log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR: users_company_id [" + feedActionId + "] not found");
				}
				
			}
			else
			{

				if(ostResult.str().length() > 10) ostResult << ",";

				ostResult << "{";
				ostResult << "\"avatar\":\""			  << srcAvatarPath															<< "\",";
				ostResult << "\"srcObj\":{"	<< messageSrcObject << "},";
				ostResult << "\"actionCategoryTitle\":\"" << itemsList[i].action_category_title		   << "\",";
				ostResult << "\"actionCategoryTitleMale\":\"" << itemsList[i].action_category_title_male	<< "\",";
				ostResult << "\"actionCategoryTitleFemale\":\""<< itemsList[i].action_category_title_female   << "\",";
				ostResult << "\"actionTypesTitle\":\""  << itemsList[i].action_types_title			  << "\",";
				ostResult << "\"actionTypesTitleMale\":\""<< itemsList[i].action_types_title_male		 << "\",";
				ostResult << "\"actionTypesTitleFemale\":\""<< itemsList[i].action_types_title_female	 << "\",";
				ostResult << "\"actionTypesId\":\""	<< itemsList[i].feed_actionTypeId										   << "\",";
				ostResult << "\"eventTimestamp\":\""	  << itemsList[i].feed_eventTimestamp										<< "\",";
				// ostResult << "\"eventTimestampDelta\":\""  << GetHumanReadableTimeDifferenceFromNow(itemsList[i].feed_eventTimestamp)   << "\"";
				ostResult << "\"eventTimestampDelta\":\"" << to_string(GetTimeDifferenceFromNow(itemsList[i].feed_eventTimestamp))  << "\"";
				ostResult << "}";
				
				// if(i < (affected - 1)) ostResult << ",";
			}
		}

		return ostResult.str();
}

// --- Function returns list of users "liked" messageID in JSON-format
// --- input: messageID, user, db
//			user - used for friendship status definition
// --- output: was this message "liked" by particular user or not
string GetMessageLikesUsersList(string messageID, CUser *user, CMysql *db)
{
	ostringstream   ost;
	int			 affected;
	string		  result = "";

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: start");
	}

	affected = db->Query("select * from `feed_message_params` where `parameter`='like' and `messageID`='" + messageID + "';");
	if(affected > 0) 
	{
		ost.str("");
		ost << "select * from users where id in (";
		for(int i = 0; i < affected; i++)
		{
			if(i > 0) ost << ",";
			ost << db->Get(i, "userID");
		}
		ost << ") and `isactivated`='Y' and `isblocked`='N';";
		result = GetUserListInJSONFormat(ost.str(), db, user);
	}

	{
		CLog			log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: end (returning string length " + to_string(result.length()) + ")");
	}

	return result;
}

string  GetUserNotificationSpecificDataByType(unsigned long typeID, unsigned long actionID, CMysql *db, CUser *user)
{
	ostringstream   ostResult, ost;

	{
		CLog	log;
		
		log.Write(DEBUG, string(__func__) + "(typeID=" + to_string(typeID) + ", actionID=" + to_string(actionID) + ")[" + to_string(__LINE__) + "]: start");
	}


	ostResult.str("");

	// --- comment provided
	if(typeID == 19)
	{
		unsigned long   comment_id = actionID;

		if(db->Query("select * from  `feed_message_comment` where `id`=\"" + to_string(comment_id) + "\";"))
		{
			string  friend_userID = db->Get(0, "userID");
			string  commentTitle = db->Get(0, "comment");
			string  commentTimestamp = db->Get(0, "eventTimestamp");
			string  messageID = db->Get(0, "messageID");
			string  commentType = db->Get(0, "type");

			if(commentType == "message")
			{
				if(db->Query("select * from  `feed_message` where `id`=\"" + messageID + "\";"))
				{
					string  messageTitle = db->Get(0, "title");
					string  messageBody = db->Get(0, "message");
					string  messageImageSetID = db->Get(0, "imageSetID");

					if(db->Query("select * from `users` where `id`=\"" + friend_userID + "\";"))
					{
						string	  friend_userName = db->Get(0, "name");
						string	  friend_userNameLast = db->Get(0, "nameLast");
						string	  friend_sex = db->Get(0, "sex");
						string	  imageSetFolder = "";
						string	  imageSetPic = "";
						string	  messageMediaType = "";

						if(messageImageSetID.length() && (messageImageSetID != "0") && (db->Query("select * from `feed_images` where `setID`=\"" + messageImageSetID + "\";")))
						{
							imageSetFolder = db->Get(0, "folder");
							imageSetPic = db->Get(0, "filename");
							messageMediaType = db->Get(0, "mediaType");
						}
						else
						{
							CLog log;
							log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]:typeID=" + to_string(typeID) + ": there is no media assigned to message");
						}

						ostResult << "\"notificationMessageID\":\"" << messageID << "\",";
						ostResult << "\"notificationMessageTitle\":\"" << messageTitle << "\",";
						ostResult << "\"notificationMessageBody\":\"" << messageBody << "\",";
						ostResult << "\"notificationMessageImageFolder\":\"" << imageSetFolder << "\",";
						ostResult << "\"notificationMessageImageName\":\"" << imageSetPic << "\",";
						ostResult << "\"notificationMessageMediaType\":\"" << messageMediaType << "\",";
						ostResult << "\"notificationCommentType\":\"message\",";
						ostResult << "\"notificationCommentID\":\"" << comment_id << "\",";
						ostResult << "\"notificationCommentTitle\":\"" << commentTitle << "\",";
						ostResult << "\"notificationCommentEventTimestamp\":\"" << commentTimestamp << "\",";
						ostResult << "\"notificationFriendUserID\":\"" << friend_userID << "\",";
						ostResult << "\"notificationFriendUserName\":\"" << friend_userName << "\",";
						ostResult << "\"notificationFriendUserNameLast\":\"" << friend_userNameLast << "\",";
						ostResult << "\"notificationFriendUserSex\":\"" << friend_sex << "\",";
						ostResult << "\"notificationFriendUserAvatar\":\"" << GetUserAvatarByUserID(friend_userID, db) << "\"";

					}
					else
					{
						CLog log;
						log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR:typeID=" + to_string(typeID) + ": ERROR selecting from users");
					}

				}
				else
				{
					CLog log;
					log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR:typeID=" + to_string(typeID) + ": ERROR finding messageID[" + messageID + "] in feed_message");
				}
			} // --- comment type "message"

			if(commentType == "book")
			{
				string	  bookID = messageID;

				if(db->Query("select * from  `book` where `id`=\"" + bookID + "\";"))
				{
					string  bookTitle = db->Get(0, "title");
					string  bookAuthorID = db->Get(0, "authorID");
					string  bookAuthor;
					string  bookISBN10 = db->Get(0, "isbn10");
					string  bookISBN13 = db->Get(0, "isbn13");
					string  coverPhotoFolder = db->Get(0, "coverPhotoFolder");
					string  coverPhotoFilename = db->Get(0, "coverPhotoFilename");

					if(bookAuthorID.length() && db->Query("select * from `book_author` where `id`=\"" + bookAuthorID + "\";"))
						bookAuthor = db->Get(0, "name");

					if(db->Query("select * from `users` where `id`=\"" + friend_userID + "\";"))
					{
						string	  friend_userName = db->Get(0, "name");
						string	  friend_userNameLast = db->Get(0, "nameLast");
						string	  friend_sex = db->Get(0, "sex");

						ostResult << "\"notificationBookID\":\"" << bookID << "\",";
						ostResult << "\"notificationBookTitle\":\"" << bookTitle << "\",";
						ostResult << "\"notificationBookAuthorID\":\"" << bookAuthorID << "\",";
						ostResult << "\"notificationBookAuthor\":\"" << bookAuthor << "\",";
						ostResult << "\"notificationBookISBN10\":\"" << bookISBN10 << "\",";
						ostResult << "\"notificationBookISBN13\":\"" << bookISBN13 << "\",";
						ostResult << "\"notificationBookImageFolder\":\"" << coverPhotoFolder << "\",";
						ostResult << "\"notificationBookImageName\":\"" << coverPhotoFilename << "\",";
						ostResult << "\"notificationCommentType\":\"" << commentType << "\",";
						ostResult << "\"notificationCommentID\":\"" << comment_id << "\",";
						ostResult << "\"notificationCommentTitle\":\"" << commentTitle << "\",";
						ostResult << "\"notificationCommentEventTimestamp\":\"" << commentTimestamp << "\",";
						ostResult << "\"notificationFriendUserID\":\"" << friend_userID << "\",";
						ostResult << "\"notificationFriendUserName\":\"" << friend_userName << "\",";
						ostResult << "\"notificationFriendUserNameLast\":\"" << friend_userNameLast << "\",";
						ostResult << "\"notificationFriendUserSex\":\"" << friend_sex << "\",";
						ostResult << "\"notificationFriendUserAvatar\":\"" << GetUserAvatarByUserID(friend_userID, db) << "\"";

					}
					else
					{
						CLog log;
						log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR:typeID=" + to_string(typeID) + ": ERROR selecting from users");
					}

				}
				else
				{
					CLog log;
					log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR:typeID=" + to_string(typeID) + ": ERROR finding book in feed_message");
				}
			} // --- comment type "book"

			if(commentType == "certification")
			{
				string	  trackID = messageID;
				string	  usersCertificationsID = "";
				string	  certificationNumber = "";

				if(db->Query("select * from `users_certifications` where `track_id`=\"" + trackID + "\" and `user_id`=\"" + friend_userID + "\";"))
				{
					string	  usersCertificationsID = db->Get(0, "id");
					string	  certificationNumber = db->Get(0, "certification_number");
				}
				else
				{
					CLog log;
					log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]:typeID=" + to_string(typeID) + ": finding users_certifications by (track_id[" + trackID + "] and user_id[" + (user ? user->GetID() : "NULL") + "]) or (user[" + (user ? "not null" : "null") + "] == NULL)");
				}


				if(db->Query("select * from  `certification_tracks` where `id`=\"" + trackID + "\";"))
				{
					string  certificationTitle = db->Get(0, "title");
					string  vendorID = db->Get(0, "vendor_id");
					string  logoFolder = db->Get(0, "logo_folder");
					string  logoFilename = db->Get(0, "logo_filename");
					string  companyName = "";

					if(vendorID.length() && db->Query("select * from `company` where `id`=\"" + vendorID + "\";"))
						companyName = db->Get(0, "name");

					if(db->Query("select * from `users` where `id`=\"" + friend_userID + "\";"))
					{
						string	  friend_userName = db->Get(0, "name");
						string	  friend_userNameLast = db->Get(0, "nameLast");
						string	  friend_sex = db->Get(0, "sex");

						ostResult << "\"notificationUsersCertificationID\":\"" << usersCertificationsID << "\",";
						ostResult << "\"notificationCertificationTitle\":\"" << certificationTitle << "\",";
						ostResult << "\"notificationCertificationNumber\":\"" << certificationNumber << "\",";							
						ostResult << "\"notificationCertificationID\":\"" << trackID << "\",";
						ostResult << "\"notificationCertificationCompanyID\":\"" << vendorID << "\",";
						ostResult << "\"notificationCertificationCompanyName\":\"" << companyName << "\",";
						ostResult << "\"notificationCertificationImageFolder\":\"" << logoFolder << "\",";
						ostResult << "\"notificationCertificationImageName\":\"" << logoFilename << "\",";
						ostResult << "\"notificationCommentType\":\"" << commentType << "\",";
						ostResult << "\"notificationCommentID\":\"" << comment_id << "\",";
						ostResult << "\"notificationCommentTitle\":\"" << commentTitle << "\",";
						ostResult << "\"notificationCommentEventTimestamp\":\"" << commentTimestamp << "\",";
						ostResult << "\"notificationFriendUserID\":\"" << friend_userID << "\",";
						ostResult << "\"notificationFriendUserName\":\"" << friend_userName << "\",";
						ostResult << "\"notificationFriendUserNameLast\":\"" << friend_userNameLast << "\",";
						ostResult << "\"notificationFriendUserSex\":\"" << friend_sex << "\",";
						ostResult << "\"notificationFriendUserAvatar\":\"" << GetUserAvatarByUserID(friend_userID, db) << "\"";

					}
					else
					{
						CLog log;
						log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR:typeID=" + to_string(typeID) + ": ERROR: users.id[" + friend_userID + "] not found");
					}

				}
				else
				{
					CLog log;
					log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR:typeID=" + to_string(typeID) + ": ERROR: certification.id[" + trackID + "] not found");
				}
			} // --- if(likeType == "certification")

			if(commentType == "course")
			{
				string	  trackID = messageID;
				string	  usersCoursesID = "";

				if(db->Query("select * from `users_courses` where `track_id`=\"" + trackID + "\" and `user_id`=\"" + friend_userID + "\";"))
				{
					usersCoursesID = db->Get(0, "id");
				}
				else
				{
					CLog log;
					log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]:typeID=" + to_string(typeID) + ": finding users_courses by (track_id[" + trackID + "] and user_id[" + (user ? user->GetID() : "NULL") + "]) or (user[" + (user ? "not null" : "null") + "] == NULL)");
				}


				if(db->Query("select * from  `certification_tracks` where `id`=\"" + trackID + "\";"))
				{
					string  courseTitle = db->Get(0, "title");
					string  vendorID = db->Get(0, "vendor_id");
					string  logoFolder = db->Get(0, "logo_folder");
					string  logoFilename = db->Get(0, "logo_filename");
					string  companyName = "";

					if(vendorID.length() && db->Query("select * from `company` where `id`=\"" + vendorID + "\";"))
						companyName = db->Get(0, "name");

					if(db->Query("select * from `users` where `id`=\"" + friend_userID + "\";"))
					{
						string	  friend_userName = db->Get(0, "name");
						string	  friend_userNameLast = db->Get(0, "nameLast");
						string	  friend_sex = db->Get(0, "sex");

						ostResult << "\"notificationUsersCourseID\":\"" << usersCoursesID << "\",";
						ostResult << "\"notificationCourseTitle\":\"" << courseTitle << "\",";
						ostResult << "\"notificationCourseID\":\"" << trackID << "\",";
						ostResult << "\"notificationCourseCompanyID\":\"" << vendorID << "\",";
						ostResult << "\"notificationCourseCompanyName\":\"" << companyName << "\",";
						ostResult << "\"notificationCourseImageFolder\":\"" << logoFolder << "\",";
						ostResult << "\"notificationCourseImageName\":\"" << logoFilename << "\",";
						ostResult << "\"notificationCommentType\":\"" << commentType << "\",";
						ostResult << "\"notificationCommentID\":\"" << comment_id << "\",";
						ostResult << "\"notificationCommentTitle\":\"" << commentTitle << "\",";
						ostResult << "\"notificationCommentEventTimestamp\":\"" << commentTimestamp << "\",";
						ostResult << "\"notificationFriendUserID\":\"" << friend_userID << "\",";
						ostResult << "\"notificationFriendUserName\":\"" << friend_userName << "\",";
						ostResult << "\"notificationFriendUserNameLast\":\"" << friend_userNameLast << "\",";
						ostResult << "\"notificationFriendUserSex\":\"" << friend_sex << "\",";
						ostResult << "\"notificationFriendUserAvatar\":\"" << GetUserAvatarByUserID(friend_userID, db) << "\"";

					}
					else
					{
						CLog log;
						log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR:typeID=" + to_string(typeID) + ": ERROR: users.id[" + friend_userID + "] not found");
					}

				}
				else
				{
					CLog log;
					log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR:typeID=" + to_string(typeID) + ": ERROR: course.id[" + trackID + "] not found");
				}
			} // --- if(likeType == "course")


			if(commentType == "university")
			{
				string	  universityID = messageID;
				string	  usersUniversityDegreeID = "";
				string	  degree = "";
				string	  studyStart = "";
				string	  studyFinish = "";

				if(db->Query("select * from `users_university` where `university_id`=\"" + universityID + "\" and `user_id`=\"" + friend_userID + "\";"))
				{
					usersUniversityDegreeID = db->Get(0, "id");
					degree = db->Get(0, "degree");
					studyStart = db->Get(0, "occupation_start");
					studyFinish = db->Get(0, "occupation_finish");
				}
				else
				{
					CLog log;
					log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: typeID=" + to_string(typeID) + ": finding users_universities by (university_id[" + universityID + "] and user_id[" + (user ? user->GetID() : "NULL") + "]) or (user[" + (user ? "not null" : "null") + "] == NULL)");
				}


				if(db->Query("select * from  `university` where `id`=\"" + universityID + "\";"))
				{
					string  universityTitle = db->Get(0, "title");
					string  logoFolder = db->Get(0, "logo_folder");
					string  logoFilename = db->Get(0, "logo_filename");
					string  universityCountryID = "";
					string  universityCountryName = "";
					string  universityRegionID = db->Get(0, "geo_region_id");
					string  universityRegionName = "";

					if(universityRegionID.length() && db->Query("SELECT * FROM `geo_region` WHERE `id`=\"" + universityRegionID + "\";"))
					{
						universityRegionName = db->Get(0, "title");
						universityCountryID = db->Get(0, "geo_country_id");

						if(universityCountryID.length() && db->Query("SELECT * FROM `geo_country` WHERE `id`=\"" + universityCountryID + "\";"))
							universityCountryName = db->Get(0, "title");
					}

					if(db->Query("select * from `users` where `id`=\"" + friend_userID + "\";"))
					{
						string	  friend_userName = db->Get(0, "name");
						string	  friend_userNameLast = db->Get(0, "nameLast");
						string	  friend_sex = db->Get(0, "sex");

						ostResult << "\"notificationUsersUniversityID\":\"" << usersUniversityDegreeID << "\",";
						ostResult << "\"notificationUniversityID\":\"" << universityID << "\",";
						ostResult << "\"notificationUniversityTitle\":\"" << universityTitle << "\",";
						ostResult << "\"notificationUniversityDegree\":\"" << degree << "\",";
						ostResult << "\"notificationUniversityStart\":\"" << studyStart << "\",";
						ostResult << "\"notificationUniversityFinish\":\"" << studyFinish << "\",";
						ostResult << "\"notificationUniversityRegionID\":\"" << universityRegionID << "\",";
						ostResult << "\"notificationUniversityRegionName\":\"" << universityRegionName << "\",";
						ostResult << "\"notificationUniversityCountryID\":\"" << universityCountryID << "\",";
						ostResult << "\"notificationUniversityCountryName\":\"" << universityCountryName << "\",";
						ostResult << "\"notificationUniversityImageFolder\":\"" << logoFolder << "\",";
						ostResult << "\"notificationUniversityImageName\":\"" << logoFilename << "\",";
						ostResult << "\"notificationCommentType\":\"" << commentType << "\",";
						ostResult << "\"notificationCommentID\":\"" << comment_id << "\",";
						ostResult << "\"notificationCommentTitle\":\"" << commentTitle << "\",";
						ostResult << "\"notificationCommentEventTimestamp\":\"" << commentTimestamp << "\",";
						ostResult << "\"notificationFriendUserID\":\"" << friend_userID << "\",";
						ostResult << "\"notificationFriendUserName\":\"" << friend_userName << "\",";
						ostResult << "\"notificationFriendUserNameLast\":\"" << friend_userNameLast << "\",";
						ostResult << "\"notificationFriendUserSex\":\"" << friend_sex << "\",";
						ostResult << "\"notificationFriendUserAvatar\":\"" << GetUserAvatarByUserID(friend_userID, db) << "\"";

					}
					else
					{
						CLog log;
						log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR:typeID=" + to_string(typeID) + ": ERROR: users.id[" + friend_userID + "] not found");
					}

				}
				else
				{
					CLog log;
					log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR:typeID=" + to_string(typeID) + ": ERROR: university.id[" + universityID + "] not found");
				}
			} // --- if(commentType == "university")
			if(commentType == "company")
			{
				string	  companyID = messageID;

				string	  usersCompanyID = "";
				string	  positionTitleID = "";
				string	  employmentStart = "";
				string	  employmentFinish = "";

				if(db->Query("select * from `users_company` where `company_id`=\"" + companyID + "\" and `user_id`=\"" + friend_userID + "\";"))
				{
					usersCompanyID = db->Get(0, "id");
					positionTitleID = db->Get(0, "position_title_id");
					employmentStart = db->Get(0, "occupation_start");
					employmentFinish = db->Get(0, "occupation_finish");
				}
				else
				{
					CLog log;
					log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]:typeID=" + to_string(typeID) + ": finding users_companies by (company_id[" + companyID + "] and user_id[" + (user ? user->GetID() : "NULL") + "]) or (user[" + (user ? "not null" : "null") + "] == NULL)");
				}


				if(db->Query("select * from  `company` where `id`=\"" + companyID + "\";"))
				{
					string  companyName = db->Get(0, "name");
					string  logoFolder = db->Get(0, "logo_folder");
					string  logoFilename = db->Get(0, "logo_filename");
					string  companyWebSite = db->Get(0, "webSite");
					string  companyFoundationDate = db->Get(0, "foundationDate");
					string  positionTitle = "";

					if(positionTitleID.length() && db->Query("SELECT * FROM `company_position` WHERE `id`=\"" + positionTitleID + "\";"))
						positionTitle = db->Get(0, "title");

					if(db->Query("select * from `users` where `id`=\"" + friend_userID + "\";"))
					{
						string	  friend_userName = db->Get(0, "name");
						string	  friend_userNameLast = db->Get(0, "nameLast");
						string	  friend_sex = db->Get(0, "sex");

						ostResult << "\"notificationUsersCompanyID\":\"" << usersCompanyID << "\",";
						ostResult << "\"notificationCompanyID\":\"" << companyID << "\",";
						ostResult << "\"notificationCompanyName\":\"" << companyName << "\",";
						ostResult << "\"notificationCompanyPositionTitleID\":\"" << positionTitleID << "\",";
						ostResult << "\"notificationCompanyPositionTitle\":\"" << positionTitle << "\",";
						ostResult << "\"notificationCompanyStart\":\"" << employmentStart << "\",";
						ostResult << "\"notificationCompanyFinish\":\"" << employmentFinish << "\",";
						ostResult << "\"notificationCompanyFoundationDate\":\"" << companyFoundationDate << "\",";
						ostResult << "\"notificationCompanyWebSite\":\"" << companyWebSite << "\",";
						ostResult << "\"notificationCompanyImageFolder\":\"" << logoFolder << "\",";
						ostResult << "\"notificationCompanyImageName\":\"" << logoFilename << "\",";
						ostResult << "\"notificationCommentType\":\"" << commentType << "\",";
						ostResult << "\"notificationCommentID\":\"" << comment_id << "\",";
						ostResult << "\"notificationCommentTitle\":\"" << commentTitle << "\",";
						ostResult << "\"notificationCommentEventTimestamp\":\"" << commentTimestamp << "\",";
						ostResult << "\"notificationFriendUserID\":\"" << friend_userID << "\",";
						ostResult << "\"notificationFriendUserName\":\"" << friend_userName << "\",";
						ostResult << "\"notificationFriendUserNameLast\":\"" << friend_userNameLast << "\",";
						ostResult << "\"notificationFriendUserSex\":\"" << friend_sex << "\",";
						ostResult << "\"notificationFriendUserAvatar\":\"" << GetUserAvatarByUserID(friend_userID, db) << "\"";

					}
					else
					{
						CLog log;
						log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR:typeID=" + to_string(typeID) + ": ERROR: users.id[" + friend_userID + "] not found");
					}

				}
				else
				{
					CLog log;
					log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR:typeID=" + to_string(typeID) + ": ERROR: company.id[" + companyID + "] not found");
				}
			} // --- if(commentType == "company")
			if(commentType == "language")
			{
				string	  languageID = messageID;
				string	  usersLanguageID = "";
				string	  languageLevel = "";

				if(db->Query("select * from `users_language` where `language_id`=\"" + languageID + "\" and `user_id`=\"" + friend_userID + "\";"))
				{
					usersLanguageID = db->Get(0, "id");
					languageLevel = db->Get(0, "level");
				}
				else
				{
					CLog log;
					log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]:typeID=" + to_string(typeID) + ": finding users_languages by (language_id[" + languageID + "] and user_id[" + (user ? user->GetID() : "NULL") + "]) or (user[" + (user ? "not null" : "null") + "] == NULL)");
				}


				if(db->Query("select * from  `language` where `id`=\"" + languageID + "\";"))
				{
					string  languageTitle = db->Get(0, "title");
					string  logoFolder = db->Get(0, "logo_folder");
					string  logoFilename = db->Get(0, "logo_filename");

					if(db->Query("select * from `users` where `id`=\"" + friend_userID + "\";"))
					{
						string	  friend_userName = db->Get(0, "name");
						string	  friend_userNameLast = db->Get(0, "nameLast");
						string	  friend_sex = db->Get(0, "sex");

						ostResult << "\"notificationUsersLanguageID\":\"" << usersLanguageID << "\",";
						ostResult << "\"notificationLanguageID\":\"" << languageID << "\",";
						ostResult << "\"notificationLanguageTitle\":\"" << languageTitle << "\",";
						ostResult << "\"notificationLanguageLevel\":\"" << languageLevel << "\",";
						ostResult << "\"notificationLanguageImageFolder\":\"" << logoFolder << "\",";
						ostResult << "\"notificationLanguageImageName\":\"" << logoFilename << "\",";
						ostResult << "\"notificationCommentType\":\"" << commentType << "\",";
						ostResult << "\"notificationCommentID\":\"" << comment_id << "\",";
						ostResult << "\"notificationCommentTitle\":\"" << commentTitle << "\",";
						ostResult << "\"notificationCommentEventTimestamp\":\"" << commentTimestamp << "\",";
						ostResult << "\"notificationFriendUserID\":\"" << friend_userID << "\",";
						ostResult << "\"notificationFriendUserName\":\"" << friend_userName << "\",";
						ostResult << "\"notificationFriendUserNameLast\":\"" << friend_userNameLast << "\",";
						ostResult << "\"notificationFriendUserSex\":\"" << friend_sex << "\",";
						ostResult << "\"notificationFriendUserAvatar\":\"" << GetUserAvatarByUserID(friend_userID, db) << "\"";

					}
					else
					{
						CLog log;
						log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR:typeID=" + to_string(typeID) + ": ERROR: users.id[" + friend_userID + "] not found");
					}

				}
				else
				{
					CLog log;
					log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR:typeID=" + to_string(typeID) + ": ERROR: language.id[" + languageID + "] not found");
				}
			} // --- if(commentType == "language")
		}
		else
		{
			CLog log;
			log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR:typeID=" + to_string(typeID) + ": ERROR selecting from users_recommendation");
		}
	}


	// --- skill confirmed
	if(typeID == 43)
	{
		unsigned long   skill_confirmed_id = actionID;

		if(db->Query("select * from `skill_confirmed` where `id`=\"" + to_string(skill_confirmed_id) + "\";"))
		{
			string  users_skill_id = db->Get(0, "users_skill_id");
			string  approver_userID = db->Get(0, "approver_userID");

			ost.str("");
			ost << "select * from `users` where `id`='" << approver_userID << "';";
			if(db->Query(ost.str()))
			{
				string  approver_userName = db->Get(0, "name");
				string  approver_userNameLast = db->Get(0, "nameLast");

				ost.str("");
				ost << "select * from `users_skill` where `id`='" << users_skill_id << "';";
				if(db->Query(ost.str()))
				{
					string  skillID = db->Get(0, "skill_id");

					ost.str("");
					ost << "select * from `skill` where `id`='" << skillID << "';";
					if(db->Query(ost.str()))
					{
						string  skillTitle = db->Get(0, "title");

						ostResult << "\"notificationFriendUserID\":\"" << approver_userID << "\",";
						ostResult << "\"notificationFriendUserName\":\"" << approver_userName << "\",";
						ostResult << "\"notificationFriendUserNameLast\":\"" << approver_userNameLast << "\",";
						ostResult << "\"notificationFriendUserAvatar\":\"" << GetUserAvatarByUserID(approver_userID, db) << "\",";
						ostResult << "\"notificationSkillTitle\":\"" << skillTitle << "\"";

					}
					else
					{
						CLog log;
						log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR:typeID=" + to_string(typeID) + ": ERROR selecting from skill");
					}

				}
				else
				{
					CLog log;
					log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR:typeID=" + to_string(typeID) + ": ERROR selecting from users_skill");
				}

			}
			else
			{
				CLog log;
				log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR:typeID=" + to_string(typeID) + ": ERROR selecting from users");
			}

		}
		else
		{
			CLog log;
			log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR:typeID=" + to_string(typeID) + ": ERROR selecting from skill_confirmed");
		}
	}

	// --- skill removed
	if(typeID == 44)
	{
		ost.str("");
		{
			unsigned long   users_skill_id = actionID;

			{
				ost.str("");
				ost << "select * from `users_skill` where `id`='" << users_skill_id << "';";
				if(db->Query(ost.str()))
				{
					string  skillID = db->Get(0, "skill_id");

					ost.str("");
					ost << "select * from `skill` where `id`='" << skillID << "';";
					if(db->Query(ost.str()))
					{
						string  skillTitle = db->Get(0, "title");

						ostResult << "\"notificationSkillTitle\":\"" << skillTitle << "\"";

					}
					else
					{
						CLog log;
						log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR:typeID=" + to_string(typeID) + ": ERROR selecting from skill");
					}

				}
				else
				{
					CLog log;
					log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR:typeID=" + to_string(typeID) + ": ERROR selecting from users_skill");
				}

			}

		}
	}

	// --- recommendation provided
	if(typeID == 45)
	{
		ost.str("");
		{
			unsigned long   users_recommendation_id = actionID;

			{
				ost.str("");
				ost << "select * from `users_recommendation` where `id`=\"" << users_recommendation_id << "\";";
				if(db->Query(ost.str()))
				{
					string  recommended_userID = db->Get(0, "recommended_userID");
					string  recommending_userID = db->Get(0, "recommending_userID");
					string  title = db->Get(0, "title");
					string  eventTimestamp = db->Get(0, "eventTimestamp");

					ost.str("");
					ost << "select * from `users` where `id`='" << recommending_userID << "';";
					if(db->Query(ost.str()))
					{
						string  recommending_userName = db->Get(0, "name");
						string  recommending_userNameLast = db->Get(0, "nameLast");

						ostResult << "\"notificationFriendUserID\":\"" << recommending_userID << "\",";
						ostResult << "\"notificationFriendUserName\":\"" << recommending_userName << "\",";
						ostResult << "\"notificationFriendUserNameLast\":\"" << recommending_userNameLast << "\",";
						ostResult << "\"notificationFriendUserAvatar\":\"" << GetUserAvatarByUserID(recommending_userID, db) << "\",";
						ostResult << "\"notificationRecommendationID\":\"" << users_recommendation_id << "\",";
						ostResult << "\"notificationRecommendationTitle\":\"" << title << "\",";
						ostResult << "\"notificationRecommendationEventTimestamp\":\"" << eventTimestamp << "\"";

					}
					else
					{
						CLog log;
						log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR:typeID=" + to_string(typeID) + ": ERROR selecting from skill");
					}

				}
				else
				{
					CLog log;
					log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR:typeID=" + to_string(typeID) + ": ERROR selecting from users_recommendation (probably deleted already)");
				}

			}

		}
	}

	// --- 46/47 recommendation deleted by benefit-owner
	if((typeID == 46) || (typeID == 47))
	{
		ost.str("");
		{
			unsigned long   friend_userID = actionID;

			{
				{
					ost.str("");
					ost << "select * from `users` where `id`='" << friend_userID << "';";
					if(db->Query(ost.str()))
					{
						string  friend_userName = db->Get(0, "name");
						string  friend_userNameLast = db->Get(0, "nameLast");

						ostResult << "\"notificationFriendUserID\":\"" << friend_userID << "\",";
						ostResult << "\"notificationFriendUserName\":\"" << friend_userName << "\",";
						ostResult << "\"notificationFriendUserNameLast\":\"" << friend_userNameLast << "\",";
						ostResult << "\"notificationFriendUserAvatar\":\"" << GetUserAvatarByUserID(to_string(friend_userID), db) << "\"";

					}
					else
					{
						CLog log;
						log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR:typeID=" + to_string(typeID) + ": ERROR selecting from skill");
					}

				}
			}
		}
	}

	// --- recommendation modified
	if(typeID == 48)
	{
		unsigned long   user_recommendation_id = actionID;

		ost.str("");
		ost << "select * from  `users_recommendation` where `id`=\"" << user_recommendation_id << "\";";
		if(db->Query(ost.str()))
		{
			string  recommended_userID = db->Get(0, "recommended_userID");
			string  friend_userID = db->Get(0, "recommending_userID");
			string  recommendationTitle = db->Get(0, "title");
			string  recommendationEventTimestamp = db->Get(0, "eventTimestamp");

			{
				{
					ost.str("");
					ost << "select * from `users` where `id`='" << friend_userID << "';";
					if(db->Query(ost.str()))
					{
						string  friend_userName = db->Get(0, "name");
						string  friend_userNameLast = db->Get(0, "nameLast");

						ostResult << "\"notificationRecommendationID\":\"" << user_recommendation_id << "\",";
						ostResult << "\"notificationRecommendationTitle\":\"" << recommendationTitle << "\",";
						ostResult << "\"notificationRecommendationEventTimestamp\":\"" << recommendationEventTimestamp << "\",";
						ostResult << "\"notificationFriendUserID\":\"" << friend_userID << "\",";
						ostResult << "\"notificationFriendUserName\":\"" << friend_userName << "\",";
						ostResult << "\"notificationFriendUserNameLast\":\"" << friend_userNameLast << "\",";
						ostResult << "\"notificationFriendUserAvatar\":\"" << GetUserAvatarByUserID(friend_userID, db) << "\"";

					}
					else
					{
						CLog log;
						log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR:typeID=" + to_string(typeID) + ": ERROR selecting from skill");
					}

				}
			}
		}
		else
		{
			CLog log;
			log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR:typeID=" + to_string(typeID) + ": ERROR selecting from users_recommendation");
		}
	}

	// --- message liked
	if(typeID == 49)
	{
		unsigned long   feed_message_params_id = actionID;

		if(db->Query("select * from  `feed_message_params` where `id`=\"" + to_string(feed_message_params_id) + "\";"))
		{
			string  friend_userID = db->Get(0, "userID");
			string  messageID = db->Get(0, "messageID");
			string  commentType = db->Get(0, "parameter");

			if((commentType == "like"))
			{
				if(db->Query("select * from  `feed_message` where `id`=\"" + messageID + "\";"))
				{
					string  messageTitle = db->Get(0, "title");
					string  messageBody = db->Get(0, "message");
					string  messageImageSetID = db->Get(0, "imageSetID");

						if(db->Query("select * from `users` where `id`=\"" + friend_userID + "\";"))
						{
							string	  friend_userName = db->Get(0, "name");
							string	  friend_userNameLast = db->Get(0, "nameLast");
							string	  friend_sex = db->Get(0, "sex");
							string	  imageSetFolder = "";
							string	  imageSetPic = "";
							string	  messageMediaType = "";

							if((messageImageSetID != "0") && (db->Query("select * from `feed_images` where `setID`=\"" + messageImageSetID + "\";")))
							{
								imageSetFolder = db->Get(0, "folder");
								imageSetPic = db->Get(0, "filename");
								messageMediaType = db->Get(0, "mediaType");
							}
							else
							{
								CLog log;
								log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: typeID=49: there is no media assigned to message");
							} // --- imageSet is empty

							ostResult << "\"notificationCommentType\":\"" << commentType << "\",";
							ostResult << "\"notificationMessageID\":\"" << messageID << "\",";
							ostResult << "\"notificationMessageTitle\":\"" << messageTitle << "\",";
							ostResult << "\"notificationMessageBody\":\"" << messageBody << "\",";
							ostResult << "\"notificationMessageImageFolder\":\"" << imageSetFolder << "\",";
							ostResult << "\"notificationMessageImageName\":\"" << imageSetPic << "\",";
							ostResult << "\"notificationMessageMediaType\":\"" << messageMediaType << "\",";
							ostResult << "\"notificationFriendUserID\":\"" << friend_userID << "\",";
							ostResult << "\"notificationFriendUserName\":\"" << friend_userName << "\",";
							ostResult << "\"notificationFriendUserSex\":\"" << friend_sex << "\",";
							ostResult << "\"notificationFriendUserNameLast\":\"" << friend_userNameLast << "\",";
							ostResult << "\"notificationFriendUserAvatar\":\"" << GetUserAvatarByUserID(friend_userID, db) << "\"";
						}
						else
						{
							CLog log;
							log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR:typeID=" + to_string(typeID) + ": ERROR: selecting from users");
						}
				}
				else
				{
					CLog log;
					log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR:typeID=" + to_string(typeID) + ": ERROR: finding message in feed_message");
				}
			} // --- if(likeType == "message")


			if(commentType == "likeBook")
			{
				string	  usersBooksID = messageID;

				if(db->Query("select * from `users_books` where `id`=\"" + usersBooksID + "\";"))
				{
					string	  bookID = db->Get(0, "bookID");

					if(db->Query("select * from  `book` where `id`=\"" + bookID + "\";"))
					{
						string  bookTitle = db->Get(0, "title");
						string  bookAuthorID = db->Get(0, "authorID");
						string  bookAuthor;
						string  bookISBN10 = db->Get(0, "isbn10");
						string  bookISBN13 = db->Get(0, "isbn13");
						string  coverPhotoFolder = db->Get(0, "coverPhotoFolder");
						string  coverPhotoFilename = db->Get(0, "coverPhotoFilename");

						if(bookAuthorID.length() && db->Query("select * from `book_author` where `id`=\"" + bookAuthorID + "\";"))
							bookAuthor = db->Get(0, "name");

						if(db->Query("select * from `users` where `id`=\"" + friend_userID + "\";"))
						{
							string	  friend_userName = db->Get(0, "name");
							string	  friend_userNameLast = db->Get(0, "nameLast");
							string	  friend_sex = db->Get(0, "sex");

							ostResult << "\"notificationBookID\":\"" << bookID << "\",";
							ostResult << "\"notificationBookTitle\":\"" << bookTitle << "\",";
							ostResult << "\"notificationBookAuthorID\":\"" << bookAuthorID << "\",";
							ostResult << "\"notificationBookAuthor\":\"" << bookAuthor << "\",";
							ostResult << "\"notificationBookISBN10\":\"" << bookISBN10 << "\",";
							ostResult << "\"notificationBookISBN13\":\"" << bookISBN13 << "\",";
							ostResult << "\"notificationBookImageFolder\":\"" << coverPhotoFolder << "\",";
							ostResult << "\"notificationBookImageName\":\"" << coverPhotoFilename << "\",";
							ostResult << "\"notificationCommentType\":\"" << commentType << "\",";
							ostResult << "\"notificationFriendUserID\":\"" << friend_userID << "\",";
							ostResult << "\"notificationFriendUserName\":\"" << friend_userName << "\",";
							ostResult << "\"notificationFriendUserNameLast\":\"" << friend_userNameLast << "\",";
							ostResult << "\"notificationFriendUserSex\":\"" << friend_sex << "\",";
							ostResult << "\"notificationFriendUserAvatar\":\"" << GetUserAvatarByUserID(friend_userID, db) << "\"";

						}
						else
						{
							CLog log;
							log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR:typeID=" + to_string(typeID) + ": ERROR: users.id[" + friend_userID + "] not found");
						}

					}
					else
					{
						CLog log;
						log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR:typeID=" + to_string(typeID) + ": ERROR: book.id[" + bookID + "] not found");
					}
				}
				else
				{
					CLog log;
					log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR:typeID=" + to_string(typeID) + ": ERROR: finding users_books.id[" + usersBooksID + "]");
				}
			} // --- if(likeType == "book")

			if(commentType == "likeCertification")
			{
				string	  usersCertificationsID = messageID;

				if(db->Query("select * from `users_certifications` where `id`=\"" + usersCertificationsID + "\";"))
				{
					string	  trackID = db->Get(0, "track_id");
					string	  certificationNumber = db->Get(0, "certification_number");

					if(db->Query("select * from  `certification_tracks` where `id`=\"" + trackID + "\";"))
					{
						string  certificationTitle = db->Get(0, "title");
						string  vendorID = db->Get(0, "vendor_id");
						string  logoFolder = db->Get(0, "logo_folder");
						string  logoFilename = db->Get(0, "logo_filename");
						string  companyName = "";

						if(vendorID.length() && db->Query("select * from `company` where `id`=\"" + vendorID + "\";"))
							companyName = db->Get(0, "name");

						if(db->Query("select * from `users` where `id`=\"" + friend_userID + "\";"))
						{
							string	  friend_userName = db->Get(0, "name");
							string	  friend_userNameLast = db->Get(0, "nameLast");
							string	  friend_sex = db->Get(0, "sex");

							ostResult << "\"notificationUsersCertificationID\":\"" << usersCertificationsID << "\",";
							ostResult << "\"notificationCertificationID\":\"" << trackID << "\",";
							ostResult << "\"notificationCertificationTitle\":\"" << certificationTitle << "\",";
							ostResult << "\"notificationCertificationNumber\":\"" << certificationNumber << "\",";
							ostResult << "\"notificationCertificationCompanyID\":\"" << vendorID << "\",";
							ostResult << "\"notificationCertificationCompanyName\":\"" << companyName << "\",";
							ostResult << "\"notificationCertificationImageFolder\":\"" << logoFolder << "\",";
							ostResult << "\"notificationCertificationImageName\":\"" << logoFilename << "\",";
							ostResult << "\"notificationCommentType\":\"" << commentType << "\",";
							ostResult << "\"notificationFriendUserID\":\"" << friend_userID << "\",";
							ostResult << "\"notificationFriendUserName\":\"" << friend_userName << "\",";
							ostResult << "\"notificationFriendUserNameLast\":\"" << friend_userNameLast << "\",";
							ostResult << "\"notificationFriendUserSex\":\"" << friend_sex << "\",";
							ostResult << "\"notificationFriendUserAvatar\":\"" << GetUserAvatarByUserID(friend_userID, db) << "\"";

						}
						else
						{
							CLog log;
							log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR:typeID=" + to_string(typeID) + ": ERROR: users.id[" + friend_userID + "] not found");
						}

					}
					else
					{
						CLog log;
						log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR:typeID=" + to_string(typeID) + ": ERROR: certification.id[" + trackID + "] not found");
					}
				}
				else
				{
					CLog log;
					log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR:typeID=" + to_string(typeID) + ": ERROR: finding users_certifications.id[" + usersCertificationsID + "]");
				}
			} // --- if(likeType == "certification")

			if(commentType == "likeCourse")
			{
				string	  usersCoursesID = messageID;

				if(db->Query("select * from `users_courses` where `id`=\"" + usersCoursesID + "\";"))
				{
					string	  trackID = db->Get(0, "track_id");

					if(db->Query("select * from  `certification_tracks` where `id`=\"" + trackID + "\";"))
					{
						string  courseTitle = db->Get(0, "title");
						string  vendorID = db->Get(0, "vendor_id");
						string  logoFolder = db->Get(0, "logo_folder");
						string  logoFilename = db->Get(0, "logo_filename");
						string  companyName = "";

						if(vendorID.length() && db->Query("select * from `company` where `id`=\"" + vendorID + "\";"))
							companyName = db->Get(0, "name");

						if(db->Query("select * from `users` where `id`=\"" + friend_userID + "\";"))
						{
							string	  friend_userName = db->Get(0, "name");
							string	  friend_userNameLast = db->Get(0, "nameLast");
							string	  friend_sex = db->Get(0, "sex");

							ostResult << "\"notificationUsersCourseID\":\"" << usersCoursesID << "\",";
							ostResult << "\"notificationCourseID\":\"" << trackID << "\",";
							ostResult << "\"notificationCourseTitle\":\"" << courseTitle << "\",";
							ostResult << "\"notificationCourseCompanyID\":\"" << vendorID << "\",";
							ostResult << "\"notificationCourseCompanyName\":\"" << companyName << "\",";
							ostResult << "\"notificationCourseImageFolder\":\"" << logoFolder << "\",";
							ostResult << "\"notificationCourseImageName\":\"" << logoFilename << "\",";
							ostResult << "\"notificationCommentType\":\"" << commentType << "\",";
							ostResult << "\"notificationFriendUserID\":\"" << friend_userID << "\",";
							ostResult << "\"notificationFriendUserName\":\"" << friend_userName << "\",";
							ostResult << "\"notificationFriendUserNameLast\":\"" << friend_userNameLast << "\",";
							ostResult << "\"notificationFriendUserSex\":\"" << friend_sex << "\",";
							ostResult << "\"notificationFriendUserAvatar\":\"" << GetUserAvatarByUserID(friend_userID, db) << "\"";

						}
						else
						{
							CLog log;
							log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR:typeID=" + to_string(typeID) + ": ERROR: users.id[" + friend_userID + "] not found");
						}

					}
					else
					{
						CLog log;
						log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR:typeID=" + to_string(typeID) + ": ERROR: course.id[" + trackID + "] not found");
					}
				}
				else
				{
					CLog log;
					log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR:typeID=" + to_string(typeID) + ": ERROR: finding users_courses.id[" + usersCoursesID + "]");
				}
			} // --- if(likeType == "course")
			if(commentType == "likeLanguage")
			{
				string	  usersLanguagesID = messageID;

				if(db->Query("select * from `users_language` where `id`=\"" + usersLanguagesID + "\";"))
				{
					string	  languageID = db->Get(0, "language_id");
					string	  languageLevel = db->Get(0, "level");

					if(db->Query("select * from  `language` where `id`=\"" + languageID + "\";"))
					{
						string  languageTitle = db->Get(0, "title");
						string  logoFolder = db->Get(0, "logo_folder");
						string  logoFilename = db->Get(0, "logo_filename");

						if(db->Query("select * from `users` where `id`=\"" + friend_userID + "\";"))
						{
							string	  friend_userName = db->Get(0, "name");
							string	  friend_userNameLast = db->Get(0, "nameLast");
							string	  friend_sex = db->Get(0, "sex");

							ostResult << "\"notificationUsersLanguageID\":\"" << usersLanguagesID << "\",";
							ostResult << "\"notificationLanguageID\":\"" << languageID << "\",";
							ostResult << "\"notificationLanguageTitle\":\"" << languageTitle << "\",";
							ostResult << "\"notificationLanguageLevel\":\"" << languageLevel << "\",";
							ostResult << "\"notificationLanguageImageFolder\":\"" << logoFolder << "\",";
							ostResult << "\"notificationLanguageImageName\":\"" << logoFilename << "\",";
							ostResult << "\"notificationCommentType\":\"" << commentType << "\",";
							ostResult << "\"notificationFriendUserID\":\"" << friend_userID << "\",";
							ostResult << "\"notificationFriendUserName\":\"" << friend_userName << "\",";
							ostResult << "\"notificationFriendUserNameLast\":\"" << friend_userNameLast << "\",";
							ostResult << "\"notificationFriendUserSex\":\"" << friend_sex << "\",";
							ostResult << "\"notificationFriendUserAvatar\":\"" << GetUserAvatarByUserID(friend_userID, db) << "\"";

						}
						else
						{
							CLog log;
							log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR:typeID=" + to_string(typeID) + ": ERROR: users.id[" + friend_userID + "] not found");
						}

					}
					else
					{
						CLog log;
						log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR:typeID=" + to_string(typeID) + ": ERROR: language.id[" + languageID + "] not found");
					}
				}
				else
				{
					CLog log;
					log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR:typeID=" + to_string(typeID) + ": ERROR: finding users_languages.id[" + usersLanguagesID + "]");
				}
			} // --- if(likeType == "language")

			if(commentType == "likeCompany")
			{
				string	  usersCompanysID = messageID;

				if(db->Query("select * from `users_company` where `id`=\"" + usersCompanysID + "\";"))
				{
					string	  companyID = db->Get(0, "company_id");
					string	  positionTitleID = db->Get(0, "position_title_id");
					string	  positionTitle = "";

					if(db->Query("select * from  `company` where `id`=\"" + companyID + "\";"))
					{
						string  companyName = db->Get(0, "name");
						string  logoFolder = db->Get(0, "logo_folder");
						string  logoFilename = db->Get(0, "logo_filename");

						if(positionTitleID.length() && db->Query("SELECT * FROM `company_position` WHERE `id`=\"" + positionTitleID + "\";"))
							positionTitle = db->Get(0, "title");

						if(db->Query("select * from `users` where `id`=\"" + friend_userID + "\";"))
						{
							string	  friend_userName = db->Get(0, "name");
							string	  friend_userNameLast = db->Get(0, "nameLast");
							string	  friend_sex = db->Get(0, "sex");

							ostResult << "\"notificationUsersCompanyID\":\"" << usersCompanysID << "\",";
							ostResult << "\"notificationCompanyID\":\"" << companyID << "\",";
							ostResult << "\"notificationCompanyName\":\"" << companyName << "\",";
							ostResult << "\"notificationCompanyCompanyName\":\"" << companyName << "\",";
							ostResult << "\"notificationCompanyPositionTitleID\":\"" << positionTitleID << "\",";
							ostResult << "\"notificationCompanyPositionTitle\":\"" << positionTitle << "\",";
							ostResult << "\"notificationCompanyImageFolder\":\"" << logoFolder << "\",";
							ostResult << "\"notificationCompanyImageName\":\"" << logoFilename << "\",";
							ostResult << "\"notificationCommentType\":\"" << commentType << "\",";
							ostResult << "\"notificationFriendUserID\":\"" << friend_userID << "\",";
							ostResult << "\"notificationFriendUserName\":\"" << friend_userName << "\",";
							ostResult << "\"notificationFriendUserNameLast\":\"" << friend_userNameLast << "\",";
							ostResult << "\"notificationFriendUserSex\":\"" << friend_sex << "\",";
							ostResult << "\"notificationFriendUserAvatar\":\"" << GetUserAvatarByUserID(friend_userID, db) << "\"";

						}
						else
						{
							CLog log;
							log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR:typeID=" + to_string(typeID) + ": ERROR: users.id[" + friend_userID + "] not found");
						}

					}
					else
					{
						CLog log;
						log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR:typeID=" + to_string(typeID) + ": ERROR: company.id[" + companyID + "] not found");
					}
				}
				else
				{
					CLog log;
					log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR:typeID=" + to_string(typeID) + ": ERROR: finding users_companies.id[" + usersCompanysID + "]");
				}
			} // --- if(likeType == "company")

			if(commentType == "likeUniversityDegree")
			{
				string	  usersUniversityDegreeID = messageID;

				if(db->Query("select * from `users_university` where `id`=\"" + usersUniversityDegreeID + "\";"))
				{
					string	  universityID = db->Get(0, "university_id");
					string	  degree = db->Get(0, "degree");
					string	  studyStart = db->Get(0, "occupation_start");
					string	  studyFinish = db->Get(0, "occupation_finish");

					if(db->Query("select * from  `university` where `id`=\"" + universityID + "\";"))
					{
						string  universityTitle = db->Get(0, "title");
						string  logoFolder = db->Get(0, "logo_folder");
						string  logoFilename = db->Get(0, "logo_filename");
						string  universityCountryID = "";
						string  universityCountryName = "";
						string  universityRegionID = db->Get(0, "geo_region_id");
						string  universityRegionName = "";
						
						if(universityRegionID.length() && db->Query("SELECT * FROM `geo_region` WHERE `id`=\"" + universityRegionID + "\";"))
						{
							universityRegionName = db->Get(0, "title");
							universityCountryID = db->Get(0, "geo_country_id");

							if(universityCountryID.length() && db->Query("SELECT * FROM `geo_country` WHERE `id`=\"" + universityCountryID + "\";"))
								universityCountryName = db->Get(0, "title");
						}


						if(db->Query("select * from `users` where `id`=\"" + friend_userID + "\";"))
						{
							string	  friend_userName = db->Get(0, "name");
							string	  friend_userNameLast = db->Get(0, "nameLast");
							string	  friend_sex = db->Get(0, "sex");

							ostResult << "\"notificationUsersUniversityID\":\"" << messageID << "\",";
							ostResult << "\"notificationUniversityID\":\"" << universityID << "\",";
							ostResult << "\"notificationUniversityTitle\":\"" << universityTitle << "\",";
							ostResult << "\"notificationUniversityDegree\":\"" << degree << "\",";
							ostResult << "\"notificationUniversityStart\":\"" << studyStart << "\",";
							ostResult << "\"notificationUniversityFinish\":\"" << studyFinish << "\",";
							ostResult << "\"notificationUniversityRegionID\":\"" << universityRegionID << "\",";
							ostResult << "\"notificationUniversityRegionName\":\"" << universityRegionName << "\",";
							ostResult << "\"notificationUniversityCountryID\":\"" << universityCountryID << "\",";
							ostResult << "\"notificationUniversityCountryName\":\"" << universityCountryName << "\",";
							ostResult << "\"notificationUniversityImageFolder\":\"" << logoFolder << "\",";
							ostResult << "\"notificationUniversityImageName\":\"" << logoFilename << "\",";
							ostResult << "\"notificationCommentType\":\"" << commentType << "\",";
							ostResult << "\"notificationFriendUserID\":\"" << friend_userID << "\",";
							ostResult << "\"notificationFriendUserName\":\"" << friend_userName << "\",";
							ostResult << "\"notificationFriendUserNameLast\":\"" << friend_userNameLast << "\",";
							ostResult << "\"notificationFriendUserSex\":\"" << friend_sex << "\",";
							ostResult << "\"notificationFriendUserAvatar\":\"" << GetUserAvatarByUserID(friend_userID, db) << "\"";

						}
						else
						{
							CLog log;
							log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR:typeID=" + to_string(typeID) + ": ERROR: users.id[" + friend_userID + "] not found");
						}

					}
					else
					{
						CLog log;
						log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR:typeID=" + to_string(typeID) + ": ERROR: university.id[" + universityID + "] not found");
					}
				}
				else
				{
					CLog log;
					log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR:typeID=" + to_string(typeID) + ": ERROR: finding users_universities.id[" + usersUniversityDegreeID + "]");
				}
			} // --- if(commentType == "university")



		}
		else
		{
			CLog log;
			log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR:typeID=" + to_string(typeID) + ": ERROR selecting from feed_message_params");
		}
	}

	// --- message disliked
	if(typeID == 50)
	{
		unsigned long   messageID = actionID;

		{

			ost.str("");
			ost << "select * from  `feed_message` where `id`=\"" << messageID << "\";";
			if(db->Query(ost.str()))
			{
				string	  messageTitle = db->Get(0, "title");
				string	  messageBody = db->Get(0, "message");
				string	  messageImageSetID = db->Get(0, "imageSetID");
				string	  imageSetFolder = "";
				string	  imageSetPic = "";
				string	  messageMediaType = "";

				if(db->Query("select * from `feed_images` where `setID`=\"" + messageImageSetID + "\";"))
				{
					imageSetFolder = db->Get(0, "folder");
					imageSetPic = db->Get(0, "filename");
					messageMediaType = db->Get(0, "mediaType");
				}
				else
				{
					CLog log;
					log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) + "]: typeID=50: there is no media assigned to message");
				}

				ostResult << "\"notificationMessageID\":\"" << messageID << "\",";
				ostResult << "\"notificationMessageTitle\":\"" << messageTitle << "\",";
				ostResult << "\"notificationMessageBody\":\"" << messageBody << "\",";
				ostResult << "\"notificationMessageMediaType\":\"" << messageMediaType << "\",";
				ostResult << "\"notificationMessageImageFolder\":\"" << imageSetFolder << "\",";
				ostResult << "\"notificationMessageImageName\":\"" << imageSetPic << "\"";
			}
			else
			{
				CLog log;
				log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR:typeID=" + to_string(typeID) + ": ERROR finding message int feed_message");
			}
		}
	}

	// --- birthday notification
	if(typeID == 58)
	{
		string   birthdayUserID = to_string(actionID);

		if(birthdayUserID.length() && db->Query("select * from `users` where `id`='" + birthdayUserID + "';"))
		{
			string  birthday_userName = db->Get(0, "name");
			string  birthday_userNameLast = db->Get(0, "nameLast");
			string	birthdayDate = db->Get(0, "birthday");


			ostResult << "\"notificationBirthdayDate\":\"" << birthdayDate << "\",";
			ostResult << "\"notificationFriendUserID\":\"" << birthdayUserID << "\",";
			ostResult << "\"notificationFriendUserName\":\"" << birthday_userName << "\",";
			ostResult << "\"notificationFriendUserNameLast\":\"" << birthday_userNameLast << "\",";
			ostResult << "\"notificationFriendUserAvatar\":\"" << GetUserAvatarByUserID(birthdayUserID, db) << "\"";
		}
		else
		{
			CLog log;
			log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR:typeID=" + to_string(typeID) + ": ERROR selecting from users[userID = " + birthdayUserID + "]");
		}
	}

	// --- gift thank notification
	if(typeID == 66)
	{
		string   giftThankID = to_string(actionID);

		if(giftThankID.length() && db->Query("select * from `gift_thanks` where `id`='" + giftThankID + "';"))
		{
			string  friend_userID = db->Get(0, "src_user_id");
			string  dst_user_id = db->Get(0, "dst_user_id");
			string  gift_id = db->Get(0, "gift_id");
			string  comment = db->Get(0, "comment");

			if(db->Query("select * from `users` where `id`=\"" + friend_userID + "\";"))
			{
				string	  friend_userName = db->Get(0, "name");
				string	  friend_userNameLast = db->Get(0, "nameLast");

				ostResult << "\"gifts\":[" << GetGiftListInJSONFormat("SELECT * FROM `gifts` WHERE `id`=\"" + gift_id + "\";", db, user) << "],";
				ostResult << "\"notificationComment\":\"" << comment << "\",";
				ostResult << "\"notificationFriendUserID\":\"" << friend_userID << "\",";
				ostResult << "\"notificationFriendUserName\":\"" << friend_userName << "\",";
				ostResult << "\"notificationFriendUserNameLast\":\"" << friend_userNameLast << "\",";
				ostResult << "\"notificationFriendUserAvatar\":\"" << GetUserAvatarByUserID(friend_userID, db) << "\"";

			}
			else
			{
				CLog log;
				log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR:typeID=" + to_string(typeID) + ": ERROR: users.id[" + friend_userID + "] not found");
			}

		}
		else
		{
			CLog log;
			log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR:typeID=" + to_string(typeID) + ": ERROR selecting from gift_thanks[id = " + giftThankID + "]");
		}
	}

	// --- Vacancy Rejected
	if(typeID == 59)
	{
		string   company_candidates_id = to_string(actionID);

		if(company_candidates_id.length() && db->Query("select * from `company_candidates` where `id`='" + company_candidates_id + "';"))
		{
			string  user_id = db->Get(0, "user_id");
			string  vacancy_id = db->Get(0, "vacancy_id");

			if(vacancy_id.length() && db->Query("select * from `company_vacancy` where `id`='" + vacancy_id + "';"))
			{
				string		company_id = db->Get(0, "company_id");

				ostResult << "\"notificationCompanyCandidatesID\":\"" << company_candidates_id << "\",";
				ostResult << "\"notificationVacancy\":[" << GetOpenVacanciesInJSONFormat("SELECT * FROM `company_vacancy` WHERE `id`=\"" + vacancy_id + "\";", db) << "],";
				ostResult << "\"notificationFromCompany\":[" << GetCompanyListInJSONFormat("SELECT * FROM `company` WHERE `id`=\"" + company_id + "\";", db, NULL) << "]";
			}
			else
			{
				CLog log;
				log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR:typeID=" + to_string(typeID) + ": ERROR selecting from company_vacancy[id = " + vacancy_id + "]");
			}

		}
		else
		{
			CLog log;
			log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR:typeID=" + to_string(typeID) + ": ERROR selecting from company_candidates[id = " + company_candidates_id + "]");
		}
	}

	// --- Company Possession Request
	if(typeID == 60)
	{
		string   company_possession_request_id = to_string(actionID);

		if(company_possession_request_id.length() && db->Query("select * from `company_possession_request` where `id`='" + company_possession_request_id + "';"))
		{
			string  friendUserID = db->Get(0, "requester_user_id");
			string  company_id = db->Get(0, "requested_company_id");
			string	description = db->Get(0, "description");
			string	status = db->Get(0, "status");
			string	eventTimestamp = db->Get(0, "eventTimestamp");

			if(company_id.length() && db->Query("select * from `company` where `id`='" + company_id + "';"))
			{

				if(friendUserID.length() && db->Query("select * from `users` where `id`='" + friendUserID + "';"))
				{
					string  friend_userName = db->Get(0, "name");
					string  friend_userNameLast = db->Get(0, "nameLast");

					ostResult << "\"notificationCompanyPossessionRequestID\":\"" << company_possession_request_id << "\",";
					ostResult << "\"notificationDescription\":\"" << description << "\",";
					ostResult << "\"notificationFriendUserID\":\"" << friendUserID << "\",";
					ostResult << "\"notificationFriendUserName\":\"" << friend_userName << "\",";
					ostResult << "\"notificationFriendUserNameLast\":\"" << friend_userNameLast << "\",";
					ostResult << "\"notificationFriendUserAvatar\":\"" << GetUserAvatarByUserID(friendUserID, db) << "\",";
					ostResult << "\"notificationRequestedCompany\":[" << GetCompanyListInJSONFormat("SELECT * FROM `company` WHERE `id`=\"" + company_id + "\";", db, NULL) << "],";
					ostResult << "\"notificationPossessionStatus\":\"" << status << "\",";
					ostResult << "\"notificationEventTimestamp\":\"" << eventTimestamp << "\"";

				}
				else
				{
					CLog log;
					log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR:typeID=" + to_string(typeID) + ": ERROR selecting from users[userID = " + friendUserID + "]");
				}
			}
			else
			{
				CLog log;
				log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR:typeID=" + to_string(typeID) + ": ERROR selecting from company_vacancy[id = " + company_id + "]");
			}

		}
		else
		{
			CLog log;
			log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR:typeID=" + to_string(typeID) + ": ERROR selecting from company_possession_request[id = " + company_possession_request_id + "]");
		}
	}

	// --- company possession approved / rejected
	if((typeID == 61) || (typeID == 62))
	{
		string   company_id = to_string(actionID);

		if(company_id.length())
		{
			ostResult << "\"notificationFromCompany\":[" << GetCompanyListInJSONFormat("SELECT * FROM `company` WHERE `id`=\"" + company_id + "\";", db, NULL) << "]";
		}
		else
		{
			CLog log;
			log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR:typeID=" + to_string(typeID) + ": ERROR selecting from company_candidates[id = " + company_id + "]");
		}
	}

	// --- 67 - Event host added
	// --- 68 - Event guest added
	// --- 69 - Guest accepted
	if((typeID == 67) || (typeID == 68) || (typeID == 69))
	{
		string		event_host_id = to_string(actionID);
		string		host_guest_sql;

		if(typeID == 67)
			host_guest_sql = "SELECT * FROM `event_hosts` WHERE `id`=\"" + event_host_id + "\"";
		else
			host_guest_sql = "SELECT * FROM `event_guests` WHERE `id`=\"" + event_host_id + "\"";

		if(event_host_id.length() && db->Query(host_guest_sql))
		{
			string  event_id = db->Get(0, "event_id");
			string	eventTimestamp = db->Get(0, "eventTimestamp");
			string  friendUserID = "";

			if(typeID == 69) friendUserID = db->Get(0, "user_id");

			if(event_id.length() && db->Query("SELECT * FROM `events` WHERE `id`=\"" + event_id + "\";"))
			{
				if(!friendUserID.length()) friendUserID = db->Get(0, "owner_id");

				if(friendUserID.length() && db->Query("select * from `users` where `id`='" + friendUserID + "';"))
				{
					string  friend_userName = db->Get(0, "name");
					string  friend_userNameLast = db->Get(0, "nameLast");
					string  friend_userSex = db->Get(0, "sex");

					ostResult << "\"notificationFriendUserID\":\"" << friendUserID << "\",";
					ostResult << "\"notificationFriendUserName\":\"" << friend_userName << "\",";
					ostResult << "\"notificationFriendUserNameLast\":\"" << friend_userNameLast << "\",";
					ostResult << "\"notificationFriendUserSex\":\"" << friend_userSex << "\",";
					ostResult << "\"notificationFriendUserAvatar\":\"" << GetUserAvatarByUserID(friendUserID, db) << "\",";
					ostResult << "\"notificationEvent\":[" << GetEventListInJSONFormat("SELECT * FROM `events` WHERE `id`=\"" + event_id + "\";", db, NULL) << "]";
					// ostResult << "\"notificationEventTimestamp\":\"" << eventTimestamp << "\"";

				}
				else
				{
					CLog log;
					log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR:typeID=" + to_string(typeID) + ": ERROR selecting from users[userID = " + friendUserID + "]");
				}
			}
			else
			{
				CLog log;
				log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR:typeID=" + to_string(typeID) + ": ERROR selecting from event[id = " + event_id + "]");
			}

		}
		else
		{
			CLog log;
			log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR:typeID=" + to_string(typeID) + ": ERROR selecting from event_hosts/event_guests[id = " + event_host_id + "]");
		}
	}

	// --- 70 - 1 day notification about start
	if(typeID == 70)
	{
		string		event_id = to_string(actionID);

		if(event_id.length() && db->Query("SELECT * FROM `events` WHERE `id`=\"" + event_id + "\";"))
		{
			string	eventTimestamp = db->Get(0, "startTimestamp");
			string  friendUserID = db->Get(0, "owner_id");

			if(friendUserID.length() && db->Query("select * from `users` where `id`='" + friendUserID + "';"))
			{
				string  friend_userName = db->Get(0, "name");
				string  friend_userNameLast = db->Get(0, "nameLast");

				ostResult << "\"notificationFriendUserID\":\"" << friendUserID << "\",";
				ostResult << "\"notificationFriendUserName\":\"" << friend_userName << "\",";
				ostResult << "\"notificationFriendUserNameLast\":\"" << friend_userNameLast << "\",";
				ostResult << "\"notificationFriendUserAvatar\":\"" << GetUserAvatarByUserID(friendUserID, db) << "\",";
				ostResult << "\"notificationEvent\":[" << GetEventListInJSONFormat("SELECT * FROM `events` WHERE `id`=\"" + event_id + "\";", db, NULL) << "]";
			}
			else
			{
				CLog log;
				log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR:typeID=" + to_string(typeID) + ": ERROR selecting from users[userID = " + friendUserID + "]");
			}
		}
		else
		{
			CLog log;
			log.Write(ERROR, string(__func__) + "[" + to_string(__LINE__) + "]:ERROR:typeID=" + to_string(typeID) + ": ERROR selecting from event[id = " + event_id + "]");
		}

	}



	{
		CLog	log;
		ostringstream   ost;

		ost.str("");
		ost << __func__ << "(typeID=" << typeID << ", actionID=" << actionID << ")[" << __LINE__ << "]: end (return strlen=" << ostResult.str().length() << ")";
		log.Write(DEBUG, ost.str());
	}

	return  ostResult.str();
}

string  GetUserNotificationInJSONFormat(string sqlRequest, CMysql *db, CUser *user)
{
	int			 affected;
	ostringstream   ostUserNotifications;

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) +  "]: start");
	}

	ostUserNotifications.str("");
	ostUserNotifications << "[";

	affected = db->Query(sqlRequest);
	if(affected)
	{
		class DBUserNotification
		{
			public:
				string	  notificationID;
				string	  notificationStatus;
				string	  feed_eventTimestamp;
				string	  feed_actionId;
				string	  feed_actionTypeId;
				string	  action_types_title;
				string	  action_types_title_male;
				string	  action_types_title_female;
				string	  user_id;
				string	  user_name;
				string	  user_nameLast;
				string	  user_sex;
				string	  user_email;
				string	  action_category_title;
				string	  action_category_title_male;
				string	  action_category_title_female;
				string	  action_category_id;
		};

		vector<DBUserNotification>  dbResult;

		for(int i = 0; i < affected; ++i)
		{
			DBUserNotification	  tmpObj;

			tmpObj.notificationID = db->Get(i, "users_notification_id");
			tmpObj.notificationStatus = db->Get(i, "users_notification_notificationStatus");
			tmpObj.feed_actionTypeId = db->Get(i, "feed_actionTypeId");
			tmpObj.action_types_title = db->Get(i, "action_types_title");
			tmpObj.action_types_title_male = db->Get(i, "action_types_title_male");
			tmpObj.action_types_title_female = db->Get(i, "action_types_title_female");
			tmpObj.action_category_title = db->Get(i, "action_category_title");
			tmpObj.action_category_title_male = db->Get(i, "action_category_title_male");
			tmpObj.action_category_title_female = db->Get(i, "action_category_title_female");
			tmpObj.feed_eventTimestamp = db->Get(i, "feed_eventTimestamp");
			tmpObj.user_id = db->Get(i, "user_id");
			tmpObj.user_name = db->Get(i, "user_name");
			tmpObj.user_nameLast = db->Get(i, "user_nameLast");
			tmpObj.user_sex = db->Get(i, "user_sex");
			tmpObj.user_email = db->Get(i, "user_email");
			tmpObj.action_category_id = db->Get(i, "action_category_id");
			tmpObj.feed_actionId = db->Get(i, "feed_actionId");

			dbResult.push_back(tmpObj);
		}


		for(auto &it: dbResult)
		{
			string	  userNotificationEnrichment = "";

			if(ostUserNotifications.str().length() > 20) ostUserNotifications << ",";
			ostUserNotifications << "{";

			// --- common part
			ostUserNotifications << "\"notificationID\":\"" << it.notificationID << "\",";
			ostUserNotifications << "\"notificationTypeID\":\"" << it.feed_actionTypeId << "\",";
			ostUserNotifications << "\"notificationTypeTitle\":\"" << it.action_types_title << "\",";
			ostUserNotifications << "\"notificationTypeTitleMale\":\"" << it.action_types_title_male << "\",";
			ostUserNotifications << "\"notificationTypeTitleFemale\":\"" << it.action_types_title_female << "\",";
			ostUserNotifications << "\"notificationCategoryID\":\"" << it.action_category_id << "\",";
			ostUserNotifications << "\"notificationCategoryTitle\":\"" << it.action_category_title << "\",";
			ostUserNotifications << "\"notificationCategoryTitleMale\":\"" << it.action_category_title_male << "\",";
			ostUserNotifications << "\"notificationCategoryTitleFemale\":\"" << it.action_category_title_female << "\",";
			ostUserNotifications << "\"notificationEventTimestamp\":\"" << it.feed_eventTimestamp << "\",";
			ostUserNotifications << "\"notificationOwnerUserID\":\"" << it.user_id << "\",";
			ostUserNotifications << "\"notificationOwnerUserName\":\"" << it.user_name << "\",";
			ostUserNotifications << "\"notificationOwnerUserNameLast\":\"" << it.user_nameLast << "\",";
			ostUserNotifications << "\"notificationOwnerUserSex\":\"" << it.user_sex << "\",";
			ostUserNotifications << "\"notificationActionID\":\"" << it.feed_actionId << "\",";
			ostUserNotifications << "\"notificationStatus\":\"" << it.notificationStatus << "\"";

			userNotificationEnrichment = GetUserNotificationSpecificDataByType(atol(it.feed_actionTypeId.c_str()), atol(it.feed_actionId.c_str()), db, user);
			if(userNotificationEnrichment.length()) ostUserNotifications << "," << userNotificationEnrichment;

			ostUserNotifications << "}";
		}
	}
	ostUserNotifications << "]";

	{
		CLog	log;
		log.Write(DEBUG, string(__func__) + "[" + to_string(__LINE__) +  "]: finish");
	}
	
	return ostUserNotifications.str();
}

string GetGiftListInJSONFormat(string dbQuery, CMysql *db, CUser *user)
{
	struct ItemClass 
	{
		string	id;
		string	link;
		string	title;
		string	description;
		string	folder;
		string	filename;
		string	requested_quantity;
		string	gained_quantity;
		string	estimated_price;
		string	user_id;
		string	eventTimestamp;
	};

	ostringstream			ost, ostFinal;
	string					sessid, lookForKey;
	int						affected;
	vector<ItemClass>		groupsList;

	{
		MESSAGE_DEBUG("", "", "start");
	}

	ostFinal.str("");

	if((affected = db->Query(dbQuery)) > 0)
	{
		int			groupCounter = affected;

		groupsList.reserve(groupCounter);  // --- reserving allows avoid moving vector in memory
											// --- to fit vector into continuous memory piece

		for(int i = 0; i < affected; i++)
		{
			ItemClass	gift;

			gift.id = db->Get(i, "id");
			gift.link = db->Get(i, "link");
			gift.title = db->Get(i, "title");
			gift.description = db->Get(i, "description");
			gift.folder = db->Get(i, "logo_folder");
			gift.filename = db->Get(i, "logo_filename");
			gift.requested_quantity = db->Get(i, "requested_quantity");
			gift.gained_quantity = db->Get(i, "gained_quantity");
			gift.estimated_price = db->Get(i, "estimated_price");
			gift.user_id = db->Get(i, "user_id");
			gift.eventTimestamp = db->Get(i, "eventTimestamp");

			groupsList.push_back(gift);
		}

		for(int i = 0; i < groupCounter; i++)
		{
				if(ostFinal.str().length()) ostFinal << ", ";

				ostFinal << "{";
				ostFinal << "\"id\": \""				  	<< groupsList[i].id << "\",";
				ostFinal << "\"link\": \""					<< groupsList[i].link << "\",";
				ostFinal << "\"title\": \""					<< groupsList[i].title << "\",";
				ostFinal << "\"description\": \""			<< groupsList[i].description << "\",";
				ostFinal << "\"logo_folder\": \""			<< groupsList[i].folder << "\",";
				ostFinal << "\"logo_filename\": \""			<< groupsList[i].filename << "\",";
				ostFinal << "\"requested_quantity\": \""	<< groupsList[i].requested_quantity << "\",";
				ostFinal << "\"gained_quantity\": \""		<< groupsList[i].gained_quantity << "\",";
				ostFinal << "\"estimated_price\": \""		<< groupsList[i].estimated_price << "\",";
				ostFinal << "\"user_id\": \""				<< groupsList[i].user_id << "\",";
				ostFinal << "\"eventTimestamp\": \""		<< groupsList[i].eventTimestamp << "\"";
				ostFinal << "}";
		} // --- for loop through gift list

	} // --- if sql-query on gift selection success
	else
	{
		MESSAGE_DEBUG("", "", "there are no gifts returned by request [" + dbQuery + "]");
	}

	{
		MESSAGE_DEBUG("", "", "end (result length = " + to_string(ostFinal.str().length()) + ")");
	}

	return ostFinal.str();
}

