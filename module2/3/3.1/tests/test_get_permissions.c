#include "unity.h"
#include "getPermissions.h"
#include "Strings.h"

#include <stdlib.h>
#include <sys/stat.h>

void setUp(void)    {}
void tearDown(void) {}

//file_get_st_mode

static void test_file_st_mode_existing(void) {
    TEST_ASSERT_TRUE(file_get_st_mode("/tmp") >= 0);
}

static void test_file_st_mode_tmp_is_dir(void) {
    int mode = file_get_st_mode("/tmp");
    TEST_ASSERT_TRUE(mode >= 0);
    TEST_ASSERT_EQUAL_CHAR('d', st_mode_type_char((unsigned)mode));
}

static void test_file_st_mode_nonexistent(void) {
    TEST_ASSERT_EQUAL_INT(-1, file_get_st_mode("/no/such/path/xyz"));
}

//st_mode_type_char

static void test_type_char_reg(void)  { TEST_ASSERT_EQUAL_CHAR('-', st_mode_type_char(S_IFREG  | 0644)); }
static void test_type_char_dir(void)  { TEST_ASSERT_EQUAL_CHAR('d', st_mode_type_char(S_IFDIR  | 0755)); }
static void test_type_char_lnk(void)  { TEST_ASSERT_EQUAL_CHAR('l', st_mode_type_char(S_IFLNK  | 0777)); }
static void test_type_char_chr(void)  { TEST_ASSERT_EQUAL_CHAR('c', st_mode_type_char(S_IFCHR  | 0666)); }
static void test_type_char_blk(void)  { TEST_ASSERT_EQUAL_CHAR('b', st_mode_type_char(S_IFBLK  | 0666)); }
static void test_type_char_fifo(void) { TEST_ASSERT_EQUAL_CHAR('p', st_mode_type_char(S_IFIFO  | 0644)); }
static void test_type_char_sock(void) { TEST_ASSERT_EQUAL_CHAR('s', st_mode_type_char(S_IFSOCK | 0755)); }
static void test_type_char_unknown(void){ TEST_ASSERT_EQUAL_CHAR('?', st_mode_type_char(0u)); }

//st_mode_to_ls_symbolic

static void test_ls_symbolic_reg(void) {
    char *s = st_mode_to_ls_symbolic(S_IFREG | 0755);
    TEST_ASSERT_NOT_NULL(s);
    TEST_ASSERT_EQUAL_STRING("-rwxr-xr-x", s);
    free(s);
}

static void test_ls_symbolic_dir(void) {
    char *s = st_mode_to_ls_symbolic(S_IFDIR | 0755);
    TEST_ASSERT_NOT_NULL(s);
    TEST_ASSERT_EQUAL_STRING("drwxr-xr-x", s);
    free(s);
}

static void test_ls_symbolic_setuid(void) {
    char *s = st_mode_to_ls_symbolic(S_IFREG | 04755);
    TEST_ASSERT_NOT_NULL(s);
    TEST_ASSERT_EQUAL_STRING("-rwsr-xr-x", s);
    free(s);
}

static void test_ls_symbolic_dir_sticky(void) {
    char *s = st_mode_to_ls_symbolic(S_IFDIR | 01777);
    TEST_ASSERT_NOT_NULL(s);
    TEST_ASSERT_EQUAL_STRING("drwxrwxrwt", s);
    free(s);
}

//Сквозная проверка через реальный /tmp

static void test_tmp_full_pipeline(void) {
    int st_mode = file_get_st_mode("/tmp");
    TEST_ASSERT_TRUE(st_mode >= 0);

    char *s = st_mode_to_ls_symbolic((unsigned)st_mode);
    TEST_ASSERT_NOT_NULL(s);
    TEST_ASSERT_EQUAL_CHAR('d', s[0]);
    TEST_ASSERT_EQUAL_INT(10, (int)my_strlen(s));
    free(s);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_file_st_mode_existing);
    RUN_TEST(test_file_st_mode_tmp_is_dir);
    RUN_TEST(test_file_st_mode_nonexistent);

    RUN_TEST(test_type_char_reg);
    RUN_TEST(test_type_char_dir);
    RUN_TEST(test_type_char_lnk);
    RUN_TEST(test_type_char_chr);
    RUN_TEST(test_type_char_blk);
    RUN_TEST(test_type_char_fifo);
    RUN_TEST(test_type_char_sock);
    RUN_TEST(test_type_char_unknown);

    RUN_TEST(test_ls_symbolic_reg);
    RUN_TEST(test_ls_symbolic_dir);
    RUN_TEST(test_ls_symbolic_setuid);
    RUN_TEST(test_ls_symbolic_dir_sticky);

    RUN_TEST(test_tmp_full_pipeline);

    return UNITY_END();
}