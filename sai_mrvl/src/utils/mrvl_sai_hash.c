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


#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <string.h>

#include "utils/mrvl_sai_hash.h"

static uint32_t   mrvl_sai_utl_prv_calc_hash_new_func    (void  *id,  
                                               void  *key_ptr);
/*static FPA_STATUS   mrvl_sai_utl_prv_allocate_more_entries (void  *id);*/
static uint32_t   mrvl_sai_utl_prv_calc_hash_func        (void  *id,
                                               void  *key_ptr);
static uint32_t   mrvl_sai_utl_prv_calc_hash_16bit_func  (void  *id,
                                               void  *key_ptr);
static void*    mrvl_sai_utl_prv_get_pre_sorted        (void  *id,
                                               void  *key_ptr);
static void     mrvl_sai_utl_prv_refresh_sorted_header  (void  *id);

static void     mrvl_sai_utl_prv_find_sorted_header_entry(
                            void                    *id,
                            void                    *key_ptr,
                            mrvl_sai_utl_prv_hash_entry_STC  **low_sorted_ptr);

static mrvl_sai_utl_prv_hash_entry_STC* mrvl_sai_utl_prv_search_by_key(
                            mrvl_sai_utl_prv_hash_id_STC    *hash_id,
                            void                   *key_ptr);

static mrvl_sai_utl_prv_hash_entry_STC* mrvl_sai_utl_prv_insertion_time_delete(
                            mrvl_sai_utl_prv_hash_entry_STC *entry_ptr,
                            mrvl_sai_utl_prv_hash_id_STC    *hash_id);

/*static mrvl_sai_utl_prv_hash_entry_STC* mrvl_sai_utl_prv_insertion_time_insert(
                            mrvl_sai_utl_prv_hash_entry_STC *new_entry_ptr,
                            mrvl_sai_utl_prv_hash_id_STC    *hash_id);*/

#define mrvl_sai_utl_prv_mix_MAC(a,b,c) \
{\
  a -= b; a -= c; a ^= (c>>13); \
  b -= c; b -= a; b ^= (a<<8); \
  c -= a; c -= b; c ^= (b>>13); \
  a -= b; a -= c; a ^= (c>>12);  \
  b -= c; b -= a; b ^= (a<<16); \
  c -= a; c -= b; c ^= (b>>5); \
  a -= b; a -= c; a ^= (c>>3);  \
  b -= c; b -= a; b ^= (a<<10); \
  c -= a; c -= b; c ^= (b>>15);\
}

#ifndef MAX
#define MAX(a,b) (((a)>(b))?(a):(b))
#define MIN(a,b) (((a)<(b))?(a):(b))
#endif

#define mrvl_sai_utl_VALID_PTR(ptr)  (ptr != 0)
#define mrvl_sai_utl_CHECK_PTR(ptr, code, app_id, severity)  do {                        \
    if (!mrvl_sai_utl_VALID_PTR(ptr))                                                    \
    {                                                                           \
        MRVL_SAI_TRACE_MAC(((app_id), (severity), "== HAL == %s:%d: invalid ptr %p\n",    \
                  __func__, __LINE__, (void *) (ptr)));                      \
        return code;                                                            \
    }                                                                           \
} while (0)


/*------------------------------------------------------------------------------
NAME
    mrvl_sai_utl_prv_allocate_more_entries

DESCRIPTION
    Allocate more free entries (according to required parameters)

INPUTS
    id  - Hash table identification

OUTPUTS
    None

RETURN VALUE
    mrvl_sai_utl_NO_FREE_ENTRY
    mrvl_sai_utl_SUCCESS

PROCESSING
     
------------------------------------------------------------------------------*/
static sai_status_t mrvl_sai_utl_prv_allocate_more_entries (void  *id)
{
    mrvl_sai_utl_prv_hash_id_STC     *hash_id = NULL;
    mrvl_sai_utl_prv_hash_entry_STC  *entry_ptr = NULL, 
                            *free_entry_ptr = NULL, 
                            *next = NULL;
    int                     i;
    uint32_t                  total_entry_size;
    uint32_t                  alloc_portion_size, alloc_portion;

    /*MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_DBG3,
                 "%s:%d: HASH entries allocation \n",
                 __func__, __LINE__));*/

    hash_id = (mrvl_sai_utl_prv_hash_id_STC *)id;
    
    if (hash_id->num_used_entries >= hash_id->max_num_of_entries){
        MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_ERROR, 
                 "%s:%d: full table: %d %d \n", 
                 __func__, __LINE__, hash_id->num_used_entries,
                 hash_id->max_num_of_entries));
        return SAI_STATUS_TABLE_FULL;
    }
    
    alloc_portion = hash_id->max_num_of_entries - hash_id->num_used_entries;
    
    if (alloc_portion >= hash_id->alloc_portion)
    {
        alloc_portion = hash_id->alloc_portion;
        alloc_portion_size = hash_id->alloc_portion_size;
    }
    else
    {
        alloc_portion_size = alloc_portion * hash_id->total_entry_size;
    }
    /*TODO cpssOsMalloc*/
    alloc_portion_size = ((alloc_portion_size) + (ALIGNMENT_SIZE-1) ) & (~(ALIGNMENT_SIZE-1));
    entry_ptr = (mrvl_sai_utl_prv_hash_entry_STC *)malloc(alloc_portion_size);
    
    if (entry_ptr == NULL){
        MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_ERROR, 
                 "%s:%d: allocation failure \n", __func__, __LINE__));
        return SAI_STATUS_TABLE_FULL;
    }
    
    total_entry_size = hash_id->total_entry_size;
    
    next = entry_ptr;
    
    /* Initialize all HASH entries. */
    for (i = 1; i < (int) alloc_portion; i++)
    {
        free_entry_ptr = next;
    
        next = (mrvl_sai_utl_prv_hash_entry_STC*)(((uint8_t*)free_entry_ptr) + total_entry_size);
    
        /* Set mrvl_sai_utl_prv_free_entry_bit_CNS only on free entries structure. */
        /*  This LSB can not be set in a real pointer. */
        free_entry_ptr->next_ptr[hash_id->next_index] =
            (mrvl_sai_utl_prv_hash_entry_STC*)((PTR_TO_INT)next | mrvl_sai_utl_prv_free_entry_bit_CNS);
    
        /* Update the sorted field only for sorted data base. */
        /* For unsorted data base the fiels does not exist. */
        /* This initialize is not needed , only for debug resons. */
#if (HASH_DEBUG_FLAG==1)
        if (hash_id->sort_db_flag)
            free_entry_ptr->next_ptr[hash_id->next_by_key_index] =
                (mrvl_sai_utl_prv_hash_entry_STC*)(-1);
#endif
    }
    
    free_entry_ptr = next;
    
    hash_id->last_free_entry->next_ptr[hash_id->next_index] =
        (mrvl_sai_utl_prv_hash_entry_STC*)((PTR_TO_INT)entry_ptr | mrvl_sai_utl_prv_free_entry_bit_CNS);
    free_entry_ptr->next_ptr[hash_id->next_index] =
        (mrvl_sai_utl_prv_hash_entry_STC*)mrvl_sai_utl_prv_free_entry_bit_CNS;
    
    hash_id->last_free_entry = free_entry_ptr;
    
#if (HASH_DEBUG_FLAG==1)
    /* This initialize is not needed , only for debug resons. */
    if (hash_id->sort_db_flag)
        free_entry_ptr->next_ptr[hash_id->next_by_key_index] =
            (mrvl_sai_utl_prv_hash_entry_STC*)(-1);
#endif
    
    hash_id->num_free_entries += alloc_portion;
    
    /*MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_DBG3,
                  "%s:%d: exit function (success)\n",
                  __func__, __LINE__));*/

    return SAI_STATUS_SUCCESS;
}
/*------------------------------------------------------------------------------
NAME
    mrvl_sai_utl_prv_calc_hash_func

DESCRIPTION
    There are two cases:
   1. The key length is 2/4/6/8 and the table size is less then 0x10000, then
   use the old hash function mrvl_sai_utl_prv_calc_hash_16bit_func,that was designed to
   handle this specific values.
   2.Otherwise use mrvl_sai_utl_prv_calc_hash_new_func that handles all values.

INPUTS
    id      - Hash table identification
    key_ptr - Element key

OUTPUTS
    None

RETURN VALUE
    calculated hash value

PROCESSING
     
------------------------------------------------------------------------------*/
static uint32_t mrvl_sai_utl_prv_calc_hash_func( void  *id,
                                      void  *key_ptr)
{
    mrvl_sai_utl_prv_hash_id_STC*    hash_id = NULL;

    /*MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_DBG3,
                 "%s:%d: HASH calculation \n",
                 __func__, __LINE__));*/

    hash_id = (mrvl_sai_utl_prv_hash_id_STC*)id;
    
    if (hash_id->header_size > 0x10000 ||
        ( hash_id->key_len != 2 && hash_id->key_len != 4 &&
          hash_id->key_len != 6 && hash_id->key_len != 8 )) {

       /* MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_DBG3,
                      "%s:%d: exit function (success)\n",
                      __func__, __LINE__));*/

        return mrvl_sai_utl_prv_calc_hash_new_func(id, key_ptr); /*new func*/
    }
    else {
      /*  MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_DBG3,
                      "%s:%d: exit function (success)\n",
                      __func__, __LINE__));*/

        return mrvl_sai_utl_prv_calc_hash_16bit_func(id, key_ptr);/*old func*/
    }
}

/*------------------------------------------------------------------------------
NAME
    mrvl_sai_utl_prv_calc_hash_16bit_func

DESCRIPTION
    Routine calculate HASH function according to KEY SIZE and HEADER SIZE.
    Uses 32 bit index.

INPUTS
    id      - Hash table identification
    key_ptr - Element key

OUTPUTS
    None

RETURN VALUE
    calculated hash value

PROCESSING
     
------------------------------------------------------------------------------*/
static uint32_t mrvl_sai_utl_prv_calc_hash_16bit_func (void  *id,
                                            void  *key_ptr)
{
    mrvl_sai_utl_prv_hash_id_STC *hash_id = NULL;
    uint32_t              temp = 0;

    /*MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_DBG3,
                 "%s:%d: HASH calculation 16 \n",
                 __func__, __LINE__));*/

    hash_id = (mrvl_sai_utl_prv_hash_id_STC *) id;
    
    switch (hash_id->key_len)
    {
    case 2:
        /* index = byte1=b2 , byte0=b2^b1 */
        temp = *(uint16_t*)key_ptr;
        temp = temp ^ (temp>>8);

       /* MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_DBG3,
                      "%s:%d: exit function (success)\n",
                      __func__, __LINE__));*/

        return (temp & hash_id->header_mask);
    
    case 4:
        /* index = byte1=b4^b2 , byte0=b4^b3^b2^b1 */
        temp = *(uint16_t*)key_ptr ^ *((uint16_t*)key_ptr + 1);
        temp = temp ^ (temp>>8);

        /*MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_DBG3,
                      "%s:%d: exit function (success)\n",
                      __func__, __LINE__));*/

        return (temp & hash_id->header_mask);
    
    case 6:
        /* index = byte1=b6^b4^b2 , byte0=b5^b3^b1 */
        temp = *(uint16_t*)key_ptr ^ *((uint16_t*)key_ptr + 1) ^
               *((uint16_t*)key_ptr + 2);

        /*MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_DBG3,
                      "%s:%d: exit function (success)\n",
                      __func__, __LINE__));*/

        return (temp & hash_id->header_mask);
    
    case 8:
        /* index = byte1=b8^b4^b2 , byte0=b7^b3^b1 */
        temp = *(uint16_t*)key_ptr ^ *((uint16_t*)key_ptr + 1) ^
               *((uint16_t*)key_ptr + 3);

       /* MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_DBG3,
                      "%s:%d: exit function (success)\n",
                      __func__, __LINE__));*/

        return (temp & hash_id->header_mask);
    
    default:
        MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_ERROR, 
                 "%s:%d: invalid key length: %d \n", 
                 __func__, __LINE__, hash_id->key_len));
        break;
    }
    
    return 0;
}

