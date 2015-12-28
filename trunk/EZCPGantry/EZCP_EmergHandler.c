/*
 * EZCP_EmergHandler.c
 *
 *  Created on: Mar 27, 2011
 *      Author: Group 8
 */
#include "EZCP_Common.h"

extern char EmergString[DISPLAY_MSG_LEN];
extern char WelcomeString[DISPLAY_MSG_LEN];
extern unsigned int EMERGENCY_MODE;
void *pf_EmergencyHandler()
{
	MsgBuffer buf_local_data_Gantry;
	int Start_Parse = 0, Length_Parse = 0,Ret_Parse = 0;
	u32 uiIndex;

	int schdPol;
	u32 maxPrio, minPrio, ret;
	struct sched_param param;

	schdPol = sched_getscheduler(0);	//Policy for current thread
	maxPrio = sched_get_priority_max (schdPol);
	minPrio = sched_get_priority_min (schdPol);
	ret     = pthread_getschedparam (pthread_self(), &schdPol, &param );
	print ("Emergency Handler: schdPol %d [minPri=%d] [maxPrio=%d] [asgnPrio=%d] [curPrio=%d]\n", schdPol, minPrio, maxPrio, param.sched_priority, param.sched_curpriority);
	ret     = pthread_setschedprio (pthread_self(), maxPrio);	//pthread_setschedparam ();
	ret     = pthread_getschedparam (pthread_self(), &schdPol, &param );
	print ("Emergency Handler: schdPol %d [minPri=%d] [maxPrio=%d] [asgnPrio=%d] [curPrio=%d]\n", schdPol, minPrio, maxPrio, param.sched_priority, param.sched_curpriority);

	//Initialize setup
	//print("%s: Emergency Handler Initiated\n",__func__);
	//Wait for Messages
	while(1)
	{
		pthread_mutex_lock(&g1_pt_mutex_Emerg);
		print("%s: Wait on Condition!\n",__func__);
		if (gl_ui_cvFlag_Emerg == WAIT )
		{
			pthread_cond_wait(&g1_pt_cv_Emerg, &g1_pt_mutex_Emerg);
		}

		gl_ui_cvFlag_Emerg = WAIT;
		buf_local_data_Gantry.Length = gl_MsgBuf_Incoming_Emerg.Length;
		buf_local_data_Gantry.MsgID = gl_MsgBuf_Incoming_Emerg.MsgID;
		memcpy(buf_local_data_Gantry.Data, gl_MsgBuf_Incoming_Emerg.Data,buf_local_data_Gantry.Length);

		pthread_mutex_unlock(&g1_pt_mutex_Emerg);

		Start_Parse = 4;
		Length_Parse = 2;
		Ret_Parse = ParseBits (buf_local_data_Gantry.Data,Start_Parse,Length_Parse);
		print("%s: ParseBits returned Value: %x\n",__func__,Ret_Parse);

		if ( (Ret_Parse == 2) || (Ret_Parse == 3) )
		{
			//Alarm has been triggerd
			print(" %s: ******** FIRE ALARM >> RUN RUN RUN ***** \n",__func__);
			for (uiIndex = 0; uiIndex < MAX_NO_GANTRY ;uiIndex++)
			{
				EMERGENCY_MODE = 1;
				GantryArmMsg(uiIndex,GANTRY_ARM_UP,EmergString);
				database_delete();
			}
		}

		else if  (Ret_Parse == 1)
		{
			print(" %s: Power Supply Cut-off. Switch to Back-up line.\n",__func__);
		}

		else
		{
			print(" %s: Power Supply Resume. Switch to main line.\n",__func__);
			EMERGENCY_MODE = 0;
			for (uiIndex = 0; uiIndex < MAX_NO_GANTRY ;uiIndex++)
			{
				DisplayMsg (uiIndex,WelcomeString);
				GantryArmMsg(uiIndex,GANTRY_ARM_DOWN,NULL);
			}
		}
	}

	return NULL;
}
