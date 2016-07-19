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

#include "mrvl_sai.h"

#ifndef exp_lib_dlist_h_INCLUDED
#define exp_lib_dlist_h_INCLUDED
/*!**************************************************RND Template version 4.1 
*!                      P A C K A G E       S P E C I F I C A T I O N
*!==========================================================================
*$ TITLE: Double linked list utilities
*!--------------------------------------------------------------------------
*$ FILENAME: R:\routers\src\lib\dlist\dlist.c
*!--------------------------------------------------------------------------
*$ SYSTEM, SUBSYSTEM: RS
*!--------------------------------------------------------------------------
*$ AUTHORS: RutyS,User
*!--------------------------------------------------------------------------
*$ LATEST UPDATE: 12-Mar-2009, 11:12 AM CREATION DATE: 08-Jan-95
*!************************************************************************** 
*!
*!************************************************************************** 
*!
*$ GENERAL DESCRIPTION:
*! This package is a double linked list.
*$
*! PROCESS AND ALGORITHM: (local)
*!
*$ PACKAGE GLOBAL SERVICES:
*!
*$ PACKAGE LOCAL SERVICES:  (local)
*!
*$ PACKAGE USAGE:
*!
*$ ASSUMPTIONS:
*!
*$ SIDE EFFECTS:
*!
*$ RELATED DOCUMENTS:     (local)
*!
*$ REMARKS:               (local)
*!
*!************************************************************************** 
*!*/


/*===========================================================================*/
/*!**************************************************RND Template version 4.1
 *!          S T R U C T U R E   T Y P E   D E F I N I T I O N
 *!==========================================================================
 *$ TITLE: list structure
 *!--------------------------------------------------------------------------
 *$ FILENAME: R:\routers\src\lib\dlist\inc\dlist.stc
 *!--------------------------------------------------------------------------
 *$ SYSTEM, SUBSYSTEM: lib
 *!--------------------------------------------------------------------------
 *$ AUTHORS: RutyS,User
 *!--------------------------------------------------------------------------
 *$ LATEST UPDATE: 10-Mar-2009, 10:10 AM CREATION DATE: 08-Sep-96
 *!**************************************************************************
 *!
 *!**************************************************************************
 *!
 *$ GENERAL DESCRIPTION:
 *! A  doubly-linked list.
 *! The list counts the number of elements (kept in the list's header).
 *!
 *!
 *$ INCLUDE REQUIRED:
 *!
 *$ REMARKS:
 *!
 *!**************************************************************************
 *!*/

/* an element in the list */
typedef struct mrvl_sai_utl_dlist_elem_STCT
{
    struct mrvl_sai_utl_dlist_elem_STCT *next_PTR;
    struct mrvl_sai_utl_dlist_elem_STCT *prev_PTR;
} mrvl_sai_utl_dlist_elem_STC;



/*===========================================================================*/
/*!**************************************************************************
 *$              PUBLIC VARIABLE DEFINITIONS (EXPORT)
 *!**************************************************************************
 *!*/
/*!**************************************************************************
 *$              PUBLIC FUNCTION DEFINITIONS (EXPORT)
 *!**************************************************************************
 *!*/
/*===========================================================================*/
/*!**************************************************RND Template version 4.1
 *!     F U N C T I O N    P R O T O T Y P E    D E F I N I T I O N
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

);
/*$ END OF  mrvl_sai_utl_dlist_init */

/*===========================================================================*/
/*===========================================================================*/
/*!**************************************************RND Template version 4.1
 *!     F U N C T I O N    P R O T O T Y P E    D E F I N I T I O N
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

);
/*$ END OF  mrvl_sai_utl_dlist_is_empty */

/*===========================================================================*/
/*===========================================================================*/
/*!**************************************************RND Template version 4.1
 *!     F U N C T I O N    P R O T O T Y P E    D E F I N I T I O N
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

);
/*$ END OF  mrvl_sai_utl_dlist_insert_after */

