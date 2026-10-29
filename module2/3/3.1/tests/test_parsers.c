#include "unity.h"
#include "parsers.h"
#include "perm_bits.h"

#include <stdlib.h>

void setUp(void)    {}
void tearDown(void) {}

//parse_numeric

static void test_parse_numeric_755(void) {
    TEST_ASSERT_EQUAL_INT(0755, parse_numeric("755"));
}
static void test_parse_numeric_644(void) {
    TEST_ASSERT_EQUAL_INT(0644, parse_numeric("644"));
}
static void test_parse_numeric_4755(void) {
    TEST_ASSERT_EQUAL_INT(04755, parse_numeric("4755"));
}
static void test_parse_numeric_000(void) {
    TEST_ASSERT_EQUAL_INT(0, parse_numeric("000"));
}
static void test_parse_numeric_7777(void) {
    TEST_ASSERT_EQUAL_INT(07777, parse_numeric("7777"));
}
static void test_parse_numeric_short(void) {
    TEST_ASSERT_EQUAL_INT(PARSE_ERROR, parse_numeric("75"));
}
static void test_parse_numeric_long(void) {
    TEST_ASSERT_EQUAL_INT(PARSE_ERROR, parse_numeric("75550"));
}
static void test_parse_numeric_bad_digit(void) {
    TEST_ASSERT_EQUAL_INT(PARSE_ERROR, parse_numeric("789"));
}
static void test_parse_numeric_letters(void) {
    TEST_ASSERT_EQUAL_INT(PARSE_ERROR, parse_numeric("rwx"));
}

//parse_symbolic

static void test_parse_symbolic_755(void) {
    TEST_ASSERT_EQUAL_INT(0755, parse_symbolic("rwxr-xr-x"));
}
static void test_parse_symbolic_644(void) {
    TEST_ASSERT_EQUAL_INT(0644, parse_symbolic("rw-r--r--"));
}
static void test_parse_symbolic_setuid(void) {
    TEST_ASSERT_EQUAL_INT(04755, parse_symbolic("rwsr-xr-x"));
}
static void test_parse_symbolic_setuid_noexec(void) {
    TEST_ASSERT_EQUAL_INT(04655, parse_symbolic("rwSr-xr-x"));
}
static void test_parse_symbolic_setgid(void) {
    TEST_ASSERT_EQUAL_INT(02755, parse_symbolic("rwxr-sr-x"));
}
static void test_parse_symbolic_sticky(void) {
    TEST_ASSERT_EQUAL_INT(01777, parse_symbolic("rwxrwxrwt"));
}
static void test_parse_symbolic_sticky_noexec(void) {
    TEST_ASSERT_EQUAL_INT(01776, parse_symbolic("rwxrwxrwT"));
}
static void test_parse_symbolic_all_dashes(void) {
    TEST_ASSERT_EQUAL_INT(0, parse_symbolic("---------"));
}
static void test_parse_symbolic_bad_length(void) {
    TEST_ASSERT_EQUAL_INT(PARSE_ERROR, parse_symbolic("rwxr-xr-"));
}
static void test_parse_symbolic_bad_char(void) {
    TEST_ASSERT_EQUAL_INT(PARSE_ERROR, parse_symbolic("rwxr-xr-z"));
}

//parse_mode (диспетчер)

static void test_parse_mode_numeric(void) {
    TEST_ASSERT_EQUAL_INT(0755, parse_mode("755"));
}
static void test_parse_mode_symbolic(void) {
    TEST_ASSERT_EQUAL_INT(0755, parse_mode("rwxr-xr-x"));
}
static void test_parse_mode_null(void) {
    TEST_ASSERT_EQUAL_INT(PARSE_ERROR, parse_mode(NULL));
}
static void test_parse_mode_empty(void) {
    TEST_ASSERT_EQUAL_INT(PARSE_ERROR, parse_mode(""));
}

//mode_to_symbolic

static void test_to_symbolic_755(void) {
    char *s = mode_to_symbolic(0755);
    TEST_ASSERT_NOT_NULL(s);
    TEST_ASSERT_EQUAL_STRING("rwxr-xr-x", s);
    free(s);
}
static void test_to_symbolic_644(void) {
    char *s = mode_to_symbolic(0644);
    TEST_ASSERT_NOT_NULL(s);
    TEST_ASSERT_EQUAL_STRING("rw-r--r--", s);
    free(s);
}
static void test_to_symbolic_0(void) {
    char *s = mode_to_symbolic(0);
    TEST_ASSERT_NOT_NULL(s);
    TEST_ASSERT_EQUAL_STRING("---------", s);
    free(s);
}
static void test_to_symbolic_setuid(void) {
    char *s = mode_to_symbolic(04755);
    TEST_ASSERT_NOT_NULL(s);
    TEST_ASSERT_EQUAL_STRING("rwsr-xr-x", s);
    free(s);
}
static void test_to_symbolic_setuid_noexec(void) {
    char *s = mode_to_symbolic(04655);
    TEST_ASSERT_NOT_NULL(s);
    TEST_ASSERT_EQUAL_STRING("rwSr-xr-x", s);
    free(s);
}
static void test_to_symbolic_setgid(void) {
    char *s = mode_to_symbolic(02755);
    TEST_ASSERT_NOT_NULL(s);
    TEST_ASSERT_EQUAL_STRING("rwxr-sr-x", s);
    free(s);
}
static void test_to_symbolic_sticky(void) {
    char *s = mode_to_symbolic(01777);
    TEST_ASSERT_NOT_NULL(s);
    TEST_ASSERT_EQUAL_STRING("rwxrwxrwt", s);
    free(s);
}
static void test_to_symbolic_sticky_noexec(void) {
    char *s = mode_to_symbolic(01776);
    TEST_ASSERT_NOT_NULL(s);
    TEST_ASSERT_EQUAL_STRING("rwxrwxrwT", s);
    free(s);
}

