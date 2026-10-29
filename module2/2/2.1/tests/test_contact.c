#include "unity.h"
#include "../Contact.h"

void setUp(void) {}
void tearDown(void) {}

static void test_set_surname_basic(void) {
    Contact c;
    contact_init(&c);

    TEST_ASSERT_EQUAL_INT(0, contact_set_surname(&c, "Иванов"));
    TEST_ASSERT_EQUAL_STRING("Иванов", c.surname);

    TEST_ASSERT_EQUAL_INT(0, contact_set_surname(&c, "Петров"));
    TEST_ASSERT_EQUAL_STRING("Петров", c.surname);

    contact_free(&c);
}

static void test_set_empty_clears(void) {
    Contact c;
    contact_init(&c);
    contact_set_workplace(&c, "ООО Ромашка");
    TEST_ASSERT_EQUAL_INT(0, contact_set_workplace(&c, ""));
    TEST_ASSERT_NULL(c.workplace);
    contact_free(&c);
}

static void test_add_phone(void) {
    Contact c;
    contact_init(&c);
    TEST_ASSERT_EQUAL_INT(0, contact_add_phone(&c, "+7-900-000-00-00"));
    TEST_ASSERT_EQUAL_INT(0, contact_add_phone(&c, "+7-900-111-11-11"));
    TEST_ASSERT_EQUAL_size_t(2, c.phone_count);
    TEST_ASSERT_EQUAL_STRING("+7-900-000-00-00", c.phones[0]);
    TEST_ASSERT_EQUAL_STRING("+7-900-111-11-11", c.phones[1]);
    contact_free(&c);
}

static void test_remove_phone_middle(void) {
    Contact c;
    contact_init(&c);
    contact_add_phone(&c, "A");
    contact_add_phone(&c, "B");
    contact_add_phone(&c, "C");

    TEST_ASSERT_EQUAL_INT(0, contact_remove_phone(&c, 1));
    TEST_ASSERT_EQUAL_size_t(2, c.phone_count);
    TEST_ASSERT_EQUAL_STRING("A", c.phones[0]);
    TEST_ASSERT_EQUAL_STRING("C", c.phones[1]);

    contact_free(&c);
}

static void test_clone_deep_copy(void) {
    Contact src;
    contact_init(&src);
    contact_set_surname(&src, "Иванов");
    contact_add_phone(&src, "123");

    Contact dst;
    TEST_ASSERT_EQUAL_INT(0, contact_clone(&dst, &src));

    TEST_ASSERT_EQUAL_STRING("Иванов", dst.surname);
    TEST_ASSERT_EQUAL_size_t(1, dst.phone_count);
    TEST_ASSERT_EQUAL_STRING("123", dst.phones[0]);

    src.surname[0] = 'X';
    TEST_ASSERT_EQUAL_STRING("Иванов", dst.surname);

    contact_free(&src);
    contact_free(&dst);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_set_surname_basic);
    RUN_TEST(test_set_empty_clears);
    RUN_TEST(test_add_phone);
    RUN_TEST(test_remove_phone_middle);
    RUN_TEST(test_clone_deep_copy);
    return UNITY_END();
}