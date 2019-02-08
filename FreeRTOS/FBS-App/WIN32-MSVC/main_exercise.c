/*
 * FreeRTOS Kernel V10.1.1
 * Copyright (C) 2018 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */

/******************************************************************************
 * NOTE: Windows will not be running the FreeRTOS demo threads continuously, so
 * do not expect to get real time behaviour from the FreeRTOS Windows port, or
 * this demo application.  Also, the timing information in the FreeRTOS+Trace
 * logs have no meaningful units.  See the documentation page for the Windows
 * port for further information:
 * http://www.freertos.org/FreeRTOS-Windows-Simulator-Emulator-for-Visual-Studio-and-Eclipse-MingW.html
 * 
 ******************************************************************************
 *
 * NOTE:  Console input and output relies on Windows system calls, which can
 * interfere with the execution of the FreeRTOS Windows port.  This demo only
 * uses Windows system call occasionally.  Heavier use of Windows system calls
 * can crash the port.
 */

/* Standard includes. */
#include "stdio.h"
#include "conio.h"

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"

/* Definations */
#define NUM_OF_FRAMES 5
#define NUM_OF_TASKS 6

#define FRAMESIZE 120 //in mS

#define TASK0_PRIORITY 6
#define TASK1_PRIORITY 5
#define TASK2_PRIORITY 4
#define TASK3_PRIORITY 3
#define TASK4_PRIORITY 2
#define TASK5_PRIORITY 1
#define SCHEDULINGTASK_PRIORITY 7

/* Shared Data Structures */
uint8_t FrameData[NUM_OF_FRAMES][NUM_OF_TASKS];  //This Matrix(2-D Array) maps frames and tasks

typedef struct _controlParms {
	TaskHandle_t xHandle;
	uint8_t TaskCompletionFlag;
	SemaphoreHandle_t TaskSemaphore;
	uint8_t TaskValidity;
	uint8_t ExecutionTime;
}controlParms_t;

controlParms_t paramstoTask[6]; //Task Control and Inialiaization Parameters

/* Prototypes */
void DeleteTask(uint8_t TaskNum);
static void Worker_Task_Behaving(void *pvParameters);
static void Worker_Task_Misbehaving(void *pvParameters);
static void Schedular_Task(void *pvParameters);

void main_exercise( void )
{
	/* Initilaization */
	for (uint8_t loop = 0; loop < NUM_OF_TASKS; loop++) {
		paramstoTask[loop].TaskCompletionFlag = 0;
		paramstoTask[loop].TaskValidity = 1;
		paramstoTask[loop].TaskSemaphore = xSemaphoreCreateBinary();
	}
	/* Execusion times for 6 tasks */
	paramstoTask[0].ExecutionTime = 1;
	paramstoTask[1].ExecutionTime = 2;
	paramstoTask[2].ExecutionTime = 3;
	paramstoTask[3].ExecutionTime = 4;
	paramstoTask[4].ExecutionTime = 5;
	paramstoTask[5].ExecutionTime = 6;

	/* 2D Array for Frame and Tasks per Frame Information */
	/* The first dimention signifies the Frames and the second the Tasks for the particular frame */
	/* A 1 enables the task for the particular frame */
	FrameData[0][0] = 1; FrameData[0][1] = 1; FrameData[0][2] = 1; FrameData[0][3] = 1; FrameData[0][4] = 1; FrameData[0][5] = 1;
	FrameData[1][0] = 0; FrameData[1][1] = 0; FrameData[1][2] = 0; FrameData[1][3] = 0; FrameData[1][4] = 0; FrameData[1][5] = 0;
	FrameData[2][0] = 1; FrameData[2][1] = 1; FrameData[2][4] = 1; FrameData[2][2] = 0; FrameData[2][3] = 0; FrameData[2][5] = 0;
	FrameData[3][2] = 1; FrameData[3][3] = 1; FrameData[3][0] = 0; FrameData[3][1] = 0; FrameData[3][4] = 0; FrameData[3][5] = 0;
	FrameData[4][0] = 1; FrameData[4][1] = 1; FrameData[4][4] = 1; FrameData[4][2] = 0; FrameData[4][3] = 0; FrameData[4][5] = 0;

	/* Create the task instances. */
	xTaskCreate(Worker_Task_Behaving, "Task0", 500, &paramstoTask[0], TASK0_PRIORITY, &paramstoTask[0].xHandle);
	xTaskCreate(Worker_Task_Behaving, "Task1", 500, &paramstoTask[1], TASK1_PRIORITY, &paramstoTask[1].xHandle);
	xTaskCreate(Worker_Task_Behaving, "Task2", 500, &paramstoTask[2], TASK2_PRIORITY, &paramstoTask[2].xHandle);
	xTaskCreate(Worker_Task_Behaving, "Task3", 500, &paramstoTask[3], TASK3_PRIORITY, &paramstoTask[3].xHandle);
	xTaskCreate(Worker_Task_Behaving, "Task4", 500, &paramstoTask[4], TASK4_PRIORITY, &paramstoTask[4].xHandle);
	xTaskCreate(Worker_Task_Misbehaving, "Task5", 500, &paramstoTask[5], TASK5_PRIORITY, &paramstoTask[5].xHandle);
	xTaskCreate(Schedular_Task, "Schedular", 500, NULL, SCHEDULINGTASK_PRIORITY, NULL);

	/* Start Schedular */
	vTaskStartScheduler();

	/* */
	for( ;; );
}
/*-----------------------------------------------------------*/