//mode_to_numeric

static void test_to_numeric_755(void) {
    char *s = mode_to_numeric(0755);
    TEST_ASSERT_NOT_NULL(s);
    TEST_ASSERT_EQUAL_STRING("755", s);
    free(s);
}
static void test_to_numeric_0(void) {
    char *s = mode_to_numeric(0);
    TEST_ASSERT_NOT_NULL(s);
    TEST_ASSERT_EQUAL_STRING("000", s);
    free(s);
}
static void test_to_numeric_setuid(void) {
    char *s = mode_to_numeric(04755);
    TEST_ASSERT_NOT_NULL(s);
    TEST_ASSERT_EQUAL_STRING("4755", s);
    free(s);
}
static void test_to_numeric_sticky(void) {
    char *s = mode_to_numeric(01777);
    TEST_ASSERT_NOT_NULL(s);
    TEST_ASSERT_EQUAL_STRING("1777", s);
    free(s);
}

//mode_to_binary

static void test_to_binary_755(void) {
    char *s = mode_to_binary(0755);
    TEST_ASSERT_NOT_NULL(s);
    TEST_ASSERT_EQUAL_STRING("000 111 101 101", s);
    free(s);
}
static void test_to_binary_0(void) {
    char *s = mode_to_binary(0);
    TEST_ASSERT_NOT_NULL(s);
    TEST_ASSERT_EQUAL_STRING("000 000 000 000", s);
    free(s);
}
static void test_to_binary_7777(void) {
    char *s = mode_to_binary(07777);
    TEST_ASSERT_NOT_NULL(s);
    TEST_ASSERT_EQUAL_STRING("111 111 111 111", s);
    free(s);
}
static void test_to_binary_setuid(void) {
    char *s = mode_to_binary(04755);
    TEST_ASSERT_NOT_NULL(s);
    TEST_ASSERT_EQUAL_STRING("100 111 101 101", s);
    free(s);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_parse_numeric_755);
    RUN_TEST(test_parse_numeric_644);
    RUN_TEST(test_parse_numeric_4755);
    RUN_TEST(test_parse_numeric_000);
    RUN_TEST(test_parse_numeric_7777);
    RUN_TEST(test_parse_numeric_short);
    RUN_TEST(test_parse_numeric_long);
    RUN_TEST(test_parse_numeric_bad_digit);
    RUN_TEST(test_parse_numeric_letters);

    RUN_TEST(test_parse_symbolic_755);
    RUN_TEST(test_parse_symbolic_644);
    RUN_TEST(test_parse_symbolic_setuid);
    RUN_TEST(test_parse_symbolic_setuid_noexec);
    RUN_TEST(test_parse_symbolic_setgid);
    RUN_TEST(test_parse_symbolic_sticky);
    RUN_TEST(test_parse_symbolic_sticky_noexec);
    RUN_TEST(test_parse_symbolic_all_dashes);
    RUN_TEST(test_parse_symbolic_bad_length);
    RUN_TEST(test_parse_symbolic_bad_char);

    RUN_TEST(test_parse_mode_numeric);
    RUN_TEST(test_parse_mode_symbolic);
    RUN_TEST(test_parse_mode_null);
    RUN_TEST(test_parse_mode_empty);

    RUN_TEST(test_to_symbolic_755);
    RUN_TEST(test_to_symbolic_644);
    RUN_TEST(test_to_symbolic_0);
    RUN_TEST(test_to_symbolic_setuid);
    RUN_TEST(test_to_symbolic_setuid_noexec);
    RUN_TEST(test_to_symbolic_setgid);
    RUN_TEST(test_to_symbolic_sticky);
    RUN_TEST(test_to_symbolic_sticky_noexec);

    RUN_TEST(test_to_numeric_755);
    RUN_TEST(test_to_numeric_0);
    RUN_TEST(test_to_numeric_setuid);
    RUN_TEST(test_to_numeric_sticky);

    RUN_TEST(test_to_binary_755);
    RUN_TEST(test_to_binary_0);
    RUN_TEST(test_to_binary_7777);
    RUN_TEST(test_to_binary_setuid);

    return UNITY_END();
}