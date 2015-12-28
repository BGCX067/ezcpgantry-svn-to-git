/*
 * EZCP_Simulation.c
 *
 *  Created on: Mar 27, 2011
 *      Author: Group 8
 */
#include "EZCP_Common.h"

#define CODE_TIMER 1
void setupPulseAndTimer (void);
void processTimer();

name_attach_t *pst_nameattach_attach=NULL;
extern unsigned int EMERGENCY_MODE;

unsigned int MSG_DATA_LENGTHS [EZCP_NUM_OF_MSGS] =
{1, 1, 1, 4, 2, 4, 2, 2, 1, 121, 9, 3, 1, 1, 1, 1, 1,9};
extern unsigned int GANTRY_ATTRIBUTE [MAX_NO_GANTRY];
extern unsigned int VEHICLE_DETECTED[MAX_NO_GANTRY];
extern unsigned int OBSTACLE_DETECTED[MAX_NO_GANTRY];
unsigned int GANTRY_TIMER[MAX_NO_GANTRY];
//unsigned int GANTRY_TIMEOUT[MAX_NO_GANTRY];
static s32 si_server_coid;

//Create channel to receive from Simulator
void EZCP_App_CreateChannel()
{
	print ("%s: Attach Channel with Simulator\n",__func__);

	if (NULL == (pst_nameattach_attach = name_attach(NULL, EZCP_ATTACH_SIMU2APP, 0)))
	{
		err(-1,"name_attach error");
	}
}

//Receive messages from Simulator
void EZCP_App_RecvIncomingMsg()
{
	s32 si_RcvId, si_Ret;
	msg_header_t st_my_hdr;
	msg_data_buf_t buf_data;
	u32 ui_GantryID;

	if (NULL == pst_nameattach_attach)
	{
		err (-1,"EZCP_App_Receive: Channel not created for receiving simulation...");
	}

	setupPulseAndTimer();

	while (1)
	{
		print ("%s: Waiting to receive Simulation data ...\n",__func__);

		si_RcvId = MsgReceive(pst_nameattach_attach->chid, &st_my_hdr, EZCP_HDR_LEN, NULL);
		if (-1 == si_RcvId )
		{/* Error condition, exit */
			err (-1,"EZCP_App_Receive: MsgReceive error\n");
		}

		if (0 == si_RcvId)
		{// pulse received
			if (_PULSE_CODE_DISCONNECT == st_my_hdr.code)
				break;
			if (CODE_TIMER == st_my_hdr.code)
				processTimer();
			continue;
		}

		/* name_open() from Simulator sends a connect message, must EOK this */
		if (_IO_CONNECT  == st_my_hdr.type)
		{
			//Acknowledge receipt of message
			MsgReply( si_RcvId, EOK, NULL, 0 );
			continue;
		}

		/* Some other QNX IO message was received; reject it */
		if (st_my_hdr.type > _IO_BASE && st_my_hdr.type <= _IO_MAX )
		{
			MsgError( si_RcvId, ENOSYS );
			continue;
		}

		/* A message (presumably ours) received, handle */
		print ("%s: Msg ID %d received from Simulator\n",__func__,st_my_hdr.type);
		si_Ret = MsgRead(si_RcvId, buf_data, st_my_hdr.subtype, sizeof(st_my_hdr));
		if (-1 == si_Ret)
		{/* Error condition, exit */
			err (-1,"MsgRead error\n");
		}

		//Delegate message to Gantry or Emergency
		if ( st_my_hdr.type == _EZCP_16)	//Emergency
		{
			print("%s: Message for Emergency Thread \n",__func__);
			pthread_mutex_lock(&g1_pt_mutex_Emerg);
			gl_MsgBuf_Incoming_Emerg.MsgID = st_my_hdr.type ;
			gl_MsgBuf_Incoming_Emerg.Length = st_my_hdr.subtype ;
			memcpy (gl_MsgBuf_Incoming_Emerg.Data,buf_data,sizeof(msg_data_buf_t));
			gl_ui_cvFlag_Emerg = NOWAIT;
			pthread_cond_signal(&g1_pt_cv_Emerg);
			pthread_mutex_unlock(&g1_pt_mutex_Emerg);
		}
		else
		{
			if ( 0 == EMERGENCY_MODE)
			{
				//ui_GantryID = (buf_data[0] & 0xE0)>>5;
				ui_GantryID = (buf_data[0] & 0x07);
				print("%s: Message for Gantry ID: %d \n",__func__,ui_GantryID);
				//Proces only if there is no other vehicle currently being processed

				pthread_mutex_lock(&g1_pt_mutex_Gantry[ui_GantryID]);
				gl_MsgBuf_Incoming_Gantry[ui_GantryID].MsgID = st_my_hdr.type ;
				gl_MsgBuf_Incoming_Gantry[ui_GantryID].Length = st_my_hdr.subtype ;
				memcpy (gl_MsgBuf_Incoming_Gantry[ui_GantryID].Data,buf_data,sizeof(msg_data_buf_t));
				gl_ui_cvFlag_Gantry[ui_GantryID] |= NOWAIT;
				pthread_cond_signal(&g1_pt_cv_Gantry[ui_GantryID]);
				pthread_mutex_unlock(&g1_pt_mutex_Gantry[ui_GantryID]);
			}
			else
			{
				print("%s:EMERGENCY MODE. MESSAGE WILL NOT BE PROCESSED \n",__func__);
			}

		}//end of while(1)

		MsgReply(si_RcvId, EOK, 0, 0);
		//break;
	}

	/* Remove the name from the space */
	name_detach(pst_nameattach_attach, 0);

}

