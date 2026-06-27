/* 
 * File:   mainsubs1.h
 * Author: john
 *
 * Created on September 22, 2010, 5:58 AM
 */

#ifndef _MAINSUBS1_H
#define	_MAINSUBS1_H

int BuildRouteTable(void );
int BuildWNATtable(void );
int BuildPagerTable(void);
void CountMessage(int );
void UpdateWDtable(WDupdateList&, int, int);
void send_reboot_email(emailer*);
void SendStatusEmail(emailer*, std::string);
void SendNoticeEmail(emailer* , std::string);
std::string StatisticsToHTML(void);
std::string BuildStatsHTML(void);
int GetFileDescrptorLimit(void);
bool ChangeConfigSetting(const std::string& , const std::string&  , const std::string& , const std::string&  );
void CheckForDBupdates(void);
bool LoadTablesFromDB(database* );
bool LoadTable(database* aDB, datatable *, std::string);
void ResetCounters(void);

#endif	/* _MAINSUBS1_H */

