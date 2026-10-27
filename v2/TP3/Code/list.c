/*-----------------------------------------------------------------*/
/*
 Licence Informatique - Structures de données
 Mathias Paulin (Mathias.Paulin@irit.fr)
 
 Implantation du TAD List vu en cours.
 */
/*-----------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "list.h"

typedef struct s_LinkedElement {
	int value;
	struct s_LinkedElement* previous;
	struct s_LinkedElement* next;
} LinkedElement;

/* Use of a sentinel for implementing the list :
 The sentinel is a LinkedElement* whose next pointer refer always to the head of the list and previous pointer to the tail of the list
 */
struct s_List {
	LinkedElement* sentinel;
	int size;
};

typedef struct s_SubList
{
	LinkedElement* head;
	LinkedElement* tail; 
} SubList;


/*-----------------------------------------------------------------*/

List* list_create(void) {
	List* l = malloc(sizeof(List)+sizeof(LinkedElement));
	l->sentinel = (LinkedElement*)(l+1);
	l->sentinel->next = l->sentinel->previous = l->sentinel;
	l->size = 0;
	return (l);
}

/*-----------------------------------------------------------------*/

List* list_push_back(List* l, int v) {
	LinkedElement* s = l->sentinel;
	LinkedElement* t = l->sentinel->previous;
	LinkedElement* n = malloc(sizeof(LinkedElement));
	n->value = v;
	n->previous = t;
	n->next = s;
	s->previous = t->next = n;
	l->size++;
	return l;
}

/*-----------------------------------------------------------------*/

void list_delete(ptrList* l) {
	List* lt = *l;
	LinkedElement* t = lt->sentinel->next;
	LinkedElement* p;
	while(t!=lt->sentinel) {
		p = t;
		t = t->next;
		free(p);
	}
	free(*l);
	*l=NULL;
}

/*-----------------------------------------------------------------*/

List* list_push_front(List* l, int v) {
	LinkedElement* s = l->sentinel;
	LinkedElement* t = l->sentinel->next;
	LinkedElement* n = malloc(sizeof(LinkedElement));
	n->value = v;
	n->next = t;
	n->previous = s;
	t->previous = s->next = n;
	l->size++;
	return l;
}

/*-----------------------------------------------------------------*/

int list_front(const List* l) {
	assert(!list_is_empty(l));
	return l->sentinel->next->value;
}

/*-----------------------------------------------------------------*/

int list_back(const List* l) {
	assert(!list_is_empty(l));
	return l->sentinel->previous->value;
}

/*-----------------------------------------------------------------*/

List* list_pop_front(List* l) {
	assert(!list_is_empty(l));
	LinkedElement* s = l->sentinel;
	LinkedElement* d = s->next;
	LinkedElement* t = d->next;
	t->previous = s;
	s->next = t;
	free(d);
	l->size--;
	return l;
}

/*-----------------------------------------------------------------*/

List* list_pop_back(List* l){
	assert(!list_is_empty(l));
	LinkedElement* s = l->sentinel;
	LinkedElement* d = s->previous;
	LinkedElement* t = d->previous;
	t->next = s;
	s->previous = t;
	free(d);
	l->size--;
	return l;
}

/*-----------------------------------------------------------------*/

List* list_insert_at(List* l, int p, int v) {
	assert(0<=p && p<= list_size(l));
	int i = 0;
	LinkedElement* n = malloc(sizeof(LinkedElement));
	LinkedElement* t = l->sentinel->next;
	LinkedElement* pr;
	n->value = v;
	while (i<p) {
		t = t->next;
		i++;
	}
	pr = t->previous;
	pr->next = t->previous = n;
	n->next = t;
	n->previous = pr;
	l->size++;
	return l;
}

/*-----------------------------------------------------------------*/

List* list_remove_at(List* l, int p) {
	assert(0<=p && p<= list_size(l));
	int i = 0;
	LinkedElement* t = l->sentinel->next;
	LinkedElement* pr;
	LinkedElement* d;
	while (i<p) {
		t = t->next;
		i++;
	}
	d = t;
	pr = t->previous;
	pr->next = t->next;
	t->next->previous = pr;
	free(d);
	l->size--;
	return l;
}

/*-----------------------------------------------------------------*/

int list_at(const List* l, int p) {
	assert(0<=p && p<= list_size(l));
	int i = 0;
	LinkedElement* t = l->sentinel->next;
	while (i<p) {
		t = t->next;
		i++;
	}
	return t->value;
}

/*-----------------------------------------------------------------*/

bool list_is_empty(const List* l) {
	return l->sentinel->next==l->sentinel;
}

/*-----------------------------------------------------------------*/

int list_size(const List* l) {
	return l->size;
}

/*-----------------------------------------------------------------*/

List* list_map(List* l, ListFunctor f, void* environment) {
	LinkedElement* t = l->sentinel;
	while (t->next!=l->sentinel) {
		t->next->value = f(t->next->value, environment);
		t = t->next;
	}
	return l;
}

/*-----------------------------------------------------------------*/
/* 
 Other function to use merge sort.
*/

SubList list_split(SubList l) {
	int i = 0;
	LinkedElement* h = l.head;
	LinkedElement* mid = l.head;
	while (h!=l.tail)
	{
		if(i%2!=0) mid = mid->next;
		h = h->next;
		i++;
	}
	SubList sl;
	sl.head = mid;
	sl.tail = mid->next;
	return sl;
}

/*-----------------------------------------------------------------*/

SubList list_merge(SubList leftlist, SubList rightlist, OrderFunctor f) {
	SubList sl;
	LinkedElement* left = leftlist.head;
	LinkedElement* right = rightlist.head;

	LinkedElement* right_end = rightlist.tail->next;

	if(f(left->value, right->value)) {
		sl.head = left;
		left = left->next;
	} else {
		LinkedElement* right_next = right->next;
		right->previous->next = right->next;
		right->next->previous = right->previous; 
		left->previous->next = right;
		right->next = left;
		right->previous = left->previous;
		left->previous = right;
		sl.head = right;
		right = right_next;
	}
	sl.tail = sl.head;

	while(left!=right && right!=right_end) {
		if(f(left->value, right->value)) {
			sl.tail = left;
			left = left->next;
		} else {
			LinkedElement* rightlist_next = right->next;
			right->previous->next = right->next;
			right->next->previous = right->previous;
			right->previous = sl.tail;
			right->next = left;
			left->previous->next = right;
			left->previous = right;
			sl.tail = right;
			right = rightlist_next;

		}
	}

	if(left!=right) {
		sl.tail->next = left;
		sl.tail = right->previous;
	} else {
		sl.tail->next = right;
		sl.tail = right->previous;
	}

	return sl;

}

/*-----------------------------------------------------------------*/

SubList list_mergesort(SubList l, OrderFunctor f) {
	if(l.head==l.tail) return l;
	SubList splited = list_split(l);
	SubList left_subList;
	left_subList.head = l.head;
	left_subList.tail = splited.head;
	SubList right_subList;
	right_subList.head = splited.tail;
	right_subList.tail = l.tail;

	return list_merge(list_mergesort(left_subList, f), list_mergesort(right_subList, f), f);
}

/*-----------------------------------------------------------------*/
List* list_sort(List* l, OrderFunctor f) {
	SubList sl;
	sl.head = l->sentinel->next;
	sl.tail = l->sentinel->previous;
	list_mergesort(sl, f);

	return l;
}



