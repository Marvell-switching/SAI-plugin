/*
 *  Copyright (C) 2016. Marvell International Ltd. ALL RIGHTS RESERVED.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License"); you may
 *    not use this file except in compliance with the License. You may obtain
 *    a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 *    THIS CODE IS PROVIDED ON AN  *AS IS* BASIS, WITHOUT WARRANTIES OR
 *    CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
 *    LIMITATION ANY IMPLIED WARRANTIES OR CONDITIONS OF TITLE, FITNESS
 *    FOR A PARTICULAR PURPOSE, MERCHANTABLITY OR NON-INFRINGEMENT.
 *
 *    See the Apache Version 2.0 License for specific language governing
 *    permissions and limitations under the License.
 *
 */

#include "utils/mrvl_sai_dlist.h"
/*#include 	<stddef.h>*/


/*!**************************************************RND Template version 4.1
 *!     F U N C T I O N   D E S I G N   &   I M P L E M E N T A T I O N
 *!==========================================================================
 *$ TITLE: insert element to the list (private func).
 *!--------------------------------------------------------------------------
 *$ FILENAME: R:\routers\src\lib\dlist\src\insertp.cc
 *!--------------------------------------------------------------------------
 *$ SYSTEM, SUBSYSTEM: LIB
 *!--------------------------------------------------------------------------
 *$ AUTHORS: RutyS,User
 *!--------------------------------------------------------------------------
 *$ LATEST UPDATE: 11-Mar-2009, 2:28 PM CREATION DATE: 08-Sep-96
 *!**************************************************************************
 *!
 *!**************************************************************************
 *!
 *$ FUNCTION: mrvl_sai_utl_dlist_prv_insert
 *!
 *$ GENERAL DESCRIPTION:
 *!
 *$ RETURNS:
 *!
 *$ ALGORITHM:   (local)
 *!
 *$ ASSUMPTIONS:
 *!
 *$ REMARKS:     (local)
 *!
 *!**************************************************************************
 *!*/

static void mrvl_sai_utl_dlist_prv_insert (

    /*!     INPUTS:             */

    mrvl_sai_utl_dlist_elem_STC *prev_PTR,
    mrvl_sai_utl_dlist_elem_STC *next_PTR,
    mrvl_sai_utl_dlist_elem_STC *new_PTR
    /*!     INPUTS / OUTPUTS:   */

    /*!     OUTPUTS:            */

)
{
/*!*************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!*************************************************************************/

/*!*************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!*************************************************************************/
    next_PTR->prev_PTR = new_PTR;
    prev_PTR->next_PTR = new_PTR;
    new_PTR->next_PTR = next_PTR;
    new_PTR->prev_PTR = prev_PTR;
}
/*$ END OF mrvl_sai_utl_dlist_prv_insert */


/*!**************************************************RND Template version 4.1
 *!     F U N C T I O N   D E S I G N   &   I M P L E M E N T A T I O N
 *!==========================================================================
 *$ TITLE: remove element from the linked list, private func.
 *!--------------------------------------------------------------------------
 *$ FILENAME: R:\routers\src\lib\dlist\src\removep.cc
 *!--------------------------------------------------------------------------
 *$ SYSTEM, SUBSYSTEM: LIB
 *!--------------------------------------------------------------------------
 *$ AUTHORS: RutyS,User
 *!--------------------------------------------------------------------------
 *$ LATEST UPDATE: 11-Mar-2009, 2:23 PM CREATION DATE: 08-Sep-96
 *!**************************************************************************
 *!
 *!**************************************************************************
 *!
 *$ FUNCTION: LISTP_remove
 *!
 *$ GENERAL DESCRIPTION:
 *!
 *$ RETURNS:
 *!
 *$ ALGORITHM:   (local)
 *!
 *$ ASSUMPTIONS:
 *!
 *$ REMARKS:     (local)
 *!
 *!**************************************************************************
 *!*/

static void mrvl_sai_utl_dlist_prv_remove (

    /*!     INPUTS:             */

    mrvl_sai_utl_dlist_elem_STC *prev_PTR,
    mrvl_sai_utl_dlist_elem_STC *next_PTR
    /*!     INPUTS / OUTPUTS:   */

    /*!     OUTPUTS:            */

)
{
/*!*************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!*************************************************************************/

/*!*************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!*************************************************************************/

    prev_PTR->next_PTR = next_PTR;
    next_PTR->prev_PTR = prev_PTR;
}
/*$ END OF mrvl_sai_utl_dlist_prv_remove */




