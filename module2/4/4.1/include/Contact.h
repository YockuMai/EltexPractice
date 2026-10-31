#ifndef CONTACT_H
#define CONTACT_H
#include <stdlib.h>

typedef struct 
{
    char* surname;
    char* name;
    char* patronymic;
    char* workplace;
    char* position;
    char** phones;
    size_t phone_count;
    char** emails;
    size_t email_count;
    char** socials;
    size_t social_count;
    char** messengers;
    size_t messenger_count;
} Contact;

void contact_init(Contact*);
void contact_free(Contact*);
int contact_clone(Contact*, const Contact*);

int contact_set_surname(Contact* c, const char* value);
int contact_set_name(Contact* c, const char* value);
int contact_set_patronymic(Contact* c, const char* value);
int contact_set_workplace(Contact* c, const char* value);
int contact_set_position(Contact* c, const char* value);
int contact_add_phone(Contact* c, const char* value);
int contact_add_email(Contact* c, const char* value);
int contact_add_social(Contact* c, const char* value);
int contact_add_messenger(Contact* c, const char* value);

int contact_remove_phone (Contact*, size_t);
int contact_remove_email (Contact*, size_t);
int contact_remove_social (Contact*, size_t);
int contact_remove_messenger(Contact*, size_t);
#endif