#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include <hal/Timing/Time.h>
#include <hal/errors.h>

#include <satellite-subsystems/IsisTRXVU.h>
#include <satellite-subsystems/IsisAntS.h>

#include <stdlib.h>
#include <string.h>

#include "GlobalStandards.h"
#include "TRXVU.h"
#include "AckHandler.h"
#include "ActUponCommand.h"
#include "SatCommandHandler.h"
#include "TLM_management.h"

#include "SubSystemModules/PowerManagment/EPS.h"
#include "SubSystemModules/Maintenance/Maintenance.h"
#include "SubSystemModules/Housekepping/TelemetryCollector.h"
#include "SubSystemModules/Communication/SatCommandHandler.h"
#include "SubSystemModules/Communication/SatDataTx.h"
#include "SubSystemModules/Communication/Beacon.h"
#include "SubSystemModules/Housekepping/Dump.h"



int InitTrxvu() {

	ISIStrxvuI2CAddress myTRXVUAddress;
	ISIStrxvuFrameLengths myTRXVUBuffers;

	int retValInt = 0;

	//Buffer definition
	myTRXVUBuffers.maxAX25frameLengthTX = SIZE_TXFRAME;//SIZE_TXFRAME;
	myTRXVUBuffers.maxAX25frameLengthRX = SIZE_RXFRAME;

	//I2C addresses defined
	myTRXVUAddress.addressVu_rc = I2C_TRXVU_RC_ADDR;
	myTRXVUAddress.addressVu_tc = I2C_TRXVU_TC_ADDR;


	//Bitrate definition
	ISIStrxvuBitrate myTRXVUBitrates;
	myTRXVUBitrates = trxvu_bitrate_9600;
	retValInt = IsisTrxvu_initialize(&myTRXVUAddress, &myTRXVUBuffers,
			&myTRXVUBitrates, 1);
	if (retValInt != 0) {
		return retValInt;
	}
	vTaskDelay(100);

	IsisTrxvu_tcSetAx25Bitrate(ISIS_TRXVU_I2C_BUS_INDEX,myTRXVUBitrates);
	vTaskDelay(100);

	ISISantsI2Caddress myAntennaAddress;
	myAntennaAddress.addressSideA = ANTS_I2C_SIDE_A_ADDR;
	myAntennaAddress.addressSideB = ANTS_I2C_SIDE_B_ADDR;

	//Initialize the AntS system
	retValInt = IsisAntS_initialize(&myAntennaAddress, 1);
	if (retValInt != 0) {
		return retValInt;
	}

	InitTxModule();
	InitBeaconParams();

	return 0;
}

CommandHandlerErr TRX_Logic() {
	int err = 0;
	int frame_count = GetNumberOfFramesInBuffer();
	sat_packet_t cmd = { 0 };

	if (frame_count > 0) {
		err = GetOnlineCommand(&cmd);
		ResetGroundCommWDT();
		SendAckPacket(ACK_RECEIVE_COMM, &cmd, NULL, 0);

	} else if (GetDelayedCommandBufferCount() > 0) {
		err = GetDelayedCommand(&cmd);
	}
	if (cmd_command_found == err) {
		err = ActUponCommand(&cmd);
		//TODO: log error
		//TODO: send message to ground when a delayed command was not executed-> add to log
	}
	BeaconLogic();

	if (cmd_command_found != err)
		return err;

	return cmd_command_succsess;
}


