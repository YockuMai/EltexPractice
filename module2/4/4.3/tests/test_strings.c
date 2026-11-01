#include "unity.h"
#include "../Strings.h"
#include <stdlib.h>

void setUp(void) {}
void tearDown(void) {}

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

static void test_strdup_empty(void) {
    char *p = my_strdup("");
    TEST_ASSERT_NOT_NULL(p);
    TEST_ASSERT_EQUAL_STRING("", p);
    free(p);
}

static void test_strcmp_equal(void) {
    TEST_ASSERT_EQUAL_INT(0, my_strcmp("abc", "abc"));
    TEST_ASSERT_EQUAL_INT(0, my_strcmp("", ""));
    TEST_ASSERT_EQUAL_INT(0, my_strcmp("привет", "привет"));
}

static void test_strcmp_order_basic(void) {
    TEST_ASSERT_TRUE(my_strcmp("abc", "abd") < 0);
    TEST_ASSERT_TRUE(my_strcmp("abd", "abc") > 0);
    TEST_ASSERT_TRUE(my_strcmp("", "a") < 0);
    TEST_ASSERT_TRUE(my_strcmp("a", "") > 0);
}

static void test_strcmp_prefix(void) {
    TEST_ASSERT_TRUE(my_strcmp("abc", "abcd") < 0);
    TEST_ASSERT_TRUE(my_strcmp("abcd", "abc") > 0);
    TEST_ASSERT_TRUE(my_strcmp("abc", "ab") > 0);
}

static void test_strcmp_unsigned_bytes(void) {
    const char hi[2] = { (char)0x80, 0 };
    TEST_ASSERT_TRUE(my_strcmp(hi, "a") > 0);
    TEST_ASSERT_TRUE(my_strcmp("a", hi) < 0);
}

static void test_strcmp_utf8_cyrillic(void) {
    TEST_ASSERT_TRUE(my_strcmp("А", "Б") < 0);
    TEST_ASSERT_TRUE(my_strcmp("Б", "А") > 0);
    TEST_ASSERT_TRUE(my_strcmp("Иванов", "Иванова") < 0);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_strlen_empty);
    RUN_TEST(test_strlen_ascii);
    RUN_TEST(test_strlen_utf8);
    RUN_TEST(test_strdup_copies);
    RUN_TEST(test_strdup_empty);
    RUN_TEST(test_strcmp_equal);
    RUN_TEST(test_strcmp_order_basic);
    RUN_TEST(test_strcmp_prefix);
    RUN_TEST(test_strcmp_unsigned_bytes);
    RUN_TEST(test_strcmp_utf8_cyrillic);
    return UNITY_END();
}