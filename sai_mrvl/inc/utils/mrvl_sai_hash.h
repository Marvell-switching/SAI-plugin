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

#ifndef COMMON_mrvl_sai_utl_HASH
#define COMMON_mrvl_sai_utl_HASH

#ifdef __cplusplus
"C" {
#endif

#include "mrvl_sai.h"

/*-------------------------------------------------------------------------------
 HAL internal definitions
-------------------------------------------------------------------------------*/
#define MRVL_SAI_UTL_NO_SORT_CNS                 0
#define MRVL_SAI_UTL_SORT_BY_KEY_CNS             1
#define MRVL_SAI_UTL_SORT_BY_INSERTION_TIME_CNS  2
#define MRVL_SAI_UTL_SORT_BY_KEY_AND_TIME_CNS    3
#define MRVL_SAI_UTL_SORT_MAX_VAL_CNS            MRVL_SAI_UTL_SORT_BY_KEY_AND_TIME_CNS

/*! If this bit is set in the entry_STC.next then it means this entry is */
/*! not used. This LSB is not legal in a real pointer because pointer in */
/*! HASH table is always even.  */
#define  mrvl_sai_utl_prv_free_entry_bit_CNS  1


/* This average defines the number of pointers in sorted header array. */
#define  mrvl_sai_utl_prv_max_average_header_sorted_list_CNS  50

/* Minimum value of sorted_header_size */
#define  mrvl_sai_utl_prv_min_sorted_header_size_CNS      5

/* Minimum & Maximum value of refresh_treshold. */
#define  mrvl_sai_utl_prv_min_refresh_treshold_CNS        3
#define  mrvl_sai_utl_prv_max_refresh_treshold_CNS        20

/* (max) number of 'next' pointers ( = size of the next_ptr array). */
/* Set to 4: standard-next,
             next-sort-by-key,
	     next-sort-by-insertion-time, and
	     previous-sort-by-insertion-time. */

#define NUM_OF_NEXT_POINTERS        4

/*!  This entry structur is the structure of an sorted entry. */
/*!  When the HASH table is not sorted then the 'next_sorted' field is not */
/*!  exist. That is why the 'next' field MUST appear first in the strucure */

typedef struct mrvl_sai_utl_prv_hash_entry_STCT {
  /* Array of pointers to 'next' entries, each according to a sorting scheme */
  struct mrvl_sai_utl_prv_hash_entry_STCT *next_ptr [NUM_OF_NEXT_POINTERS];
} mrvl_sai_utl_prv_hash_entry_STC;

#define  mrvl_sai_utl_prv_max_key_length_CNS      200
typedef uint8_t mrvl_sai_utl_prv_max_key_TYP [mrvl_sai_utl_prv_max_key_length_CNS];

typedef struct mrvl_sai_utl_prv_hash_id_STCT {
  struct mrvl_sai_utl_prv_hash_id_STCT *debug; /* For fatal error check. */

  uint32_t           key_len;      /* key size 2,4,6,8,10,12,20,50 */
  uint32_t           entry_size;   /* Size of user entry in bytes (with the key) */
  uint32_t           min_num_of_entries; /* Basic entry allocation */
  uint32_t           max_num_of_entries;
  uint32_t           alloc_portion;   /* Portion for each entries allocation*/
  uint32_t           header_power_size;
  uint32_t           header_size;     /* header_size = 2^header_power_size.*/
  uint32_t           header_mask;     /* header_mask = header_size-1 */
                                     /* Mask for hash func calculation. */
  uint32_t           sort_db_flag;    /* True - maintain the DB sorted */
  uint32_t           sort_by_insertion_time_flag;  /* True - maintain the DB
                                                     sorted by insertion time */

  /* indexes of 'next' pointers in the next_ptr array: */
  uint32_t           next_index;          /* index of standard 'next' pointer */
  uint32_t           next_by_key_index;   /* index of next-by-key pointer */
  uint32_t           next_inserted_index; /* index of next-by-insertion-time
                                            pointer */
  uint32_t           prev_inserted_index; /* index of previous-by-insertion-time
                                            pointer */

  uint32_t           hash_entry_size; /* size in bytes of the entry without */
                                     /*  key & data. */
  uint32_t           total_entry_size;   /* size in bytes of the entry. */
  uint32_t           alloc_portion_size; /* size in bytes of alloc portion. */
  uint32_t           num_free_entries;  /* Num of free entries in free list */
                                       /* without the free_entries_margin */
  uint32_t           num_used_entries;  /* num of entries in HASH table. */
  uint32_t           free_entries_margin; /* Number of empty entries that are */
                                         /* always in the free list. This */
                                         /* margin promis us that at least 10 */
                                         /* inserts must be done before a */
                                         /* deleted entry memory area will*/
                                         /* be used again. */
  mrvl_sai_utl_prv_hash_entry_STC* *header_ptr;       /* Pointer to HASH header start. */
  mrvl_sai_utl_prv_hash_entry_STC  *first_free_entry; /* Pointer to free list */
  mrvl_sai_utl_prv_hash_entry_STC  *last_free_entry;  /* Pointer to end of free list */
  mrvl_sai_utl_prv_hash_entry_STC  *first_sorted_entry; /* Pointer to all entries  */
                                         /* sorted in a list */
  mrvl_sai_utl_prv_max_key_TYP     *sorted_header_ptr; /* Pointer to the sorted header */
  uint32_t                  sorted_header_size; /* Number of entry keies in  */
                                          /* sorted header. */
  uint32_t                  refresh_treshold_ctr; /* Change due to delete,insert or */
                                            /* other events since last refresh. */
  uint32_t                  refresh_treshold;   /* Number events before refresh. */

  /* pointers to newest and oldest entries (for insertion-time sorting): */
  mrvl_sai_utl_prv_hash_entry_STC  *newest;
  mrvl_sai_utl_prv_hash_entry_STC  *oldest;

} mrvl_sai_utl_prv_hash_id_STC;

/*-------------------------------------------------------------------------------
 HAL functions prototypes
-------------------------------------------------------------------------------*/
sai_status_t mrvl_sai_utl_NewInitHash (void      **id_ptr,
                         uint32_t    key_len,
                         uint32_t    entry_size,
                         uint32_t    min_num_of_entries,
                         uint32_t    max_num_of_entries,
                         uint32_t    alloc_portion, 
                         uint32_t    header_size, 
                         uint32_t    sort_db_val, 
                         uint32_t    free_entries_margin,
                         uint32_t    use_sorted_header);

sai_status_t  mrvl_sai_utl_InitHash ( 
    void    **id_ptr,           /* Hash id for further use of hash utilities*/
    uint32_t  key_len,            /* key size 2,4,6,8,10,12,20,50 */
    uint32_t  entry_size,         /* size of data in bytes (with the key) */
    uint32_t  min_num_of_entries, /* Basic entry allocation */ 
    uint32_t  max_num_of_entries,
    uint32_t  alloc_portion,      /* Portion for each entries allocation*/
    uint32_t  header_size,        /* Size is round up to the nearest power of 2 */
                                /* Big header improves hash hit ratio */
    uint32_t  sort_db_val,        /* FALSE - no sorting on the DB
                                   TRUE  - maintain the DB sorted by key */
    uint32_t  free_entries_margin,/* Number of empty entries that are */
                                /* always in the free list. This */
                                /* margin promis us that at least  */
                                /* this num of inserts must be done */
                                /* before a deleted entry memory */
                                /* area will be used again. */ 
    uint32_t  use_sorted_header   /* Flag indicates weather to use sorted */
                                /* header in case od sorted hash. */
);

void*   mrvl_sai_utl_SearchHash (void  *id,       /* Hash table identification. */
                         void  *key_ptr); /* Element key */

sai_status_t  mrvl_sai_utl_InsertHash (void  *id,               /* Hash table identification. */
                         void  *user_entry_ptr,   /* Element ptr with key at first */
                         void  **entry_ptr_ptr);  /* Pointer to inserted entry */

sai_status_t  mrvl_sai_utl_DeleteHash (void  *id,       /* Hash table identification. */
                         void  *key_ptr); /* Element key */

sai_status_t  mrvl_sai_utl_UpdateHash (void  *id,               /* Hash table identification. */
                         void  *user_entry_ptr,   /* Element ptr with key at first */
                         void  **entry_ptr_ptr);  /* Pointer to updated entry */

sai_status_t  mrvl_sai_utl_CleanHash (void  *id);          /* Hash table identification. */

void*   mrvl_sai_utl_GetNextSort (void  *id,         /* Hash table identification. */
                           void  *key_ptr);   /* Element key */

sai_status_t  mrvl_sai_utl_GetNextSortEntries (
                    void     *id,       /* Hash table identification. */
                    void     *key_ptr,  /* Element key */
                    uint32_t* num_of_entries_ptr, 
                                        /* Input:  Needed enties num. */
                                        /* Output: Actual entries num. */
                    void*    *entries_list_ptr);  
                                        /* Pointer to list of entrie pointers. */

void*   mrvl_sai_utl_GetNextByInsertionTime (
                    void *id,       /* Pointer to hash id */
                    void *key_ptr); /* Pointer to an entry key. 
                                       The routine will return a pointer
		    to the entry that was inserted after this one, or NULL if it
		    is the newest or if the  hash table is not sorted by
		    insertion time.
		    Use NULL to get the oldest entry. */

sai_status_t  mrvl_sai_utl_DeleteOldestN (void    *id,    /* pointer to hash id */
                             uint32_t  n,      /* number of entries to delete */
                             uint32_t  *num_deleted);  
                                            /* return parameter: number of
				                               entries successsfully deleted */
  
sai_status_t  mrvl_sai_utl_TestNextInsert (void   *id);

sai_status_t  mrvl_sai_utl_GetEntriesNum  (void   *id);

void*   mrvl_sai_utl_GetNextSortByPtr(void   *id,        /* Hash table identification */
                                 void   *entry_ptr);/* Element key */


#ifdef __cplusplus
}
#endif


#endif /* COMMON_mrvl_sai_utl_HASH */
