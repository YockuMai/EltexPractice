#include "PhoneBook.h"
#include "Strings.h"

// Инициализирует телефонную книгу
void pb_init(PhoneBook* pb) {
    pb->head = NULL;
    pb->tail = NULL;
    pb->count = 0;
}

// Освобождает память, выделенную для контактов в телефонной книге, и инициализирует её
void pb_free(PhoneBook* pb) {
    ContactNode* cur = pb->head;
    while (cur) {
        ContactNode* next = cur->next;
        contact_free(&cur->data);
        free(cur);
        cur=next;
    }
    pb_init(pb);
}

int contact_compare(const Contact* a, const Contact* b) {
    int r = my_cmp_str(a->surname, b->surname);
    if (r) return r;
    r = my_cmp_str(a->name, b->name);
    if (r) return r;
    return my_cmp_str(a->patronymic, b->patronymic);
}

// Создаёт узел с копией контакта. NULL при ошибке памяти.
static ContactNode* node_create(const Contact* c) {
    ContactNode* node = malloc(sizeof(*node));
    if (!node) return NULL;
    if (contact_clone(&node->data, c) != 0) {
        free(node);
        return NULL;
    }
    node->prev = NULL;
    node->next = NULL;
    return node;
}

// Вставляет узел на правильное место. Дубликаты попадают ПОСЛЕ существующих.
static void insert_ordered(PhoneBook* pb, ContactNode* node) {
    ContactNode* prev = NULL;
    ContactNode* cur = pb->head;
    while (cur && contact_compare(&cur->data, &node->data) <= 0) {
        prev = cur;
        cur = cur->next;
    }
    node->prev = prev;
    node->next = cur;
    if (prev) prev->next = node; else pb->head = node;
    if (cur)  cur->prev  = node; else pb->tail = node;
    pb->count++;
}

// Отвязывает узел от списка, не освобождая ни узел, ни его данные.
static void unlink_node(PhoneBook* pb, ContactNode* node) {
    if (node->prev) node->prev->next = node->next; else pb->head = node->next;
    if (node->next) node->next->prev = node->prev; else pb->tail = node->prev;
    node->prev = NULL;
    node->next = NULL;
    pb->count--;
}

// Добавляет контакт в телефонную книгу. Возвращает 0 при успехе, -1 при ошибке памяти
int add_contact(PhoneBook* pb, const Contact* c) {
    ContactNode* node = node_create(c);
    if (!node) return -1;
    insert_ordered(pb, node);
    return 0;
}

ContactNode* pb_node_at(PhoneBook* pb, size_t index) {
    if (index >= pb->count) return NULL;
    ContactNode* cur = pb->head;
    for (size_t i = 0; i < index; i++) cur = cur->next;
    return cur;
}

void pb_reinsert_node(PhoneBook* pb, ContactNode* node) {
    unlink_node(pb, node);
    insert_ordered(pb, node);
}

// Удаляет контакт из телефонной книги по индексу. Возвращает 0 при успехе, -1 при ошибке
int remove_contact(PhoneBook* pb, size_t index) {
    ContactNode* node = pb_node_at(pb, index);
    if (!node) return -1;
    unlink_node(pb, node);
    contact_free(&node->data);
    free(node);
    return 0;
}