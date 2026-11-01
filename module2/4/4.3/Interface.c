#include "Interface.h"
#include "Strings.h"
#include <stdio.h>
#include <stdlib.h>

// вспомогательные

static char* read_line(void) {
    size_t cap = 64, len = 0;
    char* buf = malloc(cap);
    if (!buf) return NULL;

    int ch;
    while ((ch = fgetc(stdin)) != EOF && ch != '\n') {
        if (ch == '\r') continue;
        if (len + 1 >= cap) {
            size_t nc = cap * 2;
            char* tmp = realloc(buf, nc);
            if (!tmp) { free(buf); return NULL; }
            buf = tmp; cap = nc;
        }
        buf[len++] = (char)ch;
    }

    if (ch == EOF && len == 0) { free(buf); return NULL; }
    buf[len] = '\0';
    return buf;
}

static char* read_line_prompt(const char* prompt) {
    if (prompt) { fputs(prompt, stdout); fflush(stdout); }
    return read_line();
}

static int read_size_t(const char* prompt, size_t* out) {
    char* line = read_line_prompt(prompt);
    if (!line) return -1;
    int rc = sscanf(line, "%zu", out);
    free(line);
    return (rc == 1) ? 0 : 1;
}

static int read_yes_no(const char* prompt) {
    while (1) {
        char* line = read_line_prompt(prompt);
        if (!line) return -1;
        char ch = line[0];
        free(line);
        if (ch == 'y' || ch == 'Y') return 1;
        if (ch == 'n' || ch == 'N') return 0;
        puts("Ответьте 'y' или 'n'.");
    }
}

static void edit_scalar(Contact* c,
                        int (*setter)(Contact*, const char*),
                        const char* label,
                        const char* current,
                        int allow_clear)
{
    printf("%s сейчас: %s\n", label, current ? current : "—");

    const char* hint = allow_clear
        ? "Новое значение (Enter — оставить, '-' — очистить): "
        : "Новое значение (Enter — оставить): ";
    char* line = read_line_prompt(hint);
    if (!line) return;

    if (line[0] == '\0') { free(line); return; }

    if (line[0] == '-' && line[1] == '\0') {
        free(line);
        if (!allow_clear) { puts("Обязательное поле нельзя очистить."); return; }
        setter(c, "");
        printf("%s очищено.\n", label);
        return;
    }

    setter(c, line);
    free(line);
    printf("%s обновлено.\n", label);
}

static void edit_list(Contact* c,
                      char*** arr, size_t* count,
                      int (*adder)(Contact*, const char*),
                      int (*remover)(Contact*, size_t),
                      const char* label)
{
    while (1) {
        printf("\n%s (%zu)\n", label, *count);
        if (*count == 0) puts("  (пусто)");
        else for (size_t i = 0; i < *count; i++)
            printf("  %zu. %s\n", i + 1, (*arr)[i]);
        puts("1. Добавить");
        puts("2. Удалить");
        puts("0. Назад");

        size_t choice;
        int rc = read_size_t("Действие: ", &choice);
        if (rc < 0) return;
        if (rc > 0) { puts("Неверный ввод."); continue; }

        if (choice == 0) return;

        if (choice == 1) {
            char* line = read_line_prompt("Значение: ");
            if (!line) return;
            if (adder(c, line) != 0) puts("Ошибка памяти.");
            free(line);
        } else if (choice == 2) {
            if (*count == 0) { puts("Нечего удалять."); continue; }
            size_t n;
            if (read_size_t("Номер для удаления: ", &n) != 0) continue;
            if (n < 1 || n > *count) { puts("Нет такого номера."); continue; }
            remover(c, n - 1);
        } else {
            puts("Нет такого пункта.");
        }
    }
}

static int fill_required(Contact* c, int (*setter)(Contact*, const char*), const char* prompt) {
    while (1) {
        char* line = read_line_prompt(prompt);
        if (!line) return -1;
        if (line[0] != '\0') {
            int rc = setter(c, line);
            free(line);
            return rc;
        }
        free(line);
        puts("Поле обязательно для заполнения.");
    }
}

