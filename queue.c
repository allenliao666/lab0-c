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
    struct list_head *p = malloc(sizeof(struct list_head));
    if (!p) {
        free(p);
        return NULL;
    }
    INIT_LIST_HEAD(p);
    return p;
}

/* Free all storage used by queue */
void q_free(struct list_head *l)
{
    if (!l) {
        return;
    }
    element_t *entry, *safe;
    list_for_each_entry_safe (entry, safe, l, list) {
        q_release_element(entry);
    }
    free(l);
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head) {
        return false;
    }
    element_t *new = malloc(sizeof(element_t));
    if (!new) {
        return false;
    }
    new->value = malloc(strlen(s) + 1);
    if (!new->value) {
        free(new);
        return false;
    }
    strncpy(new->value, s, strlen(s) + 1);
    list_add(&new->list, head);
    return true;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head) {
        return false;
    }
    element_t *new = malloc(sizeof(element_t));
    if (!new) {
        return false;
    }
    new->value = malloc(strlen(s) + 1);
    if (!new->value) {
        free(new);
        return false;
    }
    strncpy(new->value, s, strlen(s) + 1);
    list_add_tail(&new->list, head);
    return true;
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head)) {
        return NULL;
    }
    element_t *element = list_first_entry(head, element_t, list);
    if (sp) {
        strncpy(sp, element->value, bufsize);
        sp[bufsize - 1] = '\0';
    }
    list_del(head->next);
    return element;
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head)) {
        return NULL;
    }
    element_t *element = list_last_entry(head, element_t, list);
    if (sp) {
        strncpy(sp, element->value, bufsize);
        sp[bufsize - 1] = '\0';
    }
    list_del(head->prev);
    return element;
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

/* Delete the middle node in queue */
bool q_delete_mid(struct list_head *head)
{
    if (!head || list_empty(head)) {
        return false;
    }
    struct list_head *fast, *slow, *tail;
    fast = slow = head->next;
    tail = head->prev;
    while (fast->next != tail || fast != tail) {
        fast = fast->next->next;
        slow = slow->next;
    }
    list_del(slow);
    q_release_element(container_of(slow, element_t, list));
    return true;
}

/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    if (!head || list_empty(head))
        return false;
    element_t *c, *n;
    bool is_dup = false;
    list_for_each_entry_safe (c, n, head, list) {
        if (c->list.next != head && strcmp(c->value, n->value) == 0) {
            list_del(&c->list);
            q_release_element(c);
            is_dup = true;
        } else if (is_dup) {
            list_del(&c->list);
            q_release_element(c);
            is_dup = false;
        }
    }
    return true;
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/
    if (!head || list_empty(head))
        return;
    struct list_head *n, *s;
    list_for_each_safe (n, s, head) {
        if (s == head)
            break;
        list_move(n, s);
        s = n->next;
    }
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    if (!head || list_empty(head))
        return;
    struct list_head *n, *s, *t;
    list_for_each_safe (n, s, head) {
        t = n->next;
        n->next = n->prev;
        n->prev = t;
    }
    t = head->next;
    head->next = head->prev;
    head->prev = t;
}

/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    // https://leetcode.com/problems/reverse-nodes-in-k-group/
    if (!head || list_empty(head))
        return;
    struct list_head temp, *n, *s, *tail = head;
    INIT_LIST_HEAD(&temp);
    int i = 0;
    list_for_each_safe (n, s, head) {
        ++i;
        if (i == k) {
            list_cut_position(&temp, tail, n);
            q_reverse(&temp);
            list_splice_init(&temp, head);
            i = 0;
            tail = s->prev;
        }
    }
}

/* Sort elements of queue in ascending/descending order */
void q_sort(struct list_head *head, bool descend)
{
    struct list_head less, great;
    element_t *pivot;
    element_t *now = NULL, *next = NULL;
    if (!head || list_empty(head) || list_is_singular(head))
        return;
    INIT_LIST_HEAD(&less);
    INIT_LIST_HEAD(&great);
    pivot = list_first_entry(head, element_t, list);
    list_del(&pivot->list);
    list_for_each_entry_safe (now, next, head, list) {
        if (descend == false) {
            if (strcmp(now->value, pivot->value) < 0)
                list_move_tail(&now->list, &less);
            else
                list_move_tail(&now->list, &great);
        } else {
            if (strcmp(now->value, pivot->value) > 0)
                list_move_tail(&now->list, &less);
            else
                list_move_tail(&now->list, &great);
        }
    }
    q_sort(&less, descend);
    q_sort(&great, descend);
    list_add(&pivot->list, head);
    list_splice(&less, head);
    list_splice_tail(&great, head);
}

/* Remove every node which has a node with a strictly less value anywhere to
 * the right side of it */
int q_ascend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    if (!head || list_empty(head))
        return 0;
    struct list_head *n = head->next;
    struct list_head *s = n->next;
    while (n != head && s != head) {
        element_t *curr = list_entry(n, element_t, list);
        element_t *next = list_entry(s, element_t, list);
        if (strcmp(curr->value, next->value) > 0) {
            list_del(&next->list);
            q_release_element(next);
        } else {
            n = s;
        }
        s = n->next;
    }
    return q_size(head);
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    if (!head || list_empty(head))
        return 0;
    struct list_head *n = head->prev;
    struct list_head *s = n->prev;
    while (n != head && s != head) {
        element_t *curr = list_entry(n, element_t, list);
        element_t *prev = list_entry(s, element_t, list);
        if (strcmp(curr->value, prev->value) > 0) {
            list_del(&prev->list);
            q_release_element(prev);
        } else {
            n = s;
        }
        s = n->prev;
    }
    return q_size(head);
}

/* Merge all the queues into one sorted queue, which is in ascending/descending
 * order */
int q_merge(struct list_head *head, bool descend)
{
    // https://leetcode.com/problems/merge-k-sorted-lists/
    if (!head || list_empty(head))
        return 0;

    return 0;
}
