#ifndef PHONEBOOK_H
#define PHONEBOOK_H
#include "Contact.h"

typedef struct PBNode
{
    Contact contact;
    size_t id;
    int height;
    size_t size;
    struct PBNode* left;
    struct PBNode* right;
} PBNode;

typedef struct
{
    PBNode* root;
    size_t count;
    size_t next_id;
} PhoneBook;

void pb_init(PhoneBook*);
void pb_free(PhoneBook*);

int add_contact(PhoneBook*, const Contact*);
int remove_contact(PhoneBook*, size_t);

Contact* pb_get_by_index(const PhoneBook*, size_t);
int pb_rebuild(PhoneBook*);

void pb_print_tree(const PhoneBook*);
void pb_print_preorder(const PhoneBook*);
void pb_print_inorder(const PhoneBook*);
void pb_print_postorder(const PhoneBook*);

#endif