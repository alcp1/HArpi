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
typedef struct harpiActionSetsList 
{
    harpiActionSetsData actionSet;
    struct harpiActionSetsList *next;
} harpiActionSetsList;

//----------------------------------------------------------------------------//
// INTERNAL GLOBAL VARIABLES
//----------------------------------------------------------------------------//
static pthread_mutex_t g_ActionSets_mutex = PTHREAD_MUTEX_INITIALIZER;
static harpiActionSetsList* head = NULL;

//----------------------------------------------------------------------------//
// INTERNAL FUNCTIONS
//----------------------------------------------------------------------------//
static void clearElementData(harpiActionSetsList* element);
static void freeElementData(harpiActionSetsList* element);
static void addToList(harpiActionSetsList* element);
static harpiActionSetsList* getFromOffset(int offset); // NULL means error
static int deleteList(void);


// Clear all fields from gatewayList //
static void clearElementData(harpiActionSetsList* element)
{
    element->actionSet.actionsSetID = -1;
    element->actionSet.bus = NULL;
    element->actionSet.load = NULL;
    element->actionSet.eventName = NULL;   
    aux_clearHAPCANFrame(&(element->actionSet.frame));
}

// Free allocated fields from gatewayList //
static void freeElementData(harpiActionSetsList* element)
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
static void addToList(harpiActionSetsList* element)
{
    harpiActionSetsList *link;
    // Create a new element to the list
    link = (harpiActionSetsList*)malloc(sizeof(*link));
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
static harpiActionSetsList* getFromOffset(int offset) 
{
    int li_length;
    harpiActionSetsList* current = NULL;
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
    harpiActionSetsList* current;
    harpiActionSetsList* next;
    
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