//Connect to channel created by Simulator
void EZCP_App_ConnectToChannel()
{
	//Keep trying till Simulator creates channel :-)
	while (-1 == (si_server_coid = name_open(EZCP_ATTACH_APP2SIMU, 0)));
}

//Forward outgoing messages to Simulator
void EZCP_App_SendOutgoingMessage (u32 GantryID)
{
	msg_header_t st_hdr;
	msg_buf_t buf_msg;
	u32 ui_hdr_len, ui_data_len, ui_msg_len;

	ui_hdr_len  = EZCP_HDR_LEN;
    ui_data_len = MSG_DATA_LENGTHS [gl_MsgBuf_Outgoing_Msg[GantryID].MsgID - 1];
    ui_msg_len  = ui_hdr_len + ui_data_len;

    st_hdr.type    = gl_MsgBuf_Outgoing_Msg[GantryID].MsgID;
    st_hdr.subtype = ui_data_len;
    memcpy (buf_msg, &st_hdr, ui_hdr_len);
    memcpy (buf_msg+ui_hdr_len, gl_MsgBuf_Outgoing_Msg[GantryID].Data, ui_data_len);

    //print ("%s:MsgID[%d] Data sent as Binary = %s \n",__func__,gl_MsgBuf_Outgoing_Msg.MsgID,gl_MsgBuf_Outgoing_Msg.Data);

    //First ensure connection has been established with Simulator
    if ( 0 == si_server_coid  )
    {
    	//EZCP_App_ConnectToChannel();
    	err (-1,"Simulator Channel not available for sending message\n");
    }

	print ("%s: Sending message to Simulator \n",__func__);
    if (-1 == MsgSend(si_server_coid, buf_msg, ui_msg_len, NULL, 0))
    {
    	err (-1,"%s: MsgSend Error", __func__);
    }
    else
    {
    	print ("%s: Message sending successful\n",__func__);
    }

}

void setupPulseAndTimer (void)
{
    timer_t             timerid;    // timer ID for timer
    struct sigevent     event;      // event to deliver
    struct itimerspec   timer;      // the timer data structure
    int                 coid;       // connection back to ourselves
    int 				i;

    //print ("In setupPulseAndTimer()\n");

    //Initialize TIMER values for all gantry
    for(i=0; i<MAX_NO_GANTRY; i++)
    {
    	GANTRY_TIMER[i] = 0;
    }

    // create a connection back to ourselves
    coid = ConnectAttach (0, 0, pst_nameattach_attach->chid, 0, 0);
    if (coid == -1) {
        print ("%s:  couldn't ConnectAttach to self!\n", __func__);
        perror (NULL);
        exit (EXIT_FAILURE);
    }

    // set up the kind of event that we want to deliver -- a pulse
    SIGEV_PULSE_INIT (&event, coid, SIGEV_PULSE_PRIO_INHERIT, CODE_TIMER, 0);

    // create the timer, binding it to the event
    if (timer_create (CLOCK_REALTIME, &event, &timerid) == -1) {
        print ("%s:  couldn't create a timer, errno %d\n", __func__, errno);
        perror (NULL);
        exit (EXIT_FAILURE);
    }

    // setup the timer (1s delay, 1s reload)
    timer.it_value.tv_sec = 1;
    timer.it_value.tv_nsec = 0;
    timer.it_interval.tv_sec = 1;
    timer.it_interval.tv_nsec = 0;

    // and start it!
    timer_settime (timerid, 0, &timer, NULL);
    return;
}

void processTimer()
{
	u32 tmrGantryID;

	//print ("In ProcessTimer()\n");

    for(tmrGantryID = 0; tmrGantryID < MAX_NO_GANTRY; tmrGantryID++)
    {
		pthread_mutex_lock(&g1_pt_mutex_Gantry[tmrGantryID]);

		if (1 == GANTRY_TIMER[tmrGantryID]) 		//Timer has expired @ this pulse
    	{
    		GANTRY_TIMER[tmrGantryID] = 0;
			//GANTRY_TIMEOUT[tmrGantryID] = 1;
			gl_ui_cvFlag_Gantry[tmrGantryID] |= TIMEOUT;
			pthread_cond_signal(&g1_pt_cv_Gantry[tmrGantryID]);
    	}
    	else if (GANTRY_TIMER[tmrGantryID] > 1)	//Timer has been activated by gantry-thread
    		GANTRY_TIMER[tmrGantryID]--;

		pthread_mutex_unlock(&g1_pt_mutex_Gantry[tmrGantryID]);
    }

	return;
}