/*!**************************************************************************
 *$              PUBLIC FUNCTION DEFINITIONS (EXPORT)
 *!**************************************************************************
 *!*/

/*!**************************************************RND Template version 4.1
 *!     F U N C T I O N   D E S I G N   &   I M P L E M E N T A T I O N
 *!==========================================================================
 *$ TITLE: init the list.
 *!--------------------------------------------------------------------------
 *$ FILENAME: R:\routers\src\lib\dlist\src\init.cc
 *!--------------------------------------------------------------------------
 *$ SYSTEM, SUBSYSTEM: LIB
 *!--------------------------------------------------------------------------
 *$ AUTHORS: RutyS,User
 *!--------------------------------------------------------------------------
 *$ LATEST UPDATE: 10-Mar-2009, 10:12 AM CREATION DATE: 08-Sep-96
 *!**************************************************************************
 *!
 *!**************************************************************************
 *!
 *$ FUNCTION: LISTG_init
 *!
 *$ GENERAL DESCRIPTION:
 *!
 *$ RETURNS:
 *!
 *$ ALGORITHM:   (local)
 *!
 *$ ASSUMPTIONS:
 *!
 *$ REMARKS:     (local)
 *!
 *!**************************************************************************
 *!*/

extern void mrvl_sai_utl_dlist_init (

    /*!     INPUTS:             */

    mrvl_sai_utl_dlist_elem_STC *list_PTR
    /*!     INPUTS / OUTPUTS:   */

    /*!     OUTPUTS:            */

)
{
/*!*************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!*************************************************************************/

/*!*************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!*************************************************************************/

    list_PTR->next_PTR = list_PTR;
    list_PTR->prev_PTR = list_PTR;

}
/*$ END OF mrvl_sai_utl_dlist_init */


/*!**************************************************RND Template version 4.1
 *!     F U N C T I O N   D E S I G N   &   I M P L E M E N T A T I O N
 *!==========================================================================
 *$ TITLE: check if the list is empty.
 *!--------------------------------------------------------------------------
 *$ FILENAME: R:\routers\src\lib\dlist\src\is_empty.cc
 *!--------------------------------------------------------------------------
 *$ SYSTEM, SUBSYSTEM: LIB
 *!--------------------------------------------------------------------------
 *$ AUTHORS: RutyS,User
 *!--------------------------------------------------------------------------
 *$ LATEST UPDATE: 11-Mar-2009, 2:27 PM CREATION DATE: 08-Sep-96
 *!**************************************************************************
 *!
 *!**************************************************************************
 *!
 *$ FUNCTION: LISTG_is_empty
 *!
 *$ GENERAL DESCRIPTION:
 *!
 *$ RETURNS:
 *!
 *$ ALGORITHM:   (local)
 *!
 *$ ASSUMPTIONS:
 *!
 *$ REMARKS:     (local)
 *!
 *!**************************************************************************
 *!*/

extern int mrvl_sai_utl_dlist_is_empty (

    /*!     INPUTS:             */

    mrvl_sai_utl_dlist_elem_STC *header_PTR
    /*!     INPUTS / OUTPUTS:   */

    /*!     OUTPUTS:            */

)
{
/*!*************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!*************************************************************************/

/*!*************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!*************************************************************************/

    if( header_PTR->next_PTR == header_PTR)
        return 1;
    return 0;


}
/*$ END OF mrvl_sai_utl_dlist_is_empty */


/*!**************************************************RND Template version 4.1
 *!     F U N C T I O N   D E S I G N   &   I M P L E M E N T A T I O N
 *!==========================================================================
 *$ TITLE: insert element afetr another.
 *!--------------------------------------------------------------------------
 *$ FILENAME: R:\routers\src\lib\dlist\src\insert_after.cc
 *!--------------------------------------------------------------------------
 *$ SYSTEM, SUBSYSTEM: LIB
 *!--------------------------------------------------------------------------
 *$ AUTHORS: RutyS,User
 *!--------------------------------------------------------------------------
 *$ LATEST UPDATE: 11-Mar-2009, 2:31 PM CREATION DATE: 08-Sep-96
 *!**************************************************************************
 *!
 *!**************************************************************************
 *!
 *$ FUNCTION: mrvl_sai_utl_dlist_insert_after
 *!
 *$ GENERAL DESCRIPTION:
 *!
 *$ RETURNS:
 *!
 *$ ALGORITHM:   (local)
 *!
 *$ ASSUMPTIONS:
 *!
 *$ REMARKS:     (local)
 *!
 *!**************************************************************************
 *!*/

