#include "PhoneBook.h"

// Инициализирует телефонную книгу
void pb_init(PhoneBook* pb) {
    pb->items = NULL;
    pb->count = 0;
}

// Освобождает память, выделенную для контактов в телефонной книге, и инициализирует её
void pb_free(PhoneBook* pb) {
    for (size_t i = 0; i < pb->count; i++)
        contact_free(&pb->items[i]);
    free(pb->items);
    pb_init(pb);
}

// Добавляет контакт в телефонную книгу. Возвращает 0 при успехе, -1 при ошибке памяти
int add_contact(PhoneBook* pb, const Contact* c) {
    Contact copy;
    if (contact_clone(&copy, c) != 0) return -1;

    Contact* tmp = realloc(pb->items, (pb->count + 1) * sizeof(Contact));
    if (!tmp) { contact_free(&copy); return -1; }
    pb->items = tmp;
    pb->items[pb->count] = copy;
    pb->count++;
    return 0;
}

// Удаляет контакт из телефонной книги по индексу. Возвращает 0 при успехе, -1 при ошибке
int remove_contact(PhoneBook* pb, size_t index) {
    if (index >= pb->count) {
        return -1;
    }
    contact_free(&pb->items[index]);
    for (size_t i = index; i < pb->count - 1; i++) {
        pb->items[i] = pb->items[i + 1];
    }
    pb->count--;
    return 0;
}