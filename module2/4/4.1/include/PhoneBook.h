#ifndef PHONEBOOK_H
#define PHONEBOOK_H
#include "Contact.h"

typedef struct ContactNode
{
    Contact data;
    struct ContactNode* next;
    struct ContactNode* prev;
} ContactNode;

typedef struct
{
    ContactNode* head;
    ContactNode* tail;
    size_t count;
} PhoneBook;

void pb_init(PhoneBook*);
void pb_free(PhoneBook*);

int contact_compare(const Contact*, const Contact*);

int add_contact(PhoneBook*, const Contact*);
int remove_contact(PhoneBook*, size_t);

ContactNode* pb_node_at(PhoneBook*, size_t index);
void pb_reinsert_node(PhoneBook*, ContactNode*);

#endif