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
#define CHANNEL_BYTE        2
#define STATUS_BYTE         3
#define INITIAL_DELAY       10 // 5 seconds (see HARPILOADS_PERIOD)
#define WAIT_DELAY_NORMAL   1  // 500ms (see HARPILOADS_PERIOD)
#define WAIT_DELAY_ERROR    60 // 30 seconds (see HARPILOADS_PERIOD)

//----------------------------------------------------------------------------//
// INTERNAL TYPES
//----------------------------------------------------------------------------//
typedef struct  
{
    bool send;
    hapcanCANData frame;
} hlFrameInfo_t;

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

// Periodic actions control
typedef struct  
{
    int16_t initial_delay;
    int16_t current_delay;
    int16_t wait;
    uint8_t last_node;
    uint8_t last_group;
} hlPeriodic_t;

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
static hlPeriodic_t periodInfo;

//----------------------------------------------------------------------------//
// INTERNAL FUNCTIONS
//----------------------------------------------------------------------------//
static bool copyListToArray(harpiLinkedList* element);
static void initLoadsArray(void);
static void initStateMachinesArray(void);
static void getLoadOFFInfo(harpiSMLoadsData* load, hlFrameInfo_t* frame_info);

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

// Generate a HAPCAN frame based on the load's info
// INPUT: load
// OUTPUT: frame (to be filled)
static void getLoadOFFInfo(harpiSMLoadsData* load, hlFrameInfo_t* frame_info)
{
    uint8_t channel_bit;
    aux_clearHAPCANFrame(&(frame_info->frame));
    frame_info->send = false;
    switch(load->type)
    {
        case HARPI_LOAD_TYPE_RELAY:
            //----------------------------------------
            // Get shift. Examples:
            // 0x01 - <00000001> - only relay K1
            // 0x02 - <00000010> - only relay K2
            // 0x03 - <00000011> - relay K1 & K2
            // 0x04 - <00000100> - only relay K3
            //----------------------------------------
            if( (load->channel > 0) && (load->channel <= 6) )
            {
                channel_bit =  1 << (load->channel - 1);
                // Send this frame
                frame_info->send = true;
            }
            // Get initial frame for DIRECT CONTROL
            hapcan_getSystemFrame(&(frame_info->frame),
                HAPCAN_DIRECT_CONTROL_FRAME_TYPE, load->node, load->group);
            // Fill INSTR1 - Turn OFF
            frame_info->frame.data[0] = 0x00;
            // Fill INSTR2 - Channel
            frame_info->frame.data[1] = channel_bit;
            // Fill INSTR3 - Timer (immediate)
            frame_info->frame.data[4] = 0x00;
            break;
        default:
            frame_info->send = false;
            break;   
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
    // Init periodic check
    periodInfo.current_delay = 0;
    periodInfo.initial_delay = 0;
    periodInfo.wait = WAIT_DELAY_NORMAL;
    periodInfo.last_group = 0;
    periodInfo.last_node = 0;
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
    int16_t i_Load;
    uint8_t node;
    uint8_t group;
    bool update;
    hapcanCANData hd_result;
    unsigned long long timestamp;
    int ret;
    // Init
    ret = HAPCAN_NO_RESPONSE;
    update = false;
    //-------------------------------------------------
    // Update counters - Do not need thread protection
    //-------------------------------------------------
    if(periodInfo.initial_delay < INITIAL_DELAY)
    {
        periodInfo.initial_delay++;
    }
    if(periodInfo.current_delay < WAIT_DELAY_ERROR)
    {
        periodInfo.current_delay++;
    }
    // Check if initial delay is over and wait is over
    if(periodInfo.initial_delay >= INITIAL_DELAY && 
        periodInfo.current_delay >= periodInfo.wait)
    {
        // LOCK
        pthread_mutex_lock(&g_SMLoads_mutex);
        //-------------------------------------------------
        // Check all loads to define which status is missing
        //-------------------------------------------------
        if(loadsStatusArrayLen > 0)
        {
            for(i_Load = 0; i_Load < loadsStatusArrayLen; i_Load++)
            {
                if(loadsStatusArray[i_Load].status == 
                    HARPI_LOAD_STATUS_UNDEFINED)
                {
                    update = true;
                    node = loadsStatusArray[i_Load].load.node;
                    group = loadsStatusArray[i_Load].load.group;
                    break;
                }
            }
        }
        // UNLOCK
        pthread_mutex_unlock(&g_SMLoads_mutex);
    }
    // Check if update is needed
    if(update)
    {
        //-------------------------------------------------
        // Request STATUS update for the given module
        //-------------------------------------------------
        hapcan_getSystemFrame(&hd_result,
                HAPCAN_STATUS_REQUEST_NODE_FRAME_TYPE, node, group);
        // Get Timestamp
        timestamp = aux_getmsSinceEpoch();
        ret = hapcan_addToCANWriteBuffer(&hd_result, timestamp);
        if(ret == HAPCAN_CAN_RESPONSE_ERROR)
        {
            #ifdef DEBUG_HARPILOADS_ERRORS
            debug_print("harpiloads_periodic error: CAN Write!\n");
            #endif
        }
        //-------------------------------------------------
        // Update timers - no thread protection needed
        //-------------------------------------------------
        periodInfo.current_delay = 0;
        // Check if it is the same as the last sent
        if( (periodInfo.last_node == node) && 
            (periodInfo.last_group == group) )
        {
            periodInfo.wait = WAIT_DELAY_ERROR;
            #ifdef DEBUG_HARPIEVENTS_ERRORS
            debug_print("harpiloads_periodic - Load Status retry!\n");
            debug_print("Retry: node %d, group %d: %d\n", node, group);
            #endif
        }
        else
        {
            periodInfo.wait = WAIT_DELAY_NORMAL;
        }
        // Update last sent
        periodInfo.last_group = group;
        periodInfo.last_node = node;
    }    
}

void harpiloads_handleCAN(hapcanCANData* hapcanData, 
        unsigned long long timestamp)
{
    harpiLoadType_t type;
    harpiSMLoadsData load;
    int16_t i_Load;
    int16_t i_SM;
    bool update;
    bool update_sm;
    harpiLoadStatus_t status;
    int16_t smID;
    // Init to default
    type = HARPI_LOAD_TYPE_OTHER;
    update = false;
    update_sm = false;
    //---------------------------------------
    // Check the frame type
    //---------------------------------------
    switch(hapcanData->frametype)
    {
        case HAPCAN_RELAY_FRAME_TYPE:
            type = HARPI_LOAD_TYPE_RELAY;
            break;
        default:
            break;
    }
    //---------------------------------------
    // Check load and frame
    //---------------------------------------
    // LOCK
    pthread_mutex_lock(&g_SMLoads_mutex);
    for(i_Load = 0; i_Load < loadsStatusArrayLen; i_Load++)
    {
        load = loadsStatusArray[i_Load].load;
        if(type == load.type)
        {
            switch(type)
            {
                //---------------------------------------
                // Relay: Check Node, Group, Channel
                //---------------------------------------
                case HARPI_LOAD_TYPE_RELAY:
                    update = (load.channel == hapcanData->data[CHANNEL_BYTE]);
                    update = update && (load.node == hapcanData->module);
                    update = update && (load.group == hapcanData->group);
                    // Check if match was found
                    if(update)
                    {
                        if(hapcanData->data[STATUS_BYTE] == 0x00)
                        {
                            // Load is OFF
                            loadsStatusArray[i_Load].status = 
                                HARPI_LOAD_STATUS_OFF;
                        }
                        else if(hapcanData->data[STATUS_BYTE] == 0xFF)
                        {
                            // Load is ON
                            loadsStatusArray[i_Load].status = 
                                HARPI_LOAD_STATUS_ON;
                        }
                        // Update State Machines
                        update_sm = true;
                    }
                    break;
                //---------------------------------------
                // Default: not updated
                //---------------------------------------
                default:
                    update = false;
                    break;
            }
        }        
    }
    //---------------------------------------
    // Check the loads to update the state machine status
    //---------------------------------------
    if(update_sm)
    {
        // At least one load changed - check all
        for(i_SM = 0; i_SM < smStatusArrayLen; i_SM++)
        {
            smID = smStatusArray[i_SM].stateMachineID;
            //------------------------------------------
            // - If any is uninitialized, it is undefined
            // - If all are initialized, and any is ON, it is ON
            // - If all are initialized, and all are OFF, it is OFF
            //------------------------------------------
            status = HARPI_LOAD_STATUS_OFF;
            for(i_Load = 0; i_Load < loadsStatusArrayLen; i_Load++)
            {
                // Check if state machine ID is the same
                if(loadsStatusArray[i_Load].load.stateMachineID == smID)
                {
                    if(loadsStatusArray[i_Load].status == 
                        HARPI_LOAD_STATUS_UNDEFINED)
                    {
                        // Undefined - set as undefined and leave
                        status = HARPI_LOAD_STATUS_UNDEFINED;
                        break;
                    }
                    if(loadsStatusArray[i_Load].status == HARPI_LOAD_STATUS_ON)
                    {
                        // Set as ON, and check remaining for undefined
                        status = HARPI_LOAD_STATUS_ON;
                    }
                }
            }
            smStatusArray[i_SM].status = status;
        }
    }
    // UNLOCK
    pthread_mutex_unlock(&g_SMLoads_mutex);
}

harpiLoadStatus_t harpiloads_isAnyLoadON(int16_t stateMachineID)
{
    int16_t i_SM;
    harpiLoadStatus_t status;
    int16_t smID;
    // Init - If no state machine is matched, no load is available for the ID
    status = HARPI_LOAD_STATUS_NO_LOADS;
    // Check the state machines
    // LOCK
    pthread_mutex_lock(&g_SMLoads_mutex);
    // Check all state machine IDs
    for(i_SM = 0; i_SM < smStatusArrayLen; i_SM++)
    {
        smID = smStatusArray[i_SM].stateMachineID;
        if(smID == stateMachineID)
        {
            status = smStatusArray[i_SM].status;
            break;
        }
    }
    // UNLOCK
    pthread_mutex_unlock(&g_SMLoads_mutex);
    return status;
}

void harpiloads_setLoadsOFF(int16_t stateMachineID)
{
    int16_t i;
    int16_t smID;
    hlFrameInfo_t* hframeInfoArray = NULL;
    int16_t hframeInfoArrayLen = 0;
    int16_t check;
    unsigned long long millisecondsSinceEpoch;
    // LOCK
    pthread_mutex_lock(&g_SMLoads_mutex);
    //-----------------------------------------
    // Get the number of loads to be turned OFF
    //-----------------------------------------
    hframeInfoArrayLen = 0;
    for(i = 0; i < harpiSMLoadsArrayLen; i++)
    {
        smID = harpiSMLoadsArray[i].stateMachineID;
        if(smID == stateMachineID)
        {
            hframeInfoArrayLen++;
        }
    }
    //-----------------------------------------
    // Allocate memory
    //-----------------------------------------
    if(hframeInfoArrayLen > 0)
    {
        hframeInfoArray = (hlFrameInfo_t*)malloc(hframeInfoArrayLen * 
            sizeof(hlFrameInfo_t));
        //-----------------------------------------
        // Get frame info - Fill hframeInfoArray
        //-----------------------------------------
        hframeInfoArrayLen = 0;
        for(i = 0; i < harpiSMLoadsArrayLen; i++)
        {
            smID = harpiSMLoadsArray[i].stateMachineID;
            if(smID == stateMachineID)
            {
                // Get frame info
                getLoadOFFInfo(&(harpiSMLoadsArray[i]), 
                    &(hframeInfoArray[hframeInfoArrayLen]));
                hframeInfoArrayLen++;
            }
        }
    }
    // UNLOCK
    pthread_mutex_unlock(&g_SMLoads_mutex);
    //-----------------------------------------
    // Send frames
    //-----------------------------------------
    // Get Timestamp
    millisecondsSinceEpoch = aux_getmsSinceEpoch();
    // Send each frame
    for(i = 0; i < hframeInfoArrayLen; i++)
    {
        if(hframeInfoArray[i].send)
        {
            // Send CAN Frame
            check = hapcan_addToCANWriteBuffer(&(hframeInfoArray[i].frame),
                millisecondsSinceEpoch);
            if(check != HAPCAN_CAN_RESPONSE)
            {
                #ifdef DEBUG_HAPCAN_ERRORS
                debug_print("harpiloads_setLoadsOFF - ERROR!\n");
                #endif
            }
        }
    }
    //-----------------------------------------
    // Free memory
    //-----------------------------------------
    free(hframeInfoArray);
    hframeInfoArray = NULL;
}