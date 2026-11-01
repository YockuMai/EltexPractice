#include "unity.h"
#include "../Contact.h"
#include <stdlib.h>

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

static void test_set_null_clears(void) {
    Contact c;
    contact_init(&c);
    contact_set_position(&c, "Инженер");
    TEST_ASSERT_EQUAL_INT(0, contact_set_position(&c, NULL));
    TEST_ASSERT_NULL(c.position);
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

static void test_add_all_lists(void) {
    Contact c;
    contact_init(&c);
    TEST_ASSERT_EQUAL_INT(0, contact_add_phone(&c,     "p"));
    TEST_ASSERT_EQUAL_INT(0, contact_add_email(&c,     "e"));
    TEST_ASSERT_EQUAL_INT(0, contact_add_social(&c,    "s"));
    TEST_ASSERT_EQUAL_INT(0, contact_add_messenger(&c, "m"));
    TEST_ASSERT_EQUAL_size_t(1, c.phone_count);
    TEST_ASSERT_EQUAL_size_t(1, c.email_count);
    TEST_ASSERT_EQUAL_size_t(1, c.social_count);
    TEST_ASSERT_EQUAL_size_t(1, c.messenger_count);
    TEST_ASSERT_EQUAL_STRING("p", c.phones[0]);
    TEST_ASSERT_EQUAL_STRING("e", c.emails[0]);
    TEST_ASSERT_EQUAL_STRING("s", c.socials[0]);
    TEST_ASSERT_EQUAL_STRING("m", c.messengers[0]);
    contact_free(&c);
}

static void test_add_empty_ignored(void) {
    Contact c;
    contact_init(&c);
    TEST_ASSERT_EQUAL_INT(0, contact_add_phone(&c, ""));
    TEST_ASSERT_EQUAL_INT(0, contact_add_phone(&c, NULL));
    TEST_ASSERT_EQUAL_size_t(0, c.phone_count);
    TEST_ASSERT_NULL(c.phones);
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

static void test_remove_all_emails(void) {
    Contact c;
    contact_init(&c);
    contact_add_email(&c, "a@x");
    contact_add_email(&c, "b@x");
    TEST_ASSERT_EQUAL_INT(0, contact_remove_email(&c, 0));
    TEST_ASSERT_EQUAL_INT(0, contact_remove_email(&c, 0));
    TEST_ASSERT_EQUAL_size_t(0, c.email_count);
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

static void test_clone_empty(void) {
    Contact src;
    contact_init(&src);

    Contact dst;
    TEST_ASSERT_EQUAL_INT(0, contact_clone(&dst, &src));
    TEST_ASSERT_NULL(dst.surname);
    TEST_ASSERT_NULL(dst.name);
    TEST_ASSERT_NULL(dst.patronymic);
    TEST_ASSERT_EQUAL_size_t(0, dst.phone_count);
    TEST_ASSERT_EQUAL_size_t(0, dst.email_count);

    contact_free(&src);
    contact_free(&dst);
}

static void test_free_resets_to_initial(void) {
    Contact c;
    contact_init(&c);
    contact_set_surname(&c, "X");
    contact_add_phone(&c, "1");

    contact_free(&c);

    TEST_ASSERT_NULL(c.surname);
    TEST_ASSERT_NULL(c.name);
    TEST_ASSERT_NULL(c.patronymic);
    TEST_ASSERT_NULL(c.workplace);
    TEST_ASSERT_NULL(c.position);
    TEST_ASSERT_NULL(c.phones);
    TEST_ASSERT_NULL(c.emails);
    TEST_ASSERT_NULL(c.socials);
    TEST_ASSERT_NULL(c.messengers);
    TEST_ASSERT_EQUAL_size_t(0, c.phone_count);
    TEST_ASSERT_EQUAL_size_t(0, c.email_count);
    TEST_ASSERT_EQUAL_size_t(0, c.social_count);
    TEST_ASSERT_EQUAL_size_t(0, c.messenger_count);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_set_surname_basic);
    RUN_TEST(test_set_empty_clears);
    RUN_TEST(test_set_null_clears);
    RUN_TEST(test_add_phone);
    RUN_TEST(test_add_all_lists);
    RUN_TEST(test_add_empty_ignored);
    RUN_TEST(test_remove_phone_middle);
    RUN_TEST(test_remove_all_emails);
    RUN_TEST(test_clone_deep_copy);
    RUN_TEST(test_clone_empty);
    RUN_TEST(test_free_resets_to_initial);
    return UNITY_END();
}