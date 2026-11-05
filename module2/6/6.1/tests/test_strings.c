#include "unity.h"
#include "../include/Strings.h"

void setUp(void) {}
void tearDown(void) {}

static void test_cmp_str_equal(void) {
    TEST_ASSERT_EQUAL_INT(0, my_cmp_str("abc", "abc"));
    TEST_ASSERT_EQUAL_INT(0, my_cmp_str("", ""));
}

static void test_cmp_str_order(void) {
    TEST_ASSERT_LESS_THAN   (0, my_cmp_str("abc", "abd"));
    TEST_ASSERT_GREATER_THAN(0, my_cmp_str("abd", "abc"));
    TEST_ASSERT_LESS_THAN   (0, my_cmp_str("", "a"));
    TEST_ASSERT_GREATER_THAN(0, my_cmp_str("a", ""));
}

static void test_cmp_str_prefix(void) {
    TEST_ASSERT_LESS_THAN   (0, my_cmp_str("abc", "abcd"));
    TEST_ASSERT_GREATER_THAN(0, my_cmp_str("abcd", "abc"));
}

static void test_cmp_str_null_safe(void) {
    TEST_ASSERT_EQUAL_INT   (0, my_cmp_str(NULL, NULL));
    TEST_ASSERT_EQUAL_INT   (0, my_cmp_str(NULL, ""));
    TEST_ASSERT_EQUAL_INT   (0, my_cmp_str("", NULL));
    TEST_ASSERT_LESS_THAN   (0, my_cmp_str(NULL, "a"));
    TEST_ASSERT_GREATER_THAN(0, my_cmp_str("a", NULL));
}

static void test_strlen_empty(void) {
    TEST_ASSERT_EQUAL_size_t(0, my_strlen(""));
}

static void test_strlen_ascii(void) {
    TEST_ASSERT_EQUAL_size_t(5, my_strlen("hello"));
}

static void test_strlen_utf8(void) {
    /* "привет" в UTF-8 — 12 байт */
    TEST_ASSERT_EQUAL_size_t(12, my_strlen("привет"));
}

static void test_strdup_copies(void) {
    char *p = my_strdup("abc");
    TEST_ASSERT_NOT_NULL(p);
    TEST_ASSERT_EQUAL_STRING("abc", p);
    p[0] = 'X';
    TEST_ASSERT_EQUAL_STRING("Xbc", p);
    free(p);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_strlen_empty);
    RUN_TEST(test_strlen_ascii);
    RUN_TEST(test_strlen_utf8);
    RUN_TEST(test_strdup_copies);
    RUN_TEST(test_cmp_str_equal);
    RUN_TEST(test_cmp_str_order);
    RUN_TEST(test_cmp_str_prefix);
    RUN_TEST(test_cmp_str_null_safe);
    return UNITY_END();
}