extern void mrvl_sai_utl_dlist_insert_after (

    /*!     INPUTS:             */

    mrvl_sai_utl_dlist_elem_STC *prev_PTR,
    mrvl_sai_utl_dlist_elem_STC *new_PTR
    /*!     INPUTS / OUTPUTS:   */

    /*!     OUTPUTS:            */

)
{
/*!*************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!*************************************************************************/

/*!*************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!*************************************************************************/
    mrvl_sai_utl_dlist_prv_insert(prev_PTR,prev_PTR->next_PTR,new_PTR);
}
/*$ END OF mrvl_sai_utl_dlist_insert_after */


/*!**************************************************RND Template version 4.1
 *!     F U N C T I O N   D E S I G N   &   I M P L E M E N T A T I O N
 *!==========================================================================
 *$ TITLE: insert element before another.
 *!--------------------------------------------------------------------------
 *$ FILENAME: R:\routers\src\lib\dlist\src\insert_before.cc
 *!--------------------------------------------------------------------------
 *$ SYSTEM, SUBSYSTEM: LIB
 *!--------------------------------------------------------------------------
 *$ AUTHORS: RutyS,User
 *!--------------------------------------------------------------------------
 *$ LATEST UPDATE: 11-Mar-2009, 2:30 PM CREATION DATE: 08-Sep-96
 *!**************************************************************************
 *!
 *!**************************************************************************
 *!
 *$ FUNCTION: mrvl_sai_utl_dlist_insert_before
 *!
 *$ GENERAL DESCRIPTION:
 *!
 *$ RETURNS:
 *!
 *$ ALGORITHM:   (local)
 *!
 *$ ASSUMPTIONS:
 *!
 *$ REMARKS:     (local)
 *!
 *!**************************************************************************
 *!*/

extern void mrvl_sai_utl_dlist_insert_before (

    /*!     INPUTS:             */

    mrvl_sai_utl_dlist_elem_STC *next_PTR,
    mrvl_sai_utl_dlist_elem_STC *new_PTR
    /*!     INPUTS / OUTPUTS:   */

    /*!     OUTPUTS:            */

)
{
/*!*************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!*************************************************************************/

/*!*************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!*************************************************************************/
    mrvl_sai_utl_dlist_prv_insert(next_PTR->prev_PTR,next_PTR,new_PTR);
}
/*$ END OF mrvl_sai_utl_dlist_insert_before */


/*!**************************************************RND Template version 4.1
 *!     F U N C T I O N   D E S I G N   &   I M P L E M E N T A T I O N
 *!==========================================================================
 *$ TITLE: insert element to the list head.
 *!--------------------------------------------------------------------------
 *$ FILENAME: R:\routers\src\lib\dlist\src\insert_to_head.cc
 *!--------------------------------------------------------------------------
 *$ SYSTEM, SUBSYSTEM: LIB
 *!--------------------------------------------------------------------------
 *$ AUTHORS: RutyS,User
 *!--------------------------------------------------------------------------
 *$ LATEST UPDATE: 11-Mar-2009, 2:29 PM CREATION DATE: 08-Sep-96
 *!**************************************************************************
 *!
 *!**************************************************************************
 *!
 *$ FUNCTION: mrvl_sai_utl_dlist_insert_to_head
 *!
 *$ GENERAL DESCRIPTION:
 *!
 *$ RETURNS:
 *!
 *$ ALGORITHM:   (local)
 *!
 *$ ASSUMPTIONS:
 *!
 *$ REMARKS:     (local)
 *!
 *!**************************************************************************
 *!*/

extern void mrvl_sai_utl_dlist_insert_to_head (

    /*!     INPUTS:             */

    mrvl_sai_utl_dlist_elem_STC *header_PTR,
    mrvl_sai_utl_dlist_elem_STC *new_PTR
    /*!     INPUTS / OUTPUTS:   */

    /*!     OUTPUTS:            */

)
{
/*!*************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!*************************************************************************/

/*!*************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!*************************************************************************/
    mrvl_sai_utl_dlist_prv_insert(header_PTR,header_PTR->next_PTR,new_PTR);
}
/*$ END OF mrvl_sai_utl_dlist_insert_to_head */