static int fill_optional(Contact* c, int (*setter)(Contact*, const char*), const char* prompt) {
    char* line = read_line_prompt(prompt);
    if (!line) return -1;
    int rc = setter(c, line);
    free(line);
    return rc;
}

static int fill_list(Contact* c, int (*adder)(Contact*, const char*), const char* item_name) {
    char prompt[128];
    while (1) {
        snprintf(prompt, sizeof(prompt), "Добавить %s? (y/n): ", item_name);
        int yn = read_yes_no(prompt);
        if (yn <= 0) return yn;

        snprintf(prompt, sizeof(prompt), "  %s: ", item_name);
        char* line = read_line_prompt(prompt);
        if (!line) return -1;
        int rc = adder(c, line);
        free(line);
        if (rc != 0) return -1;
    }
}

static void print_str_or_dash(const char* label, const char* value) {
    printf("%s: %s\n", label, value ? value : "—");
}

static void print_str_list(const char* label, char* const* items, size_t count) {
    printf("%s:\n", label);
    if (count == 0) { puts("  —"); return; }
    for (size_t i = 0; i < count; i++)
        printf("  - %s\n", items[i]);
}

// публичные меню

void menu_print_contact(const Contact* c) {
    print_str_or_dash("Фамилия", c->surname);
    print_str_or_dash("Имя", c->name);
    print_str_or_dash("Отчество", c->patronymic);
    print_str_or_dash("Место работы", c->workplace);
    print_str_or_dash("Должность", c->position);
    print_str_list("Телефоны", c->phones, c->phone_count);
    print_str_list("Email", c->emails, c->email_count);
    print_str_list("Соцсети", c->socials, c->social_count);
    print_str_list("Мессенджеры", c->messengers, c->messenger_count);
}

void menu_print_contact_list(const PhoneBook* pb) {
    if (pb->count == 0) { puts("Список пуст."); return; }
    for (size_t i = 0; i < pb->count; i++) {
        Contact* c = pb_get_by_index(pb, i);
        if (!c) continue;
        printf("\nКОНТАКТ [%zu]\n", i + 1);
        menu_print_contact(c);
    }
}

#define TREE_PREFIX_MAX 1024

static void print_node_label(const PBNode* n) {
    const char* s  = n->contact.surname ? n->contact.surname : "—";
    const char* nm = n->contact.name ? n->contact.name : "";
    printf("%s %s [h=%d]", s, nm, n->height);
}

static void print_tree_rec(const PBNode* n,
                           const char* prefix, int is_last, int is_root)
{
    if (!n) return;

    if (is_root) {
        print_node_label(n);
        putchar('\n');
    } else {
        printf("%s%s", prefix, is_last ? "└── " : "├── ");
        print_node_label(n);
        putchar('\n');
    }

    char child_prefix[TREE_PREFIX_MAX];
    if (is_root) {
        child_prefix[0] = '\0';
    } else {
        int w = snprintf(child_prefix, sizeof(child_prefix),
                         "%s%s", prefix, is_last ? "    " : "│   ");
        if (w < 0 || (size_t)w >= sizeof(child_prefix))
            child_prefix[sizeof(child_prefix) - 1] = '\0';
    }

    int has_left  = n->left  != NULL;
    int has_right = n->right != NULL;

    if (has_left)  print_tree_rec(n->left,  child_prefix, !has_right, 0);
    if (has_right) print_tree_rec(n->right, child_prefix, 1,          0);
}

void menu_print_tree(const PhoneBook* pb) {
    puts("\nДЕРЕВО КОНТАКТОВ (AVL)\n");
    if (!pb->root) { puts("(дерево пусто)"); return; }
    print_tree_rec(pb->root, "", 1, 1);
}

// три обхода

static void print_short_contact(const Contact* c, size_t num) {
    printf("%3zu. %s %s",
           num,
           c->surname ? c->surname : "—",
           c->name    ? c->name    : "");
    if (c->patronymic) printf(" %s", c->patronymic);
    putchar('\n');
}

