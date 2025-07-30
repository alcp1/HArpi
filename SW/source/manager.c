//----------------------------------------------------------------------------//
//                               OBJECT HISTORY                               //
//----------------------------------------------------------------------------//
//  REVISION |    DATE     |                               |      AUTHOR      //
//----------------------------------------------------------------------------//
//  1.00     | 01/Jun/2025 |                               | ALCP             //
// - First Version from HMSG 01.12                                            //
//----------------------------------------------------------------------------//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <app.h>
#include <auxiliary.h>
#include <buffer.h>
#include <canbuf.h>
#include <config.h>
#include <debug.h>
#include <errorhandler.h>
#include <gateway.h>
#include <hapcan.h>
#include <manager.h>
#include <harpievents.h>

//----------------------------------------------------------------------------//
// INTERNAL DEFINITIONS
//----------------------------------------------------------------------------//
#define NUMBER_OF_THREADS   6
#define INIT_RETRIES    5

//----------------------------------------------------------------------------//
// INTERNAL TYPES
//----------------------------------------------------------------------------//
typedef void* (*vp_thread_t)( void *arg );

//----------------------------------------------------------------------------//
// INTERNAL FUNCTIONS - THREADS
//----------------------------------------------------------------------------//
void* managerHandleCAN0Conn(void *arg);
void* managerHandleCAN0Read(void *arg);
void* managerHandleCAN0Write(void *arg);
void* managerHandleCAN0Buffers(void *arg);
void* managerHandleHAPCANPeriodic(void *arg);
void* managerHandleConfigFile(void *arg);

//----------------------------------------------------------------------------//
// INTERNAL GLOBAL VARIABLES
//----------------------------------------------------------------------------//
pthread_t pt_threadID[NUMBER_OF_THREADS];
vp_thread_t vp_thread[NUMBER_OF_THREADS] = 
{   managerHandleCAN0Conn,              // Manage Socket connection for CAN0
    managerHandleCAN0Read,              // Fill the CAN0 Buffer IN (Read)
    managerHandleCAN0Write,             // Fill the CAN0 Buffer OUT (Write)
    managerHandleCAN0Buffers,           // Manage CAN0 Buffers
    managerHandleHAPCANPeriodic,        // Manage Periodic events (System)
    managerHandleConfigFile};           // Handle Config File Updates

//----------------------------------------------------------------------------//
// INTERNAL FUNCTIONS - AUXILIARY
//----------------------------------------------------------------------------//


//----------------------------------------------------------------------------//
// INTERNAL FUNCTIONS - THREADS
//----------------------------------------------------------------------------//
/* THREAD - Handle CAN0 Connection */
void* managerHandleCAN0Conn(void *arg)
{
    const int channel = 0;
    int check;
    stateCAN_t sc_state;
    while(1)
    {
        /* STATE CHECK AND RE-INIT */
        check = canbuf_getState(channel, &sc_state);
        if( check == EXIT_SUCCESS )
        {
            if(sc_state != CAN_CONNECTED)
            {
                canbuf_connect(channel);
            }
        }
        // 1 second loop to check CAN Bus 
        sleep(1);        
    }
}

/* THREAD - Handle CAN0 Reads */
void* managerHandleCAN0Read(void *arg)
{
    const int channel = 0;
    int check;
    stateCAN_t sc_state;
    bool b_retry;
    while(1)
    {
        /* STATE CHECK AND RE-INIT */
        check = canbuf_getState(channel, &sc_state);
        if( check == EXIT_SUCCESS )
        {
            if(sc_state == CAN_CONNECTED)
            {
                b_retry = true; 
                while(b_retry)
                {                    
                    // Check with 5ms timeout
                    check = canbuf_receive(channel, 5000);
                    // Check and handle the error
                    b_retry = !errorh_isError(ERROR_MODULE_CAN_RECEIVE, check);
                    // Stay in loop if a message was just read successfully
                    b_retry = b_retry && (check == CAN_RECEIVE_OK);                    
                }
                // 2ms delay after reading all messages while connected
                usleep(2000);
            }
            else
            {
                // 5ms loop to check for new messages to be read 
                // when not connected
                usleep(5000);
            }            
        }        
    }
}

/* THREAD - Handle CAN0 Writes */
void* managerHandleCAN0Write(void *arg)
{
    const int channel = 0;
    int check;
    stateCAN_t sc_state;
    bool b_retry;
    while(1)
    {
        /* STATE CHECK AND RE-INIT
         */
        check = canbuf_getState(channel, &sc_state);
        if( check == EXIT_SUCCESS )
        {
            if(sc_state == CAN_CONNECTED)
            {
                b_retry = true;
                while(b_retry)
                {
                    // Send CAN Write Buffer message
                    check = canbuf_send(channel);
                    // Check and handle the error
                    b_retry = !errorh_isError(ERROR_MODULE_CAN_SEND, check);
                    // Stay in loop if a message was just sent successfully
                    b_retry = b_retry && (check == CAN_SEND_OK);
                }
                // 2ms delay after sending all messages while connected
                usleep(2000);
            }
            else
            {
                // 5ms loop to check for new messages to be sent 
                // when not connected
                usleep(5000);
            }            
        }        
    }
}