/*------------------------------------------------------------------------------
NAME
    mrvl_sai_utl_prv_calc_hash_new_func

DESCRIPTION
    Routine calculate HASH function according to KEY SIZE and HEADER SIZE.
    Uses 32 bit index.

INPUTS
    id      - Hash table identification
    key_ptr - Element key

OUTPUTS
    None

RETURN VALUE
    calculated hash value

PROCESSING
    This function avoids collision very efficiently ,even if the keys differ
    in only a small set of bits/bytes.It works with all sizes of keys,sizes
    of tables and it is "fast" compared to other hash function.

------------------------------------------------------------------------------*/
static uint32_t mrvl_sai_utl_prv_calc_hash_new_func(void  *id, 
                                         void  *key_ptr)
{
    mrvl_sai_utl_prv_hash_id_STC     *hash_id = NULL;
    uint32_t                  len;
    uint32_t                  a = 0x9e3779b9;/*golden ratio*/
    uint32_t                  b = 0x9e3779b9;
    uint32_t                  c = 0;
    uint8_t                   *key = NULL;

    /*MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_DBG3,
                 "%s:%d: HASH new calculation \n",
                 __func__, __LINE__));*/

    hash_id = (mrvl_sai_utl_prv_hash_id_STC *)id;
    len     = hash_id->key_len;
    key     = (uint8_t *)key_ptr;
    
    while (len >= 12){
      a += (key[0] +((uint32_t)key[1]<<8) +((uint32_t)key[2]<<16) +((uint32_t)key[3]<<24));
      b += (key[4] +((uint32_t)key[5]<<8) +((uint32_t)key[6]<<16) +((uint32_t)key[7]<<24));
      c += (key[8] +((uint32_t)key[9]<<8) +((uint32_t)key[10]<<16)+((uint32_t)key[11]<<24));
      mrvl_sai_utl_prv_mix_MAC(a,b,c);
      key += 12; len -= 12;
   }

   /*------------------------------------- handle the last 11 bytes */
   c += len;
   switch(len){              /* all the case statements fall through */
   case 11: c+=((uint32_t)key[10]<<24);
   case 10: c+=((uint32_t)key[9]<<16);
   case 9 : c+=((uint32_t)key[8]<<8);
                            /* the first byte of c is reserved for the length */
   case 8 : b+=((uint32_t)key[7]<<24);
   case 7 : b+=((uint32_t)key[6]<<16);
   case 6 : b+=((uint32_t)key[5]<<8);
   case 5 : b+=key[4];
   case 4 : a+=((uint32_t)key[3]<<24);
   case 3 : a+=((uint32_t)key[2]<<16);
   case 2 : a+=((uint32_t)key[1]<<8);
   case 1 : a+=key[0];
   /* case 0: nothing left to add */
   }
   mrvl_sai_utl_prv_mix_MAC(a,b,c);
   /*-------------------------------------------- report the result */
   c &= hash_id->header_mask;

   /*MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_DBG3,
                 "%s:%d: exit function (success)\n",
                 __func__, __LINE__));*/

   return c;
}

/*------------------------------------------------------------------------------
NAME
    mrvl_sai_utl_CleanHash

DESCRIPTION
    Routine delete all HASH table entries

INPUTS
    id  - Hash table identification

OUTPUTS
    None

RETURN VALUE
    mrvl_sai_utl_SUCCESS ot FPA_ERROR

PROCESSING
     
------------------------------------------------------------------------------*/
sai_status_t mrvl_sai_utl_CleanHash (void  *id)
{
    mrvl_sai_utl_prv_hash_id_STC     *hash_id = NULL; 
    mrvl_sai_utl_prv_hash_entry_STC  **header_ptr = NULL;  
    mrvl_sai_utl_prv_hash_entry_STC  *entry_ptr = NULL, *prev_entry_ptr = NULL;  
    int                     i;
    uint32_t                  free_entries;

    MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_DBG3,
                 "%s:%d: HASH clean \n",
                 __func__, __LINE__));

    mrvl_sai_utl_CHECK_PTR(id, FPA_ERROR, APPID_GENERAL, SEVERITY_LEVEL_ERROR);

    hash_id = (mrvl_sai_utl_prv_hash_id_STC *)id;
    
    /* Check input. */
    if (hash_id->debug != hash_id){
        MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_ERROR, 
                 "%s:%d: invalid parameter: %p \n", 
                 __func__, __LINE__, hash_id->debug));
        return SAI_STATUS_SUCCESS;
    }
    
    free_entries    = 0;
    prev_entry_ptr  = hash_id->last_free_entry;
    
    /* Can't be NULL because of free_entries_margin. */
    if (prev_entry_ptr == NULL){
        MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_ERROR, 
                 "%s:%d: NULL pointer \n", __func__, __LINE__));
        return SAI_STATUS_FAILURE;
    }
    
    /* Initialize HASH header. */
    header_ptr = hash_id->header_ptr;
    for (i = 0 ; i < (int) hash_id->header_size; i++){
        entry_ptr    = *header_ptr;
        *header_ptr  = NULL;
        header_ptr++;

        while (entry_ptr != NULL){
            prev_entry_ptr->next_ptr[hash_id->next_index] =
              (mrvl_sai_utl_prv_hash_entry_STC*)((PTR_TO_INT)entry_ptr | mrvl_sai_utl_prv_free_entry_bit_CNS) ;
            prev_entry_ptr = entry_ptr;
            entry_ptr = entry_ptr->next_ptr[hash_id->next_index];
            free_entries++;
        }
    }
    
    /* Initialize all HASH entries. */
    hash_id->last_free_entry    = prev_entry_ptr;
    hash_id->last_free_entry->next_ptr[hash_id->next_index] =
      (mrvl_sai_utl_prv_hash_entry_STC*)mrvl_sai_utl_prv_free_entry_bit_CNS;
    hash_id->first_sorted_entry = NULL;
    hash_id->newest             = NULL;
    hash_id->oldest             = NULL;
    hash_id->num_free_entries  += free_entries;
    hash_id->num_used_entries   = 0;
    if (hash_id->sorted_header_size != 0){
        for(i = sizeof(mrvl_sai_utl_prv_max_key_TYP); 
            i < (int) ((hash_id->sorted_header_size - 1) * sizeof(mrvl_sai_utl_prv_max_key_TYP));
            i = i + 4) {
            *(uint32_t*)(((uint8_t*)hash_id->sorted_header_ptr) + i) = 0;
        }
    
        hash_id->refresh_treshold_ctr   = 0;
        hash_id->refresh_treshold       = mrvl_sai_utl_prv_min_refresh_treshold_CNS*2;
    }
    
    /*MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_DBG3,
                  "%s:%d: exit function (success)\n",
                  __func__, __LINE__));*/

    return SAI_STATUS_SUCCESS;
}
/*------------------------------------------------------------------------------
NAME
    mrvl_sai_utl_SearchHash

DESCRIPTION
    Search an entry

INPUTS
    id      - Hash table identification
    key_ptr - Element key

OUTPUTS
    None

RETURN VALUE
    Returns pointer to required entry.
    If entry not found returns NULL

PROCESSING
     
------------------------------------------------------------------------------*/
void* mrvl_sai_utl_SearchHash (void  *id,
                       void  *key_ptr)
{
    mrvl_sai_utl_prv_hash_id_STC     *hash_id = NULL;
    mrvl_sai_utl_prv_hash_entry_STC  *entry_ptr = NULL;

    /*MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_DBG3,
                 "%s:%d: HASH search \n",
                 __func__, __LINE__));*/

    if ((id == NULL) || (key_ptr == NULL)) {
        MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_ERROR, 
                 "%s:%d: NULL pointer \n", __func__, __LINE__));
        return NULL;
    }

    hash_id = (mrvl_sai_utl_prv_hash_id_STC *) id;
    
    entry_ptr = mrvl_sai_utl_prv_search_by_key(hash_id, key_ptr);
    
    if (entry_ptr != NULL){
       /* MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_DBG3,
                      "%s:%d: exit function (success)\n",
                      __func__, __LINE__));*/

        return ((void*)((uint8_t*)entry_ptr + hash_id->hash_entry_size));
    }
    else{
        MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_DBG3,
                      "%s:%d: exit function (not found)\n",
                      __func__, __LINE__));

        return NULL;
    }
}

/*------------------------------------------------------------------------------
NAME
    mrvl_sai_utl_GetNextSort

DESCRIPTION
    Get next sorted (by key) entry

INPUTS
    id      - Hash table identification
    key_ptr - Element key

OUTPUTS
    None

RETURN VALUE
    

PROCESSING
    In case input key_ptr is NULL the smallest entry, otherwise the routine 
    returns the next sorted (by key) entry. 
    In case input key_ptr is equal or bigger then the bigest entry in table 
    routine returns NULL.
------------------------------------------------------------------------------*/
void* mrvl_sai_utl_GetNextSort (IN void  *id,
                         IN void  *key_ptr)
{
    mrvl_sai_utl_prv_hash_id_STC     *hash_id = NULL;
    mrvl_sai_utl_prv_hash_entry_STC  *entry_ptr = NULL;
    void                    *search_result = NULL;

   /* MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_DBG3,
                 "%s:%d: HASH next sorted entry \n",
                 __func__, __LINE__));*/

    if (id == NULL) {
        MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_ERROR, 
                 "%s:%d: NULL pointer \n", __func__, __LINE__));
        return NULL;
    }

    hash_id = (mrvl_sai_utl_prv_hash_id_STC *) id;
    
#if (HASH_DEBUG_FLAG==1)
    /* Check input. */
        if (hash_id->debug != hash_id)
            MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_ERROR, 
                     "%s:%d: invalid parameter: %d \n", 
                     __func__, __LINE__, hash_id->debug));
#endif
    
    /* Handle sorted insert. */
