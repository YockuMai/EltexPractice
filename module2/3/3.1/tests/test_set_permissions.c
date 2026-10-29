#include "unity.h"
#include "setPermissions.h"
#include "perm_bits.h"

void setUp(void)    {}
void tearDown(void) {}

//Числовой режим
static void test_chmod_numeric_755(void)        { TEST_ASSERT_EQUAL_INT(0755, apply_chmod(0644u, "755")); }
static void test_chmod_numeric_4755(void)       { TEST_ASSERT_EQUAL_INT(04755, apply_chmod(0u, "4755")); }
static void test_chmod_numeric_0(void)          { TEST_ASSERT_EQUAL_INT(0, apply_chmod(07777u, "0")); }
static void test_chmod_numeric_short_7(void)    { TEST_ASSERT_EQUAL_INT(0007, apply_chmod(0644u, "7")); }
static void test_chmod_numeric_invalid_long(void){ TEST_ASSERT_EQUAL_INT(PARSE_ERROR, apply_chmod(0644u, "75550")); }
static void test_chmod_numeric_invalid_digit(void){ TEST_ASSERT_EQUAL_INT(PARSE_ERROR, apply_chmod(0644u, "789")); }

//Базовые who/op/perm
static void test_chmod_u_plus_x(void)           { TEST_ASSERT_EQUAL_INT(0744, apply_chmod(0644u, "u+x")); }
static void test_chmod_u_minus_r(void)          { TEST_ASSERT_EQUAL_INT(0244, apply_chmod(0644u, "u-r")); }
static void test_chmod_go_minus_w(void)         { TEST_ASSERT_EQUAL_INT(0644, apply_chmod(0666u, "go-w")); }
static void test_chmod_a_eq_r(void)             { TEST_ASSERT_EQUAL_INT(0444, apply_chmod(0666u, "a=r")); }
static void test_chmod_no_who_plus_x(void)      { TEST_ASSERT_EQUAL_INT(0755, apply_chmod(0644u, "+x")); }
static void test_chmod_no_who_minus_w(void)     { TEST_ASSERT_EQUAL_INT(0555, apply_chmod(0755u, "-w")); }
static void test_chmod_u_eq_empty(void)         { TEST_ASSERT_EQUAL_INT(0044, apply_chmod(0644u, "u=")); }

//Спецбиты
static void test_chmod_u_plus_s(void)           { TEST_ASSERT_EQUAL_INT(04744, apply_chmod(0744u, "u+s")); }
static void test_chmod_u_minus_s(void)          { TEST_ASSERT_EQUAL_INT(0744, apply_chmod(04744u, "u-s")); }
static void test_chmod_g_plus_s(void)           { TEST_ASSERT_EQUAL_INT(02644, apply_chmod(0644u, "g+s")); }
static void test_chmod_o_plus_t(void)           { TEST_ASSERT_EQUAL_INT(01644, apply_chmod(0644u, "o+t")); }
static void test_chmod_o_minus_t(void)          { TEST_ASSERT_EQUAL_INT(0644, apply_chmod(01644u, "o-t")); }
static void test_chmod_u_eq_rwx_drops_setuid(void){ TEST_ASSERT_EQUAL_INT(0744, apply_chmod(04744u, "u=rwx")); }

//Копирование
static void test_chmod_g_eq_u(void)             { TEST_ASSERT_EQUAL_INT(0664, apply_chmod(0644u, "g=u")); }
static void test_chmod_o_eq_g(void)             { TEST_ASSERT_EQUAL_INT(0644, apply_chmod(0644u, "o=g")); }
static void test_chmod_a_eq_u(void)             { TEST_ASSERT_EQUAL_INT(0777, apply_chmod(0700u, "a=u")); }

//Списки клаузов
static void test_chmod_list_g_eq_u_o_eq_g(void) { TEST_ASSERT_EQUAL_INT(0666, apply_chmod(0600u, "g=u,o=g")); }
static void test_chmod_list_a_rwx_then_u_w(void){ TEST_ASSERT_EQUAL_INT(0577, apply_chmod(0644u, "a=rwx,u-w")); }
static void test_chmod_list_full_reset(void)    { TEST_ASSERT_EQUAL_INT(0644, apply_chmod(07777u, "a=r,u+w,g+r")); }

