/*
 * EZCP_Config.c
 *
 *  Created on: Apr 20, 2011
 *      Author: Group 8
 */

#include "EZCP_Common.h"

extern char WelcomeString[DISPLAY_MSG_LEN];
unsigned int GANTRY_ATTRIBUTE [MAX_NO_GANTRY];
unsigned int VEHICLE_DETECTED[MAX_NO_GANTRY];
unsigned int OBSTACLE_DETECTED[MAX_NO_GANTRY];
unsigned int INTERCOM_BUTTON[MAX_NO_GANTRY];
unsigned int VALID_LABEL[MAX_NO_GANTRY];
unsigned int MANUAL_MODE[MAX_NO_GANTRY];
unsigned int EMERGENCY_MODE;

unsigned int COST_PER_UNIT;
unsigned int UNIT_DURATION;

static qdb_hdl_t *hdl;

void EZCP_ConfigManagerInit()
{
	u32 ui_Index;
	//Configure Entry & Exit Gantries
	GANTRY_ATTRIBUTE[0] = ENTRY;
	GANTRY_ATTRIBUTE[1] = ENTRY;
	GANTRY_ATTRIBUTE[2] = EXIT;
	GANTRY_ATTRIBUTE[3] = EXIT;
	GANTRY_ATTRIBUTE[4] = ENTRY;
	GANTRY_ATTRIBUTE[5] = ENTRY;
	GANTRY_ATTRIBUTE[6] = EXIT;
	GANTRY_ATTRIBUTE[7] = EXIT;

	EMERGENCY_MODE = 0;

	for (ui_Index = 0; ui_Index < MAX_NO_GANTRY ; ui_Index++ )
	{
		VEHICLE_DETECTED[ui_Index] = 0;
		OBSTACLE_DETECTED[ui_Index] = 0;
		INTERCOM_BUTTON[ui_Index] = 0;
		VALID_LABEL[ui_Index] = 0;
		MANUAL_MODE[ui_Index] = 0;
	}

}

void EZCP_DBManagerInit()
{

	// Connect to the database
   hdl = qdb_connect("/dev/qdb/vehicledb", 0);
   if (hdl == NULL)
   {
	  err(-1,"Error connecting to database\n");
   }
   else
   {
	   print("%s: Datbase Connected Successfully\n",__func__);
   }

   database_delete();

   database_insert(0x100,TYPE_COMPLICARD,1);
   database_insert(0x200,TYPE_COMPLICARD,1);
   database_insert(0x300,TYPE_COMPLICARD,1);
   database_insert(0x400,TYPE_COMPLICARD,1);
   database_insert(0x500,TYPE_COMPLICARD,1);
   database_insert(0x600,TYPE_COMPLICARD,1);
   database_insert(0x700,TYPE_COMPLICARD,1);
   database_insert(0x800,TYPE_COMPLICARD,1);
}

//TODO : Check functionality of DB & way it returns error

void database_insert(int card_num, int card_type,int in_time)
{
	int rc;

   char buffer[100];


   sprintf(buffer,"INSERT INTO vehicle VALUES(%d,%d,%d);" ,card_num,card_type,in_time);
   rc = qdb_statement(hdl,buffer);
   if (rc == -1)
	 {
		 err(-1,"Error executing INSERT statement\n");
	 }
   else
   {
	   print("%s: database_insert Successful \n",__func__);
   }
}


int database_retrieve(int card_num, int card_type)
 {
		int rc,DBRet;

		char ret_buffer[100];
		qdb_result_t *res;
		sprintf(ret_buffer,"SELECT in_time FROM vehicle where card_num=%d AND card_type=%d;" ,card_num,card_type);
		rc = qdb_statement(hdl,ret_buffer);

		if (rc == -1)
		{
				  err(-1, "Error executing SELECT statement\n");
		}
		res = qdb_getresult(hdl); // Get the result
		if (res == NULL)
		{
		   fprintf(stderr, "Error getting result: %s\n", strerror(errno));
		   return EXIT_FAILURE;
		}
		if (qdb_rows(res) >= 1)
		{
		   print("The in_time is %d \n", *(int *)qdb_cell(res,0,0));

		}
		else
		{
		   print("No customers in the database!\n");
		}

		DBRet =  *((int *)qdb_cell(res,0,0));

		// Free the result
		qdb_freeresult(res);

		database_clear_entry(card_num);

		return DBRet;

 }

