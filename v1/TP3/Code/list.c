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

/** \defgroup Type Type definition.
  @{
*/
/** Definition of type SubList.
*/
typedef struct s_SubList {
	LinkedElement* head;
	LinkedElement* tail;
} SubList;
/** @}*/

/*-----------------------------------------------------------------*/

List* list_create(void) {
	List* l = malloc(sizeof(List)+sizeof(LinkedElement));
	l->sentinel = (LinkedElement*)(l+1);
	l->sentinel->previous = l->sentinel;
	l->sentinel->next = l->sentinel;
	l->size = 0;
	return l;
}

/*-----------------------------------------------------------------*/

List* list_push_back(List* l, int v) {
	LinkedElement* elem = malloc(sizeof(LinkedElement));
	elem->value = v;
	elem->next = l->sentinel;
	elem->previous = l->sentinel->previous;
	LinkedElement* current = l->sentinel->previous;
	current->next = elem;
	l->sentinel->previous = elem;
	l->size++;
	return l;
}

/*-----------------------------------------------------------------*/

void list_delete(ptrList* l) {
	List* l_temp = *l;
	LinkedElement* cursor = l_temp->sentinel->next;
	while(cursor->next!=l_temp->sentinel) {	
		cursor = cursor->next;
		free(cursor->previous);
	}
	free(cursor);
	free(l_temp);
	l = NULL;
}

/*-----------------------------------------------------------------*/

List* list_push_front(List* l, int v) {
	LinkedElement* elem = malloc(sizeof(LinkedElement));
	elem->value = v;
	elem->next = l->sentinel->next;
	elem->previous = l->sentinel;
	LinkedElement* current = l->sentinel->next;
	l->sentinel->next = elem;
	current->previous = elem;
	l->size++;
	return l;
}

/*-----------------------------------------------------------------*/

int list_front(const List* l) {
	return l->sentinel->next->value;
}

/*-----------------------------------------------------------------*/

int list_back(const List* l) {
	return l->sentinel->previous->value;
}

/*-----------------------------------------------------------------*/

List* list_pop_front(List* l) {
	LinkedElement *current = l->sentinel->next;
	l->sentinel->next = l->sentinel->next->next;
	l->sentinel->next->previous = l->sentinel;
	free(current);
	l->size--;
	return l;
}

/*-----------------------------------------------------------------*/

List* list_pop_back(List* l){
	LinkedElement *current = l->sentinel->previous;
	l->sentinel->previous = l->sentinel->previous->previous;
	l->sentinel->previous->next = l->sentinel;
	free(current);
	l->size--;
	return l;
}

/*-----------------------------------------------------------------*/

List* list_insert_at(List* l, int p, int v) {
	LinkedElement *elem = malloc(sizeof(LinkedElement));
	elem->value = v;
	LinkedElement *current = l->sentinel->next;
	for (int i=0; i<p; i++)
	{
		current=current->next;
	}
	elem->previous = current->previous;
	elem->next = current;
	current->previous->next = elem;
	current->previous = elem;
	l->size++;
	return l;
}

/*-----------------------------------------------------------------*/

List* list_remove_at(List* l, int p) {
	LinkedElement *current = l->sentinel->next;
	for (int i=0; i<p; i++)
	{
		current=current->next;
	}
	current->previous->next = current->next;
	current->next->previous = current->previous;
	free(current);
	l->size--;
	return l;
}

/*-----------------------------------------------------------------*/

int list_at(const List* l, int p) {
	LinkedElement *current = l->sentinel->next;
	for (int i=0; i<p; i++)
	{
		current=current->next;
	}
	return current->value;
}

/*-----------------------------------------------------------------*/

bool list_is_empty(const List* l) {
	return l->size==0;
}

/*-----------------------------------------------------------------*/

int list_size(const List* l) {
	return l->size;
}

/*-----------------------------------------------------------------*/

List* list_map(List* l, ListFunctor f, void* environment) {
	LinkedElement *current = l->sentinel->next;
	for (int i=l->size; i>0; --i)
	{
		current->value = f(current->value, environment);
		current=current->next;
	}
	return l;
}

/*-----------------------------------------------------------------*/

SubList list_split(SubList l) {
	LinkedElement *l_temp = l.head;
	//LinkedElement *sentinel = l_temp;
	LinkedElement *cursor = l.head;
	int odd = 1; // even : 0 et odd : 1
	while (cursor != l.tail) {
		if (odd == 0) {
			l_temp = l_temp->next;
			odd = 1;
		} else {
			odd = 0;
		}
		cursor = cursor->next;
		
	}
	SubList subList;
	subList.head = l_temp;
	subList.tail = l_temp->next;
	
	return subList;
}

/*-----------------------------------------------------------------*/

SubList list_merge(SubList leftlist, SubList rightlist, OrderFunctor f) {
	LinkedElement *l_left = leftlist.head;
	LinkedElement *l_right = rightlist.head;
	LinkedElement * Lastright = rightlist.tail->next; 
	SubList slist;
	// init head
	if(f(l_left->value, l_right->value)) {
		slist.head = l_left;
		slist.tail = l_left;
		l_left = l_left->next;
	} else {
		//save right
		LinkedElement *save_right = l_right->next; 

		// sort
		l_right->next->previous = l_right->previous;
		l_right->previous->next = l_right->next;
		l_right->previous = l_left->previous;
		l_right->next = l_left;
		l_left->previous->next = l_right;
		l_left->previous = l_right;
		
		// update the sublist
		slist.head =	l_right;
		slist.tail = l_right;

		// next text sort
		l_right->next = save_right;
	} 
	
	// sort and search tail
	while ((l_right!=Lastright) && (l_right!=l_left)) {
		if(f(l_left->value, l_right->value)) {
			slist.tail = l_left;
			l_left = l_left->next;
		} else {
		//save right
			LinkedElement *save_right = l_right->next; 

			// sort
			l_right->next->previous = l_right->previous;
			l_right->previous->next = l_right->next;
			l_right->previous = slist.tail; // change because of the loop
			l_right->next = l_left;
			l_left->previous->next = l_right;
			l_left->previous = l_right;
		
			// update the sublist
			slist.tail = l_right;

			// next text sort
			l_right->next = save_right;
		}
		if(l_left!=l_right) {
			slist.tail->next = l_left;
			slist.tail = l_right->previous; 
		} else {
			slist.tail->next = l_right;
			slist.tail = Lastright->previous; 
		}
	}
	return slist;
}

/*-----------------------------------------------------------------*/

SubList list_mergesort(SubList l, OrderFunctor f) {
	if(l.head == l.tail) {
		return l;
	}
	SubList right, sright, left, sleft, splited_list;
	left.head = l.head;
	splited_list = list_split(l);
	left.tail = splited_list.head;
	right.head = splited_list.tail;
	right.tail = l.tail;
	sright = list_mergesort(right, f);
	sleft = list_mergesort(left, f);
	return list_merge(sleft, sright, f);
}

/*-----------------------------------------------------------------*/

List* list_sort(List* l, OrderFunctor f) {
	SubList sub_l;
	sub_l.head = l->sentinel->next;
	sub_l.tail = l->sentinel->previous;
	list_mergesort(sub_l, f);
	return l;
}

