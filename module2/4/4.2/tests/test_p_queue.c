#include "unity.h"
#include "p_queue.h"

// Один общий PQueue на тест — инициализируется в setUp
static PQueue pq;

void setUp(void) {
    pq_init(&pq);
}

void tearDown(void) {
    /* Дренируем, чтобы освободить все узлы и не было утечек
       даже если тест завершился раньше, чем ожидалось. */
    int v;
    while (pq_pop_first(&pq, &v)) { /* drain */ }
}

// инициализация / пустая очередь

void test_init_empty_pop_first(void) {
    int v = -1;
    TEST_ASSERT_EQUAL_INT(0, pq_pop_first(&pq, &v));
}

void test_init_empty_pop_prio(void) {
    int v = -1;
    TEST_ASSERT_EQUAL_INT(0, pq_pop_prio(&pq, 100, &v));
}

void test_init_empty_pop_at_least(void) {
    int v = -1;
    TEST_ASSERT_EQUAL_INT(0, pq_pop_at_least(&pq, 100, &v));
}

// pq_push: валидация

void test_push_valid(void) {
    TEST_ASSERT_EQUAL_INT(1, pq_push(&pq, 42, 100));
    int v = 0;
    TEST_ASSERT_EQUAL_INT(1, pq_pop_prio(&pq, 100, &v));
    TEST_ASSERT_EQUAL_INT(42, v);
}

void test_push_prio_negative_rejected(void) {
    TEST_ASSERT_EQUAL_INT(0, pq_push(&pq, 1, -1));
}

void test_push_prio_256_rejected(void) {
    TEST_ASSERT_EQUAL_INT(0, pq_push(&pq, 1, 256));
}

void test_push_boundary_prios_accepted(void) {
    TEST_ASSERT_EQUAL_INT(1, pq_push(&pq, 1, 0));
    TEST_ASSERT_EQUAL_INT(1, pq_push(&pq, 2, 255));
    TEST_ASSERT_EQUAL_INT(1, pq_push(&pq, 3, 128));
}

// FIFO внутри одного приоритета

void test_fifo_same_prio(void) {
    pq_push(&pq, 1, 10);
    pq_push(&pq, 2, 10);
    pq_push(&pq, 3, 10);
    int v;
    TEST_ASSERT_EQUAL_INT(1, pq_pop_prio(&pq, 10, &v)); TEST_ASSERT_EQUAL_INT(1, v);
    TEST_ASSERT_EQUAL_INT(1, pq_pop_prio(&pq, 10, &v)); TEST_ASSERT_EQUAL_INT(2, v);
    TEST_ASSERT_EQUAL_INT(1, pq_pop_prio(&pq, 10, &v)); TEST_ASSERT_EQUAL_INT(3, v);
    TEST_ASSERT_EQUAL_INT(0, pq_pop_prio(&pq, 10, &v));
}

// pop_first: порядок по приоритету

void test_pop_first_order(void) {
    pq_push(&pq, 100, 100);
    pq_push(&pq,   5,   5);
    pq_push(&pq, 200, 200);
    pq_push(&pq,   0,   0);
    int v;
    TEST_ASSERT_EQUAL_INT(1, pq_pop_first(&pq, &v)); TEST_ASSERT_EQUAL_INT(  0, v);
    TEST_ASSERT_EQUAL_INT(1, pq_pop_first(&pq, &v)); TEST_ASSERT_EQUAL_INT(  5, v);
    TEST_ASSERT_EQUAL_INT(1, pq_pop_first(&pq, &v)); TEST_ASSERT_EQUAL_INT(100, v);
    TEST_ASSERT_EQUAL_INT(1, pq_pop_first(&pq, &v)); TEST_ASSERT_EQUAL_INT(200, v);
    TEST_ASSERT_EQUAL_INT(0, pq_pop_first(&pq, &v));
}

