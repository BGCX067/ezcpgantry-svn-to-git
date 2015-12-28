/*
 * EZCP_UserDisplay.c
 *
 *  Created on: Apr 23, 2011
 *      Author: Group 8
 */

#include "EZCP_Common.h"


char WelcomeString[DISPLAY_MSG_LEN] = {" Welcome \n Group 8 EZCP."};
char ArrivalString[DISPLAY_MSG_LEN] = {" Processing.\n Please wait ..."};
char ProceedString[DISPLAY_MSG_LEN] = {" Please Proceed."};
char CompliValidString[DISPLAY_MSG_LEN] = {" Complimentary Card.\n Valid \n Please Proceed"};
char CompliInvalidString[DISPLAY_MSG_LEN] = {" Complimentary Card.\n Invalid.\n Please Check."};
char CPAttendString[DISPLAY_MSG_LEN] = {" Assistance Requested."};
char AuthorizeExit[DISPLAY_MSG_LEN] = {" Authorized to Proceed. "};
char NoAuthorizeExit[DISPLAY_MSG_LEN] = {" NOT Authorized to Proceed. \nPlease Check"};
char MaintenanceString[DISPLAY_MSG_LEN] = {" Under Maintenance "};
char MaintenanceString_Up[DISPLAY_MSG_LEN] = {" Under Maintenance. \n Gantry ARM UP "};
char MaintenanceString_Down[DISPLAY_MSG_LEN] = {" Under Maintenance. \n Gantry ARM DOWN "};
char MaintenanceString_BIT[DISPLAY_MSG_LEN] = {" Under Maintenance. \n Running Tests "};
char CashNotEnough[DISPLAY_MSG_LEN] = {" Insufficient balance. \nPlease Check "};

char ObstacleString[DISPLAY_MSG_LEN] = {" Obstacle Detected. "};
char ObstacleClearString[DISPLAY_MSG_LEN] = {" Obstacle Cleared. "};
char FaultyString[DISPLAY_MSG_LEN] = {" Faulty Gantry.\n Please Proceed to \n another Gantry."};
char EmergString[DISPLAY_MSG_LEN] = {" Fire Emergency !!!\n PLEASE LEAVE \n IMMEDIATELY"};

void GantryArmMsg(u32 ui_GantryID, u32 uiDir,char *Msg)
{
	int Output = 0;

	//Send Gantry Open message
	print("%s: Message to Gantry Arm - %d \n",__func__,uiDir);
	gl_MsgBuf_Outgoing_Msg[ui_GantryID].MsgID = 1;
	Output = AppendBits_GantryArm(ui_GantryID,uiDir);
	memcpy(gl_MsgBuf_Outgoing_Msg[ui_GantryID].Data,&Output,sizeof(int));

	pthread_mutex_lock(&g1_pt_mutex_OutgoingMsg);
		EZCP_App_SendOutgoingMessage (ui_GantryID);
	pthread_mutex_unlock(&g1_pt_mutex_OutgoingMsg);

	if (uiDir == GANTRY_ARM_UP )
		DisplayMsg(ui_GantryID,Msg);
}

void CashDeductedMsg(u32 ui_GantryID, u32 ui_CashDeducted)
{
	int Output = 0;
	//Send Cash deducted
	print("%s: Send Cash deducted message to Antenna\n",__func__);
	gl_MsgBuf_Outgoing_Msg[ui_GantryID].MsgID = 5;
	Output = AppendBits_CashDeduct(ui_GantryID,ui_CashDeducted);
	memcpy(gl_MsgBuf_Outgoing_Msg[ui_GantryID].Data,&Output,sizeof(int));
	pthread_mutex_lock(&g1_pt_mutex_OutgoingMsg);
		EZCP_App_SendOutgoingMessage (ui_GantryID);
	pthread_mutex_unlock(&g1_pt_mutex_OutgoingMsg);
}