static void print_preorder_rec(const PBNode* n, size_t* counter) {
    if (!n) return;
    print_short_contact(&n->contact, ++(*counter));
    print_preorder_rec(n->left,  counter);
    print_preorder_rec(n->right, counter);
}

static void print_inorder_rec(const PBNode* n, size_t* counter) {
    if (!n) return;
    print_inorder_rec(n->left,  counter);
    print_short_contact(&n->contact, ++(*counter));
    print_inorder_rec(n->right, counter);
}

static void print_postorder_rec(const PBNode* n, size_t* counter) {
    if (!n) return;
    print_postorder_rec(n->left,  counter);
    print_postorder_rec(n->right, counter);
    print_short_contact(&n->contact, ++(*counter));
}

void menu_print_traversal(const PhoneBook* pb) {
    if (pb->count == 0) { puts("Список пуст."); return; }

    while (1) {
        puts("\nОБХОД ДЕРЕВА\n");
        puts("1. Прямой: корень → левое → правое");
        puts("2. Симметричный: левое → корень → правое");
        puts("3. Обратный: левое → правое → корень");
        puts("0. Назад");

        size_t choice;
        int rc = read_size_t("Действие: ", &choice);
        if (rc < 0) return;
        if (rc > 0) { puts("Неверный ввод."); continue; }

        if (choice == 0) return;

        size_t counter = 0;
        switch (choice) {
            case 1:
                puts("\nПрямой обход:");
                print_preorder_rec(pb->root, &counter);
                break;
            case 2:
                puts("\nСимметричный обход:");
                print_inorder_rec(pb->root, &counter);
                break;
            case 3:
                puts("\nОбратный обход:");
                print_postorder_rec(pb->root, &counter);
                break;
            default:
                puts("Нет такого пункта.");
                continue;
        }
    }
}

void menu_add_contact(PhoneBook* pb) {
    puts("\nДОБАВЛЕНИЕ КОНТАКТА\n");
    Contact c;
    contact_init(&c);

    int rc = 0;
    if (rc == 0) rc = fill_required(&c, contact_set_surname, "Фамилия: ");
    if (rc == 0) rc = fill_required(&c, contact_set_name, "Имя: ");
    if (rc == 0) rc = fill_optional(&c, contact_set_patronymic, "Отчество (Enter — пропустить): ");
    if (rc == 0) rc = fill_optional(&c, contact_set_workplace, "Место работы (Enter — пропустить): ");
    if (rc == 0) rc = fill_optional(&c, contact_set_position, "Должность (Enter — пропустить): ");
    if (rc == 0) rc = fill_list(&c, contact_add_phone, "телефон");
    if (rc == 0) rc = fill_list(&c, contact_add_email, "email");
    if (rc == 0) rc = fill_list(&c, contact_add_social, "ссылку на соцсеть");
    if (rc == 0) rc = fill_list(&c, contact_add_messenger, "мессенджер");

    if (rc != 0) {
        contact_free(&c);
        puts("Отменено или ошибка памяти.");
        return;
    }

    if (add_contact(pb, &c) != 0) {
        contact_free(&c);
        puts("Ошибка выделения памяти.");
        return;
    }

    contact_free(&c);
    puts("Контакт добавлен.");
}

void menu_remove_contact(PhoneBook* pb) {
    puts("\nУДАЛЕНИЕ КОНТАКТА\n");
    if (pb->count == 0) { puts("Список пуст."); return; }

    menu_print_contact_list(pb);

    size_t n;
    if (read_size_t("Номер контакта для удаления: ", &n) != 0) {
        puts("Некорректный ввод."); return;
    }
    if (n < 1 || n > pb->count) { puts("Нет такого номера."); return; }

    if (remove_contact(pb, n - 1) != 0) { puts("Ошибка удаления."); return; }
    printf("Контакт #%zu удалён.\n", n);
}

