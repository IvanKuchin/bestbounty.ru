#ifndef __UTILITIES_SQL_QUERIES__H__
#define __UTILITIES_SQL_QUERIES__H__

#include <string>

inline auto Get_ChecklistItemsByChecklistID_sqlquery(const string &id)
{
	return (
			"SELECT * FROM `checklist_items` "
			"INNER JOIN `checklist_predefined` ON `checklist_predefined`.`id`=`checklist_items`.`checklist_predefined_id` "
			"WHERE `checklist_items`.`event_checklist_id` IN (" + id +  ") "
		);
}

inline auto Get_ChecklistItemsIDByChecklistID_sqlquery(const string &id)
{
	return (
			"SELECT `id` FROM `checklist_items` "
			"WHERE `checklist_items`.`event_checklist_id` IN (" + id +  ") "
		);
}

inline auto Get_EventIDByChecklistItemID(const string &id)
{
	return (
			"SELECT `event_id` FROM `event_checklists` WHERE `id` IN ("
				"SELECT `event_checklist_id` FROM `checklist_items` WHERE `id` IN (" + id + ")"
			") "
		);
}

inline auto Get_ChecklistItemsIDByEventID(const string &id)
{
	return (
			"SELECT `id` FROM `event_checklists` WHERE `event_id` IN (" + id + ")"
		);
}

#endif