/*!**************************************************RND Template version 4.1
 *!     F U N C T I O N   D E S I G N   &   I M P L E M E N T A T I O N
 *!==========================================================================
 *$ TITLE: insert element to the list tail.
 *!--------------------------------------------------------------------------
 *$ FILENAME: R:\routers\src\lib\dlist\src\insert_to_tail.cc
 *!--------------------------------------------------------------------------
 *$ SYSTEM, SUBSYSTEM: LIB
 *!--------------------------------------------------------------------------
 *$ AUTHORS: RutyS,User
 *!--------------------------------------------------------------------------
 *$ LATEST UPDATE: 11-Mar-2009, 2:29 PM CREATION DATE: 08-Sep-96
 *!**************************************************************************
 *!
 *!**************************************************************************
 *!
 *$ FUNCTION: mrvl_sai_utl_dlist_insert_to_tail
 *!
 *$ GENERAL DESCRIPTION:
 *!
 *$ RETURNS:
 *!
 *$ ALGORITHM:   (local)
 *!
 *$ ASSUMPTIONS:
 *!
 *$ REMARKS:     (local)
 *!
 *!**************************************************************************
 *!*/

extern void mrvl_sai_utl_dlist_insert_to_tail (

    /*!     INPUTS:             */

    mrvl_sai_utl_dlist_elem_STC *header_PTR,
    mrvl_sai_utl_dlist_elem_STC *new_PTR
    /*!     INPUTS / OUTPUTS:   */

    /*!     OUTPUTS:            */

)
{
/*!*************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!*************************************************************************/

/*!*************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!*************************************************************************/
    mrvl_sai_utl_dlist_prv_insert(header_PTR->prev_PTR,header_PTR,new_PTR);
}
/*$ END OF mrvl_sai_utl_dlist_insert_to_tail */


/*!**************************************************RND Template version 4.1
 *!     F U N C T I O N   D E S I G N   &   I M P L E M E N T A T I O N
 *!==========================================================================
 *$ TITLE: remove element from list.
 *!--------------------------------------------------------------------------
 *$ FILENAME: lib\dlist\src\remove.cc
 *!--------------------------------------------------------------------------
 *$ SYSTEM, SUBSYSTEM: LIB
 *!--------------------------------------------------------------------------
 *$ AUTHORS: RutyS,User,LiranP
 *!--------------------------------------------------------------------------
 *$ LATEST UPDATE: 20-Feb-2012, 9:26 AM CREATION DATE: 08-Sep-96
 *!**************************************************************************
 *!
 *!**************************************************************************
 *!
 *$ FUNCTION: mrvl_sai_utl_dlist_remove
 *!
 *$ GENERAL DESCRIPTION:
 *!
 *$ RETURNS:
 *!
 *$ ALGORITHM:   (local)
 *!
 *$ ASSUMPTIONS:
 *!
 *$ REMARKS:     (local)
 *!
 *!**************************************************************************
 *!*/

extern void mrvl_sai_utl_dlist_remove (

    /*!     INPUTS:             */

    mrvl_sai_utl_dlist_elem_STC *elem_PTR

    /*!     INPUTS / OUTPUTS:   */

    /*!     OUTPUTS:            */

)
{
/*!*************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!*************************************************************************/

/*!*************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!*************************************************************************/

    /* support for "removing" elements which are not part of any list */
    if((elem_PTR->prev_PTR != NULL) && (elem_PTR->next_PTR != NULL))
    {
        mrvl_sai_utl_dlist_prv_remove(elem_PTR->prev_PTR, elem_PTR->next_PTR);
    }
    elem_PTR->next_PTR = NULL;
    elem_PTR->prev_PTR = NULL;
}
/*$ END OF mrvl_sai_utl_dlist_remove */


/*!**************************************************RND Template version 4.1
 *!     F U N C T I O N   D E S I G N   &   I M P L E M E N T A T I O N
 *!==========================================================================
 *$ TITLE: remove element from the list head.
 *!--------------------------------------------------------------------------
 *$ FILENAME: R:\routers\src\lib\dlist\src\remove_from_head.cc
 *!--------------------------------------------------------------------------
 *$ SYSTEM, SUBSYSTEM: LIB
 *!--------------------------------------------------------------------------
 *$ AUTHORS: RutyS,User
 *!--------------------------------------------------------------------------
 *$ LATEST UPDATE: 11-Mar-2009, 2:25 PM CREATION DATE: 08-Sep-96
 *!**************************************************************************
 *!
 *!**************************************************************************
 *!
 *$ FUNCTION: mrvl_sai_utl_dlist_remove_from_head
 *!
 *$ GENERAL DESCRIPTION:
 *!
 *$ RETURNS:
 *!
 *$ ALGORITHM:   (local)
 *!
 *$ ASSUMPTIONS:
 *!
 *$ REMARKS:     (local)
 *!
 *!**************************************************************************
 *!*/

