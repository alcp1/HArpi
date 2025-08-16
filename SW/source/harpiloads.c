//----------------------------------------------------------------------------//
//                               OBJECT HISTORY                               //
//----------------------------------------------------------------------------//
//  REVISION |    DATE     |                               |      AUTHOR      //
//----------------------------------------------------------------------------//
//  1.00     | 01/Jun/2025 |                               | ALCP             //
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
#include <debug.h>
#include <harpiloads.h>

//----------------------------------------------------------------------------//
// INTERNAL DEFINITIONS
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
// INTERNAL TYPES
//----------------------------------------------------------------------------//
// State of each load
typedef struct  
{
    harpiSMLoadsData load;
    harpiLoadStatus_t status;
} hlLoads_t;

// State of each state machine
typedef struct  
{
    int16_t stateMachineID;
    harpiLoadStatus_t status;
} hlSM_t;

//----------------------------------------------------------------------------//
// INTERNAL GLOBAL VARIABLES
//----------------------------------------------------------------------------//
static pthread_mutex_t g_SMLoads_mutex = PTHREAD_MUTEX_INITIALIZER;
static harpiSMLoadsData* harpiSMLoadsArray = NULL;
static int16_t harpiSMLoadsArrayLen = 0;
static hlLoads_t* loadsStatusArray = NULL;
static int16_t loadsStatusArrayLen = 0;
static hlSM_t* smStatusArray = NULL;
static int16_t smStatusArrayLen = 0;

//----------------------------------------------------------------------------//
// INTERNAL FUNCTIONS
//----------------------------------------------------------------------------//
static bool copyListToArray(harpiLinkedList* element);
static void initLoadsArray(void);
static void initStateMachinesArray(void);

// Copy from the Linked List to the Array
static bool copyListToArray(harpiLinkedList* element)
{
    int16_t i;
    harpiLinkedList* current;
    bool isOK;
    // Check all
    i = 0;
    isOK = true;
    if(element != NULL)
    {
        for(current = element; current != NULL; current = current->next) 
        {
            if(current->section == CSV_SECTION_STATE_MACHINES_AND_LOADS)
            {
                if(i >= harpiSMLoadsArrayLen)
                {
                    #ifdef DEBUG_HARPILOADS_ERRORS
                    debug_print("harpiloads_load error!\n");
                    #endif
                    isOK = false;
                    break;
                }
                memcpy(&(harpiSMLoadsArray[i]), &(current->smLoadsData), 
                    sizeof(harpiSMLoadsData));
                i++;
            }
        }
    }
    return isOK;
}

// From the State Machine loads array, create the load status and initialize it
static void initLoadsArray(void)
{
    int16_t i;    
    // Init array
    if(loadsStatusArray != NULL)
    {
        free(loadsStatusArray);
        loadsStatusArray = NULL;
    }
    loadsStatusArrayLen = 0;
    // Update if "State Machines and Loads" exists and is OK
    if(harpiSMLoadsArrayLen > 0)
    {
        // Update Len
        loadsStatusArrayLen = harpiSMLoadsArrayLen;
        // Allocate memory for array
        loadsStatusArray = (hlLoads_t*)malloc(loadsStatusArrayLen * 
        sizeof(hlLoads_t));
        // Init status
        for(i = 0; i < loadsStatusArrayLen; i++)
        {
            // Copy data from the state machine loads array
            memcpy(&(loadsStatusArray[i].load), &(harpiSMLoadsArray[i]),
                sizeof(harpiSMLoadsData));
            // Init each load as undefined state
            loadsStatusArray[i].status = HARPI_LOAD_STATUS_UNDEFINED;
        }
    }
}