static void Worker_Task_Behaving(void *pvParameters) {
	controlParms_t* control = (controlParms_t*) pvParameters;
	uint32_t localcounter;
	for (;;) {
		/* wait for semaphore to be given to begin execution */
		xSemaphoreTake(control->TaskSemaphore, portMAX_DELAY);
		/* Reset local counter */
		localcounter = 0;
		/* Reset Task Completion Flag */
		control->TaskCompletionFlag = 0;
		/* Start Dummy Count */
		while (localcounter < (control->ExecutionTime*1000000)) {
			localcounter++;
		}
		printf("I counted to %d\n", localcounter);
		/* Set Task Completion Flag */
		control->TaskCompletionFlag = 1;
	}
}
/*-----------------------------------------------------------*/
static void Worker_Task_Misbehaving(void *pvParameters) {
	controlParms_t* control = (controlParms_t*)pvParameters;
	uint32_t localcounter;
	for (;;) {
		/* wait for semaphore to be given to begin execution */
		xSemaphoreTake(control->TaskSemaphore, portMAX_DELAY);
		/* Reset local counter */
		localcounter = 0;
		/* Reset Task Completion Flag */
		control->TaskCompletionFlag = 0;
		/* Start Dummy Count */
		while (localcounter < (control->ExecutionTime * 1000000)){
			localcounter=0; //Error introduced intentionally
		}
		printf("I counted to %d\n", localcounter);
		/* Set Task Completion Flag */
		control->TaskCompletionFlag = 1;
	}
}
/*-----------------------------------------------------------*/

static void Schedular_Task(void *pvParameters) {
	pvParameters = NULL;
	/* This task uses the globally avaibale FrameData Matrix */
	TickType_t xLastWakeTime;
	const TickType_t xFrequency = pdMS_TO_TICKS(FRAMESIZE);
	xLastWakeTime = xTaskGetTickCount();
	uint8_t frame_iternation = 0;
	uint8_t firstIternationflag = 1;
	for (;;) {
		if (!firstIternationflag) {
			/* Task Overrun Check */
			printf("Checking for Task OverRun\n");
			/* Iterate through the tasks */
			for (uint8_t loop = 0; loop < NUM_OF_TASKS; loop++) {
				/* Check for Tasks valid in the last frame*/
				if (FrameData[frame_iternation][loop]) {
					/* Check for task already marked as invalid */
					if (paramstoTask[loop].TaskValidity) {
						/* Check for completion flag */
						if (!paramstoTask[loop].TaskCompletionFlag) {
							printf("Task%d Missed Deadline. Deleting Task\n", loop);
							DeleteTask(loop);
						}
					}
				}
			}

			/* Increment frame iternation */
			frame_iternation++;
			if (frame_iternation >= NUM_OF_FRAMES)
				frame_iternation = 0;
		}
		else {
			/* First Iteration to bypass the Task Overrun test */
			printf("This is the very first frame!\n");
			firstIternationflag = 0;
		}

		printf("Frame Number: %d\n", frame_iternation);

		/* Check the 2D Array (Matrix) to know which task to enable for the respective frame*/
		for (uint8_t loop = 0; loop < NUM_OF_TASKS; loop++) {
			if (FrameData[frame_iternation][loop]) {
				if(paramstoTask[loop].TaskValidity)
					/* Give Semaphore to the task if it needs to be enabled in the next frame */
					xSemaphoreGive(paramstoTask[loop].TaskSemaphore);
			}
		}

		/* Put the task to sleep until start of new frame */
		vTaskDelayUntil(&xLastWakeTime, xFrequency);
	}
}
/*-----------------------------------------------------------*/

void DeleteTask(uint8_t TaskNum) {
	/* Mark the Task as Invalid */
	paramstoTask[TaskNum].TaskValidity = 0;
	printf("Task%d is deleted\n", TaskNum);
	/* Suspend the task and then delete the semaphore */
	vTaskSuspend(paramstoTask[TaskNum].xHandle);
	vSemaphoreDelete(paramstoTask[TaskNum].TaskSemaphore);
	/* Delete the Task */
	if (paramstoTask[TaskNum].xHandle != NULL) {
		vTaskDelete(paramstoTask[TaskNum].xHandle);
	}
}
/*-----------------------------------------------------------*/