void test_pop_first_fifo_across_same_prio(void) {
    /* Смешанные приоритеты, FIFO должен соблюдаться внутри каждого. */
    pq_push(&pq, 'A', 5);
    pq_push(&pq, 'B', 3);
    pq_push(&pq, 'C', 5);
    pq_push(&pq, 'D', 3);
    pq_push(&pq, 'E', 5);
    int v;
    TEST_ASSERT_EQUAL_INT(1, pq_pop_first(&pq, &v)); TEST_ASSERT_EQUAL_INT('B', v);
    TEST_ASSERT_EQUAL_INT(1, pq_pop_first(&pq, &v)); TEST_ASSERT_EQUAL_INT('D', v);
    TEST_ASSERT_EQUAL_INT(1, pq_pop_first(&pq, &v)); TEST_ASSERT_EQUAL_INT('A', v);
    TEST_ASSERT_EQUAL_INT(1, pq_pop_first(&pq, &v)); TEST_ASSERT_EQUAL_INT('C', v);
    TEST_ASSERT_EQUAL_INT(1, pq_pop_first(&pq, &v)); TEST_ASSERT_EQUAL_INT('E', v);
}

// pop_prio: точное совпадение

void test_pop_prio_exact(void) {
    pq_push(&pq, 10, 10);
    pq_push(&pq, 20, 20);
    int v;
    TEST_ASSERT_EQUAL_INT(1, pq_pop_prio(&pq, 20, &v)); TEST_ASSERT_EQUAL_INT(20, v);
    TEST_ASSERT_EQUAL_INT(1, pq_pop_prio(&pq, 10, &v)); TEST_ASSERT_EQUAL_INT(10, v);
}

void test_pop_prio_missing_level(void) {
    pq_push(&pq, 10, 10);
    int v;
    TEST_ASSERT_EQUAL_INT(0, pq_pop_prio(&pq, 20, &v));
    /* элемент с prio=10 не должен пострадать */
    TEST_ASSERT_EQUAL_INT(1, pq_pop_prio(&pq, 10, &v));
    TEST_ASSERT_EQUAL_INT(10, v);
}

void test_pop_prio_invalid_args(void) {
    int v;
    TEST_ASSERT_EQUAL_INT(0, pq_pop_prio(&pq, -1,  &v));
    TEST_ASSERT_EQUAL_INT(0, pq_pop_prio(&pq, 256, &v));
}

// Ваш пример из ТЗ: при заданном 10 идут 9, 7, 5 — по убыванию "лучшести"
void test_pop_at_least_nearest_first(void) {
    pq_push(&pq, 51, 5);
    pq_push(&pq, 71, 7);
    pq_push(&pq, 91, 9);
    int v;
    TEST_ASSERT_EQUAL_INT(1, pq_pop_at_least(&pq, 10, &v)); TEST_ASSERT_EQUAL_INT(91, v); /* prio 9 */
    TEST_ASSERT_EQUAL_INT(1, pq_pop_at_least(&pq, 10, &v)); TEST_ASSERT_EQUAL_INT(71, v); /* prio 7 */
    TEST_ASSERT_EQUAL_INT(1, pq_pop_at_least(&pq, 10, &v)); TEST_ASSERT_EQUAL_INT(51, v); /* prio 5 */
    TEST_ASSERT_EQUAL_INT(0, pq_pop_at_least(&pq, 10, &v));
}

// Дырки в диапазоне [prio-1 .. 0] пропускаются
void test_pop_at_least_skips_gaps(void) {
    pq_push(&pq, 50, 5);
    int v;
    /* 9, 8, 7, 6 пусты — должен взять 5 */
    TEST_ASSERT_EQUAL_INT(1, pq_pop_at_least(&pq, 10, &v));
    TEST_ASSERT_EQUAL_INT(50, v);
}

// Граница: prio = 256 — «всё лучше 256» = вся очередь, начиная с 255
void test_pop_at_least_256_drains_all(void) {
    pq_push(&pq, 1, 255);
    pq_push(&pq, 2,   0);
    int v;
    TEST_ASSERT_EQUAL_INT(1, pq_pop_at_least(&pq, 256, &v)); TEST_ASSERT_EQUAL_INT(1, v);
    TEST_ASSERT_EQUAL_INT(1, pq_pop_at_least(&pq, 256, &v)); TEST_ASSERT_EQUAL_INT(2, v);
    TEST_ASSERT_EQUAL_INT(0, pq_pop_at_least(&pq, 256, &v));
}

