/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * @file util_list.c
 * @author Rex Huang (rex.huang@rafaelmicro.com)
 * @brief 
 * @version 0.1
 * @date 2023-07-27
 * 
 * 
 */

#include <stdint.h>
#include <string.h>
#include <util_list.h>

#define CHECK_NULL_TYPE(cond, ret_val)                                         \
    {                                                                          \
        if (cond) {                                                            \
            return (ret_val);                                                  \
        }                                                                      \
    }

#define CHECK_NULL(cond)                                                       \
    {                                                                          \
        if (cond) {                                                            \
            return;                                                            \
        }                                                                      \
    }

void utils_list_init(struct utils_list* list) {
    list->first = NULL;
    list->last = NULL;
}

void utils_list_pool_init(struct utils_list* list, void* pool, size_t elmt_size,
                          unsigned int elmt_cnt, void* default_value) {
    unsigned int i;

    // initialize the free list relative to the pool
    utils_list_init(list);

    // Add each element of the pool to this list, and init them one by one
    for (i = 0; i < elmt_cnt; i++) {
        if (default_value)
            memcpy(pool, default_value, elmt_size);
        utils_list_push_back(list, (struct utils_list_hdr*)pool);

        // move to the next pool element
        pool = (void*)((uint8_t*)pool + (unsigned int)elmt_size);
    }
}

void utils_list_push_back(struct utils_list* list,
                          struct utils_list_hdr* list_hdr) {
    // Sanity check
    // FIXME assert error here
    //ASSERT_ERR(list_hdr != NULL);

    // check if list is empty
    // list empty => pushed element is also head
    // list not empty => update next of last
    list->first = (utils_list_is_empty(list)) ? list_hdr : list->first;

    // add element at the end of the list
    list->last = list_hdr;
    list_hdr->next = NULL;
}

void utils_list_push_front(struct utils_list* list,
                           struct utils_list_hdr* list_hdr) {
    // Sanity check
    // FIXME assert error here
    //ASSERT_ERR(list_hdr != NULL);

    // check if list is empty
    if (utils_list_is_empty(list)) {
        // list empty => pushed element is also head
        list->last = list_hdr;
    }

    // add element at the beginning of the list
    list_hdr->next = list->first;
    list->first = list_hdr;
}

struct utils_list_hdr* utils_list_pop_front(struct utils_list* list) {
    // check if list is empty
    CHECK_NULL_TYPE(list == NULL || list->first == NULL, NULL);
    struct utils_list_hdr* element = list->first;
    list->first = element->next;
    return element;
}

void utils_list_extract(struct utils_list* list,
                        struct utils_list_hdr* list_hdr) {
    // Rest of the function logic
    CHECK_NULL(list == NULL || list_hdr == NULL);
    // Check if list is empty or not
    CHECK_NULL(list->first == NULL);

    struct utils_list_hdr* scan_list = list->first;

    // Check if searched element is first
    if (scan_list == list_hdr) {
        // Extract first element
        list->first = scan_list->next;
        if (list->last == list_hdr)
            list->last = NULL; // If it was the only element
        return;
    }

    // Look for the element in the list
    while (scan_list->next != NULL && scan_list->next != list_hdr)
        scan_list = scan_list->next;

    // Check if element was found in the list
    if (scan_list->next != NULL) {
        // Check if the removed element is the last in the list
        if (list->last == list_hdr)
            list->last = scan_list;
        // Extract the element from the list
        scan_list->next = list_hdr->next;
    }
}

int utils_list_find(struct utils_list* list, struct utils_list_hdr* list_hdr) {
    CHECK_NULL_TYPE(list == NULL || list_hdr == NULL, 0);

    struct utils_list_hdr* tmp_list_hdr = list->first;
    // Go through the list to find the element
    while (tmp_list_hdr != NULL && tmp_list_hdr != list_hdr)
        tmp_list_hdr = tmp_list_hdr->next;
    return (tmp_list_hdr == list_hdr);
}

unsigned int utils_list_cnt(const struct utils_list* const list) {
    CHECK_NULL_TYPE(list == NULL, 0); // Check if list is NULL

    unsigned int cnt = 0;
    struct utils_list_hdr* elt = utils_list_pick(list);

    // Go through the list to count the number of elements
    while (elt != NULL) {
        cnt++;
        elt = utils_list_next(elt);
    }

    return cnt;
}

/**
 ****************************************************************************************
 * @brief Insert an element in a sorted list.
 *
 * This primitive use a comparison function from the parameter list to select where the
 * element must be inserted.
 *
 * @param[in]  list     Pointer to the list.
 * @param[in]  element  Pointer to the element to insert.
 * @param[in]  cmp      Comparison function (return true if first element has to be inserted
 *                      before the second one).
 *
 * @return              Pointer to the element found and removed (NULL otherwise).
 ****************************************************************************************
 */
void utils_list_insert(struct utils_list* const list,
                       struct utils_list_hdr* const element,
                       int (*cmp)(struct utils_list_hdr const* elementA,
                                  struct utils_list_hdr const* elementB)) {
    CHECK_NULL(list == NULL || element == NULL || cmp == NULL);

    struct utils_list_hdr* prev = NULL;
    struct utils_list_hdr* scan = list->first;

    while (scan && !cmp(element, scan)) {
        prev = scan;
        scan = scan->next;
    }

    element->next = scan;

    if (prev)
        prev->next = element; // second or more
    else
        list->first = element; // first element

    // if inserted at the end, update the last pointer
    if (scan == NULL)
        list->last = element;
}

void utils_list_insert_after(struct utils_list* const list,
                             struct utils_list_hdr* const prev_element,
                             struct utils_list_hdr* const element) {
    CHECK_NULL(list == NULL || element == NULL);

    if (prev_element == NULL) {
        // Insert the element in front of the list
        utils_list_push_front(list, element);
        return;
    }

    // Insert element after prev_element
    element->next = prev_element->next;
    prev_element->next = element;

    if (element->next == NULL)
        list->last = element;
}

void utils_list_insert_before(struct utils_list* const list,
                              struct utils_list_hdr* const next_element,
                              struct utils_list_hdr* const element) {
    CHECK_NULL(list == NULL || element == NULL);

    if (next_element == NULL) {
        // Insert the element at the end of the list
        utils_list_push_back(list, element);
        return;
    }

    if (next_element == list->first) {
        // Insert the element in front of the list
        utils_list_push_front(list, element);
        return;
    }

    struct utils_list_hdr* scan = list->first;

    // Look for next_element in the list
    while (scan && scan->next != next_element)
        scan = scan->next;

    // Insert element before next_element
    if (scan) {
        element->next = next_element;
        scan->next = element;
    }
}

void utils_list_concat(struct utils_list* list1, struct utils_list* list2) {
    CHECK_NULL(list1 == NULL || list2 == NULL);
    CHECK_NULL(list2->first == NULL);

    if (list1->first == NULL)
        *list1 = *list2;
    else {
        list1->last->next = list2->first;
        list1->last = list2->last;
    }

    list2->first = NULL;
    list2->last = NULL;
}

void utils_list_remove(struct utils_list* list,
                       struct utils_list_hdr* prev_element,
                       struct utils_list_hdr* element) {
    CHECK_NULL(list == NULL || element == NULL);

    if (prev_element == NULL) {
        list->first = element->next;
        if (list->last == element)
            list->last = NULL;
    } else {
        prev_element->next = element->next;
        if (list->last == element)
            list->last = prev_element;
    }
    element->next = NULL;
}
