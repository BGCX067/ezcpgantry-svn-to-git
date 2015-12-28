/*
 * EZCP_GantryHandler.c
 *
 *  Created on: Mar 27, 2011
 *      Author: Group 8
 */

#define EZCPMAIN 1
#include "EZCP_Common.h"

int main(int argc, char *argv[])
{
	s32 si_RetVal = 0, schdPol;
	u32 ui_index;
	pthread_t	CommsReceiver_thread_id;
	pthread_t	CommsSender_thread_id;
	pthread_t	GantryHandler_thread_id[MAX_NO_GANTRY];
	pthread_t	EmergHandler_thread_id;

	u32 maxPrio, minPrio, ret;
	struct sched_param param;

	// Configure required values using Config Manager
	print("%s: Initialize Config Manager.\n",__func__);
	EZCP_ConfigManagerInit();

	// Configure database manager
	print("%s: Initialize Database Manager.\n",__func__);
	EZCP_DBManagerInit();


	// Initialize charging unit
	print("%s: Initialize Charging unit.\n",__func__);
	EZCP_CHUInit();

	//Initialize all mutexes and condition variables
	for (ui_index =0;ui_index <MAX_NO_GANTRY ;ui_index++ )
	{
		pthread_mutexattr_init ( &g1_pt_mutexattr_Gantry[ui_index] );
		pthread_mutex_init (&g1_pt_mutex_Gantry[ui_index],&g1_pt_mutexattr_Gantry[ui_index]);
		pthread_mutexattr_init ( &g1_pt_mutexattr_Emerg);
		pthread_mutex_init (&g1_pt_mutex_Emerg,&g1_pt_mutexattr_Emerg);
		pthread_mutexattr_init ( &g1_pt_mutexattr_OutgoingMsg);
		pthread_mutex_init (&g1_pt_mutex_OutgoingMsg,&g1_pt_mutexattr_OutgoingMsg);

		pthread_condattr_init (&g1_pt_cvattr_Gantry[ui_index]);
		pthread_cond_init (&g1_pt_cv_Gantry[ui_index],&g1_pt_cvattr_Gantry[ui_index]);
		pthread_condattr_init (&g1_pt_cvattr_Emerg);
		pthread_cond_init (&g1_pt_cv_Emerg,&g1_pt_cvattr_Emerg);
		pthread_condattr_init (&g1_pt_cvattr_OutgoingMsg);
		pthread_cond_init (&g1_pt_cv_OutgoingMsg,&g1_pt_cvattr_OutgoingMsg);
	}

	//Start Commns Handler Threads - CommsReceiver & CommsSender
	print("%s: Creating CommsReceiver_thread now.\n",__func__);
	si_RetVal = pthread_create(&CommsReceiver_thread_id, NULL, &pf_CommsReceiver ,NULL);
	if (si_RetVal != EOK)
	{
		err(-1,"Thread creation returned error %d", si_RetVal );
	}

	print("%s: Creating CommsSender_thread now.\n",__func__);
	si_RetVal = pthread_create(&CommsSender_thread_id, NULL, &pf_CommsSender ,NULL);
	if (si_RetVal != EOK)
	{
		err(-1,"Thread creation returned error %d", si_RetVal );
	}

	//Start Gantry Handler threads
	print("%s: Creating GantryHandler_thread(s) now.\n",__func__);
	for (ui_index =0;ui_index <MAX_NO_GANTRY ;ui_index++ )
	{
		si_RetVal = pthread_create(&GantryHandler_thread_id[ui_index], NULL, &pf_GantryHandler ,(void *)ui_index);
		if (si_RetVal != EOK)
		{
			err(-1,"Thread creation returned error %d", si_RetVal );
		}
	}

	// Start Emergency Handler Thread
	print("%s: Creating EmergHandler_thread now.\n",__func__);
	si_RetVal = pthread_create(&EmergHandler_thread_id, NULL, &pf_EmergencyHandler ,NULL);
	if (si_RetVal != EOK)
	{
		err(-1,"Thread creation returned error %d", si_RetVal );
	}

	schdPol = sched_getscheduler(0);	//Policy for main thread
	maxPrio = sched_get_priority_max (schdPol);
	minPrio = sched_get_priority_min (schdPol);
	ret     = pthread_getschedparam (pthread_self(), &schdPol, &param );
	print ("Main: schdPol %d [minPri=%d] [maxPrio=%d] [asgnPrio=%d] [curPrio=%d]\n", schdPol, minPrio, maxPrio, param.sched_priority, param.sched_curpriority);

	//Wait for all threads to exit.
	pthread_join (CommsReceiver_thread_id, NULL);
	//pthread_join (GantryHandler_thread_id[5], NULL);
	//pthread_join (EmergHandler_thread_id, NULL);
	database_delete();
	print ("%s: Exit from EZCP Application Program \n",__func__);

	return EXIT_SUCCESS;
}
