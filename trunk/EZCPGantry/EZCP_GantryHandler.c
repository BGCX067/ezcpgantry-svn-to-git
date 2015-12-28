/*
 * EZCP_GantryHandler.c
 *
 *  Created on: Mar 27, 2011
 *      Author: Group 8
 */
#include "EZCP_Common.h"

extern unsigned int GANTRY_ATTRIBUTE [MAX_NO_GANTRY];
extern unsigned int VEHICLE_DETECTED[MAX_NO_GANTRY];
extern unsigned int OBSTACLE_DETECTED[MAX_NO_GANTRY];
extern unsigned int INTERCOM_BUTTON[MAX_NO_GANTRY];
extern unsigned int VALID_LABEL[MAX_NO_GANTRY];
extern unsigned int MANUAL_MODE[MAX_NO_GANTRY];
extern unsigned int EMERGENCY_MODE[MAX_NO_GANTRY];

extern unsigned int GANTRY_TIMER[MAX_NO_GANTRY];
extern unsigned int GANTRY_TIMEOUT[MAX_NO_GANTRY];

extern char WelcomeString[DISPLAY_MSG_LEN];
extern char ArrivalString[DISPLAY_MSG_LEN];
extern char ProceedString[DISPLAY_MSG_LEN];
extern char CompliValidString[DISPLAY_MSG_LEN];
extern char CompliInvalidString[DISPLAY_MSG_LEN];
extern char CPAttendString[DISPLAY_MSG_LEN];
extern char AuthorizeExit[DISPLAY_MSG_LEN];
extern char NoAuthorizeExit[DISPLAY_MSG_LEN];
extern char MaintenanceString[DISPLAY_MSG_LEN];
extern char ObstacleString[DISPLAY_MSG_LEN];
extern char ObstacleClearString[DISPLAY_MSG_LEN];
extern char MaintenanceString_Up[DISPLAY_MSG_LEN];
extern char MaintenanceString_Down[DISPLAY_MSG_LEN];
extern char MaintenanceString_BIT[DISPLAY_MSG_LEN];
extern char FaultyString[DISPLAY_MSG_LEN];
extern char CashNotEnough[DISPLAY_MSG_LEN];

void setGantryTimer (u32 ui_GantryID, int secs);
void resetGantryTimer (u32 ui_GantryID);