extern mrvl_sai_utl_dlist_elem_STC * mrvl_sai_utl_dlist_remove_from_head (

    /*!     INPUTS:             */

    mrvl_sai_utl_dlist_elem_STC *header_PTR
     /*!     INPUTS / OUTPUTS:   */

    /*!     OUTPUTS:            */

)
{
/*!*************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!*************************************************************************/
   mrvl_sai_utl_dlist_elem_STC *removed_PTR;
/*!*************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!*************************************************************************/
    removed_PTR = header_PTR->next_PTR;
    mrvl_sai_utl_dlist_prv_remove(header_PTR,header_PTR->next_PTR->next_PTR);
    return removed_PTR;
}
/*$ END OF mrvl_sai_utl_dlist_remove_from_head */


/*!**************************************************RND Template version 4.1
 *!     F U N C T I O N   D E S I G N   &   I M P L E M E N T A T I O N
 *!==========================================================================
 *$ TITLE: remove element from the list tail.
 *!--------------------------------------------------------------------------
 *$ FILENAME: R:\routers\src\lib\dlist\src\remove_from_tail.cc
 *!--------------------------------------------------------------------------
 *$ SYSTEM, SUBSYSTEM: LIB
 *!--------------------------------------------------------------------------
 *$ AUTHORS: RutyS,User
 *!--------------------------------------------------------------------------
 *$ LATEST UPDATE: 11-Mar-2009, 2:24 PM CREATION DATE: 08-Sep-96
 *!**************************************************************************
 *!
 *!**************************************************************************
 *!
 *$ FUNCTION: mrvl_sai_utl_dlist_remove_from_tail
 *!
 *$ GENERAL DESCRIPTION:
 *!
 *$ RETURNS:
 *!
 *$ ALGORITHM:   (local)
 *!
 *$ ASSUMPTIONS:
 *!
 *$ REMARKS:     (local)
 *!
 *!**************************************************************************
 *!*/

extern mrvl_sai_utl_dlist_elem_STC * mrvl_sai_utl_dlist_remove_from_tail (

    /*!     INPUTS:             */

    mrvl_sai_utl_dlist_elem_STC *header_PTR
     /*!     INPUTS / OUTPUTS:   */

    /*!     OUTPUTS:            */

)
{
/*!*************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!*************************************************************************/
   mrvl_sai_utl_dlist_elem_STC *removed_PTR;
/*!*************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!*************************************************************************/
    removed_PTR = header_PTR->prev_PTR;
    mrvl_sai_utl_dlist_prv_remove(header_PTR->prev_PTR->prev_PTR,header_PTR);
    return removed_PTR;
}
/*$ END OF mrvl_sai_utl_dlist_remove_from_tail */


/*!**************************************************RND Template version 4.1
 *!     F U N C T I O N   D E S I G N   &   I M P L E M E N T A T I O N
 *!==========================================================================
 *$ TITLE: move element to the list head.
 *!--------------------------------------------------------------------------
 *$ FILENAME: R:\routers\src\lib\dlist\src\move_to_head.cc
 *!--------------------------------------------------------------------------
 *$ SYSTEM, SUBSYSTEM: LIB
 *!--------------------------------------------------------------------------
 *$ AUTHORS: RutyS,User
 *!--------------------------------------------------------------------------
 *$ LATEST UPDATE: 11-Mar-2009, 2:27 PM CREATION DATE: 08-Sep-96
 *!**************************************************************************
 *!
 *!**************************************************************************
 *!
 *$ FUNCTION: mrvl_sai_utl_dlist_move_to_head
 *!
 *$ GENERAL DESCRIPTION:
 *!
 *$ RETURNS:
 *!
 *$ ALGORITHM:   (local)
 *!
 *$ ASSUMPTIONS:
 *!
 *$ REMARKS:     (local)
 *!
 *!**************************************************************************
 *!*/

extern void mrvl_sai_utl_dlist_move_to_head (

    /*!     INPUTS:             */

    mrvl_sai_utl_dlist_elem_STC *header_PTR,
    mrvl_sai_utl_dlist_elem_STC *elem_PTR
     /*!     INPUTS / OUTPUTS:   */

    /*!     OUTPUTS:            */

)
{
/*!*************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!*************************************************************************/

/*!*************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!*************************************************************************/
    mrvl_sai_utl_dlist_prv_remove(elem_PTR->prev_PTR, elem_PTR->next_PTR);
    mrvl_sai_utl_dlist_prv_insert(header_PTR,header_PTR->next_PTR,elem_PTR);
}
/*$ END OF mrvl_sai_utl_dlist_move_to_head */


