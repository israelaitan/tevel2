#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include <hal/Timing/WatchDogTimer.h>
#include <hal/boolean.h>
#include <hal/Utility/util.h>
#include <hal/Drivers/I2C.h>
#include <hal/Drivers/SPI.h>
#include <hal/Timing/Time.h>

#include <at91/utility/trace.h>
#include <at91/peripherals/cp15/cp15.h>
#include <at91/utility/exithandler.h>
#include <at91/commons.h>

#include <hcc/api_fat.h>

#include "GlobalStandards.h"
#include "SubSystemModules/PowerManagment/EPS.h"
#include "SubSystemModules/Communication/TRXVU.h"
#include "SubSystemModules/Housekepping/TelemetryCollector.h"
#include "SubSystemModules/Maintenance/Maintenance.h"
#include "InitSystem.h"
#include "main.h"

#ifdef TESTING
#include <satellite-subsystems/GomEPS.h>
#endif

void taskMain()
{
	WDT_startWatchdogKickTask(10 / portTICK_RATE_MS, FALSE);

	InitSubsystems();

	while (TRUE) {

		EPS_Conditioning();

		TRX_Logic();

		TelemetryCollectorLogic();

		Maintenance();

		// Payload_Logic();
	}
}


void TaskTest()
{
	unsigned int i = 0;
	WDT_startWatchdogKickTask(10 / portTICK_RATE_MS, FALSE);
	InitSubsystems();
	/**
	 * 	Initialize the GOMSpace EPS with the corresponding i2cAddress. This function can only be called once.
	 *
	 * 	@param[in] i2c_address array of GOMSpace EPS I2C bus address
	 * 	@param[in] number number of attached EPS in the system to be initialized
	 * 	@return Error code according to <hal/errors.h>
	 */
	unsigned char eps_addr = EPS_I2C_ADDR;
	int err = GomEpsInitialize(&eps_addr, 1);
	gom_eps_hk_t eps_tlm;
	while(TRUE)
	{
		GomEpsGetHkData_general(0,&eps_tlm);
		printf("gomspace tele vbat: %d\n", eps_tlm.fields.vbatt);
		printf("still alive %d\n",i++);
		vTaskDelay(1000);
	}
}
// main operation function. will be called upon software boot.
int main()
{
	xTaskHandle taskMainHandle;

	TRACE_CONFIGURE_ISP(DBGU_STANDARD, 2000000, BOARD_MCK);
	// Enable the Instruction cache of the ARM9 core. Keep the MMU and Data Cache disabled.
	CP15_Enable_I_Cache();

	// The actual watchdog is already started, this only initializes the watchdog-kick interface.
	WDT_start();

	// create the main operation task of the satellite
#ifdef TESTING
	//TaskTest
	xTaskGenericCreate(TaskTest, (const signed char*) "taskTest", 4096, NULL,
				configMAX_PRIORITIES - 2, &taskMainHandle, NULL, NULL);
#else
	xTaskGenericCreate(taskMain, (const signed char*) "taskMain", 4096, NULL,
			configMAX_PRIORITIES - 2, &taskMainHandle, NULL, NULL);
#endif
	vTaskStartScheduler();
}