#if (HASH_DEBUG_FLAG==1)
    if (hash_id->sort_db_flag){
#endif
    if (key_ptr == NULL)
        /* First get_next. */
        entry_ptr = hash_id->first_sorted_entry;
    else {
        search_result = mrvl_sai_utl_SearchHash(id,key_ptr);
    
        if (search_result != NULL){
            /* Entry was found, get its next-by-key entry. */
            entry_ptr = ((mrvl_sai_utl_prv_hash_entry_STC*)((uint8_t*)search_result - 
                                               hash_id->hash_entry_size))
                                 ->next_ptr[hash_id->next_by_key_index];
        }
        else {
            /* In case of delete and insert of our key_ptr, between the previous */
            /*  search and the following get_pre_sorted, the user can get the new */
            /*  key_ptr structure (new structure with the same key_ptr as he gave) */
            /* If in the future we want to prevent this we must test it before */
            /* returning to the user the next-by-key. */
            
            /* Get the nearest entry in sorted list. */
            entry_ptr = (mrvl_sai_utl_prv_hash_entry_STC *)mrvl_sai_utl_prv_get_pre_sorted(id,key_ptr);
            if (entry_ptr != NULL)
                /* Entry_ptr is the nearest smallest entry to non existing */
                /*  get its next-by-key entry. */
                entry_ptr = entry_ptr->next_ptr[hash_id->next_by_key_index];
            else
                /* First_sorted_entry is bigger the key_ptr. */
                entry_ptr = hash_id->first_sorted_entry;
        }
    }
    
    if (entry_ptr != NULL) {
       /* MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_DBG3,
                      "%s:%d: exit function (success)\n",
                      __func__, __LINE__));*/

        return ((void*)((uint8_t*)entry_ptr + hash_id->hash_entry_size));
    }
    else {
       /* MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_DBG3,
                      "%s:%d: exit function (success)\n",
                      __func__, __LINE__));*/

        return NULL;
    }
    
#if (HASH_DEBUG_FLAG==1)
    }
    else
    {
        MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_ERROR, 
                 "%s:%d: SORT error \n", __func__, __LINE__));
        return NULL;
    }
#endif
}

/*------------------------------------------------------------------------------
NAME
    mrvl_sai_utl_GetNextSortEntries

DESCRIPTION
    Routine is used for fast walk over the table.
    The routine returnes as many next sorted (by key) entries as the user asked

INPUTS
    id                  - Hash table identification
    key_ptr             - Element key
    num_of_entries_ptr  - Needed enties num

OUTPUTS
    num_of_entries_ptr  - Actual entries num
    entries_list_ptr    - Pointer to list of entrie pointers

RETURN VALUE
    FPA_ERROR or mrvl_sai_utl_SUCCESS

PROCESSING
    In case input key_ptr is NULL the smallest entries, otherwise the routine 
    returns the next sorted (by key) entries.
------------------------------------------------------------------------------*/
sai_status_t mrvl_sai_utl_GetNextSortEntries (void    *id,
                                  void    *key_ptr,
                                  uint32_t  *num_of_entries_ptr, 
                                  void    **entries_list_ptr)
{
    mrvl_sai_utl_prv_hash_id_STC     *hash_id = NULL;
    mrvl_sai_utl_prv_hash_entry_STC  *entry_ptr = NULL;
    void                    *search_result = NULL;
    uint32_t                  need_num_of_entries , actual_num_of_entries;

   /* MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_DBG3,
                 "%s:%d: HASH next sorted entries: %d \n",
                 __func__, __LINE__, *num_of_entries_ptr));*/

    mrvl_sai_utl_CHECK_PTR(id, FPA_ERROR, APPID_GENERAL, SEVERITY_LEVEL_ERROR);
    mrvl_sai_utl_CHECK_PTR(key_ptr, FPA_ERROR, APPID_GENERAL, SEVERITY_LEVEL_ERROR);
    mrvl_sai_utl_CHECK_PTR(num_of_entries_ptr, FPA_ERROR, APPID_GENERAL, SEVERITY_LEVEL_ERROR);
    mrvl_sai_utl_CHECK_PTR(entries_list_ptr, FPA_ERROR, APPID_GENERAL, SEVERITY_LEVEL_ERROR);

    need_num_of_entries = *num_of_entries_ptr;
    if (need_num_of_entries == 0){
        MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_ERROR, 
                 "%s:%d: invalid parameter \n", __func__, __LINE__));
        return SAI_STATUS_FAILURE;
    }

    search_result = mrvl_sai_utl_GetNextSort (id, key_ptr);

    if (search_result == NULL){
        *num_of_entries_ptr = 0;
        MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_WARN, 
                 "%s:%d: not found \n", __func__, __LINE__));
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    hash_id = (mrvl_sai_utl_prv_hash_id_STC *) id;

    entry_ptr = (mrvl_sai_utl_prv_hash_entry_STC*)
                        ((uint8_t*)search_result - hash_id->hash_entry_size);

    for (actual_num_of_entries = 0 ; 
         (actual_num_of_entries < need_num_of_entries) && (entry_ptr != NULL); 
         actual_num_of_entries++)
    {
        *entries_list_ptr = (void*)((uint8_t*)entry_ptr + hash_id->hash_entry_size);
        entries_list_ptr++;
        entry_ptr = entry_ptr->next_ptr[hash_id->next_by_key_index];
    }

    *num_of_entries_ptr = actual_num_of_entries;

   /* MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_DBG3,
                  "%s:%d: exit function (success)\n",
                  __func__, __LINE__));*/

    return SAI_STATUS_SUCCESS;
}

/*------------------------------------------------------------------------------
NAME
    mrvl_sai_utl_prv_get_pre_sorted

DESCRIPTION
    Routine is used fined the nearest smallest in a sorted HASH table

INPUTS
    id                  - Hash table identification
    key_ptr             - Element key

OUTPUTS
    None

RETURN VALUE
    Routine returns the required entry,
    if there isn't a smaller key then returns NULL

PROCESSING
     
------------------------------------------------------------------------------*/
static void*  mrvl_sai_utl_prv_get_pre_sorted (void *id,
                                      void *key_ptr)
{
    mrvl_sai_utl_prv_hash_id_STC     *hash_id = NULL;
    mrvl_sai_utl_prv_hash_entry_STC  *entry_ptr = NULL, *pre_entry_ptr = NULL;
    mrvl_sai_utl_prv_hash_entry_STC  *low_sorted_ptr = NULL;

  /*  MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_DBG3,
                 "%s:%d: HASH get presorted entry \n",
                 __func__, __LINE__));*/

    if ((id == NULL) || (key_ptr == NULL)) {
        MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_ERROR, 
                 "%s:%d: NULL pointer \n", __func__, __LINE__));
        return NULL;
    }

    /* This routine will return the start of the minimal sorted */
    /* link list. The low_sorted_ptr can never be an entry with */
    /* the same key as key_ptr. */
    mrvl_sai_utl_prv_find_sorted_header_entry(id, key_ptr, &low_sorted_ptr); 

    hash_id = (mrvl_sai_utl_prv_hash_id_STC *) id;
    pre_entry_ptr = low_sorted_ptr;
    if (pre_entry_ptr == NULL)
        entry_ptr = hash_id->first_sorted_entry;
    else
        entry_ptr = pre_entry_ptr->next_ptr[hash_id->next_by_key_index];

    /* Test if entry already exist. */
    while (entry_ptr != NULL) {
        /* Stop when find key greater or equalthen the one you have. */
        if (memcmp((uint8_t*)entry_ptr + hash_id->hash_entry_size, 
                       key_ptr, 
                       hash_id->key_len) >= 0)
            break;

        pre_entry_ptr = entry_ptr;
        entry_ptr = entry_ptr->next_ptr[hash_id->next_by_key_index];
    }

    /*MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_DBG3,
                  "%s:%d: exit function (success)\n",
                  __func__, __LINE__));*/

    return ((void*)pre_entry_ptr);
}

/*------------------------------------------------------------------------------
NAME
    mrvl_sai_utl_GetNextSortByPtr

DESCRIPTION
    Get sorted hash entry

INPUTS
    id                  - Hash table identification
    entry_ptr           - Element key

OUTPUTS
    None

RETURN VALUE
    

PROCESSING
    entry_ptr MUST point to an entry that is currently in the hash  
    table. CAUTION - if using this function in the process of deleteing 
    entries - first call this function and then delete the entry
------------------------------------------------------------------------------*/
void* mrvl_sai_utl_GetNextSortByPtr(void    *id,
                               void    *entry_ptr)
{
    mrvl_sai_utl_prv_hash_id_STC     *hash_id = NULL;
    mrvl_sai_utl_prv_hash_entry_STC  *hash_entry_ptr = NULL, 
                            *next_hash_entry_ptr = NULL;

    /*MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_DBG3,
                 "%s:%d: HASH get sorted entry \n",
                 __func__, __LINE__));*/

    if ((id == NULL) || (entry_ptr == NULL)) {
        MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_ERROR, 
                 "%s:%d: NULL pointer \n", __func__, __LINE__));
        return NULL;
    }

    hash_id = (mrvl_sai_utl_prv_hash_id_STC *) id;

#if (HASH_DEBUG_FLAG==1)
    /* Check input. */
    if (hash_id->debug != hash_id)
        MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_ERROR, 
                 "%s:%d: invalid parameter: %d \n", 
                 __func__, __LINE__, hash_id->debug));
#endif

    /* Handle sorted insert. */
#if (HASH_DEBUG_FLAG==1)
    if (hash_id->sort_db_flag) {
#endif
        hash_entry_ptr = ((mrvl_sai_utl_prv_hash_entry_STC*)((uint8_t *) entry_ptr - hash_id->hash_entry_size));
        next_hash_entry_ptr = hash_entry_ptr->next_ptr[hash_id->next_by_key_index];

        if (next_hash_entry_ptr != NULL){
           /* MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_DBG3,
                          "%s:%d: exit function (success)\n",
                          __func__, __LINE__));*/

            return((void *)((uint8_t *) next_hash_entry_ptr + hash_id->hash_entry_size));
        }
        else{
          /*  MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_DBG3,
                          "%s:%d: exit function (not found)\n",
                          __func__, __LINE__));*/

            return NULL;
        }
#if (HASH_DEBUG_FLAG==1)
    }
    else {
        MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_ERROR, 
                 "%s:%d: SORT error \n", __func__, __LINE__));
        return NULL;
    }
#endif
}

