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
    ptrBinarySearchTree* cur = t;
    BinarySearchTree* par = NULL;
    int left = 0;
    while ((*cur)!=NULL) {
        //printf("parkour : %d\n", (*cur)->key);
        if((*cur)->key<v) {
            par = *cur;
            cur = &((*cur)->right);
            left=0;
        } else if((*cur)->key>v) {
            par = *cur;
            cur = &((*cur)->left);
            left=1;
        } else {
            return;   
        }
    }
    if(par==NULL) {
        *t = bstree_cons(NULL, NULL, v);
        //printf("the new tree : %d\n", (*t)->key);
    } else if(left==1) {
        (*cur) = bstree_cons(NULL, NULL, v);
        (*cur)->parent=par;
        //printf("parent : %d\n", (*cur)->parent->key);
        //printf("the new tree left : %d\n", (*cur)->key);
    } else if(left==0) {
        (*cur) = bstree_cons(NULL, NULL, v);
        (*cur)->parent = par;
        //printf("parent : %d\n", (*cur)->parent->key);
        //printf("the new tree right : %d\n", (*cur)->key);
    }
}

const BinarySearchTree* bstree_search(const BinarySearchTree* t, int v) {
    const BinarySearchTree* cur = t;
    while (cur!=NULL)
    {
        if (cur->key==v)
        {
            return cur;
        } else if(cur->key>v) {
            cur = cur->left;
        } else if(cur->key<v) {
            cur = cur->right;
        }   
    }
    return cur;
}

static const BinarySearchTree* find_next(const BinarySearchTree* x, ChildAccessor access) {
    assert(!bstree_empty(x));
    const BinarySearchTree* cur = x;
    if(access.right(cur)!=NULL) {
        cur = access.right(cur); 
        while (access.left(cur)!=NULL) {
            cur = access.left(cur);
        }
        return cur;
    } else if(cur->parent!=NULL && cur == access.right(cur->parent)) {
        cur = cur->parent;
        //printf("predessor cur 1 : %d\n", cur->key);
        while (!(cur->parent==NULL || cur == access.left(cur->parent)))
        {
            cur = cur->parent;
            //printf("predessor cur n : %d\n", cur->key);
        }
        
        return cur->parent;
    } else {
        return cur->parent;
    }
}

const BinarySearchTree* bstree_successor(const BinarySearchTree* x) {
    assert(!bstree_empty(x));
    ChildAccessor access; 
    access.left = (AccessFunction) bstree_left;
    access.right = (AccessFunction) bstree_right;
    return find_next(x, access);
}

const BinarySearchTree* bstree_predecessor(const BinarySearchTree* x) {
    assert(!bstree_empty(x));
    ChildAccessor access; 
    access.left = (AccessFunction) bstree_right;
    access.right = (AccessFunction) bstree_left;
    return find_next(x, access);
}

void bstree_swap_nodes(ptrBinarySearchTree* tree, ptrBinarySearchTree from, ptrBinarySearchTree to) {
    assert(!bstree_empty(*tree) && !bstree_empty(from) && !bstree_empty(to));
    ptrBinarySearchTree to_right_child = to->right;
    ptrBinarySearchTree to_left_child = to->left;
    ptrBinarySearchTree from_parent = from->parent;
    ptrBinarySearchTree to_parent = to->parent;
    ptrBinarySearchTree from_right_child = from->right;
    ptrBinarySearchTree from_left_child = from->left;

    if(from == to_parent) {
        to->parent = from_parent;
        to->left = from_left_child;
        to->right = from_right_child;
        from->parent = to;
        from->left = to_left_child;
        from->right=to_right_child;
        if(to->left==to) to->left = from;
        if(to->right==to) to->right = from;
        if(to->parent==NULL) {
            *tree = to;
        } else if(to->parent->left == from) {
            to->parent->left = to;
        } else if(to->parent->right == from) {
            to->parent->right = to;
        }
        if(to->left!=NULL && to->left!=from) to->left->parent = to;
        if(to->right!=NULL && to->right!=from) to->right->parent = to;
        if(from->left!=NULL) from->left->parent = from;
        if(from->right!=NULL) from->right->parent = from;
        return;
    }

    from->parent = to_parent;
    to->parent = from_parent;
    from->left = to_left_child;
    from->right = to_right_child;
    to->left = from_left_child;
    to->right = from_right_child;
    if(to->parent==NULL) {
        *tree = to;
    } else if(to->parent->left == from) {
        to->parent->left = to;
    } else if(to->parent->right == from) {
        to->parent->right = to;
    }
    if(from->parent==NULL) {
        *tree = from;
    } else if(from->parent->left == to) {
        from->parent->left = from;
    } else if(from->parent->right == to) {
        from->parent->right = from;
    }

    if(to->left!=NULL) to->left->parent = to;
    if(to->right!=NULL) to->right->parent = to;
    if(from->left!=NULL) from->left->parent = from;
    if(from->right!=NULL) from->right->parent = from;
}

