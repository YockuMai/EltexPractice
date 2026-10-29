#include "Contact.h"
#include "Strings.h"

// Инициализирует контакт
void contact_init(Contact* c) {
    c->surname = NULL;
    c->name = NULL;
    c->patronymic = NULL;
    c->workplace = NULL;
    c->position = NULL;
    c->phones = NULL;
    c->phone_count = 0;
    c->emails = NULL;
    c->email_count = 0;
    c->socials = NULL;
    c->social_count = 0;
    c->messengers = NULL;
    c->messenger_count = 0;
}

// Освобождает память, выделенную для полей контакта, и инициализирует его
void contact_free(Contact* c) {
    free(c->surname);
    free(c->name);
    free(c->patronymic);
    free(c->workplace);
    free(c->position);
    for (size_t i = 0; i<c->phone_count; i++) free(c->phones[i]);
    free(c->phones);
    for (size_t i = 0; i<c->social_count; i++) free(c->socials[i]);
    free(c->socials);
    for (size_t i = 0; i<c->messenger_count; i++) free(c->messengers[i]);
    free(c->messengers);
    for (size_t i = 0; i < c->email_count; i++) free(c->emails[i]);
    free(c->emails);

    contact_init(c);
}

// Клонирует поля контакта src в dst. dst должен быть инициализирован.
static int clone_fields(Contact* dst, const Contact* src) {
    if (contact_set_surname   (dst, src->surname)    != 0) return -1;
    if (contact_set_name      (dst, src->name)       != 0) return -1;
    if (contact_set_patronymic(dst, src->patronymic) != 0) return -1;
    if (contact_set_workplace (dst, src->workplace)  != 0) return -1;
    if (contact_set_position  (dst, src->position)   != 0) return -1;

    for (size_t i = 0; i < src->phone_count; i++)
        if (contact_add_phone(dst, src->phones[i]) != 0) return -1;
    for (size_t i = 0; i < src->email_count; i++)
        if (contact_add_email(dst, src->emails[i]) != 0) return -1;
    for (size_t i = 0; i < src->social_count; i++)
        if (contact_add_social(dst, src->socials[i]) != 0) return -1;
    for (size_t i = 0; i < src->messenger_count; i++)
        if (contact_add_messenger(dst, src->messengers[i]) != 0) return -1;

    return 0;
}

// Клонирует контакт src в dst. dst должен быть инициализирован.
int contact_clone(Contact* dst, const Contact* src) {
    contact_init(dst);
    if (clone_fields(dst, src) != 0) {
        contact_free(dst);
        return -1;
    }
    return 0;
}

// Устанавливает значение поля, освобождая старое значение и дублируя новое
static int set_field(char** field, const char* value) {
    char* dup = (value && value[0]) ? my_strdup(value) : NULL;
    if (value && value[0] && !dup) return -1;
    free(*field);
    *field = dup;
    return 0;
}

// Добавляет строку в массив строк, увеличивая его размер на 1
static int push_str(char*** arr, size_t* count, const char* value) {
    if (!value || value[0] == '\0') return 0;
    char* dup = my_strdup(value);
    if (!dup) return -1;
    char** tmp = realloc(*arr, (*count + 1) * sizeof(char*));
    if (!tmp) { free(dup); return -1; }
    tmp[*count] = dup;
    *arr = tmp;
    (*count)++;
    return 0;
}

// Удаляет строку из массива строк по индексу, уменьшая его размер на 1
static int remove_at(char*** arr, size_t* count, size_t idx) {
    if (idx >= *count) return -1;
    free((*arr)[idx]);
    for (size_t i = idx; i + 1 < *count; i++)
        (*arr)[i] = (*arr)[i + 1];
    (*count)--;
    return 0;
}

// Функции для установки полей контакта и добавления/удаления элементов из массивов строк
int contact_set_surname(Contact* c, const char* value) { return set_field(&c->surname, value); }
int contact_set_name(Contact* c, const char* value) { return set_field(&c->name, value); }
int contact_set_patronymic(Contact* c, const char* value) { return set_field(&c->patronymic, value); }
int contact_set_workplace(Contact* c, const char* value) { return set_field(&c->workplace, value); }
int contact_set_position(Contact* c, const char* value) { return set_field(&c->position, value); }
int contact_add_phone(Contact* c, const char* value) { return push_str(&c->phones, &c->phone_count, value); }
int contact_add_email(Contact* c, const char* value) { return push_str(&c->emails, &c->email_count, value); }
int contact_add_social(Contact* c, const char* value) { return push_str(&c->socials, &c->social_count, value); }
int contact_add_messenger(Contact* c, const char* value) { return push_str(&c->messengers, &c->messenger_count, value); }

int contact_remove_phone (Contact* c, size_t i) { return remove_at(&c->phones, &c->phone_count, i); }
int contact_remove_email (Contact* c, size_t i) { return remove_at(&c->emails, &c->email_count, i); }
int contact_remove_social (Contact* c, size_t i) { return remove_at(&c->socials, &c->social_count, i); }
int contact_remove_messenger(Contact* c, size_t i) { return remove_at(&c->messengers, &c->messenger_count, i); }