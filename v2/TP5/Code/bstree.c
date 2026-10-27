#include "bstree.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "queue.h"


/*------------------------  BSTreeType  -----------------------------*/

struct _bstree {
    BinarySearchTree* parent;
    BinarySearchTree* left;
    BinarySearchTree* right;
    int key;
};

/*------------------------  BSTreeAccessType  -----------------------------*/

typedef const BinarySearchTree* (*AccessFunction)(const BinarySearchTree* t);

typedef struct {
    AccessFunction left;
    AccessFunction right;
} ChildAccessor;

/*------------------------  BaseBSTree  -----------------------------*/

BinarySearchTree* bstree_create(void) {
    return NULL;
}

/* This constructor is private so that we can maintain the oredring invariant on
 * nodes. The only way to add nodes to the tree is with the bstree_add function
 * that ensures the invariant.
 */
BinarySearchTree* bstree_cons(BinarySearchTree* left, BinarySearchTree* right, int key) {
    BinarySearchTree* t = malloc(sizeof(struct _bstree));
    t->parent = NULL;
    t->left = left;
    t->right = right;
    if (t->left != NULL)
        t->left->parent = t;
    if (t->right != NULL)
        t->right->parent = t;
    t->key = key;
    return t;
}

void freenode(const BinarySearchTree* t, void* n) {
    (void)n;
    free((BinarySearchTree*)t);
}

void bstree_delete(ptrBinarySearchTree* t) {
    bstree_depth_postfix(*t, freenode, NULL);
    *t=NULL;
}

bool bstree_empty(const BinarySearchTree* t) {
    return t == NULL;
}

int bstree_key(const BinarySearchTree* t) {
    assert(!bstree_empty(t));
    return t->key;
}

BinarySearchTree* bstree_left(const BinarySearchTree* t) {
    assert(!bstree_empty(t));
    return t->left;
}

BinarySearchTree* bstree_right(const BinarySearchTree* t) {
    assert(!bstree_empty(t));
    return t->right;
}

BinarySearchTree* bstree_parent(const BinarySearchTree* t) {
    assert(!bstree_empty(t));
    return t->parent;
}

/*------------------------  BSTreeDictionary  -----------------------------*/

/* Obligation de passer l'arbre par référence pour pouvoir le modifier */
void bstree_add(ptrBinarySearchTree* t, int v) {
    BinarySearchTree* p = *t;
    BinarySearchTree* parent = NULL;
    bool isLeft = true;
    while (!bstree_empty(p)) {
        if(bstree_key(p)==v) return;
        parent = p;
        if(v>bstree_key(p)) {
            p = bstree_right(p);
            isLeft = false;
        } else {
            p = bstree_left(p);
            isLeft = true;
        }
    }
    p=bstree_cons(NULL, NULL, v);
    if(bstree_empty(parent)) { 
    *t = p;
    } else {
        if(isLeft) {
            parent->left=p;
        } else {
            parent->right=p;
        }
        p->parent=parent;
    }
}

const BinarySearchTree* bstree_search(const BinarySearchTree* t, int v) {
    const BinarySearchTree* p = t;
    while (!bstree_empty(p)) {
        if(bstree_key(p)==v) return p;
        else if(bstree_key(p)>v) p = bstree_left(p);
        else p = bstree_right(p);
    }
    return p;
}

const BinarySearchTree* find_next(const BinarySearchTree* x, ChildAccessor access) {
    assert(!bstree_empty(x));
    const BinarySearchTree* cur = x;
    const BinarySearchTree* succ = access.right(x);
    if(!bstree_empty(succ)) {
        while (!bstree_empty(succ->left)) {
            succ = access.left(succ);
        }
    } else {
        succ = cur->parent;
        while (!bstree_empty(succ) && (cur == access.right(succ))) {
            cur = succ;
            succ = succ->parent;
        }
    }
    return succ;
}

