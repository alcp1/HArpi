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
// Linked List 
typedef struct linkedList 
{
    harpiActionSetsData actionSet;
    struct linkedList *next;
} linkedList;

//----------------------------------------------------------------------------//
// INTERNAL GLOBAL VARIABLES
//----------------------------------------------------------------------------//
static pthread_mutex_t g_ActionSets_mutex = PTHREAD_MUTEX_INITIALIZER;
static linkedList* head = NULL;
static harpiActionSetsData* harpiActionSetArray = NULL;
static int16_t harpiActionSetArrayLen = 0;

//----------------------------------------------------------------------------//
// INTERNAL FUNCTIONS
//----------------------------------------------------------------------------//
static void clearElementData(linkedList* element);
static void freeElementData(linkedList* element);
static void addToLinkedList(linkedList* element);
static int16_t getLinkedListNElements(void);
static void copyListToArray(void);
static int16_t deleteLinkedList(void);


// Clear all fields from a single element of the linked list //
static void clearElementData(linkedList* element)
{
    element->actionSet.actionsSetID = -1;
    aux_clearHAPCANFrame(&(element->actionSet.frame));
}

// Free allocated fields from a single element of the linked list //
static void freeElementData(linkedList* element)
{
    // Nothing to free
}

// Add element to the linked list
static void addToLinkedList(linkedList* element)
{
    linkedList *link;
    // Create a new element to the list
    link = (linkedList*)malloc(sizeof(*link));
    // Copy structure data ("shallow" copy)
    *link = *element;   
    // No need to create and copy field that are pointers    
    // Set next in list to previous header (previous first node)
    link->next = head;
    // Point header (first node) to current element (new first node)
    head = link;
}

// Get the number of elements in the linked list
static int16_t getLinkedListNElements(void)
{
    int16_t len;
    linkedList* current = NULL;
    // Check all
    len = 0;
    if(current != NULL)
    {
        for(current = head; current != NULL; current = current->next) 
        {
            len++;        
        }
    }
    return len;
}

// Copy from the Linked List to the Array
static void copyListToArray(void)
{
    int16_t i;
    linkedList* current;
    // Check all
    current = head;
    if(current != NULL)
    {
        for(i = 0; i < harpiActionSetArrayLen; i++)
        {
            memcpy(&(harpiActionSetArray[i]), &(current->actionSet), 
                sizeof(harpiActionSetsData));
            current = current->next;
            if(current == NULL)
            {
                break;
            }
        }
    }
    // Return
    return current;
}

// Delete list
static int16_t deleteLinkedList(void)
{
    linkedList* current;
    linkedList* next;
    
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
    int16_t check;
    //---------------------------------------------
    // Delete Linked List and Array - PROTECTED
    //---------------------------------------------
    // LOCK
    pthread_mutex_lock(&g_ActionSets_mutex);
    // Delete Linked List
    check = deleteLinkedList();
    if(harpiActionSetArray != NULL)
    {
        free(harpiActionSetArray);
        harpiActionSetArray = NULL;
        harpiActionSetArrayLen = 0;
    }
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
    linkedList element;
    clearElementData(&element);
    element.actionSet.actionsSetID = actionSet->actionsSetID;    
    memcpy(&element.actionSet.frame, &actionSet->frame, sizeof(hapcanCANData));
    //---------------------------------------------
    // Add to linked list - PROTECTED
    //---------------------------------------------
    // LOCK
    pthread_mutex_lock(&g_ActionSets_mutex);
    // Add
    addToLinkedList(&element);
    // UNLOCK
    pthread_mutex_unlock(&g_ActionSets_mutex);
    // Free data and then return
    freeElementData(&element);
}

void harpiactions_load(void)
{

}

void harpiactions_SendActionsFromID(int16_t actionsSetID)
{
    bool match;
    int16_t check;
    int16_t i;
    unsigned long long millisecondsSinceEpoch;
    // Get Timestamp
    millisecondsSinceEpoch = aux_getmsSinceEpoch();
    // LOCK
    pthread_mutex_lock(&g_ActionSets_mutex);
    // Check all elements of the array
    for(i = 0; i < harpiActionSetArrayLen; i++)
    {
        match = false;
        //---------------------------------
        // Check data                    
        //---------------------------------
        match = (harpiActionSetArray[i].actionsSetID == actionsSetID);
        if(match)
        {
            // Found - Send CAN Frame for action
            check = hapcan_addToCANWriteBuffer(&(harpiActionSetArray[i].frame),
                millisecondsSinceEpoch);
            if(check != HAPCAN_CAN_RESPONSE)
            {
                #ifdef DEBUG_HAPCAN_ERRORS
                debug_print("harpiactions_SendActionsFromID - ERROR: "
                    "hapcan_addToCANWriteBuffer!\n");
                #endif
            }
        }
    }
    // UNLOCK
    pthread_mutex_unlock(&g_ActionSets_mutex);
    // return
    return;
}