// Граница: prio = 0 — «лучше 0» ничего нет (0 самый высокий)
void test_pop_at_least_zero_finds_nothing(void) {
    pq_push(&pq, 1, 0);
    int v;
    TEST_ASSERT_EQUAL_INT(0, pq_pop_at_least(&pq, 0, &v));
    /* но сама очередь не повреждена */
    TEST_ASSERT_EQUAL_INT(1, pq_pop_first(&pq, &v));
    TEST_ASSERT_EQUAL_INT(1, v);
}

void test_pop_at_least_invalid_args(void) {
    int v;
    TEST_ASSERT_EQUAL_INT(0, pq_pop_at_least(&pq, -1,  &v));
    TEST_ASSERT_EQUAL_INT(0, pq_pop_at_least(&pq, 300, &v));
}

// prio = 1 → должен найти приоритет 0
void test_pop_at_least_one_finds_zero_prio(void) {
    pq_push(&pq, 7, 0);
    int v;
    TEST_ASSERT_EQUAL_INT(1, pq_pop_at_least(&pq, 1, &v));
    TEST_ASSERT_EQUAL_INT(7, v);
}

// Стресс: 500 элементов, все извлекаются

void test_stress_push_pop_all(void) {
    enum { N = 500 };
    for (int i = 0; i < N; ++i) {
        TEST_ASSERT_EQUAL_INT(1, pq_push(&pq, i, (i * 7) % N_PRIO));
    }
    int v, count = 0;
    while (pq_pop_first(&pq, &v)) ++count;
    TEST_ASSERT_EQUAL_INT(N, count);
}

// Смешанные операции: push, pop_prio, pop_first, pop_at_least
void test_mixed_operations(void) {
    int v;
    pq_push(&pq, 1, 10);
    pq_push(&pq, 2, 20);
    pq_push(&pq, 3, 30);
    TEST_ASSERT_EQUAL_INT(1, pq_pop_prio(&pq, 20, &v));   TEST_ASSERT_EQUAL_INT(2, v);
    pq_push(&pq, 4, 5);
    TEST_ASSERT_EQUAL_INT(1, pq_pop_first(&pq, &v));      TEST_ASSERT_EQUAL_INT(4, v); /* prio 5 */
    TEST_ASSERT_EQUAL_INT(1, pq_pop_at_least(&pq, 15, &v)); TEST_ASSERT_EQUAL_INT(1, v); /* prio 10 */
    TEST_ASSERT_EQUAL_INT(1, pq_pop_first(&pq, &v));      TEST_ASSERT_EQUAL_INT(3, v);
    TEST_ASSERT_EQUAL_INT(0, pq_pop_first(&pq, &v));
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_init_empty_pop_first);
    RUN_TEST(test_init_empty_pop_prio);
    RUN_TEST(test_init_empty_pop_at_least);

    RUN_TEST(test_push_valid);
    RUN_TEST(test_push_prio_negative_rejected);
    RUN_TEST(test_push_prio_256_rejected);
    RUN_TEST(test_push_boundary_prios_accepted);

    RUN_TEST(test_fifo_same_prio);
    RUN_TEST(test_pop_first_order);
    RUN_TEST(test_pop_first_fifo_across_same_prio);

    RUN_TEST(test_pop_prio_exact);
    RUN_TEST(test_pop_prio_missing_level);
    RUN_TEST(test_pop_prio_invalid_args);

    RUN_TEST(test_pop_at_least_nearest_first);
    RUN_TEST(test_pop_at_least_skips_gaps);
    RUN_TEST(test_pop_at_least_256_drains_all);
    RUN_TEST(test_pop_at_least_zero_finds_nothing);
    RUN_TEST(test_pop_at_least_one_finds_zero_prio);
    RUN_TEST(test_pop_at_least_invalid_args);

    RUN_TEST(test_stress_push_pop_all);
    RUN_TEST(test_mixed_operations);

    return UNITY_END();
}