const BinarySearchTree* bstree_successor(const BinarySearchTree* x) {
    ChildAccessor access;
    access.left = (AccessFunction)bstree_left;
    access.right = (AccessFunction)bstree_right;
    return find_next(x, access);
}

const BinarySearchTree* bstree_predecessor(const BinarySearchTree* x) {
    ChildAccessor access;
    access.left = (AccessFunction)bstree_right;
    access.right = (AccessFunction)bstree_left;
    return find_next(x, access);
}

void bstree_swap_nodes(ptrBinarySearchTree* tree, ptrBinarySearchTree from, ptrBinarySearchTree to) {
    assert(!bstree_empty(*tree) && !bstree_empty(from) && !bstree_empty(to));
    ptrBinarySearchTree temp_left = to->left;
    ptrBinarySearchTree temp_right = to->right;
    ptrBinarySearchTree temp_parent = to->parent;

    to->parent = from->parent;

    if(!bstree_empty(to->parent)) {
        if(to->parent->left == from) {
            to->parent->left = to;
        } else if(to->parent->right == from) {
            to->parent->right = to;
        }
    } else {
        *tree = to;
    }

    if(from->left == to) {
        to->left = from;
        to->right = from->right;    
        
        from->left = temp_left;
        from->right = temp_right;
        from->parent = to;
        if(!bstree_empty(to->right)) to->right->parent = to;
    } else if(from->right == to) {
        to->left = from->left;
        to->right = from;
        
        from->left = temp_left;
        from->right = temp_right;
        from->parent = to;
        if(!bstree_empty(to->left)) to->left->parent = to;
    } else {
        to->left = from->left;
        to->right = from->right;
        if(!bstree_empty(to->left)) to->left->parent = to;
        if(!bstree_empty(to->right)) to->right->parent = to;
        
        from->left = temp_left;
        from->right = temp_right;
        from->parent = temp_parent;
        if(!bstree_empty(from->left)) from->left->parent = from;
        if(!bstree_empty(from->right)) from->right->parent = from;

        if(!bstree_empty(temp_parent)) {
            if(temp_parent->left==to) {
                temp_parent->left = from;
            } else {
                temp_parent->right = from;
            }
        }
    }
}

// t -> the tree to remove from, current -> the node to remove
void bstree_remove_node(ptrBinarySearchTree* t, ptrBinarySearchTree current) {
    assert(!bstree_empty(*t) && !bstree_empty(current));
    if(bstree_empty(current->left) && bstree_empty(current->right)) {
        if(bstree_empty(current->parent)) {
            *t = NULL;
        } else {
            if(current->parent->left == current)
                current->parent->left = NULL;
            else
                current->parent->right = NULL;
        }
        free(current);
    } else if (bstree_empty(current->left)) {
       current->right->parent = current->parent;
        if(bstree_empty(current->parent)) {
            *t = current->right;
            (*t)->parent = NULL;
        } else if(current == current->parent->left) {
            current->parent->left = current->right;
        } else {
            current->parent->right = current->right;
        }
        free(current);
    } else if (bstree_empty(current->right)) {
        current->left->parent = current->parent;
        if(bstree_empty(current->parent)) {
            *t = current->left;
            (*t)->parent = NULL;
        } else if(current == current->parent->left) {
            current->parent->left = current->left;
        } else {
            current->parent->right = current->left;
        }
        free(current);
    } else {
        bstree_swap_nodes(t, current, (BinarySearchTree*)bstree_successor((const BinarySearchTree*)current));
        bstree_remove_node(t, current);
    }
}

void bstree_remove(ptrBinarySearchTree* t, int v) {
    const BinarySearchTree* p = bstree_search(*t, v);
    if(bstree_empty(p)) return;
    bstree_remove_node(t, (BinarySearchTree*)p);
}

/*------------------------  BSTreeVisitors  -----------------------------*/

