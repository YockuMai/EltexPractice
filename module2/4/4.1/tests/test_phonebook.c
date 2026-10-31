#include "unity.h"
#include "../include/PhoneBook.h"

void setUp(void) {}
void tearDown(void) {}

static Contact make_contact(const char *surname, const char *name) {
    Contact c;
    contact_init(&c);
    contact_set_surname(&c, surname);
    contact_set_name(&c, name);
    return c;
}

// Проверяет инварианты двусвязного списка: prev/next, head/tail, count.
static void assert_invariants(PhoneBook *pb) {
    size_t n = 0;
    ContactNode *prev = NULL;
    for (ContactNode *cur = pb->head; cur; cur = cur->next) {
        TEST_ASSERT_EQUAL_PTR(prev, cur->prev);
        prev = cur;
        n++;
    }
    TEST_ASSERT_EQUAL_PTR(prev, pb->tail);
    TEST_ASSERT_EQUAL_size_t(n, pb->count);
}

// Проверяет, что список отсортирован по contact_compare.
static void assert_ordered(PhoneBook *pb) {
    for (ContactNode *cur = pb->head; cur && cur->next; cur = cur->next) {
        TEST_ASSERT_LESS_OR_EQUAL_INT(0,
            contact_compare(&cur->data, &cur->next->data));
    }
}

// Возвращает фамилию i-го контакта (для читаемости проверок порядка).
static const char *nth_surname(PhoneBook *pb, size_t idx) {
    ContactNode *node = pb_node_at(pb, idx);
    return node ? node->data.surname : NULL;
}

// ---------------- init ----------------

static void test_init_empty(void) {
    PhoneBook pb;
    pb_init(&pb);
    TEST_ASSERT_NULL(pb.head);
    TEST_ASSERT_NULL(pb.tail);
    TEST_ASSERT_EQUAL_size_t(0, pb.count);
    pb_free(&pb);
}

// ---------------- contact_compare ----------------

static void test_compare_primary_surname(void) {
    Contact a = make_contact("Иванов", "Иван");
    Contact b = make_contact("Петров", "Пётр");
    TEST_ASSERT_LESS_THAN(0, contact_compare(&a, &b));
    TEST_ASSERT_GREATER_THAN(0, contact_compare(&b, &a));
    TEST_ASSERT_EQUAL_INT(0, contact_compare(&a, &a));
    contact_free(&a);
    contact_free(&b);
}

static void test_compare_secondary_name(void) {
    Contact a = make_contact("Иванов", "Алексей");
    Contact b = make_contact("Иванов", "Борис");
    TEST_ASSERT_LESS_THAN(0, contact_compare(&a, &b));
    contact_free(&a);
    contact_free(&b);
}

static void test_compare_tertiary_patronymic(void) {
    Contact a = make_contact("Иванов", "Иван");
    Contact b = make_contact("Иванов", "Иван");
    contact_set_patronymic(&a, "Алексеевич");
    contact_set_patronymic(&b, "Борисович");
    TEST_ASSERT_LESS_THAN(0, contact_compare(&a, &b));
    contact_free(&a);
    contact_free(&b);
}

static void test_compare_null_as_empty(void) {
    Contact a;
    contact_init(&a);                         // всё NULL = ""
    Contact b = make_contact("A", "B");
    TEST_ASSERT_LESS_THAN(0, contact_compare(&a, &b));   // "" < "A"
    TEST_ASSERT_GREATER_THAN(0, contact_compare(&b, &a));
    contact_free(&a);
    contact_free(&b);
}

// ---------------- add_contact ----------------

static void test_add_single(void) {
    PhoneBook pb;
    pb_init(&pb);
    Contact a = make_contact("Иванов", "Иван");
    TEST_ASSERT_EQUAL_INT(0, add_contact(&pb, &a));
    TEST_ASSERT_EQUAL_size_t(1, pb.count);
    TEST_ASSERT_NOT_NULL(pb.head);
    TEST_ASSERT_EQUAL_PTR(pb.head, pb.tail);
    TEST_ASSERT_EQUAL_STRING("Иванов", pb.head->data.surname);
    assert_invariants(&pb);
    contact_free(&a);
    pb_free(&pb);
}

static void test_add_orders(void) {
    PhoneBook pb;
    pb_init(&pb);
    Contact c = make_contact("Сидоров", "Сидр");
    Contact a = make_contact("Иванов", "Иван");
    Contact b = make_contact("Петров", "Пётр");
    TEST_ASSERT_EQUAL_INT(0, add_contact(&pb, &c));   // нарочно в беспорядке
    TEST_ASSERT_EQUAL_INT(0, add_contact(&pb, &a));
    TEST_ASSERT_EQUAL_INT(0, add_contact(&pb, &b));
    TEST_ASSERT_EQUAL_size_t(3, pb.count);
    TEST_ASSERT_EQUAL_STRING("Иванов",  nth_surname(&pb, 0));
    TEST_ASSERT_EQUAL_STRING("Петров",  nth_surname(&pb, 1));
    TEST_ASSERT_EQUAL_STRING("Сидоров", nth_surname(&pb, 2));
    assert_invariants(&pb);
    assert_ordered(&pb);
    contact_free(&a);
    contact_free(&b);
    contact_free(&c);
    pb_free(&pb);
}