void EZCPAttendantMsg(u32 ui_GantryID, u32 uiDecision)
{
	int Output = 0;
	//Send Gantry Open message
	print("%s: Message from Attendant - %d \n",__func__,uiDecision);
	gl_MsgBuf_Outgoing_Msg[ui_GantryID].MsgID = 12;
	Output = AppendBits_GantryArm(ui_GantryID,uiDecision);
	memcpy(gl_MsgBuf_Outgoing_Msg[ui_GantryID].Data,&Output,sizeof(int));
	pthread_mutex_lock(&g1_pt_mutex_OutgoingMsg);
		EZCP_App_SendOutgoingMessage (ui_GantryID);
	pthread_mutex_unlock(&g1_pt_mutex_OutgoingMsg);
}

void TicketIssueMsg(u32 ui_GantryID, u32 ui_Label,u32 ui_CashAvail,u32 ui_CashDeducted,u32 ui_InTime,u32 ui_OutTime)
{

	char Out[20];
	char Ticket[DISPLAY_MSG_LEN];
	print("%s: Issue Ticket !\n",__func__);

	//Ticket Issue message
	gl_MsgBuf_Outgoing_Msg[ui_GantryID].MsgID = 18;
	AppendBits_TicketIssue(Out,ui_GantryID,ui_Label,ui_CashAvail,ui_CashDeducted,ui_InTime,ui_OutTime);
	memset(gl_MsgBuf_Outgoing_Msg[ui_GantryID].Data,0, sizeof(gl_MsgBuf_Outgoing_Msg[ui_GantryID].Data));
	memcpy(gl_MsgBuf_Outgoing_Msg[ui_GantryID].Data,Out, 9 * sizeof(char));

	pthread_mutex_lock(&g1_pt_mutex_OutgoingMsg);
		EZCP_App_SendOutgoingMessage (ui_GantryID);
	pthread_mutex_unlock(&g1_pt_mutex_OutgoingMsg);
	if ( ui_Label == 0x199f)
		sprintf(Ticket," Peak Hour Charges. \n CashAvail: %x \n CashDeucted: %x \n Intime: %x \n Outtime: %x \n",ui_CashAvail,ui_CashDeducted,ui_InTime,ui_OutTime);
	else
		sprintf(Ticket,"\n CashAvail: %x \n CashDeucted: %x \n Intime: %x \n Outtime: %x \n",ui_CashAvail,ui_CashDeducted,ui_InTime,ui_OutTime);
	DisplayMsg(ui_GantryID,Ticket);
}

void DisplayMsg(u32 ui_GantryID, char* ui_MsgDisplay)
{
	print("%s: Display Message to User !\n",__func__);

	//Send Display message
	gl_MsgBuf_Outgoing_Msg[ui_GantryID].MsgID = 10;

	memset(gl_MsgBuf_Outgoing_Msg[ui_GantryID].Data,0,EZCP_MAX_DATA_LEN);
	gl_MsgBuf_Outgoing_Msg[ui_GantryID].Data[0] = ui_GantryID;
	memmove(gl_MsgBuf_Outgoing_Msg[ui_GantryID].Data + 1,ui_MsgDisplay, (EZCP_MAX_DATA_LEN-1));

	pthread_mutex_lock(&g1_pt_mutex_OutgoingMsg);
		EZCP_App_SendOutgoingMessage (ui_GantryID);
	pthread_mutex_unlock(&g1_pt_mutex_OutgoingMsg);
}


int ParseBits(msg_data_buf_t buf, int Start, int Length)
{
	int i = 0;
	long message_value = 0;
	int mask;
    int some = 0;
	for(i = 0; i < 4; i++)
	{
		some = pow((double)256,(double)i);
        message_value += buf[i] * some ;
	}

	message_value = message_value >> (Start-1);

	mask = pow((double)2,(double)Length)-1;

	message_value = message_value & mask;

	return message_value;

}