// t -> the tree to remove from, current -> the node to remove
void bstree_remove_node(ptrBinarySearchTree* t, ptrBinarySearchTree current) {
    assert(!bstree_empty(*t) && !bstree_empty(current));
    if(current->left==NULL && current->right==NULL) {
        if(current->parent==NULL) {
            free(*t);
            *t = NULL;
        } else if(current == current->parent->left) {
            current->parent->left = NULL;
        } else {
            current->parent->right = NULL;
        }
        free(current);
    } else if(current->left!=NULL && current->right==NULL) {
        current->left->parent = current->parent;
        if(current->parent==NULL) {

            *t = current->left;
            (*t)->parent = NULL;
        } else if(current == current->parent->left) {
            current->parent->left = current->left;
        } else {
            current->parent->right = current->left;
        }
        free(current);
    } else if(current->left==NULL && current->right!=NULL) {
        current->right->parent = current->parent;
        if(current->parent==NULL) {

            *t = current->right;
            (*t)->parent = NULL;
        } else if(current == current->parent->left) {
            current->parent->left = current->right;
        } else {
            current->parent->right = current->right;
        }
        free(current);
    } else {
        BinarySearchTree* succ = (BinarySearchTree*)bstree_successor(current);
        bstree_swap_nodes(t, current, succ);
        bstree_remove_node(t, current);
    }
    
}

void bstree_remove(ptrBinarySearchTree* t, int v) {
    BinarySearchTree* temp = (BinarySearchTree*)bstree_search(*t, v);
    if(temp!=NULL) bstree_remove_node(t, temp);
}

/*------------------------  BSTreeVisitors  -----------------------------*/

void bstree_depth_prefix(const BinarySearchTree* t, OperateFunctor f, void* environment) {
    if(!bstree_empty(t)) {
        f(t, environment);
        bstree_depth_prefix(t->left, f, environment);
        bstree_depth_prefix(t->right, f, environment);
    }
}

void bstree_depth_infix(const BinarySearchTree* t, OperateFunctor f, void* environment) {
    if(!bstree_empty(t)) {
        bstree_depth_infix(t->left, f, environment);
        f(t, environment);
        bstree_depth_infix(t->right, f, environment);
    }
}

void bstree_depth_postfix(const BinarySearchTree* t, OperateFunctor f, void* environment) {
    if(!bstree_empty(t)) {
        bstree_depth_postfix(t->left, f, environment);
        bstree_depth_postfix(t->right, f, environment);
        f(t, environment);
    }
}

void bstree_iterative_breadth(const BinarySearchTree* t, OperateFunctor f, void* environment) {
    if(!bstree_empty(t)) {
        Queue* q = create_queue();
        q=queue_push(q, t);
        while (!queue_empty(q)) {
            const BinarySearchTree* cur;
            cur = queue_top(q);
            f(cur, environment);
            q = queue_pop(q);
            if(!bstree_empty(bstree_left(cur))) q = queue_push(q, bstree_left(cur));
            if(!bstree_empty(bstree_right(cur))) q = queue_push(q, bstree_right(cur));
        }
        delete_queue(&q);
    }
}

void bstree_iterative_depth_infix(const BinarySearchTree* t, OperateFunctor f, void* environment) {
    if(bstree_empty(t)) return;

    const BinarySearchTree* cur = t;
    while (cur->left!=NULL) {
        cur = cur->left;
    }

    while(cur!=NULL) {
        f(cur, environment);
        cur = bstree_successor(cur);
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
	const BinarySearchTree* current = e;
    while (current->left!=NULL)
    {
        current=current->left;
    }
    
	return current;
}

/* maximum element of the collection */
const BinarySearchTree* goto_max(const BinarySearchTree* e) {
	const BinarySearchTree* current = e;
    while (current->right!=NULL)
    {
        current=current->right;
    }
    
	return current;
}

/* constructor */
BSTreeIterator* bstree_iterator_create(const BinarySearchTree* collection, IteratorDirection direction) {
	if(collection == NULL) return NULL;
    BSTreeIterator* i = malloc(sizeof(BSTreeIterator));
    i->collection = collection;
    if(direction==forward) {
        i->begin = goto_min;
        i->next = bstree_successor;
    } else {
        i->begin =goto_max;
        i->next = bstree_predecessor;
    }
    i->current = i->begin(collection);
	return i;
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

