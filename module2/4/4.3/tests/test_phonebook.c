#include "unity.h"
#include "../PhoneBook.h"
#include "../Strings.h"
#include <stdio.h>
#include <stdlib.h>

void setUp(void) {}
void tearDown(void) {}

static Contact make_contact(const char *surname,
                            const char *name,
                            const char *patronymic)
{
    Contact c;
    contact_init(&c);
    if (surname)    contact_set_surname(&c, surname);
    if (name)       contact_set_name(&c, name);
    if (patronymic) contact_set_patronymic(&c, patronymic);
    return c;
}

static void add_new(PhoneBook *pb, const char *surname,
                    const char *name, const char *patronymic)
{
    Contact c = make_contact(surname, name, patronymic);
    TEST_ASSERT_EQUAL_INT(0, add_contact(pb, &c));
    contact_free(&c);
}

static int key_cmp(const Contact *a, const Contact *b) {
    const char *as = a->surname    ? a->surname    : "";
    const char *bs = b->surname    ? b->surname    : "";
    int r = my_strcmp(as, bs);
    if (r) return r;
    const char *an = a->name       ? a->name       : "";
    const char *bn = b->name       ? b->name       : "";
    r = my_strcmp(an, bn);
    if (r) return r;
    const char *ap = a->patronymic ? a->patronymic : "";
    const char *bp = b->patronymic ? b->patronymic : "";
    return my_strcmp(ap, bp);
}

static int verify_avl(const PBNode *n) {
    if (!n) return 0;

    int lh = verify_avl(n->left);
    if (lh < 0) return -1;
    int rh = verify_avl(n->right);
    if (rh < 0) return -1;

    int    expected_h = 1 + (lh > rh ? lh : rh);
    size_t expected_s = 1
                      + (n->left  ? n->left->size  : 0)
                      + (n->right ? n->right->size : 0);

    if (n->height != expected_h) return -1;
    if (n->size   != expected_s) return -1;

    int bf = lh - rh;
    if (bf < -1 || bf > 1) return -1;

    return expected_h;
}

static int verify_bst_rec(const PBNode *n, const Contact **prev) {
    if (!n) return 1;
    if (!verify_bst_rec(n->left, prev)) return 0;
    if (*prev && key_cmp(*prev, &n->contact) > 0) return 0;
    *prev = &n->contact;
    return verify_bst_rec(n->right, prev);
}

static int verify_bst(const PBNode *n) {
    const Contact *prev = NULL;
    return verify_bst_rec(n, &prev);
}

static void test_init_empty(void) {
    PhoneBook pb;
    pb_init(&pb);
    TEST_ASSERT_NULL(pb.root);
    TEST_ASSERT_EQUAL_size_t(0, pb.count);
    pb_free(&pb);
}

static void test_add_and_remove(void) {
    PhoneBook pb;
    pb_init(&pb);

    add_new(&pb, "Иванов", "Иван", NULL);
    add_new(&pb, "Петров", "Пётр", NULL);
    TEST_ASSERT_EQUAL_size_t(2, pb.count);

    TEST_ASSERT_EQUAL_STRING("Иванов", pb_get_by_index(&pb, 0)->surname);
    TEST_ASSERT_EQUAL_STRING("Петров", pb_get_by_index(&pb, 1)->surname);

    TEST_ASSERT_EQUAL_INT(0, remove_contact(&pb, 0));
    TEST_ASSERT_EQUAL_size_t(1, pb.count);
    TEST_ASSERT_EQUAL_STRING("Петров", pb_get_by_index(&pb, 0)->surname);

    pb_free(&pb);
}

static void test_remove_invalid_index(void) {
    PhoneBook pb;
    pb_init(&pb);
    TEST_ASSERT_EQUAL_INT(-1, remove_contact(&pb, 0));

    add_new(&pb, "A", NULL, NULL);
    TEST_ASSERT_EQUAL_INT(-1, remove_contact(&pb, 1));
    TEST_ASSERT_EQUAL_INT(-1, remove_contact(&pb, 99));
    TEST_ASSERT_EQUAL_size_t(1, pb.count);

    pb_free(&pb);
}

