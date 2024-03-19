#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */
struct list_head *merge_two_list(struct list_head *left,
                                 struct list_head *right);

struct list_head *merge_recur(struct list_head *head);

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
    if (!head)
        return false;
    element_t *new = malloc(sizeof(element_t));
    if (!new)
        return false;
    new->value = strdup(s);
    if (!new->value) {
        free(new);
        return false;
    }
    list_add(&new->list, head);
    return true;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    return q_insert_head(head->prev, s);
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
    struct list_head *fast, *slow;
    fast = slow = head->next;
    while (fast->next->next != head && fast->next != head) {
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
    if (!head)
        return false;
    element_t *entry, *safe;
    bool dup = false;
    list_for_each_entry_safe (entry, safe, head, list) {
        if (&safe->list != head && !strcmp(entry->value, safe->value)) {
            list_del(&entry->list);
            q_release_element(entry);
            dup = true;
        } else if (dup) {
            list_del(&entry->list);
            q_release_element(entry);
            dup = false;
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
    struct list_head **indir = &head->next->next, *sub_head = head;
    int size = q_size(head);
    for (int i = 0; i < size - 1; i++) {
        list_move(*indir, sub_head);
    }
}

/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    struct list_head **indir = &head->next->next, *sub_head = head;
    int size = q_size(head);
    while (size >= k) {
        for (int i = 0; i < k - 1; i++) {
            list_move(*indir, sub_head);
        }
        sub_head = (*indir)->prev;
        indir = &(*indir)->next;
        size -= k;
    }
}

/* Sort elements of queue in ascending/descending order */
void q_sort(struct list_head *head, bool descend)
{
    if (!head || list_empty(head))
        return;
    // disconnect the circular structure
    head->prev->next = NULL;
    head->next = merge_recur(head->next);
    // reconnect the list (prev and circular)
    struct list_head *c = head, *n = head->next;
    while (n) {
        n->prev = c;
        c = n;
        n = n->next;
    }
    c->next = head;
    head->prev = c;
}

struct list_head *merge_two_list(struct list_head *left,
                                 struct list_head *right)
{
    struct list_head head;
    struct list_head *h = &head;
    if (!left && !right) {
        return NULL;
    }
    while (left && right) {
        if (strcmp(list_entry(left, element_t, list)->value,
                   list_entry(right, element_t, list)->value) < 0) {
            h->next = left;
            left = left->next;
            h = h->next;
        } else {
            h->next = right;
            right = right->next;
            h = h->next;
        }
    }
    // after merge, there are still one node still not connect yet
    h->next = left ? left : right;
    return head.next;
}

struct list_head *merge_recur(struct list_head *head)
{
    if (!head->next)
        return head;

    struct list_head *slow = head;
    // split list
    for (struct list_head *fast = head->next; fast && fast->next;
         fast = fast->next->next) {
        slow = slow->next;
    }

    struct list_head *mid = slow->next;  // the start node of right part
    slow->next = NULL;

    struct list_head *left = merge_recur(head);
    struct list_head *right = merge_recur(mid);

    return merge_two_list(left, right);
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
    queue_contex_t *output = container_of(head->next, queue_contex_t, chain);
    queue_contex_t *curr = NULL;
    list_for_each_entry (curr, head, chain) {
        if (curr == output)
            continue;
        list_splice_init(curr->q, output->q);
        output->size = output->size + curr->size;
        curr->size = 0;
    }

    q_sort(output->q, descend);
    return 0;
}