/*!**************************************************RND Template version 4.1
 *!     F U N C T I O N   D E S I G N   &   I M P L E M E N T A T I O N
 *!==========================================================================
 *$ TITLE: move element to the list tail.
 *!--------------------------------------------------------------------------
 *$ FILENAME: R:\routers\src\lib\dlist\src\move_to_tail.cc
 *!--------------------------------------------------------------------------
 *$ SYSTEM, SUBSYSTEM: LIB
 *!--------------------------------------------------------------------------
 *$ AUTHORS: RutyS,User
 *!--------------------------------------------------------------------------
 *$ LATEST UPDATE: 11-Mar-2009, 2:26 PM CREATION DATE: 08-Sep-96
 *!**************************************************************************
 *!
 *!**************************************************************************
 *!
 *$ FUNCTION: mrvl_sai_utl_dlist_move_to_tail
 *!
 *$ GENERAL DESCRIPTION:
 *!
 *$ RETURNS:
 *!
 *$ ALGORITHM:   (local)
 *!
 *$ ASSUMPTIONS:
 *!
 *$ REMARKS:     (local)
 *!
 *!**************************************************************************
 *!*/

extern void mrvl_sai_utl_dlist_move_to_tail (

    /*!     INPUTS:             */

    mrvl_sai_utl_dlist_elem_STC *header_PTR,
    mrvl_sai_utl_dlist_elem_STC *elem_PTR
     /*!     INPUTS / OUTPUTS:   */

    /*!     OUTPUTS:            */

)
{
/*!*************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!*************************************************************************/

/*!*************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!*************************************************************************/
    mrvl_sai_utl_dlist_prv_remove(elem_PTR->prev_PTR, elem_PTR->next_PTR);
    mrvl_sai_utl_dlist_prv_insert(header_PTR->prev_PTR,header_PTR,elem_PTR);
}
/*$ END OF mrvl_sai_utl_dlist_move_to_tail */


/*!**************************************************RND Template version 4.1
 *!     F U N C T I O N   D E S I G N   &   I M P L E M E N T A T I O N
 *!==========================================================================
 *$ TITLE: get next element in the list.
 *!--------------------------------------------------------------------------
 *$ FILENAME: R:\routers\src\lib\dlist\src\get_next.cc
 *!--------------------------------------------------------------------------
 *$ SYSTEM, SUBSYSTEM: LIB
 *!--------------------------------------------------------------------------
 *$ AUTHORS: RutyS,User
 *!--------------------------------------------------------------------------
 *$ LATEST UPDATE: 11-Mar-2009, 2:34 PM CREATION DATE: 08-Sep-96
 *!**************************************************************************
 *!
 *!**************************************************************************
 *!
 *$ FUNCTION: mrvl_sai_utl_dlist_get_next
 *!
 *$ GENERAL DESCRIPTION:
 *!
 *$ RETURNS:
 *!
 *$ ALGORITHM:   (local)
 *!
 *$ ASSUMPTIONS:
 *!
 *$ REMARKS:     (local)
 *!
 *!**************************************************************************
 *!*/

extern mrvl_sai_utl_dlist_elem_STC * mrvl_sai_utl_dlist_get_next (

    /*!     INPUTS:             */

    mrvl_sai_utl_dlist_elem_STC *elem_PTR

    /*!     INPUTS / OUTPUTS:   */

    /*!     OUTPUTS:            */

)
{
/*!*************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!*************************************************************************/

/*!*************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!*************************************************************************/
    return elem_PTR->next_PTR;
}
/*$ END OF mrvl_sai_utl_dlist_get_next */


/*!**************************************************RND Template version 4.1
 *!     F U N C T I O N   D E S I G N   &   I M P L E M E N T A T I O N
 *!==========================================================================
 *$ TITLE: get previous element in the list.
 *!--------------------------------------------------------------------------
 *$ FILENAME: R:\routers\src\lib\dlist\src\get_prev.cc
 *!--------------------------------------------------------------------------
 *$ SYSTEM, SUBSYSTEM: LIB
 *!--------------------------------------------------------------------------
 *$ AUTHORS: RutyS,User
 *!--------------------------------------------------------------------------
 *$ LATEST UPDATE: 11-Mar-2009, 2:34 PM CREATION DATE: 08-Sep-96
 *!**************************************************************************
 *!
 *!**************************************************************************
 *!
 *$ FUNCTION: mrvl_sai_utl_dlist_get_prev
 *!
 *$ GENERAL DESCRIPTION:
 *!
 *$ RETURNS:
 *!
 *$ ALGORITHM:   (local)
 *!
 *$ ASSUMPTIONS:
 *!
 *$ REMARKS:     (local)
 *!
 *!**************************************************************************
 *!*/