static void test_get_by_index_out_of_range(void) {
    PhoneBook pb;
    pb_init(&pb);
    TEST_ASSERT_NULL(pb_get_by_index(&pb, 0));

    add_new(&pb, "A", NULL, NULL);
    TEST_ASSERT_NOT_NULL(pb_get_by_index(&pb, 0));
    TEST_ASSERT_NULL(pb_get_by_index(&pb, 1));
    TEST_ASSERT_NULL(pb_get_by_index(&pb, 100));

    pb_free(&pb);
}

static void test_inorder_sorted(void) {
    PhoneBook pb;
    pb_init(&pb);
    add_new(&pb, "C", NULL, NULL);
    add_new(&pb, "A", NULL, NULL);
    add_new(&pb, "E", NULL, NULL);
    add_new(&pb, "B", NULL, NULL);
    add_new(&pb, "D", NULL, NULL);

    TEST_ASSERT_EQUAL_size_t(5, pb.count);
    const char *expected[] = {"A", "B", "C", "D", "E"};
    for (size_t i = 0; i < 5; i++)
        TEST_ASSERT_EQUAL_STRING(expected[i],
            pb_get_by_index(&pb, i)->surname);

    TEST_ASSERT_TRUE(verify_avl(pb.root) >= 0);
    TEST_ASSERT_TRUE(verify_bst(pb.root));

    pb_free(&pb);
}

static void test_order_by_name_then_patronymic(void) {
    PhoneBook pb;
    pb_init(&pb);
    add_new(&pb, "Иванов", "Борис", "Борисович");
    add_new(&pb, "Иванов", "Алексей", "Алексеевич");
    add_new(&pb, "Иванов", "Алексей", "Андреевич");
    add_new(&pb, "Иванов", "Алексей", NULL);

    const char *expect_name[] = {"Алексей","Алексей","Алексей","Борис"};
    const char *expect_patr[] = {NULL,      "Алексеевич","Андреевич","Борисович"};

    for (size_t i = 0; i < 4; i++) {
        const Contact *c = pb_get_by_index(&pb, i);
        TEST_ASSERT_NOT_NULL(c);
        TEST_ASSERT_EQUAL_STRING(expect_name[i], c->name);
        if (expect_patr[i])
            TEST_ASSERT_EQUAL_STRING(expect_patr[i], c->patronymic);
        else
            TEST_ASSERT_NULL(c->patronymic);
    }
    pb_free(&pb);
}

static void test_duplicates_allowed(void) {
    PhoneBook pb;
    pb_init(&pb);
    add_new(&pb, "Иванов", "Иван", "Иванович");
    add_new(&pb, "Иванов", "Иван", "Иванович");
    add_new(&pb, "Петров", "Пётр", NULL);

    TEST_ASSERT_EQUAL_size_t(3, pb.count);

    TEST_ASSERT_EQUAL_STRING("Иванов", pb_get_by_index(&pb, 0)->surname);
    TEST_ASSERT_EQUAL_STRING("Иванов", pb_get_by_index(&pb, 1)->surname);
    TEST_ASSERT_EQUAL_STRING("Петров", pb_get_by_index(&pb, 2)->surname);

    TEST_ASSERT_EQUAL_INT(0, remove_contact(&pb, 0));
    TEST_ASSERT_EQUAL_size_t(2, pb.count);
    TEST_ASSERT_EQUAL_STRING("Иванов", pb_get_by_index(&pb, 0)->surname);
    TEST_ASSERT_EQUAL_STRING("Петров", pb_get_by_index(&pb, 1)->surname);

    pb_free(&pb);
}

static void test_remove_leaf(void) {
    PhoneBook pb;
    pb_init(&pb);
    add_new(&pb, "D", NULL, NULL);
    add_new(&pb, "B", NULL, NULL);
    add_new(&pb, "F", NULL, NULL);
    add_new(&pb, "A", NULL, NULL);
    add_new(&pb, "C", NULL, NULL);
    add_new(&pb, "E", NULL, NULL);
    add_new(&pb, "G", NULL, NULL);

    TEST_ASSERT_EQUAL_INT(0, remove_contact(&pb, 0));
    TEST_ASSERT_EQUAL_size_t(6, pb.count);
    const char *expect[] = {"B","C","D","E","F","G"};
    for (size_t i = 0; i < 6; i++)
        TEST_ASSERT_EQUAL_STRING(expect[i],
            pb_get_by_index(&pb, i)->surname);
    TEST_ASSERT_TRUE(verify_avl(pb.root) >= 0);
    TEST_ASSERT_TRUE(verify_bst(pb.root));
    pb_free(&pb);
}