/*------------------------------------------------------------------------------
NAME
    mrvl_sai_utl_NewInitHash

DESCRIPTION
    Allocate and initialize HASH table entity according to user requirements

INPUTS
    id_ptr              - Hash id for further use of hash utilities
    key_len             - key size 2,4,6,8,10,12,20,50
    entry_size          - size of data in bytes (with the key)
    min_num_of_entries  - used for basic entry allocation
    max_num_of_entries  - max possible number of entries
    alloc_portion       - Portion for each entries allocation
    header_size         - Size is round up to the nearest power of 2 
                          Big header improves hash hit ratio
    sort_db_val         - FALSE - no sorting on the DB
                          TRUE  - maintain the DB sorted by key 
    free_entries_margin - Number of empty entries that are always in the 
                          free list. This margin promis us that at least 
                          this number of inserts must be done before a 
                          deleted entry memory area will be used again
    use_sorted_header   - Flag indicates weather to use sorted header in 
                          case of sorted hash

OUTPUTS
    None

RETURN VALUE
    FPA_ERROR or mrvl_sai_utl_SUCCESS

PROCESSING
     
------------------------------------------------------------------------------*/
sai_status_t mrvl_sai_utl_NewInitHash (void      **id_ptr,
                         uint32_t    key_len,
                         uint32_t    entry_size,
                         uint32_t    min_num_of_entries,
                         uint32_t    max_num_of_entries,
                         uint32_t    alloc_portion, 
                         uint32_t    header_size, 
                         uint32_t    sort_db_val, 
                         uint32_t    free_entries_margin,
                         uint32_t    use_sorted_header)
{
    uint32_t                  alloc_size;
    uint32_t                  total_entry_size;
    mrvl_sai_utl_prv_hash_id_STC     *hash_id = NULL;
    mrvl_sai_utl_prv_hash_entry_STC  **header_ptr = NULL;
    mrvl_sai_utl_prv_hash_entry_STC  *free_entry_ptr = NULL, *next = NULL;
    int                     i, size;
    uint32_t                  header_power_size; /* Actual Header size = 2^header_power_size. */
    uint32_t                  ptr_index = 0;
    uint8_t                   NumOfPointersInUse = 0;

    /* Check input. */
    /* Maximum header size is 2^32 */
    /* Key length > 0 */
    /* Maximum num of entries is 2^32 */
    /* Minimum num of entries is 1 */
    /* 0 < alloc portion < 2^32 */
    /* sort_db_val is [0..3] */
    
    if ((header_size == 0) ||
        (key_len == 0) ||
        (min_num_of_entries > max_num_of_entries) ||
        (alloc_portion > max_num_of_entries) ||
        ((alloc_portion == 0) && (max_num_of_entries > min_num_of_entries)) ||
        (free_entries_margin == 0) ||
        (sort_db_val > MRVL_SAI_UTL_SORT_MAX_VAL_CNS)
        ) {
        MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_ERROR, 
                 "%s:%d: invalid parameter: %d %d %d %d \n", 
                 __func__, __LINE__, 
                 min_num_of_entries, max_num_of_entries, alloc_portion, sort_db_val));
        return SAI_STATUS_FAILURE;
    }
    
    if (key_len > mrvl_sai_utl_prv_max_key_length_CNS) {
        MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_ERROR, 
                 "%s:%d: invalid parameter: %d \n", 
                 __func__, __LINE__, key_len));
        return SAI_STATUS_FAILURE;
    }
      
    /* Calculate the size of a hash entry (excluding key & data):
       (Basic size) minus [ (number of unused pointers) * size of pointer ] .
    */
    switch (sort_db_val){
    case MRVL_SAI_UTL_NO_SORT_CNS:
        NumOfPointersInUse = 1; /* "native" next pointer */
        break;
    case MRVL_SAI_UTL_SORT_BY_KEY_CNS:
        NumOfPointersInUse = 2; /* "native" next + sorting pointer */
        break;
    case MRVL_SAI_UTL_SORT_BY_INSERTION_TIME_CNS:
        NumOfPointersInUse = 3; /* "native" next + 2 pointers
                       for double linked list */
        break;
    case MRVL_SAI_UTL_SORT_BY_KEY_AND_TIME_CNS:
        NumOfPointersInUse = 4; /* "native" next + sorting pointer
                       + 2 pointers for double linked list */
        break;
    default:
        /* Mustn't be here */
        MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_ERROR, 
                 "%s:%d: invalid parameter: %d \n", 
                 __func__, __LINE__, sort_db_val));
        return SAI_STATUS_FAILURE;
    }
    
    size = sizeof(mrvl_sai_utl_prv_hash_entry_STC) -
      ((NUM_OF_NEXT_POINTERS - NumOfPointersInUse) * sizeof(mrvl_sai_utl_prv_hash_entry_STC*));
    
    /* Calculate and allocate memory id+header+hash entries. */
    /* Entry size is round to 32bit align. */
    total_entry_size = ((size + entry_size) + (ALIGNMENT_SIZE-1) ) & (~(ALIGNMENT_SIZE-1));
    
    header_power_size = 0;
    header_size--;
    while (header_size > 0){
        header_power_size++;
        header_size = header_size>>1;
    }
    
    /* Allocate free_entries_margin extra to ensue that deleted entry */
    /*  area wont be used before at least free_entries_margin inserts. */
    alloc_size = (sizeof(mrvl_sai_utl_prv_hash_id_STC)) +
             ((1<<header_power_size) * ALIGNMENT_SIZE) +
             ((min_num_of_entries + free_entries_margin) * total_entry_size);
    alloc_size = ((alloc_size) + (ALIGNMENT_SIZE-1) ) & (~(ALIGNMENT_SIZE-1));
    hash_id = (mrvl_sai_utl_prv_hash_id_STC *)malloc(alloc_size);
    
    if (hash_id == NULL) {
        MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_ERROR, 
                 "%s:%d: memory allocaton failure \n", __func__, __LINE__));
        return SAI_STATUS_FAILURE;
    }
    
    /* Initialize HASH id */
    
    /* For fatal error check. */
    hash_id->debug = hash_id;
    
    /* Update hash id with input parameters. */
    hash_id->key_len = key_len;
    hash_id->entry_size = entry_size;
    hash_id->min_num_of_entries = min_num_of_entries;
    hash_id->max_num_of_entries = max_num_of_entries;
    hash_id->alloc_portion = alloc_portion;
    hash_id->header_power_size = header_power_size;
    hash_id->header_size = (1<<header_power_size);
    hash_id->header_mask = hash_id->header_size-1; /* Mask for hash func calculation. */
    hash_id->sort_db_flag =
      (sort_db_val == MRVL_SAI_UTL_SORT_BY_KEY_CNS ||
       sort_db_val == MRVL_SAI_UTL_SORT_BY_KEY_AND_TIME_CNS);
    hash_id->sort_by_insertion_time_flag =
      (sort_db_val == MRVL_SAI_UTL_SORT_BY_INSERTION_TIME_CNS ||
       sort_db_val == MRVL_SAI_UTL_SORT_BY_KEY_AND_TIME_CNS);
    
    /* intialize next pointers */
    hash_id->next_index = 0;
    hash_id->next_by_key_index = 0;
    hash_id->next_inserted_index = 0;
    hash_id->prev_inserted_index = 0;
    
    /* set offset to next pointers */
    ptr_index = 0;
    hash_id->next_index = ptr_index; /* the next pointer always exists */
    
    if (hash_id->sort_db_flag)
        hash_id->next_by_key_index = ++ptr_index;
    
    if (hash_id->sort_by_insertion_time_flag) {
      hash_id->next_inserted_index = ++ptr_index;
      hash_id->prev_inserted_index = ++ptr_index;
    }
    
    hash_id->hash_entry_size = size; /* size in bytes of the entry without key & data. */
    hash_id->total_entry_size = total_entry_size; /* size in bytes of the entry. */
    hash_id->alloc_portion_size = alloc_portion * total_entry_size;
    hash_id->header_ptr = (mrvl_sai_utl_prv_hash_entry_STC**)(hash_id + 1);
    hash_id->first_free_entry = (mrvl_sai_utl_prv_hash_entry_STC*)(hash_id->header_ptr + (1<<header_power_size));
    hash_id->last_free_entry = NULL;
    hash_id->first_sorted_entry = NULL;
    hash_id->newest = NULL;
    hash_id->oldest = NULL;
    hash_id->num_free_entries = hash_id->min_num_of_entries;
    hash_id->num_used_entries = 0;
    hash_id->free_entries_margin = free_entries_margin;
    hash_id->refresh_treshold_ctr = 0;
    hash_id->refresh_treshold = mrvl_sai_utl_prv_min_refresh_treshold_CNS * 2;
    
    /* Initialize HASH header. */
    header_ptr = hash_id->header_ptr;
    for (i = 0; i < (int) hash_id->header_size; i++){
       *header_ptr = NULL;
       header_ptr++;
    }
    
    /* Initialize all HASH entries. */
    next = hash_id->first_free_entry;
    for (i = 0; i < (int) ((hash_id->min_num_of_entries + free_entries_margin)); i++){
       free_entry_ptr = next;
 
       next = (mrvl_sai_utl_prv_hash_entry_STC*)(((uint8_t*)free_entry_ptr) + total_entry_size);
    
       /* Set mrvl_sai_utl_prv_free_entry_bit_CNS only on free entries structure. */
       /*  This LSB can not be set in a real pointer. */
       free_entry_ptr->next_ptr[hash_id->next_index] =
         (mrvl_sai_utl_prv_hash_entry_STC*)((PTR_TO_INT)next | mrvl_sai_utl_prv_free_entry_bit_CNS);
    
       /* Update the sorted field only for sorted data base. */
       /* For unsorted data base the fiels does not exist. */
       /* This initialize is not needed , only for debug resons. */
       if (hash_id->sort_db_flag)
         (free_entry_ptr)->next_ptr[hash_id->next_by_key_index] =
           (mrvl_sai_utl_prv_hash_entry_STC*)(-1);
    
       /* Initialize next-by-insertion-time, if necessary */
       if (hash_id->sort_by_insertion_time_flag)
       {
        (free_entry_ptr)->next_ptr[hash_id->next_inserted_index] = NULL;
        (free_entry_ptr)->next_ptr[hash_id->prev_inserted_index] = NULL;
       }
    }
    
    hash_id->last_free_entry = free_entry_ptr;
    
    /* No need , only for debug. */
    hash_id->last_free_entry->next_ptr[hash_id->next_index] =
      (mrvl_sai_utl_prv_hash_entry_STC*)mrvl_sai_utl_prv_free_entry_bit_CNS;
    
    if ((hash_id->sort_db_flag) && use_sorted_header){
      hash_id->sorted_header_size =
        (hash_id->max_num_of_entries / mrvl_sai_utl_prv_max_average_header_sorted_list_CNS) + 1;
    
      hash_id->sorted_header_size = MAX (mrvl_sai_utl_prv_min_sorted_header_size_CNS ,
                                         hash_id->sorted_header_size);
    
      hash_id->sorted_header_ptr = (mrvl_sai_utl_prv_max_key_TYP *)
            malloc(hash_id->sorted_header_size * sizeof(mrvl_sai_utl_prv_max_key_TYP));
    
      if (hash_id->sorted_header_ptr == NULL) {
          MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_ERROR, 
                   "%s:%d: memory allocation failure \n", __func__, __LINE__));
          return SAI_STATUS_FAILURE;
      }
    
      /* memset((uint8_t*)hash_id->sorted_header_ptr, 0, (hash_id->sorted_header_size-1) * sizeof(mrvl_sai_utl_prv_max_key_TYP) * 4); */
      for (i = 0; i < (int) ((hash_id->sorted_header_size - 1) * sizeof(mrvl_sai_utl_prv_max_key_TYP)); i = i + 4)
          *(uint32_t*)(((uint8_t*)hash_id->sorted_header_ptr) + i) = 0;
    
      /* memset((uint8_t*)hash_id->sorted_header_ptr + ((hash_id->sorted_header_size-1) * sizeof(mrvl_sai_utl_prv_max_key_TYP)),
                0xFFFFFFFF,
                hash_id->sorted_header_size * sizeof(mrvl_sai_utl_prv_max_key_TYP) * 4); */
      for (i = (hash_id->sorted_header_size-1) * sizeof(mrvl_sai_utl_prv_max_key_TYP) ;
                 i < (int) (hash_id->sorted_header_size * sizeof (mrvl_sai_utl_prv_max_key_TYP)); i = i + 4)
          *(uint32_t*)(((uint8_t*)hash_id->sorted_header_ptr) + i) = 0xFFFFFFFF;
    }
    else {
      hash_id->sorted_header_size = 0;
      hash_id->sorted_header_ptr = NULL;
    }
    
    /* Return HASH ID for later uses of this HASH object. */
    *id_ptr = (void*)hash_id;
    
   /* MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_DBG3,
                  "%s:%d: exit function (success)\n",
                  __func__, __LINE__));*/

    return SAI_STATUS_SUCCESS;
}
/*------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
NAME
    mrvl_sai_utl_InitHash

DESCRIPTION
    Allocate and initialize HASH table entity according to user requirements

INPUTS
    id_ptr              - Hash id for further use of hash utilities
    key_len             - key size 2,4,6,8,10,12,20,50
    entry_size          - size of data in bytes (with the key)
    min_num_of_entries  - used for basic entry allocation
    max_num_of_entries  - max possible number of entries
    alloc_portion       - Portion for each entries allocation
    header_size         - Size is round up to the nearest power of 2 
                          Big header improves hash hit ratio
    sort_db_val         - FALSE - no sorting on the DB
                          TRUE  - maintain the DB sorted by key 
    free_entries_margin - Number of empty entries that are always in the 
                          free list. This margin promis us that at least 
                          this number of inserts must be done before a 
                          deleted entry memory area will be used again
    use_sorted_header   - Flag indicates weather to use sorted header in 
                          case of sorted hash

OUTPUTS
    None

RETURN VALUE
    FPA_ERROR or mrvl_sai_utl_SUCCESS

PROCESSING
     
------------------------------------------------------------------------------*/
sai_status_t mrvl_sai_utl_InitHash (void      **id_ptr,
                      uint32_t    key_len,
                      uint32_t    entry_size,
                      uint32_t    min_num_of_entries,
                      uint32_t    max_num_of_entries,
                      uint32_t    alloc_portion, 
                      uint32_t    header_size, 
                      uint32_t    sort_db_val, 
                      uint32_t    free_entries_margin,
                      uint32_t    use_sorted_header)
{
    MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_DBG3,
                 "%s:%d: HASH initialization: %d %d %d %d \n",
                 __func__, __LINE__, key_len, entry_size,
                 min_num_of_entries, max_num_of_entries));

    mrvl_sai_utl_CHECK_PTR(id_ptr, FPA_ERROR, APPID_GENERAL, SEVERITY_LEVEL_ERROR);

    /* sort_db_val is either TRUE or FALSE. For the NewInit function the
     translation of the parameter is as follows: 
     FALSE  => MRVL_SAI_UTL_NO_SORT_CNS
     TRUE   => MRVL_SAI_UTL_SORT_BY_KEY_CNS
     */
    if (sort_db_val != 0/*FALSE*/)
        sort_db_val = MRVL_SAI_UTL_SORT_BY_KEY_CNS;
    else
        sort_db_val = MRVL_SAI_UTL_NO_SORT_CNS;
    
   /* MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_DBG3,
                  "%s:%d: exit function (success)\n",
                  __func__, __LINE__));*/

    return mrvl_sai_utl_NewInitHash (
      id_ptr,          /* Hash id for further use of hash utilities*/
      key_len,             /* key size 2,4,6,8,10,12,20,50 */
      entry_size,          /* size of data in bytes (with the key) */
      min_num_of_entries,  /* Basic entry allocation */ 
      max_num_of_entries,
      alloc_portion,       /* Portion for each entries allocation*/
      header_size,         /* Size is round up to the nearest power
                      of 2. Big header improves hash hit ratio */
      sort_db_val,
      free_entries_margin,
      use_sorted_header);
}

