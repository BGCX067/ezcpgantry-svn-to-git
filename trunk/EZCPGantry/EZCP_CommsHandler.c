/*
 * EZCP_CommsHandler.c
 *
 *  Created on: Mar 27, 2011
 *      Author: Group 8
 */
#include "EZCP_Common.h"

#define SIMULATION

void *pf_CommsReceiver()
{
	//Initialize setup
	print("%s: CommsReceiver Initiated \n",__func__);

#ifdef SIMULATION

	//Configure channel to receive messages
	EZCP_App_CreateChannel();

	//Wait for messages and delegate appropriately
	EZCP_App_RecvIncomingMsg();

#endif

	return NULL;
}


void *pf_CommsSender()
{

	//Initialize setup
	print("%s:CommsSender Initiated \n",__func__);

#ifdef SIMULATION

	//Connect to Simulator channel to send messages
	EZCP_App_ConnectToChannel();

	InitAllGantries();

	//Currenly this is not needed
	/*
	//Wait for messages and forward when received
	while(1)
	{
		pthread_mutex_lock(&g1_pt_mutex_OutgoingMsg);
		print("%s: Waiting on Condition!\n",__func__);
		if (gl_ui_cvFlag_OutgoingMsg == WAIT )
		{
			pthread_cond_wait(&g1_pt_cv_OutgoingMsg, &g1_pt_mutex_OutgoingMsg);
			print("%s: Condition satisfied!\n",__func__);
		}
		gl_ui_cvFlag_OutgoingMsg = WAIT;

		EZCP_App_SendOutgoingMessage (3);

		pthread_mutex_unlock(&g1_pt_mutex_OutgoingMsg);

	}
	*/
#endif

		return NULL;

}
