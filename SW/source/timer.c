//----------------------------------------------------------------------------//
//                               OBJECT HISTORY                               //
//----------------------------------------------------------------------------//
//  REVISION |    DATE     |                               |      AUTHOR      //
//----------------------------------------------------------------------------//
//  1.00     | 17/Aug/2025 |                               | ALCP             //
// - First version                                                            //
//----------------------------------------------------------------------------//

/*
* Includes
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <limits.h>
#include <pthread.h>
#include <auxiliary.h>
#include <buffer.h>
#include <debug.h>
#include <timer.h>

//----------------------------------------------------------------------------//
// INTERNAL DEFINITIONS
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
// INTERNAL TYPES
//----------------------------------------------------------------------------//
// Event (for event processing)
typedef struct  
{
    int16_t stateMachineID;
    harpiTimerStatus_t status;
    int16_t value;
} timerData_t;

//----------------------------------------------------------------------------//
// INTERNAL GLOBAL VARIABLES
//----------------------------------------------------------------------------//
static pthread_mutex_t g_Timers_mutex = PTHREAD_MUTEX_INITIALIZER;
static timerData_t* timerDataArray = NULL;
static int16_t timerDataArrayLen = 0;

//----------------------------------------------------------------------------//
// INTERNAL FUNCTIONS
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
// EXTERNAL FUNCTIONS
//----------------------------------------------------------------------------//
void timer_createTimers(int16_t ntimers, int16_t* smIDArray)
{
    int16_t i;
    // LOCK
    pthread_mutex_lock(&g_Timers_mutex);
    // Init Array
    if(timerDataArray != NULL)
    {
        free(timerDataArray);
        timerDataArray = NULL;
    }
    timerDataArrayLen = 0;
    // Memory allocation
    if(ntimers > 0)
    {
        timerDataArrayLen = ntimers;
        timerDataArray = (timerData_t*)malloc(timerDataArrayLen * 
            sizeof(timerData_t));
    }
    // Set values
    for(i = 0; i < timerDataArrayLen; i++)
    {
        timerDataArray[i].stateMachineID = smIDArray[i];
        timerDataArray[i].status = HARPI_TIMER_INIT;
        timerDataArray[i].value = -1;
    }
    // UNLOCK
    pthread_mutex_unlock(&g_Timers_mutex);
}

void timer_setTimer(int16_t stateMachineID, int16_t value)
{
    int16_t i;
    // LOCK
    pthread_mutex_lock(&g_Timers_mutex);
    // Check all timers to find the one to set
    for(i = 0; i < timerDataArrayLen; i++)
    {
        if(timerDataArray[i].stateMachineID == stateMachineID)
        {
            timerDataArray[i].value = value;
            timerDataArray[i].status = HARPI_TIMER_RUNNING;
        }
    }
    // UNLOCK
    pthread_mutex_unlock(&g_Timers_mutex);
}

void timer_periodic(void)
{
    int16_t i;
    // LOCK
    pthread_mutex_lock(&g_Timers_mutex);
    // Update all timers
    for(i = 0; i < timerDataArrayLen; i++)
    {
        if(timerDataArray[i].value == 0)
        {
            timerDataArray[i].status = HARPI_TIMER_EXPIRED;
        }
        if(timerDataArray[i].value > 0)
        {
            timerDataArray[i].value--;
            timerDataArray[i].status = HARPI_TIMER_RUNNING;
        }
    }
    // UNLOCK
    pthread_mutex_unlock(&g_Timers_mutex);
}

harpiTimerStatus_t timer_getTimerStatus(int16_t stateMachineID)
{
    int16_t i;
    harpiTimerStatus_t ret;
    // Init - timer not found
    ret = HARPI_TIMER_UNAVAILABLE;
    // LOCK
    pthread_mutex_lock(&g_Timers_mutex);
    // Check all timers to find the one to get status
    for(i = 0; i < timerDataArrayLen; i++)
    {
        if(timerDataArray[i].stateMachineID == stateMachineID)
        {
            ret = timerDataArray[i].status;
        }
    }
    // UNLOCK
    pthread_mutex_unlock(&g_Timers_mutex);
}