/*------------------------------------------------------------------------------
NAME
    mrvl_sai_utl_InsertHash

DESCRIPTION
    Insert new entry, in case entry exist data is NOT changed.

INPUTS
    id              - Hash table identification.
    user_entry_ptr  - Element ptr with key at first 

OUTPUTS
    entry_ptr_ptr   - pointer to inserted entry

RETURN VALUE
    mrvl_sai_utl_SUCCESS
    mrvl_sai_utl_NO_FREE_ENTRY
    FPA_ERROR

PROCESSING
     
------------------------------------------------------------------------------*/
sai_status_t mrvl_sai_utl_InsertHash (void  *id,
                        void  *user_entry_ptr,
                        void  **entry_ptr_ptr )
{
    mrvl_sai_utl_prv_hash_id_STC     *hash_id = NULL;
    mrvl_sai_utl_prv_hash_entry_STC  **header_ptr = NULL;
    mrvl_sai_utl_prv_hash_entry_STC  *entry_ptr = NULL, *new_entry_ptr = NULL;
    uint8_t                   *copy_dest = NULL;
    uint32_t                  hash_value;

    MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_DBG3,
                 "%s:%d: HASH insert \n",
                 __func__, __LINE__));

    mrvl_sai_utl_CHECK_PTR(id, FPA_ERROR, APPID_GENERAL, SEVERITY_LEVEL_ERROR);
    mrvl_sai_utl_CHECK_PTR(user_entry_ptr, FPA_ERROR, APPID_GENERAL, SEVERITY_LEVEL_ERROR);
    mrvl_sai_utl_CHECK_PTR(entry_ptr_ptr, FPA_ERROR, APPID_GENERAL, SEVERITY_LEVEL_ERROR);

    hash_id = (mrvl_sai_utl_prv_hash_id_STC *) id;
    
#if (HASH_DEBUG_FLAG==1)
    /* Check input. */
    if (hash_id->debug != hash_id)
        MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_ERROR, 
                 "%s:%d: invalid parameter: %d \n", 
                 __func__, __LINE__, hash_id->debug));
#endif
    
    /* Access point for all entries with the same HASH function. */
    hash_value = mrvl_sai_utl_prv_calc_hash_func(id,user_entry_ptr);
    header_ptr = hash_id->header_ptr + hash_value;
    /*header_ptr = hash_id->header_ptr + mrvl_sai_utl_prv_calc_hash_func(id,user_entry_ptr);*/
    
    entry_ptr = *header_ptr;
    
    /* Test if entry already exist. */
    while (entry_ptr != NULL){
       /* If keies are equal. */
       if (memcmp((uint8_t*)entry_ptr + hash_id->hash_entry_size, user_entry_ptr,
                                                 hash_id->key_len) == 0){
           MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_DBG3,
                         "%s:%d: exit function (entry exists)\n",
                         __func__, __LINE__));

           return SAI_STATUS_ITEM_ALREADY_EXISTS;
       }
    
       entry_ptr = entry_ptr->next_ptr[hash_id->next_index];
    
#if (HASH_DEBUG_FLAG==1)
       /* Change in chain during search, start all over. */
       if ((uint32_t)entry_ptr & mrvl_sai_utl_prv_free_entry_bit_CNS)
          entry_ptr = *header_ptr;
#endif
    }
    
    /* Check if more entries are needed. */
    if ((hash_id->num_free_entries == 0) &&
        (mrvl_sai_utl_prv_allocate_more_entries(id) != FPA_OK)){
        MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_DBG3,
                      "%s:%d: exit function (full table)\n",
                      __func__, __LINE__));

        /* Table has reached its limit or no memory allocation. */
        return SAI_STATUS_TABLE_FULL;
    }
    
    /* Key is doesn't exist continue in insert. */
    new_entry_ptr = hash_id->first_free_entry;
    /* Update free lists data. */
    /* Free entry for the insert data. */
    hash_id->first_free_entry = (mrvl_sai_utl_prv_hash_entry_STC*)((PTR_TO_INT)
                     new_entry_ptr->next_ptr[hash_id->next_index] & (~mrvl_sai_utl_prv_free_entry_bit_CNS));
    
    hash_id->num_free_entries = hash_id->num_free_entries - 1;
    
    hash_id->num_used_entries++;
    
    /* Calculate start of key area and copy key. */
    copy_dest = (uint8_t*)new_entry_ptr + hash_id->hash_entry_size;
    
    /* Update OUTPUT value with pointer to key of new entry ptr. */
    *entry_ptr_ptr = (void*)copy_dest;
    
    /* Copy new data to data area. */
    memcpy (copy_dest , user_entry_ptr, hash_id->entry_size);
    
    /* Actual insert at the begining of the list. */
    new_entry_ptr->next_ptr[hash_id->next_index] = *header_ptr;
    
    *header_ptr = new_entry_ptr;
    
    /* Handle sorted insert. */
    if (hash_id->sort_db_flag){
       /* Get the nearest entry in sorted list. */
       entry_ptr = (mrvl_sai_utl_prv_hash_entry_STC *)mrvl_sai_utl_prv_get_pre_sorted(id,user_entry_ptr);
       if (entry_ptr != NULL){
          /* Put entry in sorted list. */
          new_entry_ptr->next_ptr[hash_id->next_by_key_index] =
            entry_ptr->next_ptr[hash_id->next_by_key_index];
          entry_ptr->next_ptr[hash_id->next_by_key_index] =
            new_entry_ptr;
       }
       else {
          /* Put entry in sorted list at the begining. */
          new_entry_ptr->next_ptr[hash_id->next_by_key_index] =
            hash_id->first_sorted_entry;
          hash_id->first_sorted_entry = new_entry_ptr;
       }
    
       /* Sorted header mechanism is active and refresh treshold. */
       if ((hash_id->sorted_header_size != 0) &&
           ((++hash_id->refresh_treshold_ctr) >= hash_id->refresh_treshold))
          mrvl_sai_utl_prv_refresh_sorted_header (id);
    }
    
    /* Handle insertion-time sorting */
 /*   if (hash_id->sort_by_insertion_time_flag)
      mrvl_sai_utl_prv_insertion_time_insert (new_entry_ptr, hash_id);*/
    
   /* MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_DBG3,
                  "%s:%d: exit function (success)\n",
                  __func__, __LINE__));*/

    return SAI_STATUS_SUCCESS;
}

/*------------------------------------------------------------------------------
NAME
    mrvl_sai_utl_GetEntriesNum

DESCRIPTION
    Get number of entries in the hash table

INPUTS
    id  - Hash table identification.

OUTPUTS
    None

RETURN VALUE
    number of entries

PROCESSING
     
------------------------------------------------------------------------------*/
sai_status_t mrvl_sai_utl_GetEntriesNum (void *id)
{
    /*MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_DBG3,
                 "%s:%d: HASH get entries number \n",
                 __func__, __LINE__));*/

    mrvl_sai_utl_CHECK_PTR(id, FPA_ERROR, APPID_GENERAL, SEVERITY_LEVEL_ERROR);

    /*MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_DBG3,
                  "%s:%d: exit function (success)\n",
                  __func__, __LINE__));*/

    return ((mrvl_sai_utl_prv_hash_id_STC *)id)->num_used_entries;
}

