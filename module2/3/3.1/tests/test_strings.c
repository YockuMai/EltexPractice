#include "unity.h"
#include "Strings.h"

#include <stdlib.h>

void setUp(void)    {}
void tearDown(void) {}

//my_strlen

static void test_strlen_empty(void) {
    TEST_ASSERT_EQUAL_INT(0, (int)my_strlen(""));
}

static void test_strlen_one(void) {
    TEST_ASSERT_EQUAL_INT(1, (int)my_strlen("a"));
}

static void test_strlen_normal(void) {
    TEST_ASSERT_EQUAL_INT(5, (int)my_strlen("hello"));
}

static void test_strlen_spaces(void) {
    TEST_ASSERT_EQUAL_INT(11, (int)my_strlen("hello world"));
}

static void test_strlen_embedded_dash(void) {
    TEST_ASSERT_EQUAL_INT(9, (int)my_strlen("rwxr-xr-x"));
}

//my_strdup

static void test_strdup_empty(void) {
    char *d = my_strdup("");
    TEST_ASSERT_NOT_NULL(d);
    TEST_ASSERT_EQUAL_STRING("", d);
    free(d);
}

static void test_strdup_normal(void) {
    char *d = my_strdup("hello");
    TEST_ASSERT_NOT_NULL(d);
    TEST_ASSERT_EQUAL_STRING("hello", d);
    free(d);
}

static void test_strdup_is_independent_copy(void) {
    char src[] = "hello";
    char *d = my_strdup(src);
    TEST_ASSERT_NOT_NULL(d);
    src[0] = 'X';
    TEST_ASSERT_EQUAL_STRING("hello", d);  /* копия не изменилась */
    free(d);
}

// my_strcmp

static void test_strcmp_equal(void) {
    TEST_ASSERT_EQUAL_INT(0, my_strcmp("abc", "abc"));
}

static void test_strcmp_empty_equal(void) {
    TEST_ASSERT_EQUAL_INT(0, my_strcmp("", ""));
}

static void test_strcmp_less(void) {
    TEST_ASSERT_TRUE(my_strcmp("abc", "abd") < 0);
}

static void test_strcmp_greater(void) {
    TEST_ASSERT_TRUE(my_strcmp("abd", "abc") > 0);
}

static void test_strcmp_prefix_shorter_is_less(void) {
    TEST_ASSERT_TRUE(my_strcmp("ab", "abc") < 0);
}

static void test_strcmp_prefix_longer_is_greater(void) {
    TEST_ASSERT_TRUE(my_strcmp("abc", "ab") > 0);
}

static void test_strcmp_high_byte_unsigned(void) {
    /* сравнение через unsigned char, а не signed */
    TEST_ASSERT_TRUE(my_strcmp("\xff", "\x01") > 0);
}

//my_strchr

static void test_strchr_found(void) {
    char buf[] = "hello";
    char *p = my_strchr(buf, 'l');
    TEST_ASSERT_NOT_NULL(p);
    TEST_ASSERT_EQUAL_CHAR('l', *p);
    TEST_ASSERT_EQUAL_INT(2, (int)(p - buf));   /* первая 'l' */
}

static void test_strchr_not_found(void) {
    TEST_ASSERT_NULL(my_strchr("hello", 'z'));
}

static void test_strchr_null_terminator(void) {
    char buf[] = "hello";
    char *p = my_strchr(buf, '\0');
    TEST_ASSERT_NOT_NULL(p);
    TEST_ASSERT_EQUAL_INT(5, (int)(p - buf));
}

static void test_strchr_empty_string_null(void) {
    char buf[] = "";
    char *p = my_strchr(buf, '\0');
    TEST_ASSERT_NOT_NULL(p);
    TEST_ASSERT_EQUAL_INT(0, (int)(p - buf));
}

static void test_strchr_first_char(void) {
    char buf[] = "hello";
    char *p = my_strchr(buf, 'h');
    TEST_ASSERT_NOT_NULL(p);
    TEST_ASSERT_EQUAL_CHAR('h', *p);
}

static void test_strchr_comma(void) {
    char buf[] = "u+x,go-w";
    char *p = my_strchr(buf, ',');
    TEST_ASSERT_NOT_NULL(p);
    TEST_ASSERT_EQUAL_CHAR(',', *p);
    TEST_ASSERT_EQUAL_INT(3, (int)(p - buf));
}

//my_memcpy

static void test_memcpy_simple(void) {
    char dst[6] = {0};
    my_memcpy(dst, "hello", 5);
    TEST_ASSERT_EQUAL_STRING("hello", dst);
}

static void test_memcpy_with_terminator(void) {
    char dst[6] = {0};
    my_memcpy(dst, "hello", 6);
    TEST_ASSERT_EQUAL_STRING("hello", dst);
}

static void test_memcpy_zero_bytes(void) {
    char dst[] = "abc";
    my_memcpy(dst, "xyz", 0);
    TEST_ASSERT_EQUAL_STRING("abc", dst);
}

static void test_memcpy_returns_dst(void) {
    char dst[3];
    void *ret = my_memcpy(dst, "ab", 2);
    TEST_ASSERT_EQUAL_PTR(dst, ret);
}

static void test_memcpy_binary(void) {
    unsigned char src[] = {0x00, 0x01, 0xFF, 0x80};
    unsigned char dst[4] = {0};
    my_memcpy(dst, src, 4);
    TEST_ASSERT_EQUAL_UINT8(0x00, dst[0]);
    TEST_ASSERT_EQUAL_UINT8(0x01, dst[1]);
    TEST_ASSERT_EQUAL_UINT8(0xFF, dst[2]);
    TEST_ASSERT_EQUAL_UINT8(0x80, dst[3]);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_strlen_empty);
    RUN_TEST(test_strlen_one);
    RUN_TEST(test_strlen_normal);
    RUN_TEST(test_strlen_spaces);
    RUN_TEST(test_strlen_embedded_dash);

    RUN_TEST(test_strdup_empty);
    RUN_TEST(test_strdup_normal);
    RUN_TEST(test_strdup_is_independent_copy);

    RUN_TEST(test_strcmp_equal);
    RUN_TEST(test_strcmp_empty_equal);
    RUN_TEST(test_strcmp_less);
    RUN_TEST(test_strcmp_greater);
    RUN_TEST(test_strcmp_prefix_shorter_is_less);
    RUN_TEST(test_strcmp_prefix_longer_is_greater);
    RUN_TEST(test_strcmp_high_byte_unsigned);

    RUN_TEST(test_strchr_found);
    RUN_TEST(test_strchr_not_found);
    RUN_TEST(test_strchr_null_terminator);
    RUN_TEST(test_strchr_empty_string_null);
    RUN_TEST(test_strchr_first_char);
    RUN_TEST(test_strchr_comma);

    RUN_TEST(test_memcpy_simple);
    RUN_TEST(test_memcpy_with_terminator);
    RUN_TEST(test_memcpy_zero_bytes);
    RUN_TEST(test_memcpy_returns_dst);
    RUN_TEST(test_memcpy_binary);

    return UNITY_END();
}