static void test_remove_node_with_one_child(void) {
    PhoneBook pb;
    pb_init(&pb);
    add_new(&pb, "B", NULL, NULL);
    add_new(&pb, "A", NULL, NULL);
    add_new(&pb, "C", NULL, NULL);
    add_new(&pb, "D", NULL, NULL);

    const Contact *c = pb_get_by_index(&pb, 2);
    TEST_ASSERT_EQUAL_STRING("C", c->surname);
    TEST_ASSERT_EQUAL_INT(0, remove_contact(&pb, 2));

    TEST_ASSERT_EQUAL_size_t(3, pb.count);
    const char *expect[] = {"A","B","D"};
    for (size_t i = 0; i < 3; i++)
        TEST_ASSERT_EQUAL_STRING(expect[i],
            pb_get_by_index(&pb, i)->surname);
    TEST_ASSERT_TRUE(verify_avl(pb.root) >= 0);
    pb_free(&pb);
}

static void test_remove_node_with_two_children(void) {
    PhoneBook pb;
    pb_init(&pb);
    add_new(&pb, "D", NULL, NULL);
    add_new(&pb, "B", NULL, NULL);
    add_new(&pb, "F", NULL, NULL);
    add_new(&pb, "A", NULL, NULL);
    add_new(&pb, "C", NULL, NULL);
    add_new(&pb, "E", NULL, NULL);
    add_new(&pb, "G", NULL, NULL);

    TEST_ASSERT_EQUAL_STRING("D", pb_get_by_index(&pb, 3)->surname);
    TEST_ASSERT_EQUAL_INT(0, remove_contact(&pb, 3));

    TEST_ASSERT_EQUAL_size_t(6, pb.count);
    const char *expect[] = {"A","B","C","E","F","G"};
    for (size_t i = 0; i < 6; i++)
        TEST_ASSERT_EQUAL_STRING(expect[i],
            pb_get_by_index(&pb, i)->surname);
    TEST_ASSERT_TRUE(verify_avl(pb.root) >= 0);
    TEST_ASSERT_TRUE(verify_bst(pb.root));
    pb_free(&pb);
}

static void test_avl_after_sequential_inserts(void) {
    PhoneBook pb;
    pb_init(&pb);

    for (int i = 0; i < 100; i++) {
        char buf[16];
        snprintf(buf, sizeof(buf), "S%03d", i);
        add_new(&pb, buf, NULL, NULL);
    }

    TEST_ASSERT_EQUAL_size_t(100, pb.count);
    TEST_ASSERT_NOT_NULL(pb.root);
    TEST_ASSERT_EQUAL_INT(100, (int)pb.root->size);

    TEST_ASSERT_TRUE(pb.root->height <= 10);

    TEST_ASSERT_TRUE(verify_avl(pb.root) >= 0);
    TEST_ASSERT_TRUE(verify_bst(pb.root));

    pb_free(&pb);
}

static void test_avl_after_each_remove(void) {
    PhoneBook pb;
    pb_init(&pb);
    for (int i = 0; i < 50; i++) {
        char buf[16];
        snprintf(buf, sizeof(buf), "S%03d", i);
        add_new(&pb, buf, NULL, NULL);
    }

    for (int i = 0; i < 25; i++) {
        TEST_ASSERT_EQUAL_INT(0, remove_contact(&pb, 0));
        TEST_ASSERT_TRUE(verify_avl(pb.root) >= 0);
        TEST_ASSERT_TRUE(verify_bst(pb.root));
    }
    TEST_ASSERT_EQUAL_size_t(25, pb.count);

    pb_free(&pb);
}

