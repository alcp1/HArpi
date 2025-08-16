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
#include <buffer.h>
#include <debug.h>
#include <harpievents.h>

//----------------------------------------------------------------------------//
// INTERNAL DEFINITIONS
//----------------------------------------------------------------------------//
#define HARPI_EVENTS_BUFFER_SIZE 60

//----------------------------------------------------------------------------//
// INTERNAL TYPES
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
// INTERNAL GLOBAL VARIABLES
//----------------------------------------------------------------------------//
static pthread_mutex_t g_EventSets_mutex = PTHREAD_MUTEX_INITIALIZER;
static harpiEventSetsData* harpiEventSetArray = NULL;
static int16_t harpiEventSetArrayLen = 0;
static int harpiEventsBufferID = -1;

//----------------------------------------------------------------------------//
// INTERNAL FUNCTIONS
//----------------------------------------------------------------------------//
static bool copyListToArray(harpiLinkedList* element);
static bool isMatch(harpiEventSetsData* set, hapcanCANData* hapcanData);

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
            if(current->section == CSV_SECTION_EVENT_SETS)
            {
                if(i >= harpiEventSetArrayLen)
                {
                    #ifdef DEBUG_HARPIEVENTS_ERRORS
                    debug_print("harpievents_load error!\n");
                    #endif
                    isOK = false;
                    break;
                }
                memcpy(&(harpiEventSetArray[i]), &(current->eventSetsData), 
                    sizeof(harpiEventSetsData));
                i++;
            }
        }
    }
    return isOK;
}

// Check for a match between event set data and a given hapcan frame
static bool isMatch(harpiEventSetsData* set, hapcanCANData* hapcanData)
{
    bool match;
    int16_t i;
    uint8_t frame[HAPCAN_FULL_FRAME_LEN];
    // Get byte array from HAPCAN Frame
    aux_getBytesFromHAPCAN(hapcanData, frame);
    //-----------------------------------------
    // Check the frame against the event frame
    //-----------------------------------------
    // 'x': CAN byte doesn't need checking to FILTER byte (always matched)
    // 'e':	CAN byte must be identical to FILTER byte (CANx=FILx)
    // 'n':	CAN byte must be different than FILTER byte (CANx!=FILx)
    // '<':	CAN byte must be less or equal to FILTER byte (CANx<=FILx)
    // '>':	CAN byte must be greater or equal to FILTER byte (CANx>=FILx)
    match = true;
    for(i = 0; i < HAPCAN_FULL_FRAME_LEN; i++)
    {
        switch(set->fiterCondition[i])
        {
            case 'x':
                // Filter condition "x": byte doesn't need checking
                break;
            case 'e':
                // Filter condition "=": has to be equal
                if(frame[i] != set->fiter[i])
                {
                    // Not matched
                    match = false;
                }
                break;
            case 'n':
                // Filter condition: "n": has to be different
                if(frame[i] == set->fiter[i])
                {
                    // Not matched
                    match = false;
                }
                break;
            case '<':
                // Filter condition: "<": has to be smaller or equal
                if(frame[i] > set->fiter[i])
                {
                    // Not matched
                    match = false;
                }
                break;
            case '>':
                // Filter condition: ">": has to be higher or equal
                if(frame[i] < set->fiter[i])
                {
                    // Not matched
                    match = false;
                }
                break;
            default:
                // Unknown condition - Not matched
                match = false;
                break;
        }
        if(!match)
        {
            // Leave as soon as a mismatch is detected
            break;
        }
    }
    // Return
    return match;
}

//----------------------------------------------------------------------------//
// EXTERNAL FUNCTIONS
//----------------------------------------------------------------------------//
int harpievents_createBuffer(void)
{
    int check;
    // Init Buffer
    if(harpiEventsBufferID < 0)
    {
        harpiEventsBufferID = buffer_init(HARPI_EVENTS_BUFFER_SIZE);
    }
    // Check buffer - should have ID
    check = 0;
    if(harpiEventsBufferID < 0)
    {
        #ifdef DEBUG_HARPIEVENTS_ERRORS
        debug_print("harpievents_createBuffer ERROR - Buffer Error!\n");
        debug_print("- Buffer: %d\n", harpiEventsBufferID);
        #endif
        check = 1;
    }
    if(check > 0)
    {
        // return error
        return EXIT_FAILURE;
    }
    else
    {
        // return OK
        return EXIT_SUCCESS;
    }
}

void harpievents_init(void)
{
    //---------------------------------------------
    // Delete Linked List and Array - PROTECTED
    //---------------------------------------------
    // LOCK
    pthread_mutex_lock(&g_EventSets_mutex);
    // Init array
    if(harpiEventSetArray != NULL)
    {
        free(harpiEventSetArray);
        harpiEventSetArray = NULL;
    }
    harpiEventSetArrayLen = 0;
    // Clean buffer
    buffer_clean(harpiEventsBufferID);
    // UNLOCK
    pthread_mutex_unlock(&g_EventSets_mutex);
}

void harpievents_load(harpiLinkedList* element)
{
    bool isOK;
    //---------------------------------------------
    // Clear array, copy from list to array, delete linked lisk - PROTECTED
    //---------------------------------------------
    // LOCK
    pthread_mutex_lock(&g_EventSets_mutex);
    // Clear array
    if(harpiEventSetArray != NULL)
    {
        free(harpiEventSetArray);
        harpiEventSetArray = NULL;
    }
    // Get array size and allocate memory
    harpiEventSetArrayLen = harpi_getLinkedListNElements(
        CSV_SECTION_EVENT_SETS);
    harpiEventSetArray = (harpiEventSetsData*)malloc(harpiEventSetArrayLen * 
        sizeof(harpiEventSetsData));
    // Create array from list
    isOK = copyListToArray(element);
    // UNLOCK
    pthread_mutex_unlock(&g_EventSets_mutex);
    // Clear data if copy had an error
    if(!isOK)
    {
        harpievents_init();
    }
}

void harpievents_handleCAN(hapcanCANData* hapcanData, 
    unsigned long long timestamp)
{
    int16_t i;
    int check;
    bool match;
    harpiEvent_t event;
    // Check for a match
    for(i = 0; i < harpiEventSetArrayLen; i++)
    {
        match = false;
        // LOCK
        pthread_mutex_lock(&g_EventSets_mutex);
        // Compare Frames
        match = isMatch(&(harpiEventSetArray[i]), hapcanData);
        // UNLOCK
        pthread_mutex_unlock(&g_EventSets_mutex);
        // Check compared frames
        if(match)
        {
            event.eventSetID = harpiEventSetArray[i].eventSetID;
            event.type = HARPI_EVENT_CAN;
            // Match - Add a new event to the buffer
            check = buffer_push(harpiEventsBufferID, &event, sizeof(event));
            if( check != BUFFER_OK )
            {
                //----------------
                // FATAL ERROR
                //----------------
                #ifdef DEBUG_HARPIEVENTS_ERRORS
                debug_print("harpievents_handleCAN - Buffer Error!\n");
                debug_print("- Buffer Index: %d\n", harpiEventsBufferID);
                #endif
            }
        }
    }
}