extern mrvl_sai_utl_dlist_elem_STC * mrvl_sai_utl_dlist_get_prev (

    /*!     INPUTS:             */

    mrvl_sai_utl_dlist_elem_STC *elem_PTR

    /*!     INPUTS / OUTPUTS:   */

    /*!     OUTPUTS:            */

)
{
/*!*************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!*************************************************************************/

/*!*************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!*************************************************************************/
    return elem_PTR->prev_PTR;
}
/*$ END OF mrvl_sai_utl_dlist_get_prev */


/*!**************************************************RND Template version 4.1
 *!     F U N C T I O N   D E S I G N   &   I M P L E M E N T A T I O N
 *!==========================================================================
 *$ TITLE: get first element in the list.
 *!--------------------------------------------------------------------------
 *$ FILENAME: R:\routers\src\lib\dlist\src\get_first.cc
 *!--------------------------------------------------------------------------
 *$ SYSTEM, SUBSYSTEM: LIB
 *!--------------------------------------------------------------------------
 *$ AUTHORS: RutyS,User
 *!--------------------------------------------------------------------------
 *$ LATEST UPDATE: 11-Mar-2009, 2:35 PM CREATION DATE: 08-Sep-96
 *!**************************************************************************
 *!
 *!**************************************************************************
 *!
 *$ FUNCTION: LISTP_get_first
 *!
 *$ GENERAL DESCRIPTION:
 *!
 *$ RETURNS:
 *!
 *$ ALGORITHM:   (local)
 *!
 *$ ASSUMPTIONS:
 *!
 *$ REMARKS:     (local)
 *!
 *!**************************************************************************
 *!*/

extern mrvl_sai_utl_dlist_elem_STC * mrvl_sai_utl_dlist_get_first (

    /*!     INPUTS:             */

    mrvl_sai_utl_dlist_elem_STC *header_PTR

    /*!     INPUTS / OUTPUTS:   */

    /*!     OUTPUTS:            */

)
{
/*!*************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!*************************************************************************/

/*!*************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!*************************************************************************/
    return header_PTR->next_PTR;
}
/*$ END OF mrvl_sai_utl_dlist_get_first */


/*!**************************************************RND Template version 4.1
 *!     F U N C T I O N   D E S I G N   &   I M P L E M E N T A T I O N
 *!==========================================================================
 *$ TITLE: get last element in the list.
 *!--------------------------------------------------------------------------
 *$ FILENAME: R:\routers\src\lib\dlist\src\get_last.cc
 *!--------------------------------------------------------------------------
 *$ SYSTEM, SUBSYSTEM: LIB
 *!--------------------------------------------------------------------------
 *$ AUTHORS: RutyS,User
 *!--------------------------------------------------------------------------
 *$ LATEST UPDATE: 11-Mar-2009, 2:35 PM CREATION DATE: 08-Sep-96
 *!**************************************************************************
 *!
 *!**************************************************************************
 *!
 *$ FUNCTION: mrvl_sai_utl_dlist_get_last
 *!
 *$ GENERAL DESCRIPTION:
 *!
 *$ RETURNS:
 *!
 *$ ALGORITHM:   (local)
 *!
 *$ ASSUMPTIONS:
 *!
 *$ REMARKS:     (local)
 *!
 *!**************************************************************************
 *!*/

extern mrvl_sai_utl_dlist_elem_STC * mrvl_sai_utl_dlist_get_last (

    /*!     INPUTS:             */

    mrvl_sai_utl_dlist_elem_STC *header_PTR

    /*!     INPUTS / OUTPUTS:   */

    /*!     OUTPUTS:            */

)
{
/*!*************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!*************************************************************************/

/*!*************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!*************************************************************************/
    return header_PTR->prev_PTR;
}
/*$ END OF mrvl_sai_utl_dlist_get_last */

/* look for element with data "data_to_find"
    if element was found: return elemnt and set "found" to true.
    if element was not found return th next element so we can add before the new one*/
