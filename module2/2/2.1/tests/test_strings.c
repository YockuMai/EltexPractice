#include "unity.h"
#include "../Strings.h"

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

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_strlen_empty);
    RUN_TEST(test_strlen_ascii);
    RUN_TEST(test_strlen_utf8);
    RUN_TEST(test_strdup_copies);
    return UNITY_END();
}