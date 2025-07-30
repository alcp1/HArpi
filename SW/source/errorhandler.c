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

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "canbuf.h"
#include "debug.h"
#include "errorhandler.h"

//----------------------------------------------------------------------------//
// INTERNAL DEFINITIONS
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
 * Check the error response set by a module.
 */
bool errorh_isError(errorh_module_t module, int error)
{
    bool ret = false;
    switch(module)
    {
        case ERROR_MODULE_CAN_SEND:
            switch(error)
            {
                case CAN_SEND_OK:
                case CAN_SEND_NO_DATA:
                    ret = false;
                    break;                
                case CAN_SEND_BUFFER_ERROR:
                case CAN_SEND_SOCKET_ERROR:
                case CAN_SEND_PARAMETER_ERROR:    
                default:
                    // Re-Init and clean Buffers                    
                    canbuf_close(0, 1);
                    ret = true;                
                    break;
            }
            break;        
        case ERROR_MODULE_CAN_RECEIVE:
            switch(error)
            {
                case CAN_RECEIVE_OK:                    
                case CAN_RECEIVE_NO_DATA:
                    ret = false;
                    break;
                case CAN_RECEIVE_BUFFER_ERROR:
                case CAN_RECEIVE_SOCKET_ERROR:
                case CAN_RECEIVE_PARAMETER_ERROR:
                default:
                    // Re-Init and clean Buffers                    
                    canbuf_close(0, 1);
                    ret = true;
                    break;
            }
            break; 
        default:
            ret = false;
            break;
    }
    // Return
    return ret;
}