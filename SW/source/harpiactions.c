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
#include <harpiactions.h>

//----------------------------------------------------------------------------//
// INTERNAL DEFINITIONS
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
// INTERNAL TYPES
//----------------------------------------------------------------------------//
// List 
typedef struct list 
{
    harpiActionSetsData actionSet;
    struct list *next;
} list;

//----------------------------------------------------------------------------//
// INTERNAL GLOBAL VARIABLES
//----------------------------------------------------------------------------//
static pthread_mutex_t g_ActionSets_mutex = PTHREAD_MUTEX_INITIALIZER;
static list* head = NULL;

//----------------------------------------------------------------------------//
// INTERNAL FUNCTIONS
//----------------------------------------------------------------------------//
static void clearElementData(list* element);
static void freeElementData(list* element);
static void addToList(list* element);
static list* getFromOffset(int offset); // NULL means error
static int deleteList(void);


// Clear all fields from gatewayList //
static void clearElementData(list* element)
{
    element->actionSet.actionsSetID = -1;
    element->actionSet.bus = NULL;
    element->actionSet.load = NULL;
    element->actionSet.eventName = NULL;   
    aux_clearHAPCANFrame(&(element->actionSet.frame));
}

// Free allocated fields from gatewayList //
static void freeElementData(list* element)
{
    // bus
    if(element->actionSet.bus != NULL)
    {
        free(element->actionSet.bus);
        element->actionSet.bus = NULL;
    }
    // load
    if(element->actionSet.load != NULL)
    {
        free(element->actionSet.load);
        element->actionSet.load = NULL;
    }
    // eventName
    if(element->actionSet.eventName != NULL)
    {
        free(element->actionSet.eventName);
        element->actionSet.eventName = NULL;
    }
}

// Add element to a given list
static void addToList(list* element)
{
    list *link;
    // Create a new element to the list
    link = (list*)malloc(sizeof(*link));
    // Copy structure data ("shallow" copy)
    *link = *element;   
    // Create and copy field that are pointers
    // - BUS
    if(element->actionSet.bus != NULL)
    {
        link->actionSet.bus = strdup(element->actionSet.bus);
    }
    else
    {
        link->actionSet.bus = NULL;
    }
    // - LOAD
    if(element->actionSet.load != NULL)
    {
        link->actionSet.load = strdup(element->actionSet.load);
    }
    else
    {
        link->actionSet.load = NULL;
    }
    // - EVENTNAME
    if(element->actionSet.eventName != NULL)
    {
        link->actionSet.eventName = strdup(element->actionSet.eventName);
    }
    else
    {
        link->actionSet.eventName = NULL;
    }
    // Set next in list to previous header (previous first node)
    link->next = head;
    // Point header (first node) to current element (new first node)
    head = link;
}

// get element from header (after offset positions). NULL means error
static list* getFromOffset(int offset) 
{
    int li_length;
    list* current = NULL;
    // Check offset parameter
    if( offset < 0 )
    {
        return NULL;
    }
    // Check all
    li_length = 0;
    for(current = head; current != NULL; current = current->next) 
    {
        if(li_length >= offset)
        {
            break;
        }
        li_length++;        
    }
    // Return
    return current;
}

// Delete list
static int deleteList(void)
{
    list* current;
    list* next;
    
    // Check all
    current = head;
    while(current != NULL) 
    {
        //*******************************//
        // Get next structure address    //
        //*******************************//
        next = current->next;
        //*******************************//
        // Free fields that are pointers //
        //*******************************//
        freeElementData(current);
        //*******************************//
        // Free structure itself         //
        //*******************************//
        free(current);
        //*******************************//
        // Get new current address       //
        //*******************************//
        current = next;
        head = current;
    }    
    // return
    return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------//
// EXTERNAL FUNCTIONS
//----------------------------------------------------------------------------//
void harpiactions_init(void)
{
    int check;
    //---------------------------------------------
    // Delete list - PROTECTED
    //---------------------------------------------
    // LOCK
    pthread_mutex_lock(&g_ActionSets_mutex);
    // Delete
    check = deleteList();
    // UNLOCK
    pthread_mutex_unlock(&g_ActionSets_mutex);
    if( check == EXIT_FAILURE )
    {
        #ifdef DEBUG_HARPIACTIONS_ERRORS
        debug_print("harpiactions_init error!\n");
        #endif
    }
}

void harpiactions_AddElementToList(harpiActionSetsData *actionSet)
{
    list element;
    clearElementData(&element);
    element.actionSet.actionsSetID = actionSet->actionsSetID;
    element.actionSet.bus = strdup(actionSet->bus);
    element.actionSet.load = strdup(actionSet->load);
    element.actionSet.eventName = strdup(actionSet->eventName);
    memcpy(&element.actionSet.frame, &actionSet->frame, sizeof(hapcanCANData));
    //---------------------------------------------
    // Add to list - PROTECTED
    //---------------------------------------------
    // LOCK
    pthread_mutex_lock(&g_ActionSets_mutex);
    // Add
    addToList(&element);
    // UNLOCK
    pthread_mutex_unlock(&g_ActionSets_mutex);
    // Free data and then return
    freeElementData(&element);
}

void harpiactions_SendActionsFromID(int16_t actionsSetID)
{
    list* current;
    bool match;
    int check;
    unsigned long long millisecondsSinceEpoch;
    // Get Timestamp
    millisecondsSinceEpoch = aux_getmsSinceEpoch();
    // LOCK
    pthread_mutex_lock(&g_ActionSets_mutex);
    // Get element from initial position
    current = getFromOffset(0);
    // Check remaining from offset
    match = false;
    while(current != NULL) 
    {
        //---------------------------------
        // Check data                    
        //---------------------------------
        match = (current->actionSet.actionsSetID == actionsSetID);
        if(match)
        {
            // Found - Send CAN Frame for action
            check = hapcan_addToCANWriteBuffer(&(current->actionSet.frame), 
                millisecondsSinceEpoch);
            if(check != HAPCAN_CAN_RESPONSE)
            {
                #ifdef DEBUG_HAPCAN_ERRORS
                debug_print("harpiactions_SendActionsFromID - ERROR: "
                    "hapcan_addToCANWriteBuffer!\n");
                #endif
            }
        }                
        //*******************************//
        // Get new current address       //
        //*******************************//
        current = current->next;
    }
    // UNLOCK
    pthread_mutex_unlock(&g_ActionSets_mutex);
    // return
    return;
}