/* THREAD - Handle CAN0 Buffers */
void* managerHandleCAN0Buffers(void *arg)
{
    const int channel = 0;
    int check;
    stateCAN_t sc_state;
    struct can_frame cf_Frame;
    hapcanCANData hapcanData;
    unsigned long long timestamp;
    bool b_retry;
    while(1)
    {
        // STATE CHECK AND RE-INIT
        check = canbuf_getState(channel, &sc_state);
        if( check == EXIT_SUCCESS )
        {
            if(sc_state == CAN_CONNECTED)
            {
                b_retry = true;
                while(b_retry)
                {
                    // Get data from CAN IN (Read) Buffer
                    check = canbuf_getReadMsgFromBuffer(channel, &cf_Frame, 
                            &timestamp);
                    // Check and handle the error
                    b_retry = !errorh_isError(ERROR_MODULE_CAN_RECEIVE, check);
                    // Stay in loop if a message was just read successfully
                    b_retry = b_retry && (check == CAN_RECEIVE_OK);                    
                    if(b_retry)
                    {
                        //-------------------------------------------------
                        // Process CAN Message
                        //-------------------------------------------------
                        // Error in handled within the function
                        hapcan_getHAPCANDataFromCAN(&cf_Frame, &hapcanData);
                        harpievents_handleCAN(&hapcanData, timestamp);
                    }
                }
                // 2ms loop after empty buffer
                usleep(2000); 
            }
            else
            {
                // 5ms Loop to check for buffers
                usleep(5000);
            }            
        }        
    }
}

/* THREAD - Handle Periodic Messages */
void* managerHandleHAPCANPeriodic(void *arg)
{
    int check;
    bool enable;
    stateCAN_t sc_state;
    while(1)
    {
        // Check if this feature is enabled:
        enable = false;
        if(enable)
        {
            /* STATE CHECK AND RE-INIT */
            check = canbuf_getState(0, &sc_state);
            if( (check == EXIT_SUCCESS) && (sc_state == CAN_CONNECTED) )
            {                                    
                //----------------------------------------------------------
                // Check for messages to be sent to CAN Bus for getting
                // module status
                //----------------------------------------------------------
                // Error is handled within the functions
                //hsystem_periodic();
            }
            else
            {
                // Request initial status update
                //hsystem_statusUpdate();
            }
        }
        // 50ms Loop - Give time for the modules to respond and to not increase
        // Bus load too much (some modules send 17 CAN messages for its status)
        usleep(50000);
    }
}

/* THREAD - Handle Config File updates */
void* managerHandleConfigFile(void *arg)
{    
    while(1)
    {
        if(config_isNewConfigAvailable())
        {
            #ifdef DEBUG_MANAGER_CONFIG_EVENTS
            debug_print("managerHandleConfigFile - New config available!\n");
            #endif
            // For every new configuration file, reload the gateway
            //gateway_init();
        }
        // Only check every 10 seconds
        sleep(10);        
    }
}

//----------------------------------------------------------------------------//
// EXTERNAL FUNCTIONS
//----------------------------------------------------------------------------//
/* Init */
void managerInit(void)
{    
    int li_index;
    int li_check;

    /**************************************************************************
     * Build date
     *************************************************************************/
    #ifdef DEBUG_VERSION
    debug_print("HArpi Start! Version = %s.%s\n", APP_SW_MAIN_VERSION, 
            APP_SW_SUB_VERSION);
    debug_print("HArpi Build date/time = %s - %s\n", __DATE__, __TIME__);
    #endif
    /**************************************************************************
     * INIT CONFIG AND GATEWAY
     *************************************************************************/
    config_init();
     //gateway_init();
    
    /**************************************************************************
     * INIT BUFFERS
     *************************************************************************/        
    for(li_index = 0; li_index < INIT_RETRIES; li_index++)
    {
        li_check = canbuf_init(0);
        if(li_check == EXIT_SUCCESS)
        {
            break;
        }
    }
    
    /**************************************************************************
     * INIT THREADS - CREATE AND JOIN
     *************************************************************************/    
    // Create Threads
    for(li_index = 0; li_index < NUMBER_OF_THREADS; li_index++)
    {
        li_check = pthread_create(&pt_threadID[li_index], 
                NULL, vp_thread[li_index], NULL);
        if(li_check)
        {
            /***************/
            /* FATAL ERROR */
            /***************/
            #ifdef DEBUG_MANAGER_ERRORS
            debug_print("MANAGER: THREAD CREATE ERROR!\n");
            debug_print("- Thread Index = %d\n", li_index);
            #endif
        }
    }    
    // Join Threads
    for(li_index = 0; li_index < NUMBER_OF_THREADS; li_index++)
    {
        li_check = pthread_join(pt_threadID[li_index], NULL);
        if(li_check)
        {
            /***************/
            /* FATAL ERROR */
            /***************/
            #ifdef DEBUG_MANAGER_ERRORS
            debug_print("MANAGER: THREAD JOIN ERROR!\n");
            debug_print("- Thread Index = %d\n", li_index);
            #endif
        }
    }
}
