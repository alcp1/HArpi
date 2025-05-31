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
#include <auxiliary.h>
#include <debug.h>
#include <hevents.h>

//----------------------------------------------------------------------------//
// INTERNAL DEFINITIONS
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
// INTERNAL TYPES
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
// INTERNAL GLOBAL VARIABLES
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
// INTERNAL FUNCTIONS
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
// EXTERNAL FUNCTIONS
//----------------------------------------------------------------------------//
/*
 * Add buttons configured in the configuration JSON file to the HAPCAN <--> MQTT
 * gateway
 */
void hevents_handleCAN(hapcanCANData* hapcanData, unsigned long long timestamp)
{
    int check;
    int i;
    hapcanCANData hapcanDataOut;
    bool condition;
    condition = hapcanData->frametype == HAPCAN_BUTTON_FRAME_TYPE;
    condition = condition && hapcanData->flags == 0x00;
    condition = condition && hapcanData->module == 0x01;
    condition = condition && hapcanData->group == 0x01;
    condition = condition && hapcanData->group == 0x01;
    condition = condition && hapcanData->data[2] == 0x01;
    condition = condition && hapcanData->data[3] == 0x00;
    if(condition)
    {
        hapcanDataOut.frametype = 0x400;
        hapcanDataOut.flags = 0x00;
        hapcanDataOut.module = 0xA0;
        hapcanDataOut.group = 0xC0;
        for(i = 0; i < 8; i++)
        {
            hapcanDataOut.data[i] = (uint8_t)i;
        }
    }
    condition = hapcanData->frametype == HAPCAN_BUTTON_FRAME_TYPE;
    condition = condition && hapcanData->flags == 0x00;
    condition = condition && hapcanData->module == 0x01;
    condition = condition && hapcanData->group == 0x01;
    condition = condition && hapcanData->group == 0x01;
    condition = condition && hapcanData->data[2] == 0x01;
    condition = condition && hapcanData->data[3] == 0xFF;
    if(condition)
    {
        hapcanDataOut.frametype = 0x402;
        hapcanDataOut.flags = 0x00;
        hapcanDataOut.module = 0xA2;
        hapcanDataOut.group = 0xC2;
        for(i = 0; i < 8; i++)
        {
            hapcanDataOut.data[i] = 0x10 + (uint8_t)i;
        }
    }
    check = hapcan_addToCANWriteBuffer(&hapcanDataOut, timestamp, false);
    if(check != HAPCAN_CAN_RESPONSE)
    {
        #ifdef DEBUG_HAPCAN_ERRORS
        debug_print("hevents_handleCAN - ERROR: addToCANWriteBuffer!\n");
        #endif
    }
}