// From the State Machine loads array, create the state machine status and 
// initialize it
static void initStateMachinesArray(void)
{
    int16_t i_Load;
    int16_t i_SM;
    int16_t* tempArray = NULL;
    int16_t tempArrayLen = 0;
    int16_t smcount;
    bool ignoreState;
    // Init array
    if(smStatusArray != NULL)
    {
        free(smStatusArray);
        smStatusArray = NULL;
    }
    smStatusArrayLen = 0;
    // Init counter
    smcount = 0;
    // Update if "State Machines and Loads" exists and is OK
    if(harpiSMLoadsArrayLen > 0)
    {
        // Init Temp Array
        tempArray = (int16_t*)malloc(harpiSMLoadsArrayLen * sizeof(int16_t));
        tempArrayLen = harpiSMLoadsArrayLen;
        // Init entire array
        for(i_SM = 0; i_SM < tempArrayLen; i_SM++)
        {
            tempArray[i_SM] = -1;
        }
        // Init first position
        tempArray[0] = harpiSMLoadsArray[0].stateMachineID;
        smcount++;
        // Check the State Machine loads array
        for(i_Load = 0; i_Load < harpiSMLoadsArrayLen; i_Load++)
        {
            ignoreState = false;
            for(i_SM = 0; i_SM < smcount; i_SM++)
            {
                if(harpiSMLoadsArray[i_Load].stateMachineID == tempArray[i_SM])
                {
                    ignoreState = true;
                }
            }
            if(!ignoreState)
            {
                // Add to the list of states
                tempArray[smcount] = harpiSMLoadsArray[i_Load].stateMachineID;
                smcount++;
            }
        }
        // Copy from temporary array to final array
        smStatusArrayLen = smcount;
        smStatusArray = (hlSM_t*)malloc(smStatusArrayLen * sizeof(hlSM_t));
        for(i_SM = 0; i_SM < smStatusArrayLen; i_SM++)
        {
            smStatusArray[i_SM].stateMachineID = tempArray[i_SM];
            smStatusArray[i_SM].status = HARPI_LOAD_STATUS_UNDEFINED;
        }
        // Free temporary array
        free(tempArray);
        tempArray = NULL;
    }
}

//----------------------------------------------------------------------------//
// EXTERNAL FUNCTIONS
//----------------------------------------------------------------------------//
void harpiloads_init(void)
{
    //---------------------------------------------
    // Delete Linked List and Array - PROTECTED
    //---------------------------------------------
    // LOCK
    pthread_mutex_lock(&g_SMLoads_mutex);
    // Init array
    if(harpiSMLoadsArray != NULL)
    {
        free(harpiSMLoadsArray);
        harpiSMLoadsArray = NULL;
    }
    harpiSMLoadsArrayLen = 0;
    // UNLOCK
    pthread_mutex_unlock(&g_SMLoads_mutex);
}

void harpiloads_load(harpiLinkedList* element)
{
    bool isOK;
    //---------------------------------------------
    // Clear array, copy from list to array, delete linked lisk - PROTECTED
    //---------------------------------------------
    // LOCK
    pthread_mutex_lock(&g_SMLoads_mutex);
    // Clear array
    if(harpiSMLoadsArray != NULL)
    {
        free(harpiSMLoadsArray);
        harpiSMLoadsArray = NULL;
    }
    // Get array size and allocate memory
    harpiSMLoadsArrayLen = harpi_getLinkedListNElements(
        CSV_SECTION_STATE_MACHINES_AND_LOADS);
    harpiSMLoadsArray = (harpiSMLoadsData*)malloc(harpiSMLoadsArrayLen * 
        sizeof(harpiSMLoadsData));
    // Create array from list
    isOK = copyListToArray(element);
    // UNLOCK
    pthread_mutex_unlock(&g_SMLoads_mutex);
    // Clear data if copy had an error
    if(!isOK)
    {
        harpiloads_init();
    }
    // LOCK
    pthread_mutex_lock(&g_SMLoads_mutex);
    // Init Loads and State machines status
    initLoadsArray();
    initStateMachinesArray();
    // UNLOCK
    pthread_mutex_unlock(&g_SMLoads_mutex);
}

void harpiloads_periodic(void)
{

}

void harpiloads_handleCAN(hapcanCANData* hapcanData, 
        unsigned long long timestamp)
{
    
}

harpiLoadStatus_t harpiloads_anyLoadON(int16_t stateMachineID)
{
    bool isON;
    isON = false;
    return isON;
}