//----------------------------------------------------------------------------//
//                               OBJECT HISTORY                               //
//----------------------------------------------------------------------------//
//  REVISION |    DATE     |                               |      AUTHOR      //
//----------------------------------------------------------------------------//
//  1.00     | 30/Jul/2025 |                               | ALCP             //
// - First Version: copied from HMSG 01.12                                    //
//----------------------------------------------------------------------------//
//  1.01     | 30/Jul/2025 |                               | ALCP             //
// - Updates to remove unused parts from HMSG 01.12                           //
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
#include "auxiliary.h"
#include "canbuf.h"
#include "debug.h"
#include "errorhandler.h"
#include "hapcan.h"

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
/* From CAN to HAPCAN */
void hapcan_getHAPCANDataFromCAN(struct can_frame* pcf_Frame, hapcanCANData* hCD_ptr)
{
    uint32_t ul_Id;
    uint16_t ui_Temp;
    uint8_t  ub_Temp;
    int li_index;
    
    // Frame Type
    ul_Id = pcf_Frame->can_id;
    ui_Temp = (uint16_t)(ul_Id >> 17);
    hCD_ptr->frametype = ui_Temp;
    
    // Flags
    ul_Id = pcf_Frame->can_id;
    ub_Temp = (uint8_t)(ul_Id >> 16);
    ub_Temp = ub_Temp & 0b00000001;
    hCD_ptr->flags = ub_Temp;
    
    // Module
    ul_Id = pcf_Frame->can_id;
    ub_Temp = (uint8_t)(ul_Id >> 8);
    hCD_ptr->module = ub_Temp;
    
    // Group
    ul_Id = pcf_Frame->can_id;
    ub_Temp = (uint8_t)(ul_Id);
    hCD_ptr->group = ub_Temp;
    
    // Data
    for(li_index = 0; li_index < HAPCAN_DATA_LEN; li_index++)
    {
        hCD_ptr->data[li_index] = pcf_Frame->data[li_index];
    }
}

/* From HAPCAN to CAN */
void hapcan_getCANDataFromHAPCAN(hapcanCANData* hCD_ptr, struct can_frame* pcf_Frame)
{
    uint32_t ul_Id;
    uint16_t ui_Temp;
    uint8_t  ub_Temp;
    int li_index;
    
    // Group
    pcf_Frame->can_id = hCD_ptr->group;
    
    // Module
    ub_Temp = hCD_ptr->module;
    ul_Id = (uint32_t)(ub_Temp << 8);
    pcf_Frame->can_id += ul_Id;
    
    // Flags
    ub_Temp = hCD_ptr->flags;
    ub_Temp = ub_Temp & 0b00000001;
    ul_Id = (uint32_t)(ub_Temp << 16);
    pcf_Frame->can_id += ul_Id;
    
    // Frame Type
    ui_Temp = hCD_ptr->frametype;
    ul_Id = (uint32_t)(ui_Temp << 17);
    pcf_Frame->can_id += ul_Id;
    
    // Data Length
    pcf_Frame->can_dlc = CAN_MAX_DLEN;
    
    // Data
    for(li_index = 0; li_index < HAPCAN_DATA_LEN; li_index++)
    {
        pcf_Frame->data[li_index] = hCD_ptr->data[li_index];
    }
}

/* Checksum from data */
uint8_t hapcan_getChecksumFromCAN(hapcanCANData* hCD_ptr)
{
    uint16_t  ui_Temp;
    int li_index;
    
    // Frame Type
    ui_Temp = (uint8_t)(hCD_ptr->frametype >> 4);
    ui_Temp += (uint8_t)((hCD_ptr->frametype << 4)&(0xF0));
    
    // Flags
    ui_Temp += hCD_ptr->flags;
    
    // Module
    ui_Temp += hCD_ptr->module;
    
    // Group
    ui_Temp += hCD_ptr->group;
        
    // Data
    for(li_index = 0; li_index < HAPCAN_DATA_LEN; li_index++)
    {
        ui_Temp += hCD_ptr->data[li_index];
    }
    
    // Return
    return (uint8_t)(ui_Temp & 0xFF);
}

/**
 * Fill HAPCAN data with a system message based on destination node and group, 
 * for a given frametype
 */
void hapcan_getSystemFrame(hapcanCANData *hd_result, uint16_t frametype, 
        int node, int group)
{
    int c_ID1;
    int c_ID2;
    int i;
    //---------------------------
    // Use fixed configuration
    //---------------------------
    c_ID1 = HAPCAN_DEFAULT_CIDx;
    c_ID2 = HAPCAN_DEFAULT_CIDx;    
    aux_clearHAPCANFrame(hd_result);
    hd_result->frametype = frametype;
    hd_result->flags = 0x00;
    hd_result->module = c_ID1;
    hd_result->group = c_ID2;
    for(i = 0; i < HAPCAN_DATA_LEN; i++)
    {
        hd_result->data[i] = 0xFF;
    }
    hd_result->data[2] = node;
    hd_result->data[3] = group;
    if(frametype == HAPCAN_HEALTH_CHECK_REQUEST_NODE_FRAME_TYPE)
    {
        hd_result->data[0] = 0x01;
    }
}

/**
 * Add a HAPCAN Message to the CAN Write Buffer
 */
int hapcan_addToCANWriteBuffer(hapcanCANData* hapcanData, 
        unsigned long long timestamp)
{
    int check;
    struct can_frame cf_Frame;
    int ret = HAPCAN_CAN_RESPONSE_ERROR;
    //---------------------------------
    // Add data to CAN Write Buffer
    //---------------------------------    
    aux_clearCANFrame(&cf_Frame);
    hapcan_getCANDataFromHAPCAN(hapcanData, &cf_Frame);
    check = canbuf_setWriteMsgToBuffer(0, &cf_Frame, timestamp);
    // Check if error occurred when adding to buffer
    errorh_isError(ERROR_MODULE_CAN_SEND, check);
    if(check != CAN_SEND_OK)
    {
        // Here we have to set to error to inform the application to 
        // restart CAN.
        ret = HAPCAN_CAN_RESPONSE_ERROR;        
    }        
    else
    {
        ret = HAPCAN_CAN_RESPONSE;        
    }
    // return
    return ret;
}