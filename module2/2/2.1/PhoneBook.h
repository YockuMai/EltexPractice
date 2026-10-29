#ifndef PHONEBOOK_H
#define PHONEBOOK_H
#include "Contact.h"

typedef struct
{
    Contact* items;
    size_t count;
} PhoneBook;

void pb_init(PhoneBook*);
void pb_free(PhoneBook*);

int add_contact(PhoneBook*, const Contact*);
int remove_contact(PhoneBook*, size_t);
#endif