static void test_rebuild_after_key_change(void) {
    PhoneBook pb;
    pb_init(&pb);
    add_new(&pb, "A", NULL, NULL);
    add_new(&pb, "B", NULL, NULL);
    add_new(&pb, "C", NULL, NULL);

    Contact *first = pb_get_by_index(&pb, 0);
    TEST_ASSERT_NOT_NULL(first);
    TEST_ASSERT_EQUAL_INT(0, contact_set_surname(first, "Z"));

    TEST_ASSERT_EQUAL_INT(0, pb_rebuild(&pb));

    TEST_ASSERT_EQUAL_size_t(3, pb.count);
    TEST_ASSERT_EQUAL_STRING("B", pb_get_by_index(&pb, 0)->surname);
    TEST_ASSERT_EQUAL_STRING("C", pb_get_by_index(&pb, 1)->surname);
    TEST_ASSERT_EQUAL_STRING("Z", pb_get_by_index(&pb, 2)->surname);

    TEST_ASSERT_TRUE(verify_avl(pb.root) >= 0);
    TEST_ASSERT_TRUE(verify_bst(pb.root));

    pb_free(&pb);
}

static void test_rebuild_empty(void) {
    PhoneBook pb;
    pb_init(&pb);
    TEST_ASSERT_EQUAL_INT(0, pb_rebuild(&pb));
    TEST_ASSERT_NULL(pb.root);
    TEST_ASSERT_EQUAL_size_t(0, pb.count);
    pb_free(&pb);
}

static void test_rebuild_single(void) {
    PhoneBook pb;
    pb_init(&pb);
    add_new(&pb, "A", NULL, NULL);
    TEST_ASSERT_EQUAL_INT(0, pb_rebuild(&pb));
    TEST_ASSERT_EQUAL_size_t(1, pb.count);
    TEST_ASSERT_EQUAL_STRING("A", pb_get_by_index(&pb, 0)->surname);
    pb_free(&pb);
}

static void test_stress_insert_remove(void) {
    PhoneBook pb;
    pb_init(&pb);

    for (int i = 0; i < 200; i++) {
        char buf[16];
        snprintf(buf, sizeof(buf), "S%03d", (i * 73) % 200);
        add_new(&pb, buf, NULL, NULL);
    }
    TEST_ASSERT_EQUAL_size_t(200, pb.count);
    TEST_ASSERT_TRUE(verify_avl(pb.root) >= 0);
    TEST_ASSERT_TRUE(verify_bst(pb.root));

    while (pb.count > 0) {
        size_t idx = (pb.count * 7) % pb.count;
        TEST_ASSERT_EQUAL_INT(0, remove_contact(&pb, idx));
        TEST_ASSERT_TRUE(verify_avl(pb.root) >= 0);
        TEST_ASSERT_TRUE(verify_bst(pb.root));
    }
    TEST_ASSERT_NULL(pb.root);
    TEST_ASSERT_EQUAL_size_t(0, pb.count);

    pb_free(&pb);
}

static void test_free_resets_book(void) {
    PhoneBook pb;
    pb_init(&pb);
    add_new(&pb, "A", NULL, NULL);
    add_new(&pb, "B", NULL, NULL);

    pb_free(&pb);
    TEST_ASSERT_NULL(pb.root);
    TEST_ASSERT_EQUAL_size_t(0, pb.count);

    pb_free(&pb);
    TEST_ASSERT_NULL(pb.root);
    TEST_ASSERT_EQUAL_size_t(0, pb.count);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_init_empty);
    RUN_TEST(test_add_and_remove);
    RUN_TEST(test_remove_invalid_index);
    RUN_TEST(test_get_by_index_out_of_range);
    RUN_TEST(test_inorder_sorted);
    RUN_TEST(test_order_by_name_then_patronymic);
    RUN_TEST(test_duplicates_allowed);
    RUN_TEST(test_remove_leaf);
    RUN_TEST(test_remove_node_with_one_child);
    RUN_TEST(test_remove_node_with_two_children);
    RUN_TEST(test_avl_after_sequential_inserts);
    RUN_TEST(test_avl_after_each_remove);
    RUN_TEST(test_rebuild_after_key_change);
    RUN_TEST(test_rebuild_empty);
    RUN_TEST(test_rebuild_single);
    RUN_TEST(test_stress_insert_remove);
    RUN_TEST(test_free_resets_book);
    return UNITY_END();
}