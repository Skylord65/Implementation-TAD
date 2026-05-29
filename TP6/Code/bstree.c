#include "bstree.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "queue.h"

/*------------------------  BSTreeFunction  -----------------------------*/

BinarySearchTree* fixredblack_insert(BinarySearchTree* x);
BinarySearchTree* fixredblack_remove(BinarySearchTree* p, BinarySearchTree* x);

/*------------------------  BSTreeType  -----------------------------*/

typedef enum {red, black} NodeColor;

struct _bstree {
    BinarySearchTree* parent;
    BinarySearchTree* left;
    BinarySearchTree* right;
    NodeColor color;
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
    t->color = red;
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

/* Obligation de passer l'arbre par reference pour pouvoir le modifier */
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
    BinarySearchTree* stop = fixredblack_insert(p) ;
    if (bstree_empty(stop->parent)) *t = stop;
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
    
    BinarySearchTree* substitute = NULL;
    NodeColor deleted = current->color;
    
    if(bstree_empty(current->left) && bstree_empty(current->right)) {
        
        substitute = NULL;
        
        if(deleted == black) {
            BinarySearchTree* subtreeroot = fixredblack_remove(current->parent, substitute);
            if(!bstree_empty(subtreeroot) && bstree_empty(subtreeroot->parent)) *t = subtreeroot;
        }

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

        substitute = current->right;
        current->right->parent = current->parent;

        if(bstree_empty(current->parent)) {
            *t = current->right;
            (*t)->parent = NULL;
        } else if(current == current->parent->left) {
            current->parent->left = current->right;
        } else {
            current->parent->right = current->right;
        }

        if(deleted == black) {
            if(substitute->color == black) {
                BinarySearchTree* subtreeroot = fixredblack_remove(current->parent, substitute);
                if(!bstree_empty(subtreeroot) && bstree_empty(subtreeroot->parent)) *t = subtreeroot;
            }
        } else {
            substitute->color = black;
        }

        free(current);
    } else if (bstree_empty(current->right)) {
        substitute = current->left;
        current->left->parent = current->parent;
        if(bstree_empty(current->parent)) {
            *t = current->left;
            (*t)->parent = NULL;
        } else if(current == current->parent->left) {
            current->parent->left = current->left;
        } else {
            current->parent->right = current->left;
        }

        if(deleted == black) {
            if(substitute->color == black) {
                BinarySearchTree* subtreeroot = fixredblack_remove(current->parent, substitute);
                if(!bstree_empty(subtreeroot) && bstree_empty(subtreeroot->parent)) *t = subtreeroot;   
            } 
        } else {
            substitute->color = black;
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

/* -------------- -------- RBTSpecific ------------- --------- */

BinarySearchTree* grandparent(BinarySearchTree* n) {
    assert(!bstree_empty(n) && !bstree_empty(n->parent));
    return n->parent->parent;
}

BinarySearchTree* uncle(BinarySearchTree* n) {
    BinarySearchTree* g = grandparent(n);
    if(bstree_empty(g)) return g;
    if(n->parent==g->left) {
        return g->right;
    } else {
        return g->left;
    }
}

void bstree_node_to_dot(const BinarySearchTree *t, void *stream) {
    FILE *file = (FILE *) stream;

    printf("%d ", bstree_key(t));
    fprintf(file, "\tn%d [%slabel=\"{%d|{<left>|<right>}}\"];\n",
            bstree_key(t),t->color==0?"style=filled, fillcolor=red, ":"style=filled, fillcolor=grey, ", bstree_key(t));

    if (bstree_left(t)) {
        fprintf(file, "\tn%d:left:c -> n%d:n [headclip=false, tailclip=false]\n",
                bstree_key(t), bstree_key(bstree_left(t)));
    } else {
        fprintf(file, "\tlnil%d [style=filled, fillcolor=grey, label=\"NIL\"];\n", bstree_key(t));
        fprintf(file, "\tn%d:left:c -> lnil%d:n [headclip=false, tailclip=false]\n",
                bstree_key(t), bstree_key(t));
    }
    if (bstree_right(t)) {
        fprintf(file, "\tn%d:right:c -> n%d:n [headclip=false, tailclip=false]\n",
                bstree_key(t), bstree_key(bstree_right(t)));
    } else {
        fprintf(file, "\trnil%d [style=filled, fillcolor=grey, label=\"NIL\"];\n", bstree_key(t));
        fprintf(file, "\tn%d:right:c -> rnil%d:n [headclip=false, tailclip=false]\n",
                bstree_key(t), bstree_key(t));
    }
}

void leftrotate(BinarySearchTree *x) {
    BinarySearchTree* y = x->right;
    BinarySearchTree* y_left = y->left;
    if(!bstree_empty(x->parent)) {
        if(x==x->parent->left) {
            x->parent->left = y;
        } else {
            x->parent->right = y;
        }
    }
    if(!bstree_empty(y->left)) y->left->parent = x;
    y->left = x;
    x->right = y_left;
    y->parent = x->parent;
    x->parent = y;
}

void rightrotate(BinarySearchTree *y) {
    BinarySearchTree* x = y->left;
    BinarySearchTree* x_right = x->right;
    if(!bstree_empty(y->parent)) {
        if(y==y->parent->left) {
            y->parent->left = x;
        } else {
            y->parent->right = x;
        }
    }
    if(!bstree_empty(x->right)) x->right->parent = y;
    x->right = y;
    y->left = x_right;
    x->parent = y->parent;
    y->parent = x;
}

void testrotateleft ( BinarySearchTree * t ) {
    leftrotate(t);
}
void testrotateright ( BinarySearchTree * t ) {
    rightrotate(t);
}

BinarySearchTree* fixredblack_insert_case2_left(BinarySearchTree* x) {
    if(x==x->parent->right) {
        x = x->parent;
        leftrotate(x);
    }
    BinarySearchTree* g = grandparent(x);
    x->parent->color = black;
    g->color = red;
    rightrotate(g);
    return x->parent;
}

BinarySearchTree* fixredblack_insert_case2_right(BinarySearchTree* x) {
    if(x==x->parent->left) {
        x = x->parent;
        rightrotate(x);
    }
    BinarySearchTree* g = grandparent(x);
    x->parent->color = black;
    g->color = red;
    leftrotate(g);
    return x->parent;
}

BinarySearchTree* fixredblack_insert_case2(BinarySearchTree* x) {
    BinarySearchTree* p = x->parent;
    BinarySearchTree* g = grandparent(x);
    if(p==g->left) {
        return fixredblack_insert_case2_left(x);
    } else {
        return fixredblack_insert_case2_right(x);
    }
}

BinarySearchTree* fixredblack_insert_case1(BinarySearchTree* x) {
    BinarySearchTree* u = uncle(x);
    BinarySearchTree* g = grandparent(x); 
    if(!bstree_empty(u) && u->color == red) {
        x->parent->color = black;
        u->color = black;
        g->color = red;
        return fixredblack_insert(g);
    } else {
        return fixredblack_insert_case2(x);
    }
}

BinarySearchTree* fixredblack_insert(BinarySearchTree* x) {
    if(bstree_empty(x)) return x;
    if(bstree_empty(x->parent)) {
        x->color = black;
        return x;
    }
    if(x->parent->color == black) return x;
    BinarySearchTree* g = grandparent(x);
    if(bstree_empty(g)) {
        x->parent->color = black;
        return x;
    }
    return fixredblack_insert_case1(x);
}

BinarySearchTree* fixredblack_remove_case1_left(BinarySearchTree* p){
    if(bstree_empty(p->right)) return NULL;
    rightrotate(p);
    p->color = red;
    p->right->color = black;
    return fixredblack_remove(p, p->right);
}

BinarySearchTree* fixredblack_remove_case1_right(BinarySearchTree* p) {
    if(bstree_empty(p->left)) return NULL;
    rightrotate(p);
    p->color = red;
    p->left->color = black;
    return fixredblack_remove(p, p->left);
}

BinarySearchTree* fixredblack_remove_case2_left(BinarySearchTree* p){
    if(bstree_empty(p->right)) return NULL;
    rightrotate(p);
    p->color = red;
    p->right->color = black;
    return fixredblack_remove(p, p->right);
}

BinarySearchTree* fixredblack_remove_case2_right(BinarySearchTree* p) {
    if(bstree_empty(p->left)) return NULL;
    rightrotate(p);
    p->color = red;
    p->left->color = black;
    return fixredblack_remove(p, p->left);
}



BinarySearchTree* fixredblack_remove(BinarySearchTree* p, BinarySearchTree* x) {
    if(bstree_empty(x)) return x;
    if(bstree_empty(x->parent)) return x;
    if(x->parent->left == x) {
        if(bstree_empty(x->parent->right)) return x->parent;
        if(x->parent->right->color == red) {
            return fixredblack_remove_case2(p, x);
        } else {
            return fixredblack_remove_case1(p, x);
        }
    } else {
        if(bstree_empty(x->parent->left)) return x->parent;
        if(x->parent->left->color == red) {
            return fixredblack_remove_case2(p, x);
        } else {
            return fixredblack_remove_case1(p, x);
        }
    }
}