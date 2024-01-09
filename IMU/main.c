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
#include "string.h"
#include "stdio.h"

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
	configMINIMAL_STACK_SIZE + 200,
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

// Function to display sensor data
void vSensorDataDisplay(void *pvParameters)
{
	(void)pvParameters; // Discard the unused parameter

	// Declaration of variables for raw sensor data
	int16_t raw_ax, raw_ay, raw_az; // Raw acceleration data
	int16_t raw_gx, raw_gy, raw_gz; // Raw gyro data
	int16_t raw_mx, raw_my, raw_mz; // Raw magnetometer data
	float temperature;              // Temperature data

	// Variables for processed sensor data
	float ax, ay, az; // Acceleration in g
	float gx, gy, gz; // Gyro in degrees/sec
	float mx, my, mz; // Magnetometer in µT
	
	// Display buffer strings
	char displayRow0[21];
	char displayRow1[21];
	char displayRow2[21];
	char displayRow3[21];

	// Infinite loop for continuous data display
	for (;;)
	{
		// Reading temperature data
		readTempData();
		temperature = getTemperatureData(); // Fetch and store temperature

		// Reading gyro data
		readGyroData();
		raw_gx = getGyroData(X_AXIS);
		raw_gy = getGyroData(Y_AXIS);
		raw_gz = getGyroData(Z_AXIS);

		// Reading accelerometer data
		readACCData();
		raw_ax = getACCData(X_AXIS);
		raw_ay = getACCData(Y_AXIS);
		raw_az = getACCData(Z_AXIS);

		// Reading magnetometer data
		readMagData();
		raw_mx = getMagData(X_AXIS);
		raw_my = getMagData(Y_AXIS);
		raw_mz = getMagData(Z_AXIS);

		// Converting raw data to units for LSM9DS1 with specific ranges

		// Accelerometer full-scale range ±2g, sensitivity 0.061 mg/LSB
		ax = raw_ax * 0.061 / 1000.0; // Convert to g
		ay = raw_ay * 0.061 / 1000.0;
		az = raw_az * 0.061 / 1000.0;

		// Gyroscope full-scale range ±245 degrees/sec, sensitivity 8.75 mdps/LSB
		gx = raw_gx * 8.75 / 1000.0; // Convert to degrees/sec
		gy = raw_gy * 8.75 / 1000.0;
		gz = raw_gz * 8.75 / 1000.0;

		// Magnetometer full-scale range ±4 gauss, sensitivity 0.14 µT/LSB
		mx = raw_mx * 0.14; // Convert to µT
		my = raw_my * 0.14;
		mz = raw_mz * 0.14;

		// Clearing the display before showing new data
		vDisplayClear();

		// Preparing display strings with sensor data
		sprintf(displayRow0, "Temp: %.2f C", temperature);
		sprintf(displayRow1, "x%.0f y%.0f z%.0f r/s", gx, gy, gz);
		sprintf(displayRow2, "x%.0f y%.0f z%.0f g", ax, ay, az);
		sprintf(displayRow3, "x%.0f y%.0f z%.0f uT", mx, my, mz);

		// Writing data to display
		vDisplayWriteStringAtPos(0, 0, displayRow0);
		vDisplayWriteStringAtPos(1, 0, displayRow1);
		vDisplayWriteStringAtPos(2, 0, displayRow2);
		vDisplayWriteStringAtPos(3, 0, displayRow3);

		// Delay for 1 second before refreshing the display
		vTaskDelay(1000 / portTICK_RATE_MS);
	}
}