/*===========================================================================*/
/*===========================================================================*/
/*!**************************************************RND Template version 4.1
 *!     F U N C T I O N    P R O T O T Y P E    D E F I N I T I O N
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

);
/*$ END OF  mrvl_sai_utl_dlist_insert_before */

/*===========================================================================*/
/*===========================================================================*/
/*!**************************************************RND Template version 4.1
 *!     F U N C T I O N    P R O T O T Y P E    D E F I N I T I O N
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

);
/*$ END OF  mrvl_sai_utl_dlist_insert_to_head */

/*===========================================================================*/
/*===========================================================================*/
/*!**************************************************RND Template version 4.1
 *!     F U N C T I O N    P R O T O T Y P E    D E F I N I T I O N
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

);
/*$ END OF  mrvl_sai_utl_dlist_insert_to_tail */

/*===========================================================================*/
/*===========================================================================*/
/*!**************************************************RND Template version 4.1
 *!     F U N C T I O N    P R O T O T Y P E    D E F I N I T I O N
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

);
/*$ END OF  mrvl_sai_utl_dlist_remove */

/*===========================================================================*/
/*===========================================================================*/
/*!**************************************************RND Template version 4.1
 *!     F U N C T I O N    P R O T O T Y P E    D E F I N I T I O N
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

);
/*$ END OF  mrvl_sai_utl_dlist_remove_from_head */

/*===========================================================================*/
/*===========================================================================*/
/*!**************************************************RND Template version 4.1
 *!     F U N C T I O N    P R O T O T Y P E    D E F I N I T I O N
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

);
/*$ END OF  mrvl_sai_utl_dlist_remove_from_tail */

/*===========================================================================*/
/*===========================================================================*/
/*!**************************************************RND Template version 4.1
 *!     F U N C T I O N    P R O T O T Y P E    D E F I N I T I O N
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

);
/*$ END OF  mrvl_sai_utl_dlist_move_to_head */

/*===========================================================================*/
/*===========================================================================*/
/*!**************************************************RND Template version 4.1
 *!     F U N C T I O N    P R O T O T Y P E    D E F I N I T I O N
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

);
/*$ END OF  mrvl_sai_utl_dlist_move_to_tail */

/*===========================================================================*/
/*===========================================================================*/
/*!**************************************************RND Template version 4.1
 *!     F U N C T I O N    P R O T O T Y P E    D E F I N I T I O N
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

);
/*$ END OF  mrvl_sai_utl_dlist_get_next */

/*===========================================================================*/
/*===========================================================================*/
/*!**************************************************RND Template version 4.1
 *!     F U N C T I O N    P R O T O T Y P E    D E F I N I T I O N
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

);
/*$ END OF  mrvl_sai_utl_dlist_get_prev */

/*===========================================================================*/
/*===========================================================================*/
/*!**************************************************RND Template version 4.1
 *!     F U N C T I O N    P R O T O T Y P E    D E F I N I T I O N
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

);
/*$ END OF  mrvl_sai_utl_dlist_get_first */

/*===========================================================================*/
/*===========================================================================*/
/*!**************************************************RND Template version 4.1
 *!     F U N C T I O N    P R O T O T Y P E    D E F I N I T I O N
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

);
/*$ END OF  mrvl_sai_utl_dlist_get_last */


extern void mrvl_sai_utl_dlist_insert_bigger_element (

    /*!     INPUTS:             */
    mrvl_sai_utl_dlist_elem_STC *header_PTR,
    mrvl_sai_utl_dlist_elem_STC *new_PTR,
	uint32_t		list_offset_in_header,	
	uint32_t		data_offset_in_header,
	uint32_t		new_data
    /*!     INPUTS / OUTPUTS:   */
    /*!     OUTPUTS:            */
);

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
);

extern int mrvl_sai_utl_dlist_is_last (
    /*!     INPUTS:             */
    mrvl_sai_utl_dlist_elem_STC *header_PTR,
	mrvl_sai_utl_dlist_elem_STC *elem_PTR
);
/*===========================================================================*/
/*===========================================================================*/

/*===========================================================================*/

#endif
/*$ END OF dlist */