void menu_edit_contact(PhoneBook* pb) {
    puts("\nРЕДАКТИРОВАНИЕ КОНТАКТА\n");
    if (pb->count == 0) { puts("Список пуст."); return; }

    menu_print_contact_list(pb);

    size_t n;
    if (read_size_t("Номер контакта для редактирования: ", &n) != 0) {
        puts("Некорректный ввод."); return;
    }
    if (n < 1 || n > pb->count) { puts("Нет такого номера."); return; }

    Contact* c = pb_get_by_index(pb, n - 1);
    if (!c) { puts("Нет такого номера."); return; }

    while (1) {
        printf("\nРЕДАКТИРОВАНИЕ #%zu\n", n);
        menu_print_contact(c);
        puts("\n 1. Фамилия");
        puts(" 2. Имя");
        puts(" 3. Отчество");
        puts(" 4. Место работы");
        puts(" 5. Должность");
        puts(" 6. Телефоны");
        puts(" 7. Email");
        puts(" 8. Соцсети");
        puts(" 9. Мессенджеры");
        puts(" 0. Готово");

        size_t choice;
        int rc = read_size_t("Что редактировать: ", &choice);
        if (rc < 0) {
            /* EOF — пересобрать дерево и выйти */
            if (pb_rebuild(pb) != 0)
                puts("Не удалось перебалансировать дерево.");
            return;
        }
        if (rc > 0) { puts("Неверный ввод."); continue; }

        switch (choice) {
            case 0:
                if (pb_rebuild(pb) != 0)
                    puts("Не удалось перебалансировать дерево.");
                return;
            case 1: edit_scalar(c, contact_set_surname,    "Фамилия",       c->surname,    0); break;
            case 2: edit_scalar(c, contact_set_name,       "Имя",           c->name,       0); break;
            case 3: edit_scalar(c, contact_set_patronymic, "Отчество",      c->patronymic, 1); break;
            case 4: edit_scalar(c, contact_set_workplace,  "Место работы",  c->workplace,  1); break;
            case 5: edit_scalar(c, contact_set_position,   "Должность",     c->position,   1); break;
            case 6: edit_list(c, &c->phones,    &c->phone_count,
                              contact_add_phone,     contact_remove_phone,     "Телефоны");    break;
            case 7: edit_list(c, &c->emails,    &c->email_count,
                              contact_add_email,     contact_remove_email,     "Email");       break;
            case 8: edit_list(c, &c->socials,   &c->social_count,
                              contact_add_social,    contact_remove_social,    "Соцсети");     break;
            case 9: edit_list(c, &c->messengers, &c->messenger_count,
                              contact_add_messenger, contact_remove_messenger, "Мессенджеры"); break;
            default: puts("Нет такого пункта.");
        }
    }
}

void menu_run(PhoneBook* pb) {
    while (1) {
        printf("\nТЕЛЕФОННАЯ КНИГА\n\n");
        printf("1. Добавить контакт\n");
        printf("2. Удалить контакт\n");
        printf("3. Редактировать контакт\n");
        printf("4. Список контактов\n");
        printf("5. Дерево контактов\n");
        printf("6. Обходы дерева\n");
        printf("0. Выход\n");

        size_t choice;
        int rc = read_size_t("Ваш выбор: ", &choice);
        if (rc < 0) { puts("\nЗавершение работы."); return; }
        if (rc > 0) { puts("Некорректный ввод, попробуйте снова."); continue; }

        switch (choice) {
            case 0: return;
            case 1: system("clear"); menu_add_contact(pb); break;
            case 2: system("clear"); menu_remove_contact(pb); break;
            case 3: system("clear"); menu_edit_contact(pb); break;
            case 4: system("clear"); menu_print_contact_list(pb); getchar(); break;
            case 5: system("clear"); menu_print_tree(pb); getchar(); break;
            case 6: system("clear"); menu_print_traversal(pb); break;
            default: puts("Нет такого пункта.");
        }
        system("clear");
    }
}