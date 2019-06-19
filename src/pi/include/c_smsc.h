#ifndef __C_SMSC__H__
#define __C_SMSC__H__
#include <curl/curl.h>
#include <string.h>
#include <libintl.h>

#include "utilities.h"	// --- split and CMysql
#include "clog.h"
#include "localy.h"

using namespace std;

class c_smsc
{
private:
	string					__SMSC_LOGIN = SMSC_LOGIN;// ����� �������
	string					__SMSC_PASSWORD = SMSC_PASSWORD;// ������
	char					__SMSC_HTTPS = 1;// ������������ �������� HTTPS
	char					__SMSC_POST = 0;// ������������ ����� POST

	// ��������� ��� �������� SMS �� SMTP 
	string			 		__SMTP_FROM = "api@smsc.ru";        // e-mail ����� ����������� 
	string			 		__SMTP_SERVER = "send.smsc.ru";    // ����� smtp ������� 
	string			 		__SMTP_LOGIN = "";            // ����� ��� smtp ������� 
	string			 		__SMTP_PASSWORD = "";            // ������ ��� smtp ������� 

	CMysql					*db = nullptr;

	void _print_debug(string str);
	string _smsc_send_cmd(string cmd, string arg, string files);
	string	TrackSMS(string phones, string sms_id, string sms_cost, string sms_quantity, string sms_text, string current_balance);

public:
			c_smsc(CMysql *param) : db(param)	{};
	void	SetDB(CMysql *param)				{ db = param; };
	string	urlencode(string str);
	string	urldecode(string str);

	// ������� �������� SMS
	//
	// ������������ ���������:
	//
	// phones - ������ ��������� ����� ������� ��� ����� � �������
	// message - ������������ ���������
	//
	// �������������� ���������:
	//
	// translit - ���������� ��� ��� � �������� (1,2 ��� 0)
	// time - ����������� ����� �������� � ���� ������ (DDMMYYhhmm, h1-h2, 0ts, +m)
	// id - ������������� ���������. ������������ ����� 32-������ ����� � ��������� �� 1 �� 2147483647.
	// format - ������ ��������� (0 - ������� sms, 1 - flash-sms, 2 - wap-push, 3 - hlr, 4 - bin, 5 - bin-hex, 6 - ping-sms, 7 - mms, 8 - mail, 9 - call, 10 - viber)
	// sender - ��� ����������� (Sender ID).
	// query - ������ �������������� ����������, ����������� � URL-������ ("valid=01:00&maxsms=3&tz=2")
	// files - ������ ����� � ������ ��� �������� mms ��� e-mail ���������
	//
	// ���������� <id>, <���������� sms>, <���������>, <������> � ������ �������� ��������
	// ���� <id>, -<��� ������> � ������ ������

	string send_sms (string phones, string message, int translit, string time, int id, int format, string sender, string query, string files);

	// SMTP ������ ������ �������� SMS 
	// void send_sms_mail(string phones, string mes, int translit, string time, int id, int format, string sender);

	// ��������� ��������� SMS 
	// 
	// ������������ ���������: 
	// 
	// phones - ������ ��������� ����� ������� ��� ����� � ������� 
	// message - ������������ ��������� 
	// 
	// �������������� ���������: 
	// 
	// translit - ���������� ��� ��� � �������� 
	// format - ������ ��������� (0 - ������� sms, 1 - flash-sms, 2 - wap-push, 3 - hlr, 4 - bin, 5 - bin-hex, 6 - ping-sms, 7 - mms, 8 - mail, 9 - call) 
	// sender - ��� ����������� (Sender ID) 
	// query - ������ �������������� ����������, ����������� � URL-������ ("list=79999999999:��� ������: 123\n78888888888:��� ������: 456") 
	// 
	// ���������� <���������>, <���������� sms> ���� 0, -<��� ������> � ������ ������ 

	string get_sms_cost(string phones, string mes, int translit, int format, string sender, string query);

	// �������� ������� ������������� SMS ��� HLR-������� 
	//
	// id - ID c�������� ��� ������ ID ����� �������
	// phone - ����� �������� ��� ������ ������� ����� �������
	// all - ������� ��� ������ ������������� SMS, ������� ����� ��������� (0,1 ��� 2)
	//
	//
	// ��� ���������� SMS-���������:
	// <������>, <����� ���������>, <��� ������ ��������>
	//
	// ��� HLR-�������:
	// <������>, <����� ���������>, <��� ������ sms>, <��� IMSI SIM-�����>, <����� ������-������>, <��� ������ �����������>, <��� ���������>,
	// <�������� ������ �����������>, <�������� ���������>, <�������� ����������� ������>, <�������� ������������ ���������>
	//
	// ��� all = 1 ������������� ������������ ��������:
	// <����� ��������>, <����� ��������>, <���������>, <sender id>, <�������� �������>, <����� ���������>
	//
	// ��� all = 2 ������������� ������������ �������� <������>, <��������> � <������>
	//
	// ���� all = 0, �� ��� ������� ��������� ��� HLR-������� ������������� ������������ <ID ���������> � <����� ��������>
	//
	// ���� all = 1 ��� all = 2, �� � ����� ����������� <ID ���������>
	//
	// ���� 0, -<��� ������> � ������ ������

	string get_status(string id, string phone, int all);

	// ��������� ������� 
	// 
	// ��� ���������� 
	// 
	// ���������� ������ � ���� ������ ��� ������ ������ � ������ ������ 

	string get_balance(void);
};

#endif