static void test_add_duplicates_after(void) {
    PhoneBook pb;
    pb_init(&pb);
    Contact first  = make_contact("Иванов", "Иван");
    Contact second = make_contact("Иванов", "Иван");
    contact_add_phone(&first,  "111");
    contact_add_phone(&second, "222");
    add_contact(&pb, &first);
    add_contact(&pb, &second);
    TEST_ASSERT_EQUAL_size_t(2, pb.count);
    // Дубликат вставляется ПОСЛЕ существующих (согласно insert_ordered)
    TEST_ASSERT_EQUAL_STRING("111", pb.head->data.phones[0]);
    TEST_ASSERT_EQUAL_STRING("222", pb.tail->data.phones[0]);
    assert_invariants(&pb);
    contact_free(&first);
    contact_free(&second);
    pb_free(&pb);
}

// ---------------- remove_contact ----------------

static void test_remove_head(void) {
    PhoneBook pb;
    pb_init(&pb);
    Contact a = make_contact("Иванов", "Иван");
    Contact b = make_contact("Петров", "Пётр");
    add_contact(&pb, &a);
    add_contact(&pb, &b);
    TEST_ASSERT_EQUAL_INT(0, remove_contact(&pb, 0));
    TEST_ASSERT_EQUAL_size_t(1, pb.count);
    TEST_ASSERT_EQUAL_STRING("Петров", pb.head->data.surname);
    TEST_ASSERT_EQUAL_PTR(pb.head, pb.tail);
    assert_invariants(&pb);
    contact_free(&a);
    contact_free(&b);
    pb_free(&pb);
}

static void test_remove_tail(void) {
    PhoneBook pb;
    pb_init(&pb);
    Contact a = make_contact("Иванов", "Иван");
    Contact b = make_contact("Петров", "Пётр");
    add_contact(&pb, &a);
    add_contact(&pb, &b);
    TEST_ASSERT_EQUAL_INT(0, remove_contact(&pb, 1));
    TEST_ASSERT_EQUAL_size_t(1, pb.count);
    TEST_ASSERT_EQUAL_STRING("Иванов", pb.tail->data.surname);
    assert_invariants(&pb);
    contact_free(&a);
    contact_free(&b);
    pb_free(&pb);
}

static void test_remove_middle(void) {
    PhoneBook pb;
    pb_init(&pb);
    Contact a = make_contact("Иванов",  "Иван");
    Contact b = make_contact("Петров",  "Пётр");
    Contact c = make_contact("Сидоров", "Сидр");
    add_contact(&pb, &a);
    add_contact(&pb, &b);
    add_contact(&pb, &c);
    TEST_ASSERT_EQUAL_INT(0, remove_contact(&pb, 1));
    TEST_ASSERT_EQUAL_size_t(2, pb.count);
    TEST_ASSERT_EQUAL_STRING("Иванов",  nth_surname(&pb, 0));
    TEST_ASSERT_EQUAL_STRING("Сидоров", nth_surname(&pb, 1));
    assert_invariants(&pb);
    contact_free(&a);
    contact_free(&b);
    contact_free(&c);
    pb_free(&pb);
}

static void test_remove_invalid_index(void) {
    PhoneBook pb;
    pb_init(&pb);
    TEST_ASSERT_EQUAL_INT(-1, remove_contact(&pb, 0));    // пустой список
    Contact a = make_contact("Иванов", "Иван");
    add_contact(&pb, &a);
    TEST_ASSERT_EQUAL_INT(-1, remove_contact(&pb, 5));    // вне диапазона
    TEST_ASSERT_EQUAL_size_t(1, pb.count);
    assert_invariants(&pb);
    contact_free(&a);
    pb_free(&pb);
}

// ---------------- pb_node_at ----------------

static void test_node_at(void) {
    PhoneBook pb;
    pb_init(&pb);
    Contact a = make_contact("Иванов", "Иван");
    Contact b = make_contact("Петров", "Пётр");
    add_contact(&pb, &a);
    add_contact(&pb, &b);
    TEST_ASSERT_EQUAL_PTR(pb.head, pb_node_at(&pb, 0));
    TEST_ASSERT_EQUAL_PTR(pb.tail, pb_node_at(&pb, 1));
    TEST_ASSERT_NULL(pb_node_at(&pb, 2));
    TEST_ASSERT_NULL(pb_node_at(&pb, 100));
    contact_free(&a);
    contact_free(&b);
    pb_free(&pb);
}

// ---------------- pb_reinsert_node ----------------

