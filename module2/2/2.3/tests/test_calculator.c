#define UNITY_INCLUDE_DOUBLE
#include "unity.h"
#include "../Calculator.h"
#include <math.h>

void setUp(void)    {}
void tearDown(void) {}

// ADD

void test_add_positive(void) {
    TEST_ASSERT_EQUAL_DOUBLE(5.0, add(2.0, 3.0));
}

void test_add_negative(void) {
    TEST_ASSERT_EQUAL_DOUBLE(-5.0, add(-2.0, -3.0));
}

void test_add_mixed_sign(void) {
    TEST_ASSERT_EQUAL_DOUBLE(1.0, add(3.0, -2.0));
}

void test_add_zero_is_neutral(void) {
    TEST_ASSERT_EQUAL_DOUBLE(0.0, add(0.0, 0.0));
    TEST_ASSERT_EQUAL_DOUBLE(7.5, add(7.5, 0.0));
    TEST_ASSERT_EQUAL_DOUBLE(7.5, add(0.0, 7.5));
}

void test_add_fractional(void) {
    TEST_ASSERT_EQUAL_DOUBLE(0.3, add(0.1, 0.2));
}

// SUBTRACT

void test_subtract_positive(void) {
    TEST_ASSERT_EQUAL_DOUBLE(3.0, subtract(5.0, 2.0));
}

void test_subtract_negative_result(void) {
    TEST_ASSERT_EQUAL_DOUBLE(-3.0, subtract(2.0, 5.0));
}

void test_subtract_zero_rhs(void) {
    TEST_ASSERT_EQUAL_DOUBLE(5.0, subtract(5.0, 0.0));
}

void test_subtract_zero_lhs(void) {
    TEST_ASSERT_EQUAL_DOUBLE(-5.0, subtract(0.0, 5.0));
}

void test_subtract_same_gives_zero(void) {
    TEST_ASSERT_EQUAL_DOUBLE(0.0, subtract(7.0, 7.0));
}

// MULTIPLY

void test_multiply_positive(void) {
    TEST_ASSERT_EQUAL_DOUBLE(6.0, multiply(2.0, 3.0));
}

void test_multiply_by_zero(void) {
    TEST_ASSERT_EQUAL_DOUBLE(0.0, multiply(0.0, 5.0));
    TEST_ASSERT_EQUAL_DOUBLE(0.0, multiply(5.0, 0.0));
}

void test_multiply_by_one_is_neutral(void) {
    TEST_ASSERT_EQUAL_DOUBLE(5.0, multiply(5.0, 1.0));
}

void test_multiply_signs(void) {
    TEST_ASSERT_EQUAL_DOUBLE(-6.0, multiply(-2.0,  3.0));
    TEST_ASSERT_EQUAL_DOUBLE(-6.0, multiply( 2.0, -3.0));
    TEST_ASSERT_EQUAL_DOUBLE( 6.0, multiply(-2.0, -3.0));
}

// DIVIDE

void test_divide_positive(void) {
    TEST_ASSERT_EQUAL_DOUBLE(2.0, divide(6.0, 3.0));
}

void test_divide_fraction(void) {
    TEST_ASSERT_EQUAL_DOUBLE(0.5, divide(1.0, 2.0));
}

void test_divide_signs(void) {
    TEST_ASSERT_EQUAL_DOUBLE(-2.0, divide(-6.0,  3.0));
    TEST_ASSERT_EQUAL_DOUBLE(-2.0, divide( 6.0, -3.0));
    TEST_ASSERT_EQUAL_DOUBLE( 2.0, divide(-6.0, -3.0));
}

void test_divide_zero_numerator(void) {
    TEST_ASSERT_EQUAL_DOUBLE(0.0, divide(0.0, 5.0));
}

void test_divide_by_zero_returns_nan(void) {
    TEST_ASSERT_TRUE(isnan(divide(5.0, 0.0)));
}

void test_divide_negative_by_zero_returns_nan(void) {
    TEST_ASSERT_TRUE(isnan(divide(-5.0, 0.0)));
}

void test_divide_zero_by_zero_returns_nan(void) {
    TEST_ASSERT_TRUE(isnan(divide(0.0, 0.0)));
}

void test_divide_by_negative_zero_returns_nan(void) {
    TEST_ASSERT_TRUE(isnan(divide(5.0, -0.0)));
}

void test_divide_by_zero_is_not_infinity(void) {
    double r = divide(1.0, 0.0);
    TEST_ASSERT_FALSE(isinf(r));
    TEST_ASSERT_TRUE(isnan(r));
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_add_positive);
    RUN_TEST(test_add_negative);
    RUN_TEST(test_add_mixed_sign);
    RUN_TEST(test_add_zero_is_neutral);
    RUN_TEST(test_add_fractional);

    RUN_TEST(test_subtract_positive);
    RUN_TEST(test_subtract_negative_result);
    RUN_TEST(test_subtract_zero_rhs);
    RUN_TEST(test_subtract_zero_lhs);
    RUN_TEST(test_subtract_same_gives_zero);

    RUN_TEST(test_multiply_positive);
    RUN_TEST(test_multiply_by_zero);
    RUN_TEST(test_multiply_by_one_is_neutral);
    RUN_TEST(test_multiply_signs);

    RUN_TEST(test_divide_positive);
    RUN_TEST(test_divide_fraction);
    RUN_TEST(test_divide_signs);
    RUN_TEST(test_divide_zero_numerator);
    RUN_TEST(test_divide_by_zero_returns_nan);
    RUN_TEST(test_divide_negative_by_zero_returns_nan);
    RUN_TEST(test_divide_zero_by_zero_returns_nan);
    RUN_TEST(test_divide_by_negative_zero_returns_nan);
    RUN_TEST(test_divide_by_zero_is_not_infinity);

    return UNITY_END();
}