/*------------------------------------------------------------------------------
NAME
    mrvl_sai_utl_GetNextByInsertionTime

DESCRIPTION
    Get entry in the hash table, which was inserted after the specified
    entry (or get the oldest entry, in case of the NULL pointer).

INPUTS
    id      - Pointer to hash id 
    key_ptr - Pointer to an entry key. The routine will return a pointer
		    to the entry that was inserted after this one, or NULL if it
		    is the newest or if the  hash table is not sorted by
		    insertion time.
		    Use NULL to get the oldest entry.

OUTPUTS
    None

RETURN VALUE
    The function returns a pointer to the key of the entry that was inserted
    after the given one, or NULL if it is the newest entry or if the hash table
    is not sorted by insertion time.

PROCESSING
     
------------------------------------------------------------------------------*/
void *mrvl_sai_utl_GetNextByInsertionTime (void *id,
                                      void *key_ptr)
{
    mrvl_sai_utl_prv_hash_id_STC		*hash_id = NULL;
    mrvl_sai_utl_prv_hash_entry_STC	*entry_ptr = NULL, *search_result = NULL;
    
   /* MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_DBG3,
                 "%s:%d: HASH get entry by time \n",
                 __func__, __LINE__));*/
    
    if (id == NULL) {
        MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_ERROR, 
                 "%s:%d: NULL pointer \n", __func__, __LINE__));
        return NULL;
    }
    
    hash_id = (mrvl_sai_utl_prv_hash_id_STC *)id;
    
    /* return if hash is not sorted by insertion time */
    if (!hash_id->sort_by_insertion_time_flag) {
        MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_ERROR, 
                 "%s:%d: SORT error \n", __func__, __LINE__));
      return NULL;
    }
    
    if (key_ptr == NULL)
      /* Request for the oldest entry */
      entry_ptr = hash_id->oldest;
    else {
      search_result = mrvl_sai_utl_prv_search_by_key(hash_id,key_ptr);
      if (search_result != NULL)
      /* Entry was found, get its next-by-insertion-time entry. */
          entry_ptr = search_result->next_ptr[hash_id->next_inserted_index];
      else
          entry_ptr = NULL;
    }
    
    if (entry_ptr != NULL){
       /* MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_DBG3,
                      "%s:%d: exit function (success)\n",
                      __func__, __LINE__));*/

        return ((void*)((uint8_t*)entry_ptr + hash_id->hash_entry_size));
    }
    else{
        MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_DBG3,
                      "%s:%d: exit function (not found)\n",
                      __func__, __LINE__));

        return NULL;
    }
}

/*------------------------------------------------------------------------------
NAME
    mrvl_sai_utl_prv_delete_by_key

DESCRIPTION
    Delete entry, the memory used by the deleted entry
    will be used again after at least free_entries_margin inserts
    or updates.
    In sorted DB insert update the sorted list.

INPUTS
    hash_id - Hash table identification
    key_ptr - Element key

OUTPUTS
    None

RETURN VALUE
    mrvl_sai_utl_E_NOT_FOUND
    FPA_ERROR
    mrvl_sai_utl_SUCCESS

PROCESSING
     
------------------------------------------------------------------------------*/
sai_status_t mrvl_sai_utl_prv_delete_by_key (mrvl_sai_utl_prv_hash_id_STC *hash_id, void *key_ptr)
{
    mrvl_sai_utl_prv_hash_entry_STC  **header_ptr = NULL;
    mrvl_sai_utl_prv_hash_entry_STC  *entry_ptr = NULL, *pre_entry_ptr = NULL;
    uint32_t                  hash_value;

    /*MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_DBG3,
                 "%s:%d: HASH delete entry by key \n",
                 __func__, __LINE__));*/

    mrvl_sai_utl_CHECK_PTR(hash_id, FPA_ERROR, APPID_GENERAL, SEVERITY_LEVEL_ERROR);
    mrvl_sai_utl_CHECK_PTR(key_ptr, FPA_ERROR, APPID_GENERAL, SEVERITY_LEVEL_ERROR);

#if (HASH_DEBUG_FLAG==1)
    /* Check input. */
    if (hash_id->debug != hash_id)
        MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_ERROR, 
                 "%s:%d: invalid parameter: %d \n", 
                 __func__, __LINE__, hash_id->debug));
#endif

    /* Access point for all entries with the same HASH function. */
    /*header_ptr = hash_id->header_ptr + mrvl_sai_utl_prv_calc_hash_func(hash_id,key_ptr);*/
    hash_value = mrvl_sai_utl_prv_calc_hash_func(hash_id,key_ptr);
    header_ptr = hash_id->header_ptr + hash_value;
    
    entry_ptr = *header_ptr;
    pre_entry_ptr = (mrvl_sai_utl_prv_hash_entry_STC*)header_ptr;
    
    /* Search for entry. */
    while (entry_ptr != NULL){
        /* If keies are equal. */
        if (memcmp((uint8_t*)entry_ptr + hash_id->hash_entry_size, key_ptr,
                                                 hash_id->key_len) == 0)
            break;
    
        pre_entry_ptr = entry_ptr;
        entry_ptr = entry_ptr->next_ptr[hash_id->next_index];
    
#if (HASH_DEBUG_FLAG==1)
        /* Change in chain during search, start all over. */
        if ((uint32_t)entry_ptr & mrvl_sai_utl_prv_free_entry_bit_CNS){
            entry_ptr = *header_ptr;
            pre_entry_ptr = (mrvl_sai_utl_prv_hash_entry_STC*)header_ptr;
        }
#endif
    }
    
    if (entry_ptr == NULL){
        MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_DBG3,
                      "%s:%d: exit function (not found)\n",
                      __func__, __LINE__));

        return SAI_STATUS_ITEM_NOT_FOUND;
    }
    
    /* Delete entry from chain. */
    /* It works even if entry is first because 'next' is the first field */
    /*  in header_ptr and in mrvl_sai_utl_prv_hash_entry_STC.                    */
    hash_id->num_used_entries--;
    pre_entry_ptr->next_ptr[hash_id->next_index] =
      entry_ptr->next_ptr[hash_id->next_index];
    
    /* Handle sorting-by-key */
    if (hash_id->sort_db_flag){
       /* Get the nearest entry in sorted list. */
       pre_entry_ptr = (mrvl_sai_utl_prv_hash_entry_STC *)mrvl_sai_utl_prv_get_pre_sorted(hash_id,key_ptr);
       if (pre_entry_ptr != NULL)
          /* delete entry from sorted list. */
          pre_entry_ptr->next_ptr[hash_id->next_by_key_index] =
            entry_ptr->next_ptr[hash_id->next_by_key_index];
       else
          /* delete entry from the beginning of the sorted list */
          hash_id->first_sorted_entry =
            entry_ptr->next_ptr[hash_id->next_by_key_index];
    
       /* Sorted header mechanism is active and refresh treshold. */
       if ((hash_id->sorted_header_size != 0) &&
           ((++hash_id->refresh_treshold_ctr) >= hash_id->refresh_treshold))
          mrvl_sai_utl_prv_refresh_sorted_header (hash_id);
    }
    
    /* Handle insertion-time sorting */
    if (hash_id->sort_by_insertion_time_flag)
        mrvl_sai_utl_prv_insertion_time_delete (entry_ptr, hash_id);
    
    /* Update free list data base. */
    hash_id->last_free_entry->next_ptr[hash_id->next_index] =
      (mrvl_sai_utl_prv_hash_entry_STC*)((PTR_TO_INT)entry_ptr | mrvl_sai_utl_prv_free_entry_bit_CNS);
    hash_id->last_free_entry = entry_ptr ;
    hash_id->num_free_entries++;
    
    /* Update free entry fields. */
    entry_ptr->next_ptr[hash_id->next_index] =
      (mrvl_sai_utl_prv_hash_entry_STC*)mrvl_sai_utl_prv_free_entry_bit_CNS;
    /* Update num of entries*/
    
    /*MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_DBG3,
                  "%s:%d: exit function (success)\n",
                  __func__, __LINE__));*/

    return SAI_STATUS_SUCCESS;
}

/*------------------------------------------------------------------------------
NAME
    mrvl_sai_utl_DeleteHash

DESCRIPTION
    Routine is used to delete entry, the memory used by the deleted entry  
    will be used again after at least free_entries_margin inserts or updates.
    In sorted DB insert update the sorted list.

INPUTS
    id      - Hash table identification
    key_ptr - Element key

OUTPUTS
    None

RETURN VALUE
    FPA_ERROR
    mrvl_sai_utl_SUCCESS

PROCESSING
     In large SORTED tables delete is heavy task
------------------------------------------------------------------------------*/
sai_status_t mrvl_sai_utl_DeleteHash (void  *id, 
                        void  *key_ptr)
{
    /*MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_DBG3,
                 "%s:%d: HASH delete \n",
                 __func__, __LINE__));*/

    mrvl_sai_utl_CHECK_PTR(id, FPA_ERROR, APPID_GENERAL, SEVERITY_LEVEL_ERROR);
    mrvl_sai_utl_CHECK_PTR(key_ptr, FPA_ERROR, APPID_GENERAL, SEVERITY_LEVEL_ERROR);

    MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_DBG3,
                  "%s:%d: exit function (success)\n",
                  __func__, __LINE__));

    return mrvl_sai_utl_prv_delete_by_key((mrvl_sai_utl_prv_hash_id_STC *)id, key_ptr);
}

/*------------------------------------------------------------------------------
NAME
    mrvl_sai_utl_DeleteOldestN

DESCRIPTION
    Delete N oldest entries in the hash table

INPUTS
    id          - pointer to hash id
    n           - number of entries to delete

OUTPUTS
    num_deleted - number of successfully deleted entries 
                 (which is only smaller than n in case there are not enough
                  elements)

RETURN VALUE
    FPA_ERROR
    mrvl_sai_utl_NOT_EXIST
    mrvl_sai_utl_SUCCESS

PROCESSING
     
------------------------------------------------------------------------------*/
sai_status_t mrvl_sai_utl_DeleteOldestN (void        *id,
                            uint32_t      n,
                            uint32_t      *num_deleted)
{
    mrvl_sai_utl_prv_hash_id_STC     *hash_id = NULL;
    mrvl_sai_utl_prv_hash_entry_STC  *entry_ptr = NULL, *next_ptr = NULL;
  
   /* MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_DBG3,
                 "%s:%d: HASH delete %d oldest entries \n",
                 __func__, __LINE__, n));*/

    mrvl_sai_utl_CHECK_PTR(id, FPA_ERROR, APPID_GENERAL, SEVERITY_LEVEL_ERROR);
    mrvl_sai_utl_CHECK_PTR(num_deleted, FPA_ERROR, APPID_GENERAL, SEVERITY_LEVEL_ERROR);

    hash_id = (mrvl_sai_utl_prv_hash_id_STC *)id;

    *num_deleted = 0;

    /* return if hash is not sorted by insertion time */
    if (!hash_id->sort_by_insertion_time_flag) {
        MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_ERROR, 
                 "%s:%d: SORT error \n", __func__, __LINE__));
        return SAI_STATUS_FAILURE;
    }

    /* get pointer to the oldest entry: */
    entry_ptr = hash_id->oldest;

    while ((entry_ptr != NULL) && (*num_deleted < n)) {
        next_ptr = entry_ptr->next_ptr[hash_id->next_inserted_index];
        if ( mrvl_sai_utl_prv_delete_by_key (hash_id, 
                                   (void *) ((uint8_t *) entry_ptr + hash_id->hash_entry_size))
                                    == FPA_NOT_FOUND ){
            MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_DBG3,
                          "%s:%d: exit function (not found)\n",
                          __func__, __LINE__));

            return SAI_STATUS_ITEM_NOT_FOUND;
        }

        (*num_deleted)++;
        entry_ptr = next_ptr;
    }

    MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_DBG3,
                  "%s:%d: exit function (success)\n",
                  __func__, __LINE__));

    return SAI_STATUS_SUCCESS;
}