void bstree_depth_prefix(const BinarySearchTree* t, OperateFunctor f, void* environment) {
    if(bstree_empty(t)) return;
    f(t, environment);
    bstree_depth_prefix(t->left, f, environment);
    bstree_depth_prefix(t->right, f, environment);
}

void bstree_depth_infix(const BinarySearchTree* t, OperateFunctor f, void* environment) {
    if(bstree_empty(t)) return;
    bstree_depth_infix(t->left, f, environment);
    f(t, environment);
    bstree_depth_infix(t->right, f, environment);
}

void bstree_depth_postfix(const BinarySearchTree* t, OperateFunctor f, void* environment) {
    if(bstree_empty(t)) return;
    bstree_depth_postfix(t->left, f, environment);
    bstree_depth_postfix(t->right, f, environment);
    f(t, environment);
}

void bstree_iterative_breadth(const BinarySearchTree* t, OperateFunctor f, void* environment) {
    Queue *q = create_queue();
    const BinarySearchTree* cur;
    q = queue_push(q, t);
    while (!queue_empty(q)) {
        cur = queue_top(q);
        f(cur, environment);
        if(!bstree_empty(bstree_left(cur))) queue_push(q, bstree_left(cur));
        if(!bstree_empty(bstree_right(cur))) queue_push(q, bstree_right(cur));
        q = queue_pop(q);
    }
    delete_queue(&q);
}

void bstree_iterative_depth_infix(const BinarySearchTree* t, OperateFunctor f, void* environment) {
    if(bstree_empty(t)) return;
    const BinarySearchTree* p = t;
    while (!bstree_empty(p->left)) {
        p = p->left;
    }

    while (!bstree_empty(p)) {
        f(p, environment);
        p = bstree_successor(p);
    }
}

/*------------------------  BSTreeIterator  -----------------------------*/

struct _BSTreeIterator {
    /* the collection the iterator is attached to */
    const BinarySearchTree* collection;
    /* the first element according to the iterator direction */
    const BinarySearchTree* (*begin)(const BinarySearchTree* );
    /* the current element pointed by the iterator */
    const BinarySearchTree* current;
    /* function that goes to the next element according to the iterator direction */
    const BinarySearchTree* (*next)(const BinarySearchTree* );
};

/* minimum element of the collection */
const BinarySearchTree* goto_min(const BinarySearchTree* e) {
	if(bstree_empty(e)) return e;
    const BinarySearchTree* p = e;
    while (!bstree_empty(p->left))
    {
        p = p->left;
    }
    
	return p;
}

/* maximum element of the collection */
const BinarySearchTree* goto_max(const BinarySearchTree* e) {
	if(bstree_empty(e)) return e;
    const BinarySearchTree* p = e;
    while (!bstree_empty(p->right))
    {
        p = p->right;
    }
    
	return p;
}

/* constructor */
BSTreeIterator* bstree_iterator_create(const BinarySearchTree* collection, IteratorDirection direction) {
	BSTreeIterator* it = malloc(sizeof(BSTreeIterator));
	it->collection = collection;
    if(direction == forward) {
        it->begin = goto_min;
        it->next = bstree_successor;
    } else {
        it->begin = goto_max;
        it->next = bstree_predecessor;
    }
    it->current = it->begin(collection);
    return it;
}

/* destructor */
void bstree_iterator_delete(ptrBSTreeIterator* i) {
    free(*i);
    *i = NULL;
}

BSTreeIterator* bstree_iterator_begin(BSTreeIterator* i) {
    i->current = i->begin(i->collection);
    return i;
}

bool bstree_iterator_end(const BSTreeIterator* i) {
    return i->current == NULL;
}

BSTreeIterator* bstree_iterator_next(BSTreeIterator* i) {
    i->current = i->next(i->current);
    return i;
}

const BinarySearchTree* bstree_iterator_value(const BSTreeIterator* i) {
    return i->current;
}

