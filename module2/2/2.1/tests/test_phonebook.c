#include "unity.h"
#include "../PhoneBook.h"

void setUp(void) {}
void tearDown(void) {}

static Contact make_contact(const char *surname, const char *name) {
    Contact c;
    contact_init(&c);
    contact_set_surname(&c, surname);
    contact_set_name(&c, name);
    return c;
}

static void test_add_and_remove(void) {
    PhoneBook pb;
    pb_init(&pb);

    Contact a = make_contact("Иванов", "Иван");
    Contact b = make_contact("Петров", "Пётр");

    TEST_ASSERT_EQUAL_INT(0, add_contact(&pb, &a));
    TEST_ASSERT_EQUAL_INT(0, add_contact(&pb, &b));
    TEST_ASSERT_EQUAL_size_t(2, pb.count);

    TEST_ASSERT_EQUAL_INT(0, remove_contact(&pb, 0));
    TEST_ASSERT_EQUAL_size_t(1, pb.count);
    TEST_ASSERT_EQUAL_STRING("Петров", pb.items[0].surname);

    contact_free(&a);
    contact_free(&b);
    pb_free(&pb);
}

static void test_remove_invalid_index(void) {
    PhoneBook pb;
    pb_init(&pb);
    TEST_ASSERT_EQUAL_INT(-1, remove_contact(&pb, 0));
    pb_free(&pb);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_add_and_remove);
    RUN_TEST(test_remove_invalid_index);
    return UNITY_END();
}