int AppendBits_GantryArm(u32 GantryID,u32 Dir)
{
	int StartIndex, Length;
	unsigned int Mask = 0;
	unsigned int Output = 0;

	//First for gantry
	StartIndex = 0;
	Length = GANTRYID_BIT_LENGTH;
	Mask = pow( 2 , StartIndex ) - 1;
	GantryID = (GantryID << StartIndex) | Mask ;

	Mask = pow( 2 , Length ) - 1;
	Mask = Mask << StartIndex;
	Output = Output | Mask;
	Output = Output & GantryID;

	//Then for next bit
	StartIndex = 3;
	Length = 1;
	Mask = pow( 2 , StartIndex ) - 1;
	Dir = (Dir << StartIndex) | Mask ;

	Mask = pow( 2 , Length ) - 1;
	Mask = Mask << StartIndex;
	Output = Output | Mask;
	Output = Output & Dir;

	return Output;
}

int AppendBits_CashDeduct(u32 GantryID,u32 Cash)
{
	int StartIndex, Length;
	unsigned int Mask = 0;
	unsigned int Output = 0;

	//First for gantry
	StartIndex = 0;
	Length = GANTRYID_BIT_LENGTH;
	Mask = pow( 2 , StartIndex ) - 1;
	GantryID = (GantryID << StartIndex) | Mask ;

	Mask = pow( 2 , Length ) - 1;
	Mask = Mask << StartIndex;
	Output = Output | Mask;
	Output = Output & GantryID;

	//Then for next bit
	StartIndex = 4;
	Length = CASH_BIT_LENGTH;
	Mask = pow( 2 , StartIndex ) - 1;
	Cash = (Cash << StartIndex) | Mask ;

	Mask = pow( 2 , Length ) - 1;
	Mask = Mask << StartIndex;
	Output = Output | Mask;
	Output = Output & Cash;

	return Output;
}

//HardCoded Values for Tickets
void AppendBits_TicketIssue(char Out[],u32 ui_GantryID,u32 ui_Label,u32 ui_CashAvail,u32 ui_CashDeducted,u32 ui_InTime,u32 ui_OutTime)
{
	unsigned int Output = 0, k = 0;

	for(k = 0; k < 9; k++)
			Out[k] =0;
	//Gantry ID
	Out[0] = 0;
	Output = ui_GantryID;
	Out[0] = Output & 0x07;

	// IU/CashCard

	Output = ui_Label;
	Output = Output & (0x1F);
	Output = Output << 3;
	Out[0] = (char)Out[0] | Output;

	Output = ui_Label;
	Output = Output & (0x1FE0);
	Output = Output >> 5;
	Out[1] = (char)Out[1] | Output;

	//Cash Available
	Output = ui_CashAvail;
	Output = Output & (0xFF);
	//Output = Output << 3;
	Out[2] = (char)Out[2] | Output;

	Output = ui_CashAvail;
	Output = Output & (0x0F00);
	Output = Output >> 8;
	Out[3] = (char)Out[3] | Output;

	Out[3] = Out[3] & 0x0F;
	//CashDeducted
	Output = ui_CashDeducted;
	Output = Output & (0x0F);
	Output = Output << 4;
	Out[3] = Out[3] | Output;

	Output = ui_CashDeducted;
	Output = Output & (0x0FF0);
	Output = Output >> 4;
	Out[4] = Out[4] | Output;

	//EntryTime
	Output = ui_InTime;
	Output = Output & (0xFF);
	//Output = Output << 4;
	Out[5] = Out[5] | Output;

	Output = ui_InTime;
	Output = Output & (0xFF00);
	Output = Output >> 8;
	Out[6] = Out[6] | Output;

	//Exit Time
	Output = ui_OutTime;
	Output = Output & (0xFF);
	//Output = Output << 4;
	Out[7] = Out[7] | Output;

	Output = ui_OutTime;
	Output = Output & (0xFF00);
	print("%s: Output - %x\n",__func__,Output);
	Output = Output >> 8;

	//print("Message Out to SIM \n");
	print("%s: Out[8] - Output - %x %x\n",__func__,Out[8],Output);
	Out[8] = Out[8] | Output;
	print("%s: Out[8] - %x \n",__func__,Out[8]);

}

unsigned char nibbleSwap(unsigned char a)
{
	return (a<<4) | (a>>4);
}
