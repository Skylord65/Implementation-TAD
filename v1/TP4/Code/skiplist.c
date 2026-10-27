#include <stdlib.h>
#include <assert.h>

#include "skiplist.h"
#include "rng.h"

typedef struct s_Node Node;
typedef struct s_Link
{
	Node* next;
	Node* previous;
}Link;


struct s_Node {
	int value;
	Link* level;
	int nb_level;
};


struct s_SkipList
{
	Node* sentinel;
	unsigned int size;
	RNG rng;
};


SkipList* skiplist_create(int nblevels) {
	SkipList* slist = malloc(sizeof(SkipList)+sizeof(Node));
	// initialisation skiplist
	slist->sentinel = (Node*)(slist+1);
	slist->sentinel->nb_level = nblevels;
	slist->sentinel->level = malloc(sizeof(Link)*nblevels);
	slist->sentinel->value=666;
	slist->size = 0;
	slist->rng = rng_initialize(0,nblevels);
	//initialisation level of sentinel
	for (int i = 0; i < nblevels; i++) {
		slist->sentinel->level[i].next = slist->sentinel;
		slist->sentinel->level[i].previous = slist->sentinel;
	}
	return slist;
}

void skiplist_delete(SkipList** d) {
	SkipList* l;
	l = *d;
	Node* current = l->sentinel->level[0].next;
	while (current != l->sentinel) {
		Node* next = current->level[0].next;
		free(current->level);
		free(current);
		current = next;
	}
	free(l->sentinel->level);
	free(l);
}

unsigned int skiplist_size(const SkipList* d) {
	return d->size;
}

int skiplist_at(const SkipList* d, unsigned int i) {
	Node* current = d->sentinel->level[0].next;
	unsigned int ith = 0;
	while (ith<i) {
		current = current->level[0].next;
		ith++;
	}
	return current->value;
}

void skiplist_map(const SkipList* d, ScanOperator f, void *environment) {
	Node* current = d->sentinel->level[0].next;
	while (current!=d->sentinel) {
		f(current->value, environment);
		current = current->level[0].next;
	}
}

SkipList* skiplist_insert(SkipList* d, int value) {
	int current_level = d->sentinel->nb_level-1;
	d->sentinel->value = value;
	Node* current = d->sentinel;
	Node* node_to_update[d->sentinel->nb_level];
	//printf("INSERT %d\n",  value);
	while(current_level>=0) { // verifier si il y encore des étages
		while (
			(current->level[current_level].next != d->sentinel)
			&& (current->level[current_level].next->value < value)
		)
		{
			
			current = current->level[current_level].next; // avancer
		}
		if(  current->level[current_level].next != d->sentinel && // tester si on est pas la sentinelle

			current->level[current_level].next->value == value){ // valeur est egale
			return d; // renvoyer
		}
		node_to_update[current_level] = current; // sauvegarder
        current_level--; // changer d'étage
	}

	Node* new = malloc(sizeof(Node));
	new->value = value;
	new->nb_level = rng_get_value(&(d->rng));
	if (new->nb_level < 1) new->nb_level = 1;
	if (new->nb_level > d->sentinel->nb_level) new->nb_level = d->sentinel->nb_level;
 
	new->level = malloc(sizeof(Link)*new->nb_level);

	int i = 0;
	while (i<new->nb_level) // ajouter l'element
	{
		
		node_to_update[i]->level[i].next->level[i].previous = new;
		new->level[i].next = node_to_update[i]->level[i].next;
		node_to_update[i]->level[i].next = new;
		new->level[i].previous = node_to_update[i];
		i++;
		
	}
	d->size++;
	return d;
}

bool skiplist_search(const SkipList* d, int value, unsigned int *nb_operation) {
	*nb_operation += 1;
	int current_level = d->sentinel->nb_level-1;
	d->sentinel->value = value;
	Node* current = d->sentinel;
	//printf("INSERT %d\n",  value);
	while(current_level>=0) { // verifier si il y encore des étages
		while (
			(current->level[current_level].next != d->sentinel)
			&& (current->level[current_level].next->value < value)
		)
		{
			*nb_operation += 1;
			current = current->level[current_level].next; // avancer
		}
		if(  current->level[current_level].next != d->sentinel && // tester si on est pas la sentinelle
			current->level[current_level].next->value == value){ // valeur est egale
			
			return true; // renvoyer
		}
		//*nb_operation += 1;
        current_level--; // changer d'étage
	}
	
	return false;
}

struct s_SkipListIterator {
	SkipList* collection;
	Node* current;
	IteratorDirection direction;
};

SkipListIterator* skiplist_iterator_create(SkipList* d, IteratorDirection w) {
	SkipListIterator* it = malloc(sizeof(SkipListIterator)+sizeof(Node));
	it->collection = d;
	it->current = d->sentinel;
	it->direction = w;
	return it;
}

void skiplist_iterator_delete(SkipListIterator** it) {
	SkipListIterator* i = *it;
	skiplist_delete(&(i->collection));
	free(it);
}


SkipListIterator* skiplist_iterator_begin(SkipListIterator* it) {
	while(it->current!=it->collection->sentinel) {	
		if(it->direction==FORWARD_ITERATOR)	
			it->current = it->current->level[0].next;
		else {
			it->current = it->current->level[0].previous;
		}
	}
	return it;
}

bool skiplist_iterator_end(SkipListIterator* it) {
	return (it->current = it->collection->sentinel);
}


SkipListIterator* skiplist_iterator_next(SkipListIterator* it) {
	if(it->direction==FORWARD_ITERATOR)	
		it->current = it->current->level[0].next;
	else {
		it->current = it->current->level[0].previous;
	}
	return it;
}

int skiplist_iterator_value(SkipListIterator* it){

	return it->current->value;
}
