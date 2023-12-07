/*
 * IMU.c
 *
 * Created: 05.12.2023
 * Author : annoyedmilk
 */ 

//#include <avr/io.h>
#include "avr_compiler.h"
#include "pmic_driver.h"
#include "TC_driver.h"
#include "clksys_driver.h"
#include "sleepConfig.h"
#include "port_driver.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "event_groups.h"
#include "stack_macros.h"

#include "mem_check.h"

#include "init.h"
#include "utils.h"
#include "errorHandler.h"
#include "NHD0420Driver.h"

#include "LSM9DS1Driver.h"
#include "LSM9DS1Defines.h"
#include "twiMaster.h"
#include "math.h"

extern void vApplicationIdleHook(void);
void vSensorDataDisplay(void *pvParameters);

TaskHandle_t sensorDataTask;

void vApplicationIdleHook(void)
{
	// Idle hook function body can be added here if needed
}

int main(void)
{
	vInitClock(); // Initialize the clock system
	vInitDisplay(); // Initialize the display
	LSM9DS1Init(); // Initialize LSM9DS1 sensor

	// Create the sensor data display task
	xTaskCreate(vSensorDataDisplay,
	(const char *) "SensorDataDisplay",
	configMINIMAL_STACK_SIZE + 10,
	NULL,
	1,
	&sensorDataTask);

	// Display initial messages on the screen
	vDisplayClear();

	// Start the FreeRTOS scheduler
	vTaskStartScheduler();

	// Main function ends
	return 0;
}

void vSensorDataDisplay(void *pvParameters)
{
	(void) pvParameters;
	int16_t ax, ay, az; // Variables for acceleration data
	int16_t mx, my, mz; // Variables for magnetometer data
	float temperature;  // Variable for temperature data

	// Variables to store previous values
	int16_t prev_ax = 0, prev_ay = 0, prev_az = 0;
	int16_t prev_mx = 0, prev_my = 0, prev_mz = 0;
	float prev_temperature = 0;
	bool displayUpdateRequired = false;

	for (;;)
	{
		// Read temperature data
		readTempData();
		temperature = getTemperatureData(); // Fetch temperature from LSM9DS1 sensor

		// Update temperature display only if the change is more than 0.1 degree
		if (fabs(temperature - prev_temperature) > 0.1) {
			prev_temperature = temperature;
			displayUpdateRequired = true;
		}

		// Read acceleration data
		readACCData();
		ax = getACCData(X_AXIS); // Fetch X-axis acceleration
		ay = getACCData(Y_AXIS); // Fetch Y-axis acceleration
		az = getACCData(Z_AXIS); // Fetch Z-axis acceleration

		// Read magnetometer data
		readMagData();
		mx = getMagData(X_AXIS); // Fetch X-axis magnetometer data
		my = getMagData(Y_AXIS); // Fetch Y-axis magnetometer data
		mz = getMagData(Z_AXIS); // Fetch Z-axis magnetometer data

		// Update acceleration/magnetometer display only if the change is more than 5
		if (abs(ax - prev_ax) > 5 || abs(ay - prev_ay) > 5 || abs(az - prev_az) > 5 ||
		abs(mx - prev_mx) > 5 || abs(my - prev_my) > 5 || abs(mz - prev_mz) > 5) {

			// Update the previous values
			prev_ax = ax; prev_ay = ay; prev_az = az;
			prev_mx = mx; prev_my = my; prev_mz = mz;
			displayUpdateRequired = true;
		}

		if (displayUpdateRequired) {
			vDisplayClear();
			vDisplayWriteStringAtPos(0,0,"Temp: %f C", prev_temperature);
			vDisplayWriteStringAtPos(1,0,"Acc/Mag X: %d / %d", prev_ax, prev_mx);
			vDisplayWriteStringAtPos(2,0,"Acc/Mag Y: %d / %d", prev_ay, prev_my);
			vDisplayWriteStringAtPos(3,0,"Acc/Mag Z: %d / %d", prev_az, prev_mz);
			displayUpdateRequired = false;
		}

		// Wait for 1 second before updating again
		vTaskDelay(1000 / portTICK_RATE_MS);
	}
}