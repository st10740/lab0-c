#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */


/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *l = malloc(sizeof(struct list_head));
    if (!l)
        return NULL;
    INIT_LIST_HEAD(l);
    return l;
}

/* Free all storage used by queue */
void q_free(struct list_head *head)
{
    element_t *item, *tmp;
    list_for_each_entry_safe (item, tmp, head, list) {
        free(item->value);
        free(item);
    }
    free(head);
}

bool q_insert(struct list_head *head,
              char *s,
              void (*insert)(struct list_head *, struct list_head *))
{
    if (!head)
        return false;
    element_t *new = malloc(sizeof(element_t));
    if (!new)
        return false;
    new->value = malloc((strlen(s) + 1) * sizeof(char));
    if (!new->value) {
        free(new);
        return false;
    }
    strncpy(new->value, s, strlen(s) + 1);
    insert(&new->list, head);
    return true;
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    return q_insert(head, s, list_add);
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    return q_insert(head, s, list_add_tail);
}

element_t *q_remove(struct list_head *head,
                    struct list_head *removed,
                    char *sp,
                    size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;
    element_t *removed_ele = list_entry(removed, element_t, list);
    list_del(&removed_ele->list);
    if (sp) {
        strncpy(sp, removed_ele->value, bufsize);
    }
    return removed_ele;
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    return q_remove(head, head->next, sp, bufsize);
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    return q_remove(head, head->prev, sp, bufsize);
}

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    if (!head)
        return 0;
    int len = 0;
    struct list_head *li;
    list_for_each (li, head)
        len++;
    return len;
}

/* Delete a node in queue */
void q_delete_element(struct list_head *l)
{
    list_del(l);
    element_t *entry = list_entry(l, element_t, list);
    free(entry->value);
    free(entry);
}

/* Delete the middle node in queue */
bool q_delete_mid(struct list_head *head)
{
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/

    if (!head || list_empty(head))
        return false;

    struct list_head *slow, *fast;
    for (slow = head->next, fast = head->next;
         fast != head && fast->next != head; fast = fast->next->next) {
        slow = slow->next;
    }
    q_delete_element(slow);
    return true;
}

/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/

    if (!head)
        return false;
    if (list_empty(head) || list_is_singular(head))
        return true;

    struct list_head *cur, *prev;
    struct list_head *del_q = q_new();
    for (prev = head->next, cur = prev->next; cur != head && prev != head;
         prev = cur, cur = cur->next) {
        while (cur != head &&
               strcmp(list_entry(cur, element_t, list)->value,
                      list_entry(prev, element_t, list)->value) == 0) {
            cur = cur->next;
        }
        if (prev->next != cur) {
            LIST_HEAD(tmp_del);
            list_cut_position(&tmp_del, prev->prev, cur->prev);
            list_splice(&tmp_del, del_q);
        }
    }
    q_free(del_q);
    return true;
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/

    if (!head || list_empty(head))
        return;
    struct list_head *cur, *next;
    for (cur = head->next, next = cur->next; cur != head && next != head;
         cur = cur->next, next = cur->next) {
        list_move(cur, next);
    }
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    if (!head || list_empty(head))
        return;
    struct list_head *cur, *prev, *safe, *list_tail = head->prev;
    list_for_each_safe (cur, safe, head) {
        prev = cur->prev;
        cur->next = prev;
        prev->prev = cur;
    }
    cur->next = list_tail;
    list_tail->prev = cur;
}

/**
 * Reverse specific elements in queue
 * @head: pointer to the first node of the list which will be reversed
 * @tail: pointer to the last node of the list which will be reversed
 */
void q_reverse_position(struct list_head *head, const struct list_head *tail)
{
    if (!head || head == tail)
        return;
    struct list_head *cur, *prev, *safe;
    for (cur = head->next, prev = cur->prev, safe = cur->next; prev != tail;
         prev = cur, cur = safe, safe = safe->next) {
        cur->next = prev;
        prev->prev = cur;
    }
}

