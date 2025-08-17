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
#include <harpi.h>
#include <harpiactions.h>
#include <harpievents.h>
#include <harpiloads.h>
#include <harpistatemachines.h>

//----------------------------------------------------------------------------//
// INTERNAL DEFINITIONS
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
// INTERNAL TYPES
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
// INTERNAL GLOBAL VARIABLES
//----------------------------------------------------------------------------//
static pthread_mutex_t g_HarpiList_mutex = PTHREAD_MUTEX_INITIALIZER;
static harpiLinkedList *head = NULL;
static int16_t g_period_counter = 0;

//----------------------------------------------------------------------------//
// INTERNAL FUNCTIONS
//----------------------------------------------------------------------------//
static void clearElementData(harpiLinkedList* element);
static void freeElementData(harpiLinkedList* element);
static void addToHarpiLinkedList(harpiLinkedList* element);
static int16_t deleteHarpiLinkedList(void);

// Clear all fields from a single element of the linked list //
static void clearElementData(harpiLinkedList* element)
{
    // Init section
    element->section = CSV_SECTION_OTHER;
    // Init harpiSMLoadsData
    element->smLoadsData.stateMachineID = -1;
    element->smLoadsData.type = HARPI_LOAD_TYPE_OTHER;
    element->smLoadsData.node = -1;
    element->smLoadsData.group = -1;
    element->smLoadsData.channel = -1;
    // Init harpiSMEventsData
    element->smEventsData.stateMachineID = -1;
    element->smEventsData.eventSetID = -1;
    // Init harpiActionSetsData
    element->actionSetsData.actionsSetID = -1;
    aux_clearHAPCANFrame(&(element->actionSetsData.frame));
    // Init harpiEventSetsData
    element->eventSetsData.eventSetID = -1;
    memset(element->eventSetsData.fiterCondition, 0,HAPCAN_FULL_FRAME_LEN);
    memset(element->eventSetsData.fiter, 0,HAPCAN_FULL_FRAME_LEN);
    // Init harpiStateActionsData
    element->stateActionsData.stateMachineID = -1;
    element->stateActionsData.currentStateID = -1;
    element->stateActionsData.eventSetID = -1;
    element->stateActionsData.actionsSetID = -1;
    // Init harpiStateTransitionsData
    element->stateTransitionsData.stateMachineID = -1;
    element->stateTransitionsData.currentStateID = -1;
    element->stateTransitionsData.eventSetID = -1;
    element->stateTransitionsData.newStateID = -1;
}

// Free allocated fields from a single element of the linked list //
static void freeElementData(harpiLinkedList* element)
{
    // Nothing to free
}

// Add element to the linked list
static void addToHarpiLinkedList(harpiLinkedList* element)
{
    harpiLinkedList *link;
    // Create a new element to the list
    link = (harpiLinkedList*)malloc(sizeof(*link));
    // Copy structure data ("shallow" copy)
    *link = *element;   
    // No need to create and copy field that are pointers    
    // Set next in list to previous header (previous first node)
    link->next = head;
    // Point header (first node) to current element (new first node)
    head = link;
}

// Delete list
static int16_t deleteHarpiLinkedList(void)
{
    harpiLinkedList* current;
    harpiLinkedList* next;
    
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
int harpi_initBuffers(void)
{
    int check;
    check = harpievents_createBuffer();
    return check;
}

void harpi_initList(bool init_modules)
{
    int16_t check;
    //---------------------------------------------
    // Init all modules (protection inside each module)
    //---------------------------------------------
    if(init_modules)
    {
        harpiactions_init();
        harpievents_init();
        harpiloads_init();
        harpism_init();
    }  
    //---------------------------------------------
    // Delete Linked List and Array - PROTECTED
    //---------------------------------------------
    // LOCK
    pthread_mutex_lock(&g_HarpiList_mutex);
    // Delete Linked List
    check = deleteHarpiLinkedList();
    // UNLOCK
    pthread_mutex_unlock(&g_HarpiList_mutex);
    if( check == EXIT_FAILURE )
    {
        #ifdef DEBUG_HARPIACTIONS_ERRORS
        debug_print("harpi_initList error!\n");
        #endif
    }
}

void harpi_addElementToList(harpiLinkedList* element)
{
    bool add_to_list;
    harpiLinkedList new_element;
    clearElementData(&new_element);
    new_element.section = element->section;
    add_to_list = false;
    switch(new_element.section)
    {
        case CSV_SECTION_STATE_MACHINES_AND_LOADS:
            new_element.smLoadsData = element->smLoadsData;
            add_to_list = true;
            break;
        case CSV_SECTION_STATE_MACHINES_AND_EVENTS:
            new_element.smEventsData = element->smEventsData;
            add_to_list = true;
            break;
        case CSV_SECTION_ACTION_SETS:
            new_element.actionSetsData = element->actionSetsData;
            add_to_list = true;
            break;
        case CSV_SECTION_STATES_AND_ACTIONS:
            new_element.stateActionsData = element->stateActionsData;
            add_to_list = true;
            break;
        case CSV_SECTION_STATE_TRANSITIONS:
            new_element.stateTransitionsData = element->stateTransitionsData;
            add_to_list = true;
            break;
        case CSV_SECTION_EVENT_SETS:
            new_element.eventSetsData = element->eventSetsData;
            add_to_list = true;
            break;
        case CSV_SECTION_OTHER:
            add_to_list = false;
            break;
        default:
            add_to_list = false;
            break;
    }
    if(add_to_list)
    {
        //---------------------------------------------
        // Add to linked list - PROTECTED
        //---------------------------------------------
        // LOCK
        pthread_mutex_lock(&g_HarpiList_mutex);
        // Add
        addToHarpiLinkedList(&new_element);
        // UNLOCK
        pthread_mutex_unlock(&g_HarpiList_mutex);
    }
}

int16_t harpi_getLinkedListNElements(csvconfig_file_section_t section)
{
    int16_t len;
    harpiLinkedList* current = NULL;
    // Check all
    len = 0;
    if(head != NULL)
    {
        for(current = head; current != NULL; current = current->next) 
        {
            if(current->section == section)
            {
                len++;
            }
        }
    }
    return len;
}

void harpi_load(void)
{
    //---------------------------------------------
    // Init modules, load them - PROTECTED inside each module
    //---------------------------------------------
    // Init and load all modules
    harpiactions_load(head);
    harpievents_load(head);
    harpiloads_load(head);
    harpism_load(head);
    //---------------------------------------------
    // Clear linked list - PROTECTED inside harpi_initList
    //---------------------------------------------
    harpi_initList(false);
}

void harpi_periodic(void)
{
    // Periodic check of uninitialized loads - every LOADS_PERIOD
    g_period_counter++;
    if(g_period_counter > (int16_t)(HARPILOADS_PERIOD / HARPI_PERIOD))
    {
        harpiloads_periodic();
        g_period_counter = 0;
    }
    // Periodic check of state machines
    harpism_periodic();
}

void harpi_handleCAN(hapcanCANData* hapcanData, 
        unsigned long long timestamp)
{
    // Check for CAN Events
    harpievents_handleCAN(hapcanData, timestamp);
    // Update Loads and State Machine statuses
    harpiloads_handleCAN(hapcanData, timestamp);
}