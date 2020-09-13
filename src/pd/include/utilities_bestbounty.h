#ifndef __UTILITIES_BESTNOUNTY__H__
#define __UTILITIES_BESTNOUNTY__H__

#include "utilities.h"
#include "utilities_sql_queries.h"

string      	GetUserListInJSONFormat(string dbQuery, CMysql *, CUser *);
string 			GetNewsFeedInJSONFormat(string whereStatement, int currPage, int newsOnSinglePage, CUser *, CMysql *);
string      	GetMessageLikesUsersList(string messageID, CUser *, CMysql *);
string      	GetUserNotificationSpecificDataByType(unsigned long typeID, unsigned long actionID, CMysql *, CUser *);
string      	GetUserNotificationInJSONFormat(string sqlRequest, CMysql *, CUser *);

string			GetMessageImageList(string imageSetID, CMysql *);
string			GetMessageCommentsCount(string messageID, CMysql *);
string			GetMessageSpam(string messageID, CMysql *);
string			GetMessageSpamUser(string messageID, string userID, CMysql *);

auto			GetEventGuestsListInJSONFormat(string dbQuery, CMysql *, CUser *) -> string;
auto 	     	GetCompanyListInJSONFormat(string dbQuery, CMysql *, CUser *, bool quickSearch = true, bool includeEmployedUsersList = false) -> string;
auto 	     	GetGroupListInJSONFormat(string dbQuery, CMysql *, CUser *) -> string;
auto 			GetBookListInJSONFormat(string dbQuery, CMysql *, bool includeReaders = false) -> string;
auto 			GetComplainListInJSONFormat(string dbQuery, CMysql *, bool includeReaders = false) -> string;
auto 			GetCertificationListInJSONFormat(string dbQuery, CMysql *, bool includeDevoted = false) -> string;
auto 			GetCourseListInJSONFormat(string dbQuery, CMysql *, bool includeStudents = false) -> string;
auto 			GetLanguageListInJSONFormat(string dbQuery, CMysql *, bool includeStudents = false) -> string;
auto 			GetSkillListInJSONFormat(string dbQuery, CMysql *) -> string;
auto 			GetUniversityListInJSONFormat(string dbQuery, CMysql *, bool includeStudents = false) -> string;
auto 			GetSchoolListInJSONFormat(string dbQuery, CMysql *, bool includeStudents = false) -> string;
auto			GetGiftListInJSONFormat(string dbQuery, CMysql *, CUser *) -> string;
auto 			GetGiftToGiveListInJSONFormat(string dbQuery, CMysql *, CUser *) -> string;
auto			GetEventListInJSONFormat(string dbQuery, CMysql *, CUser *) -> string;
auto			GetEventChecklistInJSONFormat(const string &dbQuery, CMysql *, CUser *) -> string;
auto			GetEventHostsListInJSONFormat(string dbQuery, CMysql *, CUser *) -> string;
auto 			GetFavoriteChecklistCategoriesInJSONFormat(const string &dbQuery, CMysql *, CUser *) -> string;
auto 			GetFavoriteChecklistItemsInJSONFormat(const string &dbQuery, CMysql *, CUser *) -> string;
auto 			GetCertificationLikesUsersList(string usersCertificationID, CUser *, CMysql *) -> string;
auto 			GetCourseLikesUsersList(string usersCourseID, CUser *, CMysql *) -> string;
auto 			GetUniversityDegreeLikesUsersList(string messageID, CUser *, CMysql *) -> string;
auto 			GetCompanyLikesUsersList(string usersCompanyID, CUser *, CMysql *) -> string;
auto 			GetLanguageLikesUsersList(string usersLanguageID, CUser *, CMysql *) -> string;
auto 			GetBookLikesUsersList(string usersBookID, CUser *, CMysql *) -> string;
auto 			GetBookRatingList(string bookID, CMysql *) -> string;
auto 			GetCourseRatingList(string courseID, CMysql *) -> string;
auto 			GetBookRatingUsersList(string bookID, CUser *, CMysql *) -> string;
auto 			GetCompanyCommentsCount(string messageID, CMysql *) -> string;
auto 			GetLanguageCommentsCount(string messageID, CMysql *) -> string;
auto 			GetBookCommentsCount(string messageID, CMysql *) -> string;
auto 			GetCertificateCommentsCount(string messageID, CMysql *) -> string;
auto 			GetUniversityDegreeCommentsCount(string messageID, CMysql *) -> string;
auto			GetLanguageIDByTitle(string title, CMysql *) -> string;
auto			GetSkillIDByTitle(string title, CMysql *) -> string;
auto			GetCompanyPositionIDByTitle(string title, CMysql *) -> string;
auto 			GetCandidatesListAppliedToVacancyInJSONFormat(string dbQuery, CMysql *) -> string;
auto 			GetOpenVacanciesInJSONFormat(string companyID, CMysql *, CUser * = NULL) -> string;
auto			addMissedChecklistItems(const string &from_checklist_id, string const &to_checklist_id, CMysql *) -> string;
auto 			addChecklistItem(string const &to_checklist_id, const string &title, const string &category, CMysql *, CUser *) -> string;
auto 			GetChecklistIDByEventID_CreateIfMissed(const string &event_id, CMysql *, CUser *) -> string;
auto			RemoveChecklistsByEventID(const string &event_id, CMysql *db, CUser *user) -> string;

auto			GetUserListInJSONFormat_BySearchString(const string lookForKey, bool include_myself, CMysql *, CUser *) -> string;

auto			amIAllowedToChangeEvent(const string id, CMysql *, CUser *) -> string;

#endif