/*------------------------------------------------------------------------------
NAME
    mrvl_sai_utl_prv_search_by_key

DESCRIPTION
    Search an entry

INPUTS
    hash_id     - Hash table identification
    key_ptr     - Element key

OUTPUTS
    None

RETURN VALUE
    Routine returns pointer to required entry.
    If entry not found returns NULL.

PROCESSING
     
------------------------------------------------------------------------------*/
static mrvl_sai_utl_prv_hash_entry_STC* mrvl_sai_utl_prv_search_by_key (
                                            mrvl_sai_utl_prv_hash_id_STC     *hash_id,
                                            void                    *key_ptr)
{
    mrvl_sai_utl_prv_hash_entry_STC  **header_ptr = NULL;
    mrvl_sai_utl_prv_hash_entry_STC  *entry_ptr = NULL;

    /*MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_DBG3,
                 "%s:%d: HASH search by key \n",
                 __func__, __LINE__));*/

    if ((hash_id == NULL) || (key_ptr == NULL)) {
        MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_ERROR, 
                 "%s:%d: NULL pointer \n", __func__, __LINE__));
        return NULL;
    }

#if (HASH_DEBUG_FLAG==1)
    /* Check input. */
    if (hash_id->debug != hash_id)
        MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_ERROR, 
                 "%s:%d: invalid parameter: %d \n", 
                 __func__, __LINE__, hash_id->debug));
#endif

    /* Access point for all entries with the same HASH function. */
    header_ptr = hash_id->header_ptr + mrvl_sai_utl_prv_calc_hash_func(hash_id,key_ptr);

    entry_ptr = *header_ptr;

/* Search for entry. */
   while (entry_ptr != NULL){
       /* If keyes are equal. */
       if (memcmp((uint8_t*)entry_ptr + hash_id->hash_entry_size, key_ptr,
                                             hash_id->key_len) == 0){
          /* MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_DBG3,
                         "%s:%d: exit function (success)\n",
                         __func__, __LINE__));*/

           return((void*)((uint8_t*)entry_ptr));
       }

       entry_ptr = entry_ptr->next_ptr[hash_id->next_index];

       /* Change in chain during search, start all over. */
       if ((PTR_TO_INT)entry_ptr & mrvl_sai_utl_prv_free_entry_bit_CNS)
           entry_ptr = *header_ptr;
   }

   MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_DBG3,
                 "%s:%d: exit function (not found)\n",
                 __func__, __LINE__));

   return NULL;
}



/*------------------------------------------------------------------------------
NAME
    mrvl_sai_utl_prv_find_sorted_header_entry

DESCRIPTION
    Find header of the sorted list 

INPUTS
    id      - Hash table identification
    key_ptr - Element key

OUTPUTS
    low_sorted_ptr  - start of the minimal sorted link list

RETURN VALUE
    None 

PROCESSING
    The low_sorted_header_ptr can never be an entry with 
    the same key as key_ptr
------------------------------------------------------------------------------*/
static void  mrvl_sai_utl_prv_find_sorted_header_entry (
                void                    *id,
                void                    *key_ptr,
                mrvl_sai_utl_prv_hash_entry_STC  **low_sorted_ptr)
{
    mrvl_sai_utl_prv_hash_id_STC *hash_id = NULL;
    void                *low_entry = NULL;    
    mrvl_sai_utl_prv_max_key_TYP *mid_ptr = NULL, *start_key = NULL;    
    uint32_t              right_sub_array_len, left_sub_array_len, array_len;

    /*MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_DBG3,
                 "%s:%d: HASH find header \n",
                 __func__, __LINE__));*/

    hash_id = (mrvl_sai_utl_prv_hash_id_STC *) id;

    /* If sorted header mechanism is not active */
    if (hash_id->sorted_header_size == 0){
       *low_sorted_ptr = NULL;
       /*MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_DBG3,
                     "%s:%d: exit function (no SORT actions)\n",
                     __func__, __LINE__));*/

       return;
    }

    /* Start binary search in sorted header. */
    array_len = hash_id->sorted_header_size;
    right_sub_array_len = array_len>>1;
    left_sub_array_len = array_len - right_sub_array_len;
    mid_ptr = hash_id->sorted_header_ptr + (left_sub_array_len - 1);

    while (1){
        /* If key_ptr is less or equal then mid ptr. */
        if (memcmp(key_ptr, mid_ptr, hash_id->key_len) <= 0){
            if (left_sub_array_len == 2){
                start_key = mid_ptr-1;         
                break;
            }

            array_len = left_sub_array_len;
            right_sub_array_len = array_len>>1;
            left_sub_array_len = array_len - right_sub_array_len;
            mid_ptr -= right_sub_array_len;
        }
        else {
            if (right_sub_array_len == 1){
                start_key = mid_ptr;         
                break;
            }

            array_len = right_sub_array_len + 1;
            right_sub_array_len = array_len>>1;
            left_sub_array_len = array_len - right_sub_array_len;
            mid_ptr += (left_sub_array_len-1);
        }
    }

    /* If first key in sorted header array ret NULL*/
    if (hash_id->sorted_header_ptr == start_key){
        *low_sorted_ptr = NULL;
       /* MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_DBG3,
                      "%s:%d: exit function (first key)\n",
                      __func__, __LINE__));*/

        return;
    }

    /* Search for the lowest key existing in table. */
    while ((low_entry = mrvl_sai_utl_SearchHash(id, (void*)start_key)) == NULL){
        /* Search fail if this key entry was deleted and wasn't refresh yet. */
        start_key = start_key-1;

        /* Force refresh because header key entry was deleted. */
        hash_id->refresh_treshold_ctr = mrvl_sai_utl_prv_max_refresh_treshold_CNS; 

        /* If first key in sorted header array ret NULL*/
        if (hash_id->sorted_header_ptr == start_key){
            *low_sorted_ptr = NULL;
           /* MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_DBG3,
                          "%s:%d: exit function (first key)\n",
                          __func__, __LINE__));*/

            return;
        }
    }

  /*  MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_DBG3,
                  "%s:%d: exit function (success)\n",
                  __func__, __LINE__));*/

    /* Return pointer to an entry in the table which its key is lower then */
    /* the input key.   */
    *low_sorted_ptr = 
        (mrvl_sai_utl_prv_hash_entry_STC*)((uint8_t*)low_entry - hash_id->hash_entry_size);
}

/*------------------------------------------------------------------------------
NAME
    mrvl_sai_utl_prv_refresh_sorted_header

DESCRIPTION
    Rearange sorted header in order to make it more efficient for sorted searches
    The sorted HAH user must activate this routine once in a while according
    to its needs.

INPUTS
    id  - Hash table identification

OUTPUTS
    None

RETURN VALUE
    None

PROCESSING
     
------------------------------------------------------------------------------*/
static void  mrvl_sai_utl_prv_refresh_sorted_header (void *id)
{
    mrvl_sai_utl_prv_hash_id_STC     *hash_id = NULL;
    uint32_t                  i, sub_len, cover_sub_len;
    uint8_t                   *copy_src = NULL;
    mrvl_sai_utl_prv_hash_entry_STC  *entry_ptr = NULL;

#if (HASH_DEBUG_FLAG==1)
    uint32_t                  total_len;
#endif

   /* MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_DBG3,
                 "%s:%d: HASH refresh header \n",
                 __func__, __LINE__));*/

    hash_id = (mrvl_sai_utl_prv_hash_id_STC *) id;

#if (HASH_DEBUG_FLAG==1)
    /* Sorted header mechanizm is not active. */
    if (hash_id->sorted_header_size == 0)
        MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_ERROR, 
                 "%s:%d: SORT error \n", __func__, __LINE__));
#endif

    sub_len = hash_id->num_used_entries / (hash_id->sorted_header_size-1) + 1;

    /*    min < (Refresh_treshold=sub_len) < max */
    hash_id->refresh_treshold = MAX (mrvl_sai_utl_prv_min_refresh_treshold_CNS , sub_len);
    hash_id->refresh_treshold = MIN (mrvl_sai_utl_prv_max_refresh_treshold_CNS ,
                                                  hash_id->refresh_treshold);

    hash_id->refresh_treshold_ctr = 0;

    entry_ptr = hash_id->first_sorted_entry;

#if (HASH_DEBUG_FLAG==1)
    total_len = 0;
#endif
    cover_sub_len = 1;

    /* Refresh sorted header (all but the first and the last keies which mark */
    /* the borders of the key.   */
    for (i=1 ; i<hash_id->sorted_header_size-1; i++){
        /* Move over the sub_len entries of the sorted link list. */
        while ((entry_ptr != NULL) && (cover_sub_len < sub_len)){
            entry_ptr = entry_ptr->next_ptr[hash_id->next_by_key_index];
            cover_sub_len++;
#if (HASH_DEBUG_FLAG==1)
            total_len++;
#endif
        }

        if (entry_ptr != NULL)
            /* Calculate start of key area . */
            copy_src = (uint8_t*)entry_ptr + hash_id->hash_entry_size;
        else
            /* Calculate start of key area last sorted header (all 0xFF). */
            copy_src = (uint8_t*)
              (hash_id->sorted_header_ptr + (hash_id->sorted_header_size-1));

        /* Copy new data to data area. */
        memcpy ((uint8_t*)(hash_id->sorted_header_ptr + i),
                    copy_src, 
                    hash_id->key_len);

        cover_sub_len = 0;
    }

#if (HASH_DEBUG_FLAG==1)
    while (entry_ptr != NULL){
      entry_ptr = entry_ptr->next_ptr[hash_id->next_by_key_index];
      total_len++;
    }

    if (total_len != hash_id->num_used_entries)
        MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_ERROR, 
                 "%s:%d: invalid length: %d %d \n", 
                 __func__, __LINE__,
                 total_len, hash_id->num_used_entries));
#endif
    /*MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_DBG3,
                  "%s:%d: exit function (success)\n",
                  __func__, __LINE__));*/

}

