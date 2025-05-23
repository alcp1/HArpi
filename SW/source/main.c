//----------------------------------------------------------------------------//
//                               OBJECT HISTORY                               //
//----------------------------------------------------------------------------//
//  REVISION |    DATE     |                               |      AUTHOR      //
//----------------------------------------------------------------------------//
//  1.00     | 23/May/2025 |                               | ALCP             //
// - First version                                                            //
//----------------------------------------------------------------------------//

/*
* Includes
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "manager.h"

int main(int argc, char *argv[])
{    
    setbuf(stdout, NULL); // disable buffering on stdout: Needed for immediate debug
    
    // Init manager
    managerInit();
}