/*
 * EZCP_Comms.h
 *
 *  Created on: Apr 4, 2011
 *      Author: Group 8
 *      Should be in sync with EZCP_Sim_comms.h [from EZCP_Simulator project]
 */

#ifndef EZCP_COMMS_H_
#define EZCP_COMMS_H_

#define EZCP_ATTACH_APP2SIMU		"EZParkService_A2S"
#define EZCP_ATTACH_SIMU2APP 		"EZParkService_S2A"

#define EZCP_NUM_OF_MSGS		18
#define EZCP_HDR_LEN			sizeof(msg_header_t) 	//16	Bytes
#define EZCP_MAX_DATA_LEN		121//4

#define DISPLAY_MSG_LEN			120

typedef struct _pulse msg_header_t;
typedef unsigned char msg_data_buf_t	[EZCP_MAX_DATA_LEN];
typedef unsigned char msg_buf_t			[EZCP_HDR_LEN + EZCP_MAX_DATA_LEN];

typedef struct stMsgBuffer {
	int		MsgID;
	int		Length;
	msg_data_buf_t Data;

}MsgBuffer;

// stored in u8. Used for Message-IDs (of EZCP_NUM_OF_MSGS) exchanged between
// gantry software and simulator. MUST follow rule:
// {each enum val} < _IO_BASE and {each enum val} > _IO_MAX
enum _ezcp_MsgType
{
_EZCP_01 =  1,
_EZCP_02,
_EZCP_03,
_EZCP_04,
_EZCP_05,
_EZCP_06,
_EZCP_07,
_EZCP_08,
_EZCP_09,
_EZCP_10,
_EZCP_11,
_EZCP_12,
_EZCP_13,
_EZCP_14,
_EZCP_15,
_EZCP_16,
_EZCP_17,
_EZCP_18
}; // Max enum value should not be greater than _IO_BASE i.e. 0x100


#endif /* EZCP_COMMS_H_ */