/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    // https://leetcode.com/problems/reverse-nodes-in-k-group/

    if (!head || list_empty(head))
        return;
    struct list_head *headK, *tailK, *pre_headK, *next_tailK;
    for (headK = head->next, tailK = headK; headK != head;
         headK = headK->next, tailK = headK) {
        int tmpK = k - 1;
        bool break_flag = false;
        while (tmpK--) {
            if (tailK == head) {
                break_flag = true;
                break;
            }
            tailK = tailK->next;
        }
        if (break_flag || tailK == head)
            break;
        pre_headK = headK->prev;
        next_tailK = tailK->next;
        q_reverse_position(headK, tailK);
        pre_headK->next = tailK;
        tailK->prev = pre_headK;
        next_tailK->prev = headK;
        headK->next = next_tailK;
    }
}

/* Merge two sorted queue */
void q_merge_two_lists(struct list_head *merged_head,
                       struct list_head *left_head,
                       struct list_head *right_head,
                       bool descend)
{
    while (!list_empty(left_head) && !list_empty(right_head)) {
        char *left_value = list_entry(left_head->next, element_t, list)->value;
        char *right_value =
            list_entry(right_head->next, element_t, list)->value;
        if ((descend && strcmp(left_value, right_value) >= 0) ||
            (!descend && strcmp(left_value, right_value) <= 0)) {
            list_move_tail(left_head->next, merged_head);
        } else {
            list_move_tail(right_head->next, merged_head);
        }
    }
    if (!list_empty(left_head)) {
        list_splice_tail_init(left_head, merged_head);
    } else {
        list_splice_tail_init(right_head, merged_head);
    }
}

/* Sort elements of queue in ascending/descending order */
void q_sort(struct list_head *head, bool descend)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;

    struct list_head *slow, *fast;
    for (slow = head->next, fast = slow; fast != head && fast->next != head;
         fast = fast->next->next) {
        slow = slow->next;
    }
    struct list_head *mid_prev = slow->prev;

    LIST_HEAD(left_head);
    LIST_HEAD(right_head);
    list_cut_position(&left_head, head, mid_prev);
    list_splice(head, &right_head);
    INIT_LIST_HEAD(head);

    q_sort(&left_head, descend);
    q_sort(&right_head, descend);
    q_merge_two_lists(head, &left_head, &right_head, descend);
}

/* Implement the common parts of q_asecnd and q_descend */
int q_descend_or_ascend(struct list_head *head, bool descend)
{
    if (!head || list_empty(head))
        return 0;

    int num = 0;
    struct list_head *cur = head->prev, *prev = cur->prev;
    while (prev != head) {
        num++;
        bool del_prev =
            descend ? (strcmp(list_entry(prev, element_t, list)->value,
                              list_entry(cur, element_t, list)->value) < 0)
                    : (strcmp(list_entry(prev, element_t, list)->value,
                              list_entry(cur, element_t, list)->value) > 0);
        if (del_prev) {
            struct list_head *tmp = prev;
            prev = prev->prev;
            list_del(tmp);
            num--;
        } else {
            cur = prev;
            prev = prev->prev;
        }
    }
    num++;
    return num;
}

/* Remove every node which has a node with a strictly less value anywhere to
 * the right side of it */
int q_ascend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/

    return q_descend_or_ascend(head, false);
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/

    return q_descend_or_ascend(head, true);
}

/* Merge all the queues into one sorted queue, which is in ascending/descending
 * order */
int q_merge(struct list_head *head, bool descend)
{
    // https://leetcode.com/problems/merge-k-sorted-lists/

    if (!head || list_empty(head))
        return 0;

    int chain_size = q_size(head);
    struct list_head *first = head->next, *second = head->prev;

    while (chain_size > 1) {
        for (;; first = first->next, second = second->prev) {
            if (first == second || first->prev == second) {
                first = head->next;
                break;
            }
            LIST_HEAD(merged_head);
            struct list_head *first_q =
                list_entry(first, queue_contex_t, chain)->q;
            struct list_head *second_q =
                list_entry(second, queue_contex_t, chain)->q;
            q_merge_two_lists(&merged_head, first_q, second_q, descend);
            list_splice_init(&merged_head, first_q);
        }
        chain_size = (chain_size + 1) / 2;
    }

    return q_size(list_entry(head->next, queue_contex_t, chain)->q);
}