//X
static void test_chmod_X_no_existing_x(void)    { TEST_ASSERT_EQUAL_INT(0644, apply_chmod(0644u, "a+X")); }
static void test_chmod_X_existing_x(void)       { TEST_ASSERT_EQUAL_INT(0755, apply_chmod(0744u, "a+X")); }

//Ошибки
static void test_chmod_empty(void)              { TEST_ASSERT_EQUAL_INT(PARSE_ERROR, apply_chmod(0644u, "")); }
static void test_chmod_null(void)               { TEST_ASSERT_EQUAL_INT(PARSE_ERROR, apply_chmod(0644u, NULL)); }
static void test_chmod_bad_op(void)             { TEST_ASSERT_EQUAL_INT(PARSE_ERROR, apply_chmod(0644u, "u*x")); }
static void test_chmod_no_op(void)              { TEST_ASSERT_EQUAL_INT(PARSE_ERROR, apply_chmod(0644u, "u")); }
static void test_chmod_plus_no_perm(void)       { TEST_ASSERT_EQUAL_INT(PARSE_ERROR, apply_chmod(0644u, "u+")); }
static void test_chmod_bad_char(void)           { TEST_ASSERT_EQUAL_INT(PARSE_ERROR, apply_chmod(0644u, "u+z")); }
static void test_chmod_trailing_comma(void)     { TEST_ASSERT_EQUAL_INT(PARSE_ERROR, apply_chmod(0644u, "u+x,")); }
static void test_chmod_double_comma(void)       { TEST_ASSERT_EQUAL_INT(PARSE_ERROR, apply_chmod(0644u, "u+x,,o+w")); }

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_chmod_numeric_755);
    RUN_TEST(test_chmod_numeric_4755);
    RUN_TEST(test_chmod_numeric_0);
    RUN_TEST(test_chmod_numeric_short_7);
    RUN_TEST(test_chmod_numeric_invalid_long);
    RUN_TEST(test_chmod_numeric_invalid_digit);

    RUN_TEST(test_chmod_u_plus_x);
    RUN_TEST(test_chmod_u_minus_r);
    RUN_TEST(test_chmod_go_minus_w);
    RUN_TEST(test_chmod_a_eq_r);
    RUN_TEST(test_chmod_no_who_plus_x);
    RUN_TEST(test_chmod_no_who_minus_w);
    RUN_TEST(test_chmod_u_eq_empty);

    RUN_TEST(test_chmod_u_plus_s);
    RUN_TEST(test_chmod_u_minus_s);
    RUN_TEST(test_chmod_g_plus_s);
    RUN_TEST(test_chmod_o_plus_t);
    RUN_TEST(test_chmod_o_minus_t);
    RUN_TEST(test_chmod_u_eq_rwx_drops_setuid);

    RUN_TEST(test_chmod_g_eq_u);
    RUN_TEST(test_chmod_o_eq_g);
    RUN_TEST(test_chmod_a_eq_u);

    RUN_TEST(test_chmod_list_g_eq_u_o_eq_g);
    RUN_TEST(test_chmod_list_a_rwx_then_u_w);
    RUN_TEST(test_chmod_list_full_reset);

    RUN_TEST(test_chmod_X_no_existing_x);
    RUN_TEST(test_chmod_X_existing_x);

    RUN_TEST(test_chmod_empty);
    RUN_TEST(test_chmod_null);
    RUN_TEST(test_chmod_bad_op);
    RUN_TEST(test_chmod_no_op);
    RUN_TEST(test_chmod_plus_no_perm);
    RUN_TEST(test_chmod_bad_char);
    RUN_TEST(test_chmod_trailing_comma);
    RUN_TEST(test_chmod_double_comma);

    return UNITY_END();
}