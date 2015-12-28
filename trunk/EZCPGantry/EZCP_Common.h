/*
 * EZCP_Common.h
 *
 *  Created on: Mar 27, 2011
 *      Author: Group 8
 */

#ifndef EZCP_COMMON_H_
#define EZCP_COMMON_H_

//Header files
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <sched.h>
#include <unistd.h>
#include <errno.h>
#include <err.h>
#include <math.h>
#include <sys/dispatch.h>
#include <string.h>

#include <qdb/qdb.h>
//#include <process.h>

#include "EZCP_Comms.h"

//Needed for printf wrapper function
#include <stdarg.h>


typedef char 			u8;
typedef unsigned int 	u32;
typedef signed int 		s32;
typedef float 			f32;
typedef double 			f64;


//Debug Levels
#define E 	1
#define W 	2
#define D 	3

#define MAX_NO_GANTRY 8

#define WAIT 0
#define NOWAIT 1	//binary 01
#define TIMEOUT	2	//binary 10

#define GANTRY_ARM_UP 1
#define GANTRY_ARM_DOWN 0

#define EXIT_AUTHORIZED 1
#define EXIT_NOTAUTHORIZED 0

#define ENTRY 1
#define EXIT 0

#define GANTRYID_BIT_LENGTH 3
#define IULABEL_BIT_LENGTH 13
#define CASH_BIT_LENGTH 12

#define TYPE_IU 1
#define TYPE_CASHCARD 2
#define TYPE_COMPLICARD 3

#define ENTRY_TIMEOUT_VAL  10	//secs

#ifdef EZCPMAIN
#define EXTERN
#else
#define EXTERN extern
#endif

#define DBG
//#undef DBG

#ifdef DBG
#define print(format, args...) printf (format, ## args)
#else
#define print(format, args...)
#endif

//Synchronize
EXTERN pthread_mutex_t g1_pt_mutex_Gantry[MAX_NO_GANTRY];
EXTERN pthread_mutex_t g1_pt_mutex_Emerg;
EXTERN pthread_mutex_t g1_pt_mutex_OutgoingMsg;
EXTERN pthread_mutexattr_t g1_pt_mutexattr_Gantry[MAX_NO_GANTRY];
EXTERN pthread_mutexattr_t g1_pt_mutexattr_Emerg;
EXTERN pthread_mutexattr_t g1_pt_mutexattr_OutgoingMsg;

EXTERN pthread_cond_t g1_pt_cv_Gantry[MAX_NO_GANTRY];
EXTERN pthread_cond_t g1_pt_cv_Emerg;
EXTERN pthread_cond_t g1_pt_cv_OutgoingMsg;
EXTERN pthread_condattr_t g1_pt_cvattr_Gantry[MAX_NO_GANTRY];
EXTERN pthread_condattr_t g1_pt_cvattr_Emerg;
EXTERN pthread_condattr_t g1_pt_cvattr_OutgoingMsg;

EXTERN u32 gl_ui_cvFlag_Gantry[MAX_NO_GANTRY];
EXTERN u32 gl_ui_cvFlag_Emerg;
EXTERN u32 gl_ui_cvFlag_OutgoingMsg;

EXTERN MsgBuffer gl_MsgBuf_Incoming_Gantry[MAX_NO_GANTRY];
EXTERN MsgBuffer gl_MsgBuf_Incoming_Emerg;
EXTERN MsgBuffer gl_MsgBuf_Outgoing_Msg[MAX_NO_GANTRY];
//EXTERN MsgBuffer gl_MsgBuf_Outgoing_Msg;
//EXTERN MsgBuffer gl_MsgBuf_Outgoing_Gantry[MAX_NO_GANTRY];
//EXTERN MsgBuffer gl_MsgBuf_Outgoing_Emerg;

//EXTERN s32 g1_message;
//EXTERN u32 g_ui_GantryID;


//Function Declarations
void EZCP_ConfigManagerInit();
void EZCP_DBManagerInit();
void EZCP_CHUInit();

void *pf_CommsReceiver();
void *pf_CommsSender();
void *pf_GantryHandler(void *ptr);
void *pf_EmergencyHandler();

void EZCP_App_CreateChannel();
void EZCP_App_RecvIncomingMsg();

void EZCP_App_ConnectToChannel();
void EZCP_App_SendOutgoingMessage (u32 GantryID);

//void GantryArmMsg(u32 ui_GantryID, u32 uiDir);
void GantryArmMsg(u32 ui_GantryID, u32 uiDir,char *Msg);
void EZCPAttendantMsg(u32 ui_GantryID, u32 uiDecision);
void CashDeductedMsg(u32 ui_GantryID, u32 ui_CashDeducted);
void TicketIssueMsg(u32 ui_GantryID, u32 ui_Label, u32 ui_CashAvail, u32 ui_CashDeducted, u32 ui_InTime, u32 ui_OutTime);
void DisplayMsg(u32 ui_GantryID, char* ui_MsgDisplay);

void database_insert(int card_num, int card_type,int in_time);
int database_retrieve(int card_num, int card_type);
int database_retrieve_Compli(int card_num, int card_type);
void database_clear_entry(int card_num);
void database_delete();

int get_in_time(int IU_LABEL);
int get_out_time(int IU_LABEL);
//u32 EZCP_CHU_CalcParkingFee(u32 uiInTime,u32 uiOutTime);
u32 EZCP_CHU_CalcParkingFee(int IU_LABEL);

int ParseBits(msg_data_buf_t buf, int Start, int Length);
void AppendBits(int Input,int StartIndex,int Length);
int AppendBits_GantryArm(u32 GantryID,u32 Dir);
int AppendBits_CashDeduct(u32 GantryID,u32 Cash);
void AppendBits_TicketIssue(char* Out,u32 ui_GantryID,u32 ui_Label,u32 ui_CashAvail,u32 ui_CashDeducted,u32 ui_InTime,u32 ui_OutTime);

void InitAllGantries();

unsigned char nibbleSwap(unsigned char a);
void DEBUG_PRINT( int DebugLevel, const char* format, ... );

#endif /* EZCP_COMMON_H_ */
