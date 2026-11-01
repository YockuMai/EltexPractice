#include "PhoneBook.h"
#include "Strings.h"

static int height(PBNode* n) {
    return n ? n->height : 0;
}

static size_t node_size(PBNode* n) {
    return n ? n->size : 0;
}

static void update(PBNode* n) {
    if (!n) return;

    int hl = height(n->left);
    int hr = height(n->right);

    n->height = 1 + (hl > hr ? hl : hr);
    n->size = 1 + node_size(n->left) + node_size(n->right);
}

static int balance_factor(PBNode* n) {
    return n ? height(n->left) - height(n->right) : 0;
}

static PBNode* rotate_left(PBNode* x) {
    PBNode* y = x->right;
    PBNode* t2 = y->left;

    y->left = x;
    x->right = t2;

    update(x);
    update(y);

    return y;
}

static PBNode* rotate_right(PBNode* y) {
    PBNode* x = y->left;
    PBNode* t2 = x->right;

    x->right = y;
    y->left = t2;

    update(y);
    update(x);

    return x;
}

static PBNode* balance(PBNode* n) {
    if (!n) return NULL;

    update(n);

    int bf = balance_factor(n);

    if (bf > 1) {
        if (balance_factor(n->left) < 0)
            n->left = rotate_left(n->left);
        return rotate_right(n);
    }

    if (bf < -1) {
        if (balance_factor(n->right) > 0)
            n->right = rotate_right(n->right);
        return rotate_left(n);
    }

    return n;
}

static int contact_cmp(const Contact* a, const Contact* b) {
    const char* as = a->surname ? a->surname : "";
    const char* bs = b->surname ? b->surname : "";
    int r = my_strcmp(as, bs);
    if (r) return r;

    const char* an = a->name ? a->name : "";
    const char* bn = b->name ? b->name : "";
    r = my_strcmp(an, bn);
    if (r) return r;

    const char* ap = a->patronymic ? a->patronymic : "";
    const char* bp = b->patronymic ? b->patronymic : "";
    return my_strcmp(ap, bp);
}

static int node_cmp(const PBNode* a, const PBNode* b) {
    int r = contact_cmp(&a->contact, &b->contact);
    if (r) return r;

    if (a->id < b->id) return -1;
    if (a->id > b->id) return 1;
    return 0;
}

static PBNode* insert_node(PBNode* root, PBNode* node) {
    if (!root) return node;

    if (node_cmp(node, root) < 0)
        root->left = insert_node(root->left, node);
    else
        root->right = insert_node(root->right, node);

    return balance(root);
}

static PBNode* delete_min(PBNode* n, PBNode** min_out) {
    if (!n) {
        *min_out = NULL;
        return NULL;
    }

    if (!n->left) {
        *min_out = n;
        return n->right;
    }

    n->left = delete_min(n->left, min_out);
    return balance(n);
}

static PBNode* delete_node(PBNode* root, PBNode* target, int* removed) {
    if (!root) return NULL;

    int cmp = node_cmp(root, target);

    if (cmp < 0) {
        /* root < target → target в правом поддереве */
        root->right = delete_node(root->right, target, removed);
    } else if (cmp > 0) {
        /* root > target → target в левом поддереве */
        root->left = delete_node(root->left, target, removed);
    } else {
        *removed = 1;

        if (!root->left || !root->right) {
            PBNode* child = root->left ? root->left : root->right;
            contact_free(&root->contact);
            free(root);
            return child;
        }

        PBNode* succ = NULL;
        root->right = delete_min(root->right, &succ);

        contact_free(&root->contact);
        root->contact = succ->contact;
        root->id = succ->id;

        contact_init(&succ->contact);
        free(succ);
    }

    return balance(root);
}

static PBNode* find_by_index(PBNode* n, size_t index) {
    if (!n) return NULL;

    size_t left_size = node_size(n->left);

    if (index < left_size)
        return find_by_index(n->left, index);

    if (index == left_size)
        return n;

    return find_by_index(n->right, index - left_size - 1);
}

static void free_nodes(PBNode* n) {
    if (!n) return;

    free_nodes(n->left);
    free_nodes(n->right);

    contact_free(&n->contact);
    free(n);
}

void pb_init(PhoneBook* pb) {
    pb->root = NULL;
    pb->count = 0;
    pb->next_id = 0;
}

void pb_free(PhoneBook* pb) {
    free_nodes(pb->root);
    pb_init(pb);
}

int add_contact(PhoneBook* pb, const Contact* c) {
    PBNode* node = (PBNode*)malloc(sizeof(PBNode));
    if (!node) return -1;

    contact_init(&node->contact);

    if (contact_clone(&node->contact, c) != 0) {
        free(node);
        return -1;
    }

    node->id = pb->next_id++;
    node->height = 1;
    node->size = 1;
    node->left = NULL;
    node->right = NULL;

    pb->root = insert_node(pb->root, node);
    pb->count++;

    return 0;
}

int remove_contact(PhoneBook* pb, size_t index) {
    PBNode* target = find_by_index(pb->root, index);
    if (!target) return -1;

    int removed = 0;
    pb->root = delete_node(pb->root, target, &removed);

    if (!removed) return -1;

    pb->count--;
    return 0;
}

Contact* pb_get_by_index(const PhoneBook* pb, size_t index) {
    PBNode* node = find_by_index(pb->root, index);
    return node ? &node->contact : NULL;
}

static void collect_nodes(PBNode* n, PBNode** arr, size_t* i) {
    if (!n) return;

    collect_nodes(n->left, arr, i);
    arr[(*i)++] = n;
    collect_nodes(n->right, arr, i);
}

static int cmp_node_ptr(const void* a, const void* b) {
    const PBNode* na = *(PBNode* const*)a;
    const PBNode* nb = *(PBNode* const*)b;
    return node_cmp(na, nb);
}

static PBNode* build_balanced(PBNode** arr, size_t l, size_t r) {
    if (l >= r) return NULL;

    size_t m = l + (r - l) / 2;
    PBNode* root = arr[m];

    root->left = build_balanced(arr, l, m);
    root->right = build_balanced(arr, m + 1, r);

    update(root);
    return root;
}

int pb_rebuild(PhoneBook* pb) {
    if (pb->count == 0) {
        pb->root = NULL;
        return 0;
    }

    PBNode** arr = (PBNode**)malloc(pb->count * sizeof(PBNode*));
    if (!arr) return -1;

    size_t i = 0;
    collect_nodes(pb->root, arr, &i);

    qsort(arr, i, sizeof(PBNode*), cmp_node_ptr);

    pb->root = build_balanced(arr, 0, i);
    pb->count = i;

    free(arr);
    return 0;
}