extern mrvl_sai_utl_dlist_elem_STC * mrvl_sai_utl_dlist_find_sorted_element (

    /*!     INPUTS:             */
    mrvl_sai_utl_dlist_elem_STC *header_PTR,
	uint32_t		list_offset_in_header,	
	uint32_t		data_offset_in_header,
	uint32_t		data_to_find,
	uint32_t		*found
    /*!     INPUTS / OUTPUTS:   */
    /*!     OUTPUTS:            */
)
{
/*!*************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!*************************************************************************/
	mrvl_sai_utl_dlist_elem_STC *prev_PTR;
	uint32_t		element_data;
/*!*************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!*************************************************************************/
	prev_PTR = header_PTR;
	*found = 0;
	while(prev_PTR->next_PTR != header_PTR){
		element_data = *(uint32_t *)((uint8_t*)prev_PTR->next_PTR -list_offset_in_header + data_offset_in_header);  
		if (data_to_find > element_data) {
			prev_PTR = prev_PTR->next_PTR;
		} else if (data_to_find == element_data) {
			*found = 1;
			break;
		} else {
			break;
		}
	}
	return prev_PTR->next_PTR;
}

/* insert element with data to a sorted list from small to big */
extern void mrvl_sai_utl_dlist_insert_bigger_element (

    /*!     INPUTS:             */
    mrvl_sai_utl_dlist_elem_STC *header_PTR,
    mrvl_sai_utl_dlist_elem_STC *new_PTR,
	uint32_t		list_offset_in_header,	
	uint32_t		data_offset_in_header,
	uint32_t		new_data
    /*!     INPUTS / OUTPUTS:   */
    /*!     OUTPUTS:            */
)
{	
/*!*************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!*************************************************************************/
	mrvl_sai_utl_dlist_elem_STC *prev_PTR;
	uint32_t		element_data;
/*!*************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!*************************************************************************/
	prev_PTR = header_PTR;
	while(prev_PTR->next_PTR != header_PTR){
		element_data = *(uint32_t *)((uint8_t*)prev_PTR->next_PTR -list_offset_in_header + data_offset_in_header);  
		if (new_data < element_data) {
			*(uint32_t *)((uint8_t*)new_PTR -list_offset_in_header + data_offset_in_header) = new_data;
			mrvl_sai_utl_dlist_prv_insert(prev_PTR,prev_PTR->next_PTR,new_PTR);
			*(uint32_t *)((uint8_t*)new_PTR->next_PTR -list_offset_in_header + data_offset_in_header) = element_data - new_data;
			return;
		} else {
			prev_PTR = prev_PTR->next_PTR;
			new_data = new_data - element_data;
		}
	}
	/* reach the end of the list */
	*(uint32_t *)((uint8_t*)new_PTR -list_offset_in_header + data_offset_in_header) = new_data;
	mrvl_sai_utl_dlist_prv_insert(prev_PTR,prev_PTR->next_PTR,new_PTR);	
	
}
/*$ END OF mrvl_sai_utl_dlist_insert_after */


/*!**************************************************RND Template version 4.1
 *!     F U N C T I O N   D E S I G N   &   I M P L E M E N T A T I O N
 *!==========================================================================
 *$ TITLE: check if the list is empty.
 *!--------------------------------------------------------------------------
 *$ FILENAME: R:\routers\src\lib\dlist\src\is_empty.cc
 *!--------------------------------------------------------------------------
 *$ SYSTEM, SUBSYSTEM: LIB
 *!--------------------------------------------------------------------------
 *$ AUTHORS: RutyS,User
 *!--------------------------------------------------------------------------
 *$ LATEST UPDATE: 11-Mar-2009, 2:27 PM CREATION DATE: 08-Sep-96
 *!**************************************************************************
 *!
 *!**************************************************************************
 *!
 *$ FUNCTION: LISTG_is_empty
 *!
 *$ GENERAL DESCRIPTION:
 *!
 *$ RETURNS:
 *!
 *$ ALGORITHM:   (local)
 *!
 *$ ASSUMPTIONS:
 *!
 *$ REMARKS:     (local)
 *!
 *!**************************************************************************
 *!*/

extern int mrvl_sai_utl_dlist_is_last (

    /*!     INPUTS:             */

    mrvl_sai_utl_dlist_elem_STC *header_PTR,
	mrvl_sai_utl_dlist_elem_STC *elem_PTR
    /*!     INPUTS / OUTPUTS:   */

    /*!     OUTPUTS:            */

)
{
/*!*************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!*************************************************************************/

/*!*************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!*************************************************************************/

    if( elem_PTR->next_PTR == header_PTR)
        return 1;
    return 0;


}
/*$ END OF mrvl_sai_utl_dlist_is_empty */




/*$ END OF LIST */