static void test_reinsert_to_tail(void) {
    PhoneBook pb;
    pb_init(&pb);
    Contact a = make_contact("Иванов",  "Иван");
    Contact b = make_contact("Петров",  "Пётр");
    Contact c = make_contact("Сидоров", "Сидр");
    add_contact(&pb, &a);
    add_contact(&pb, &b);
    add_contact(&pb, &c);

    ContactNode *first = pb.head;
    contact_set_surname(&first->data, "Яковлев");
    pb_reinsert_node(&pb, first);

    TEST_ASSERT_EQUAL_size_t(3, pb.count);
    TEST_ASSERT_EQUAL_STRING("Петров",  nth_surname(&pb, 0));
    TEST_ASSERT_EQUAL_STRING("Сидоров", nth_surname(&pb, 1));
    TEST_ASSERT_EQUAL_STRING("Яковлев", nth_surname(&pb, 2));
    assert_invariants(&pb);
    assert_ordered(&pb);
    contact_free(&a);
    contact_free(&b);
    contact_free(&c);
    pb_free(&pb);
}

static void test_reinsert_to_head(void) {
    PhoneBook pb;
    pb_init(&pb);
    Contact a = make_contact("Иванов",  "Иван");
    Contact b = make_contact("Петров",  "Пётр");
    Contact c = make_contact("Сидоров", "Сидр");
    add_contact(&pb, &a);
    add_contact(&pb, &b);
    add_contact(&pb, &c);

    ContactNode *last = pb.tail;
    contact_set_surname(&last->data, "Абрамов");
    pb_reinsert_node(&pb, last);

    TEST_ASSERT_EQUAL_STRING("Абрамов", nth_surname(&pb, 0));
    TEST_ASSERT_EQUAL_STRING("Иванов",  nth_surname(&pb, 1));
    TEST_ASSERT_EQUAL_STRING("Петров",  nth_surname(&pb, 2));
    assert_invariants(&pb);
    assert_ordered(&pb);
    contact_free(&a);
    contact_free(&b);
    contact_free(&c);
    pb_free(&pb);
}

static void test_reinsert_to_middle(void) {
    PhoneBook pb;
    pb_init(&pb);
    Contact a = make_contact("Иванов",  "Иван");
    Contact b = make_contact("Петров",  "Пётр");
    Contact c = make_contact("Сидоров", "Сидр");
    add_contact(&pb, &a);
    add_contact(&pb, &b);
    add_contact(&pb, &c);

    ContactNode *last = pb.tail;
    contact_set_surname(&last->data, "Кузнецов");
    pb_reinsert_node(&pb, last);

    TEST_ASSERT_EQUAL_STRING("Иванов",   nth_surname(&pb, 0));
    TEST_ASSERT_EQUAL_STRING("Кузнецов", nth_surname(&pb, 1));
    TEST_ASSERT_EQUAL_STRING("Петров",   nth_surname(&pb, 2));
    assert_invariants(&pb);
    assert_ordered(&pb);
    contact_free(&a);
    contact_free(&b);
    contact_free(&c);
    pb_free(&pb);
}

static void test_reinsert_keeps_count(void) {
    PhoneBook pb;
    pb_init(&pb);
    Contact a = make_contact("Иванов", "Иван");
    add_contact(&pb, &a);
    size_t before = pb.count;
    pb_reinsert_node(&pb, pb.head);           // переставляем единственный узел
    TEST_ASSERT_EQUAL_size_t(before, pb.count);
    assert_invariants(&pb);
    contact_free(&a);
    pb_free(&pb);
}

// ---------------- pb_free ----------------

static void test_free_empties(void) {
    PhoneBook pb;
    pb_init(&pb);
    Contact a = make_contact("Иванов", "Иван");
    add_contact(&pb, &a);
    pb_free(&pb);
    TEST_ASSERT_NULL(pb.head);
    TEST_ASSERT_NULL(pb.tail);
    TEST_ASSERT_EQUAL_size_t(0, pb.count);
    contact_free(&a);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_init_empty);
    RUN_TEST(test_compare_primary_surname);
    RUN_TEST(test_compare_secondary_name);
    RUN_TEST(test_compare_tertiary_patronymic);
    RUN_TEST(test_compare_null_as_empty);
    RUN_TEST(test_add_single);
    RUN_TEST(test_add_orders);
    RUN_TEST(test_add_duplicates_after);
    RUN_TEST(test_remove_head);
    RUN_TEST(test_remove_tail);
    RUN_TEST(test_remove_middle);
    RUN_TEST(test_remove_invalid_index);
    RUN_TEST(test_node_at);
    RUN_TEST(test_reinsert_to_tail);
    RUN_TEST(test_reinsert_to_head);
    RUN_TEST(test_reinsert_to_middle);
    RUN_TEST(test_reinsert_keeps_count);
    RUN_TEST(test_free_empties);
    return UNITY_END();
}