void *pf_GantryHandler(void *ptr)
{
	u32 ui_GantryID = (u32)ptr;
	u32 uiFee = 0, bTimeout, bMsgRcvd;
	u32 ui_InTime = 0,ui_OutTime = 0;

	MsgBuffer buf_local_data_Gantry;
	int Start_Parse = 0, Length_Parse = 0,Ret_Parse = 0, schdPol;
	int IU_LABEL = 0,CC_LABEL = 0,CASH_AVAIL = 0,COMPLI_LABEL= 0;
	int CardType = 0;

	u32 maxPrio, minPrio, ret;
	struct sched_param param;

	schdPol = sched_getscheduler(0);
	maxPrio = sched_get_priority_max (schdPol);
	minPrio = sched_get_priority_min (schdPol);
	ret     = pthread_getschedparam (pthread_self(), &schdPol, &param );
	print ("Gantry handler [%d]: schdPol %d [minPri=%d] [maxPrio=%d] [asgnPrio=%d] [curPrio=%d]\n", ui_GantryID, schdPol, minPrio, maxPrio, param.sched_priority, param.sched_curpriority);

	//Initialize setup
	//print("%s: GantryHandler[%d] Initiated\n",__func__,ui_GantryID);


	while(1)
	{
		//Wait for Messages
		pthread_mutex_lock(&g1_pt_mutex_Gantry[ui_GantryID]);
		//print("%s: Gantry[%d]: Waiting on Condition!\n",__func__,ui_GantryID);
		if (gl_ui_cvFlag_Gantry[ui_GantryID] == WAIT )
		{
			pthread_cond_wait(&g1_pt_cv_Gantry[ui_GantryID], &g1_pt_mutex_Gantry[ui_GantryID]);
		}

		bMsgRcvd = gl_ui_cvFlag_Gantry[ui_GantryID] & NOWAIT;
		bTimeout = gl_ui_cvFlag_Gantry[ui_GantryID] & TIMEOUT;

		if (bMsgRcvd)
		{
			buf_local_data_Gantry.Length = gl_MsgBuf_Incoming_Gantry[ui_GantryID].Length;
			buf_local_data_Gantry.MsgID = gl_MsgBuf_Incoming_Gantry[ui_GantryID].MsgID;
			memcpy(buf_local_data_Gantry.Data, gl_MsgBuf_Incoming_Gantry[ui_GantryID].Data,buf_local_data_Gantry.Length);
		}

		gl_ui_cvFlag_Gantry[ui_GantryID] = WAIT;
		pthread_mutex_unlock(&g1_pt_mutex_Gantry[ui_GantryID]);

		//Process timeout, if applicable
		if (bTimeout)
		{
			print ("Gantry Handler %d: Received Timeout\n", ui_GantryID);
			if (0 != VEHICLE_DETECTED[ui_GantryID])
			{
				GantryArmMsg(ui_GantryID, GANTRY_ARM_DOWN, NULL);
				DisplayMsg(ui_GantryID,FaultyString);
			}

		}

		//Process incoming message based on Message ID, if applicable
		if (! bMsgRcvd)
			continue;

		switch (buf_local_data_Gantry.MsgID)
		{
		case _EZCP_02://From Vehicle Sensor
				print("%s: MsgID 2 - Vehicle Sensor\n",__func__);
				//DisplayMsg(ui_GantryID,DISP_MSG);
				Start_Parse = 4;
				Length_Parse = 2;
				Ret_Parse = ParseBits(buf_local_data_Gantry.Data,Start_Parse,Length_Parse);

				//print("ParseBits returned Value: %x\n",Ret_Parse);
				if ( Ret_Parse & 0x02 )	//After gantry
				{
					//Vehicle not present
					if ( !(Ret_Parse & 0x01)  )
					{
						print("%s: Vehicle Departed at Gantry %d \n",__func__,ui_GantryID);
						//Send gantry close only if there is no obstacle
						if ( 0 == OBSTACLE_DETECTED[ui_GantryID])
						{
							resetGantryTimer (ui_GantryID);
							VEHICLE_DETECTED[ui_GantryID] = 0;
							GantryArmMsg(ui_GantryID,GANTRY_ARM_DOWN,NULL);
							DisplayMsg(ui_GantryID,WelcomeString);
						}
						else
						{
							print("%s: Gantry cannot be closed.\n",__func__);
						}
					}
				}
				else	//Before gantry
				{
					//Vehicle present
					if ( Ret_Parse & 0x01 )
					{
						// Set VEHICLE_DETECTED flag

						VEHICLE_DETECTED[ui_GantryID] = 1;
						DisplayMsg(ui_GantryID,ArrivalString);
						print("%s: Vehicle Arrived at Gantry %d.\n",__func__,ui_GantryID);
 					}
				}
				break;
		case _EZCP_03:// From Obstacle Sensor
				print("%s: MsgID 3 - From Obstacle Sensor\n",__func__);
				Start_Parse = 4;
				Length_Parse = 1;
				Ret_Parse = ParseBits(buf_local_data_Gantry.Data,Start_Parse,Length_Parse);
				if (Ret_Parse)
				{
					OBSTACLE_DETECTED[ui_GantryID] = 1;
					DisplayMsg(ui_GantryID,ObstacleString);
					print("%s: Obstacle Detected ! \n",__func__);
				}
				else
				{
					OBSTACLE_DETECTED[ui_GantryID] = 0;
					DisplayMsg(ui_GantryID,ObstacleClearString);
					print("%s: Obstacle Cleared ! \n",__func__);
					GantryArmMsg(ui_GantryID,GANTRY_ARM_DOWN,NULL);
				}
				break;
		case _EZCP_04://From Antenna
					if(VEHICLE_DETECTED[ui_GantryID] == 1)
					{
						print("%s: MsgID 4 - From Antenna\n",__func__);
						CardType = TYPE_IU;
						//IU Label
						Start_Parse = 4;
						Length_Parse = 13;
						IU_LABEL = ParseBits (buf_local_data_Gantry.Data,Start_Parse,Length_Parse);
						print("%s: ParseBits returned IU: %x\n",__func__,IU_LABEL);


						if (IU_LABEL != 0)
						{
							//Cash Avail
							Start_Parse = 17;
							Length_Parse = 12;
							CASH_AVAIL = ParseBits (buf_local_data_Gantry.Data,Start_Parse,Length_Parse);
							print("%s: ParseBits returned CashAvail: %x\n",__func__,CASH_AVAIL);



							if (GANTRY_ATTRIBUTE[ui_GantryID] == ENTRY )
							{
								print("%s: Write IU Label & Cash Avail to Database\n",__func__);
								ui_InTime = get_in_time(IU_LABEL);
								database_insert(IU_LABEL,CardType,ui_InTime);
								GantryArmMsg(ui_GantryID,GANTRY_ARM_UP,ProceedString);
								setGantryTimer (ui_GantryID, ENTRY_TIMEOUT_VAL);
							}
							else
							{
								print("%s: Read IU Label & Cash Avail from Database\n",__func__);

								ui_InTime = database_retrieve(IU_LABEL, CardType);
								ui_OutTime = get_out_time(IU_LABEL);
								print("%s: Database returned ui_InTime: %d\n",__func__,ui_InTime);

								//uiFee = EZCP_CHU_CalcParkingFee(ui_InTime, ui_OutTime );
								uiFee = EZCP_CHU_CalcParkingFee(IU_LABEL );
								print("%s: Parking Fees is : %d\n",__func__,uiFee);
								if (uiFee > CASH_AVAIL )
								{
									DisplayMsg(ui_GantryID,CashNotEnough);
									GantryArmMsg(ui_GantryID,GANTRY_ARM_DOWN,NULL);
								}
								else
								{
									CashDeductedMsg(ui_GantryID,uiFee);
									GantryArmMsg(ui_GantryID,GANTRY_ARM_UP,ProceedString);
								}
							}
						}

					}
					break;
		case _EZCP_06://From cashcard reader
				if(VEHICLE_DETECTED[ui_GantryID] == 1)
				{
					print("%s: MsgID 6 - From cashcard\n",__func__);
					CardType = TYPE_CASHCARD;
					//Cashcard Label
					Start_Parse = 4;
					Length_Parse = 13;
					CC_LABEL = ParseBits (buf_local_data_Gantry.Data,Start_Parse,Length_Parse);
					print("%s: ParseBits returned CC: %x\n",__func__,CC_LABEL);

					if (CC_LABEL != 0)
					{
						//Cash Avail
						Start_Parse = 17;
						Length_Parse = 12;
						CASH_AVAIL = ParseBits (buf_local_data_Gantry.Data,Start_Parse,Length_Parse);
						print("%s: ParseBits returned CashAvail: %x\n",__func__,Ret_Parse);

						if (GANTRY_ATTRIBUTE[ui_GantryID] == ENTRY )
						{
							print("%s: Write Cashcard Label & Cash Avail to Database\n",__func__);
							ui_InTime = get_in_time(CC_LABEL);
							database_insert(CC_LABEL,CardType,ui_InTime);
							GantryArmMsg(ui_GantryID,GANTRY_ARM_UP,ProceedString);
						}
						else
						{

							print("%s: Read Cashcard Label & Cash Avail from Database\n",__func__);
							ui_InTime = database_retrieve(CC_LABEL, CardType);
							ui_OutTime = get_out_time(CC_LABEL);
							//uiFee = EZCP_CHU_CalcParkingFee(ui_InTime, ui_OutTime );
							uiFee = EZCP_CHU_CalcParkingFee(CC_LABEL );
							if (uiFee > CASH_AVAIL )
							{
								DisplayMsg(ui_GantryID,CashNotEnough);
								GantryArmMsg(ui_GantryID,GANTRY_ARM_DOWN,NULL);
							}
							CashDeductedMsg(ui_GantryID,uiFee);
							GantryArmMsg(ui_GantryID,GANTRY_ARM_UP,ProceedString);
						}
					}
				}
				break;
		case _EZCP_08://From complimentary cash reader
				if(VEHICLE_DETECTED[ui_GantryID] == 1)
				{
					print("%s: MsgID 8 - From complimentary card\n",__func__);
					//Complicard Label
					Start_Parse = 4;
					Length_Parse = 13;
					COMPLI_LABEL = ParseBits (buf_local_data_Gantry.Data,Start_Parse,Length_Parse);
					print("%s: ParseBits returned CompCard: %x\n",__func__,COMPLI_LABEL);

					if (COMPLI_LABEL != 0)
					{
						if (GANTRY_ATTRIBUTE[ui_GantryID] == ENTRY )
						{
							print("%s: Read Complicard from Database\n",__func__);
							ui_InTime = database_retrieve_Compli(COMPLI_LABEL, TYPE_COMPLICARD);
							if (ui_InTime)
							{
								GantryArmMsg(ui_GantryID,GANTRY_ARM_UP,CompliValidString);
							}
							else
							{
								DisplayMsg(ui_GantryID,CompliInvalidString);
								GantryArmMsg(ui_GantryID,GANTRY_ARM_DOWN,NULL );
							}

						}
						else
						{
							ui_InTime = database_retrieve_Compli(COMPLI_LABEL, TYPE_COMPLICARD);
							print("%s: Read Complicard from Database\n",__func__);
							if (ui_InTime)
							{
								GantryArmMsg(ui_GantryID,GANTRY_ARM_UP,CompliValidString);
							}
							else
							{
								DisplayMsg(ui_GantryID,CompliInvalidString);
								GantryArmMsg(ui_GantryID,GANTRY_ARM_DOWN,NULL);
							}
						}
					}
				}
				break;
		case _EZCP_18://From Ticket dispenser, only for Exit
				if(VEHICLE_DETECTED[ui_GantryID] == 1)
				{
					if (GANTRY_ATTRIBUTE[ui_GantryID] == EXIT )
					{
						print("%s: MsgID 18 - From Ticket Dispenser\n",__func__);
						Start_Parse = 4;
						Length_Parse = 1;
						Ret_Parse = ParseBits (buf_local_data_Gantry.Data,Start_Parse,Length_Parse);
						print("%s: ParseBits returned TicketDisp: %x\n",__func__,Ret_Parse);
						if (Ret_Parse)
						{
							if (CardType == TYPE_IU)
							 TicketIssueMsg(ui_GantryID,IU_LABEL,CASH_AVAIL,uiFee, ui_InTime,ui_OutTime);
							else
								TicketIssueMsg(ui_GantryID,CC_LABEL,CASH_AVAIL,uiFee, ui_InTime,ui_OutTime);
						}
						else
							print("%s: No Ticket Issued !\n",__func__);
					}
				}
			break;
		case _EZCP_13:// From CP Attendant
				print("%s: MsgID 13 - From CP Attendant\n",__func__);
				Start_Parse = 4;
				Length_Parse = 1;
				Ret_Parse = ParseBits(buf_local_data_Gantry.Data,Start_Parse,Length_Parse);
				if (Ret_Parse)
				{
					print("%s: Car Park Authorized Exit ! \n",__func__);
					GantryArmMsg(ui_GantryID,GANTRY_ARM_UP,AuthorizeExit);
				}
				else
				{
					print("%s: Car Park Not Authorized to Exit ! \n",__func__);
					DisplayMsg(ui_GantryID,NoAuthorizeExit);
					GantryArmMsg(ui_GantryID,GANTRY_ARM_DOWN,NULL);
				}
				break;
		case _EZCP_14://From Maintenance Support Panel
				print("%s: MsgID 14 - From Maintenance Support Panel\n",__func__);
				Start_Parse = 4;
				Length_Parse = 1;
				Ret_Parse = ParseBits (buf_local_data_Gantry.Data,Start_Parse,Length_Parse);
				print("%s: ParseBits returned AttenSupp: %x\n",__func__,Ret_Parse);
				if (Ret_Parse)
				{
					DisplayMsg(ui_GantryID,MaintenanceString);
					print("%s: Manual Mode Activated!\n",__func__);
					MANUAL_MODE[ui_GantryID] = 1;
				}
				else
				{
					DisplayMsg(ui_GantryID,WelcomeString);
					print("%s: Automatic Mode Activated !\n",__func__);
					MANUAL_MODE[ui_GantryID] = 0;
				}
				break;
		case _EZCP_15://From maintenance Support Panel
				print("%s: MsgID 15 - From Maintenance Support Panel\n",__func__);
				if ( 1 == MANUAL_MODE[ui_GantryID] ) //Maintenance operations only in Manual Mode
				{
					print("%s: MANUAL MODE Operations Panel\n",__func__);
					Start_Parse = 4;
					Length_Parse = 2;
					Ret_Parse = ParseBits (buf_local_data_Gantry.Data,Start_Parse,Length_Parse);
					print("%s: ParseBits returned AttenSupp: %x\n",__func__,Ret_Parse);
					if (Ret_Parse == 0)
					{
						print("%s: Disable Maintenance!\n",__func__);
						MANUAL_MODE[ui_GantryID] = 0;
						DisplayMsg(ui_GantryID,WelcomeString);
					}
					else if(Ret_Parse == 1)
					{
						GantryArmMsg(ui_GantryID,GANTRY_ARM_UP,MaintenanceString_Up);
					}
					else if(Ret_Parse == 2)
					{
						//Send gantry close only if there is no obstacle
						if ( 0 == OBSTACLE_DETECTED[ui_GantryID])
						{
							DisplayMsg(ui_GantryID,MaintenanceString_Down);
							GantryArmMsg(ui_GantryID,GANTRY_ARM_DOWN,NULL);
						}
						else
						{
							print("%s: Gantry cannot be closed.\n",__func__);
							DisplayMsg(ui_GantryID,ObstacleString);
						}
					}
					else if(Ret_Parse == 3)
					{
						DisplayMsg(ui_GantryID,MaintenanceString_BIT);
						print("%s: RUN BIT!\n",__func__);
					}

				}
			break;
		case _EZCP_17://Voice intercom
				if(VEHICLE_DETECTED[ui_GantryID] == 1)
				{
					print("%s: MsgID 17 - From Voice Intercom\n",__func__);
					Start_Parse = 4;
					Length_Parse = 1;
					Ret_Parse = ParseBits(buf_local_data_Gantry.Data,Start_Parse,Length_Parse);
					if (Ret_Parse)
					{
						INTERCOM_BUTTON[ui_GantryID] = 1;
						DisplayMsg(ui_GantryID,CPAttendString);
						print("%s: INTERCOM BUTTON Pressed ! \n",__func__);
					}
					else
			 		{
						INTERCOM_BUTTON[ui_GantryID] = 0;
						print("%s: INTERCOM BUTTON Released ! \n",__func__);
					}
				}
				break;
		default:
				warn("%s: Unknown Message %d Received",__func__,buf_local_data_Gantry.MsgID);
				break;
		}

	}

	return NULL;
}

void setGantryTimer (u32 ui_GantryID, int secs)
{
	print ("Gantry Handler %d: Setting Timeout of %d secs\n", ui_GantryID, secs);

	pthread_mutex_lock(&g1_pt_mutex_Gantry[ui_GantryID]);
	GANTRY_TIMER[ui_GantryID] = secs;
	pthread_mutex_unlock(&g1_pt_mutex_Gantry[ui_GantryID]);

	print ("Gantry Handler %d: Setting Timeout of %d secs.....DONE\n", ui_GantryID, secs);
	return;
}

void resetGantryTimer (u32 ui_GantryID)
{
	print ("Gantry Handler %d: RE-Setting Timeout of %d secs\n", ui_GantryID, GANTRY_TIMER[ui_GantryID]);

	pthread_mutex_lock(&g1_pt_mutex_Gantry[ui_GantryID]);
	GANTRY_TIMER[ui_GantryID] = 0;
	pthread_mutex_unlock(&g1_pt_mutex_Gantry[ui_GantryID]);

	print ("Gantry Handler %d: RE-Setting Timeout of %d secs...DONE\n", ui_GantryID, GANTRY_TIMER[ui_GantryID]);
	return;
}