void database_clear_entry(int card_num)
{
	int rc;
	rc = qdb_statement(hdl,"DELETE FROM vehicle WHERE card_num=%d",card_num);
	if (rc == -1)
	{
	   err(-1, " Error executing DELETE statement\n");
	}

}

int database_retrieve_Compli(int card_num, int card_type)
 {
		int rc,DBRet;
		qdb_result_t *res;
		char ret_buffer[100];

		sprintf(ret_buffer,"SELECT in_time FROM vehicle where card_num=%d AND card_type=%d;" ,card_num,card_type);
		rc = qdb_statement(hdl,ret_buffer);
		if (rc == -1)
		{
				  err(-1, "Error executing SELECT statement\n");
		}
		res = qdb_getresult(hdl); // Get the result
		if (res == NULL)
		{
		   err(-1, "Error getting result\n");
		}


		if (qdb_rows(res) >= 1)
		{
		   print("The in_time is %d \n", *(int *)qdb_cell(res,0,0));
		   DBRet =  *((int *)qdb_cell(res,0,0));
		}
		else
		{
		   print("No customers in the database!\n");
		   DBRet = 0;
		}


		// Free the result
		qdb_freeresult(res);
		return DBRet;

 }

void database_delete()
{
	int rc;

	rc = qdb_statement(hdl,"DELETE FROM vehicle;");
	if (rc == -1)
	{
	   err(-1, "Error executing SELECT statement\n");
	}
}


void EZCP_CHUInit()
{
	//50 cents for every 15 min
	UNIT_DURATION = 15;
	COST_PER_UNIT = 5;
}

int get_in_time(int IU_LABEL)
{
	int RetTime = 0;
	switch(IU_LABEL)
	{
		case 0x155f:
			RetTime = 0x1000;
			break;
		case 0x177f:
			RetTime = 0x1103;
			break;
		case 0x199f:
			RetTime = 0x1149;
			break;
		case 0x1bbf:
			RetTime = 0x1300;
			break;
		case 0x1ddf:
			RetTime = 0x1421;
			break;
		case 0x1fff:
			RetTime = 0x1532;
			break;
		case 0x111f:
			RetTime = 0x1601;
			break;
		case 0x133f:
			RetTime = 0x1700;
			break;
	}
	return RetTime;
}

int get_out_time(int IU_LABEL)
{
	int RetTime = 0;
	switch(IU_LABEL)
	{
	case 0x155f:
		RetTime = 0x1105;
		break;
	case 0x177f:
		RetTime = 0x1200;
		break;
	case 0x199f:
		RetTime = 0x1220;
		break;
	case 0x1bbf:
		RetTime = 0x1305;
		break;
	case 0x1ddf:
		RetTime = 0x1532;
		break;
	case 0x1fff:
		RetTime = 0x1640;
		break;
	case 0x111f:
		RetTime = 0x1645;
		break;
	case 0x133f:
		RetTime = 0x2005;
		break;
	}
	return RetTime;

}

u32 EZCP_CHU_CalcParkingFee(int LABEL)
{
	int RetTime = 0;
	switch(LABEL)
	{
	case 0x155f:
		RetTime = 0x5	;
		break;
	case 0x177f:
		RetTime = 0x12;
		break;
	case 0x199f:
		RetTime = 0x25;
		break;
	case 0x1bbf:
		RetTime = 0x32;
		break;
	case 0x1ddf:
		RetTime = 0x30;
		break;
	case 0x1fff:
		RetTime = 0x53;
		break;
	case 0x111f:
		RetTime = 0x59;
		break;
	case 0x133f:
		RetTime = 0x111;
		break;
	}
return RetTime;

}


void InitAllGantries()
{
	int GantryIndex;

	for(GantryIndex = 0; GantryIndex < MAX_NO_GANTRY ; GantryIndex++)
	{
		GantryArmMsg( GantryIndex, GANTRY_ARM_DOWN,NULL);
		DisplayMsg (GantryIndex, WelcomeString);
	}
}