/*------------------------------------------------------------------------------
NAME
    mrvl_sai_utl_prv_insertion_time_delete

DESCRIPTION
    Handle sort-by-insertion-time part of an entry's deletion 
    from hash table

INPUTS
    entry_ptr   - pointer to deleted entry 
    hash_id     - pointer to hash id 

OUTPUTS
    None

RETURN VALUE
    pointer to deleted entry

PROCESSING
     
------------------------------------------------------------------------------*/
static mrvl_sai_utl_prv_hash_entry_STC *mrvl_sai_utl_prv_insertion_time_delete (
                mrvl_sai_utl_prv_hash_entry_STC *entry_ptr,
                mrvl_sai_utl_prv_hash_id_STC    *hash_id) 
{
    mrvl_sai_utl_prv_hash_entry_STC *next = NULL, *prev = NULL;
    
   /* MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_DBG3,
                 "%s:%d: HASH delete insertion time \n",
                 __func__, __LINE__));*/
    
    /* deleted entry's next */
    next = entry_ptr->next_ptr[hash_id->next_inserted_index];
    /* deleted entry's prev */
    prev = entry_ptr->next_ptr[hash_id->prev_inserted_index];
    
    if (next == NULL) {   /* deleted entry is the newest */
        hash_id->newest = prev;
    } else {
        next->next_ptr[hash_id->prev_inserted_index] = prev;
    }
    
    if (prev == NULL) {   /* deleted entry is the oldest */
        hash_id->oldest = next;
    } else {
        prev->next_ptr[hash_id->next_inserted_index] = next;
    }
    
    /*MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_DBG3,
                  "%s:%d: exit function (success)\n",
                  __func__, __LINE__));*/

    return entry_ptr;
}

#if 0
/*------------------------------------------------------------------------------
NAME
    mrvl_sai_utl_prv_insertion_time_insert

DESCRIPTION
    Handle sort-by-insertion-time part of an entry's insertion to hash table

INPUTS
    new_entry_ptr   - pointer to inserted entry 
    hash_id         - pointer to hash id 

OUTPUTS
    None

RETURN VALUE
    pointer to inserted entry

PROCESSING
     
------------------------------------------------------------------------------*/
static mrvl_sai_utl_prv_hash_entry_STC *mrvl_sai_utl_prv_insertion_time_insert (
            mrvl_sai_utl_prv_hash_entry_STC *new_entry_ptr,
            mrvl_sai_utl_prv_hash_id_STC    *hash_id) 
{
    /*MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_DBG3,
                 "%s:%d: HASH add insertion time \n",
                 __func__, __LINE__));*/

    if (hash_id->oldest == NULL) {      /* first entry to the hash table... */
        hash_id->oldest = new_entry_ptr;  /* ... so it's also the oldest.     */
    }
    else {
        /* the current newest entry points to the new one as its next */
        hash_id->newest->next_ptr[hash_id->next_inserted_index] = new_entry_ptr;
    }

    /* the new entry points to the current newest as its previous */
    new_entry_ptr->next_ptr[hash_id->prev_inserted_index] = hash_id->newest;
    hash_id->newest = new_entry_ptr;
    new_entry_ptr->next_ptr[hash_id->next_inserted_index] = NULL;
    
    /*MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_DBG3,
                  "%s:%d: exit function (success)\n",
                  __func__, __LINE__));*/

    return (new_entry_ptr);
}
#endif
/*------------------------------------------------------------------------------
NAME 
    mrvl_sai_utl_TestNextInsert

DESCRIPTION
    Checks whether max entries were use or not

INPUTS
    id  - Hash table identification

OUTPUTS
    None

RETURN VALUE
    mrvl_sai_utl_NO_FREE_ENTRY
    FPA_ERROR
    mrvl_sai_utl_SUCCESS

PROCESSING
     
------------------------------------------------------------------------------*/
sai_status_t mrvl_sai_utl_TestNextInsert (void *id)
{
    mrvl_sai_utl_prv_hash_id_STC *hash_id = NULL;
 
   /* MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_DBG3,
                 "%s:%d: HASH test next insert \n",
                 __func__, __LINE__));*/

    mrvl_sai_utl_CHECK_PTR(id, FPA_ERROR, APPID_GENERAL, SEVERITY_LEVEL_ERROR);

    hash_id = (mrvl_sai_utl_prv_hash_id_STC *) id;

	if (!hash_id || hash_id->num_used_entries >= hash_id->max_num_of_entries){
        MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_DBG3,
                      "%s:%d: exit function (full table)\n",
                      __func__, __LINE__));

        return SAI_STATUS_TABLE_FULL;
    }

    /*MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_DBG3,
                  "%s:%d: exit function (success)\n",
                  __func__, __LINE__));*/

    return SAI_STATUS_SUCCESS ;
}

/*------------------------------------------------------------------------------
NAME
    mrvl_sai_utl_UpdateHash

DESCRIPTION
    Update entries data in such a way that reading data is always valid and legal.
    If entry doesn't exist update inserts new entry

INPUTS
    id              - Hash table identification
    user_entry_ptr  - Element ptr with key at first 

OUTPUTS
    entry_ptr_ptr   - pointer to updated entry

RETURN VALUE
    FPA_ERROR
    mrvl_sai_utl_SUCCESS

PROCESSING
     
------------------------------------------------------------------------------*/
#if 0
FPA_STATUS mrvl_sai_utl_UpdateHash ( void  *id,
                         void  *user_entry_ptr,
                         void   **entry_ptr_ptr)
{
    mrvl_sai_utl_prv_hash_id_STC     *hash_id = NULL;
    mrvl_sai_utl_prv_hash_entry_STC  **header_ptr = NULL;    
    mrvl_sai_utl_prv_hash_entry_STC  *entry_ptr = NULL, 
                            *pre_entry_ptr = NULL, 
                            *new_entry_ptr = NULL;    
    uint8_t                   *copy_dest = NULL;
    uint32_t                  result;

    /*MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_DBG3,
                 "%s:%d: HASH update \n",
                 __func__, __LINE__));*/

    mrvl_sai_utl_CHECK_PTR(id, FPA_ERROR, APPID_GENERAL, SEVERITY_LEVEL_ERROR);
    mrvl_sai_utl_CHECK_PTR(user_entry_ptr, FPA_ERROR, APPID_GENERAL, SEVERITY_LEVEL_ERROR);
    mrvl_sai_utl_CHECK_PTR(entry_ptr_ptr, FPA_ERROR, APPID_GENERAL, SEVERITY_LEVEL_ERROR);

    hash_id = (mrvl_sai_utl_prv_hash_id_STC *) id;

    /* Check input. */
#if (HASH_DEBUG_FLAG==1)
    if (hash_id->debug != hash_id)
        MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_ERROR, 
                 "%s:%d: invalid parameter: %d \n", 
                 __func__, __LINE__, hash_id->debug));
#endif

    /* Access point for all entries with the same HASH function. */
    result = mrvl_sai_utl_prv_calc_hash_func(id,user_entry_ptr);
    header_ptr = hash_id->header_ptr + result;

    entry_ptr = *header_ptr;
    pre_entry_ptr = (mrvl_sai_utl_prv_hash_entry_STC*)header_ptr;

    /* Search for entry. */
    while (entry_ptr != NULL){
        /* If keies are equal. */
        if (memcmp((uint8_t*)entry_ptr + hash_id->hash_entry_size, user_entry_ptr,
                                             hash_id->key_len) == 0)
            break;

        pre_entry_ptr = entry_ptr;
        entry_ptr = entry_ptr->next_ptr[hash_id->next_index];

#if (HASH_DEBUG_FLAG==1)
        /* Change in chain during search, start all over. */
        if ((uint32_t)entry_ptr & mrvl_sai_utl_prv_free_entry_bit_CNS){
            entry_ptr = *header_ptr;
            pre_entry_ptr = (mrvl_sai_utl_prv_hash_entry_STC*)header_ptr;
        }
#endif
    }

    if (entry_ptr == NULL){
       /* MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_DBG3,
                      "%s:%d: exit function (success)\n",
                      __func__, __LINE__));*/

        return (mrvl_sai_utl_InsertHash(id, user_entry_ptr, entry_ptr_ptr));
    }

    /* Free entry for the insert data. */
    new_entry_ptr = hash_id->first_free_entry;

    /* Update free lists data. */
    hash_id->first_free_entry = (mrvl_sai_utl_prv_hash_entry_STC*)
        ((PTR_TO_INT) new_entry_ptr->next_ptr[hash_id->next_index] &
         (~mrvl_sai_utl_prv_free_entry_bit_CNS));

    /* Calculate start of user entry areas and copy. */
    copy_dest = (uint8_t*)new_entry_ptr + hash_id->hash_entry_size;

    /* Update OUTPUT value with pointer to key of new entry ptr. */
    *entry_ptr_ptr = (void*)copy_dest;

    /* Copy new data to data area. */
    memcpy (copy_dest, user_entry_ptr, hash_id->entry_size);

    /* Update free list data base. */
    hash_id->last_free_entry->next_ptr[hash_id->next_index] =
        (mrvl_sai_utl_prv_hash_entry_STC*)((PTR_TO_INT)entry_ptr | mrvl_sai_utl_prv_free_entry_bit_CNS);
    hash_id->last_free_entry = entry_ptr ;

    /* Actual insert at the begining of the list. */
    new_entry_ptr->next_ptr[hash_id->next_index] =
        entry_ptr->next_ptr[hash_id->next_index];

    /* Update free entry fields. */
    entry_ptr->next_ptr[hash_id->next_index] =
        (mrvl_sai_utl_prv_hash_entry_STC*)mrvl_sai_utl_prv_free_entry_bit_CNS;

    /* Replace entry in chain. */
    /* It works even if entry is first because 'next' is the first field */
    /*  in header_ptr and in mrvl_sai_utl_prv_hash_entry_STC.                    */
    pre_entry_ptr->next_ptr[hash_id->next_index] = new_entry_ptr;

    /* Handle sorted update */
   if (hash_id->sort_db_flag){
       new_entry_ptr->next_ptr[hash_id->next_by_key_index] =
           entry_ptr->next_ptr[hash_id->next_by_key_index];

       /* Get the nearest entry in sorted list. */
       pre_entry_ptr = (mrvl_sai_utl_prv_hash_entry_STC *)mrvl_sai_utl_prv_get_pre_sorted(id,user_entry_ptr);
       if (pre_entry_ptr != NULL)
           /* Put entry in sorted list. */
           pre_entry_ptr->next_ptr[hash_id->next_by_key_index] = new_entry_ptr;
       else
           /* Put entry in sorted list at the begining. */
           hash_id->first_sorted_entry = new_entry_ptr;
    }

    /* Handle sort-by-insertion-time update: unless the updated
    entry is the current newest, delete it from the sorted
    list and re-insert in the beginning (as the newest) */

    if (hash_id->sort_by_insertion_time_flag) {
        mrvl_sai_utl_prv_insertion_time_delete (entry_ptr, hash_id);
        mrvl_sai_utl_prv_insertion_time_insert (new_entry_ptr, hash_id);
    }

    if (hash_id->first_free_entry == NULL)
        hash_id->first_free_entry = hash_id->last_free_entry;

   /* MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_DBG3,
                  "%s:%d: exit function (success)\n",
                  __func__, __LINE__));*/

    return FPA_OK;
}
#endif

#ifdef __cplusplus
}
#endif
