#ifndef INTERFACE_H
#define INTERFACE_H
#include "PhoneBook.h"
#include "Contact.h"

void menu_run(PhoneBook*);
void menu_add_contact(PhoneBook*);
void menu_remove_contact(PhoneBook*);
void menu_edit_contact(PhoneBook*);
void menu_print_contact_list(const PhoneBook*);
void menu_print_contact(const Contact*);

#endif