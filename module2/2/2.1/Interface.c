#include "Interface.h"
#include "Strings.h"
#include <stdio.h>
#include <stdlib.h>

// Читает строку до \n. Возвращает malloc-строку без \n, NULL при EOF
// При использовании обязательно очищать память через free().
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
            buf = tmp;
            cap = nc;
        }
        buf[len++] = (char)ch;
    }

    if (ch == EOF && len == 0) { free(buf); return NULL; }
    buf[len] = '\0';
    return buf;
}

// Выводит prompt (если не NULL), читает строку через read_line() и возвращает её
static char* read_line_prompt(const char* prompt) {
    if (prompt) { fputs(prompt, stdout); fflush(stdout); }
    return read_line();
}

// Читает size_t с консоли. Возвращает 0 при успехе, 1 при ошибке ввода, -1 при EOF
static int read_size_t(const char* prompt, // строка для вывода пользователю
                        size_t* out) // куда записать результат
{
    char* line = read_line_prompt(prompt);
    if (!line) return -1;
    int rc = sscanf(line, "%zu", out);
    free(line);
    return (rc == 1) ? 0 : 1;
}

// Читает да/нет с консоли. Возвращает 1 при 'д', 0 при 'н', -1 при EOF
static int read_yes_no(const char* prompt) // строка для вывода пользователю
{
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

//  Редактирование полей, представленных в виде строки (surname, name, patronymic, workplace, position)
static void edit_scalar(Contact* c, // контакт, который редактируем
                        int (*setter)(Contact*, const char*), // функция, которая устанавливает значение поля
                        const char* label, // метка для вывода пользователю
                        const char* current, // текущее значение поля
                        int allow_clear) // разрешает ли очистку поля (только для необязательных полей) 
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
        if (!allow_clear) {
            puts("Обязательное поле нельзя очистить.");
            return;
        }
        setter(c, "");
        printf("%s очищено.\n", label);
        return;
    }

    setter(c, line);
    free(line);
    printf("%s обновлено.\n", label);
}

// Редактирование полей, представленных в виде списка (phones, emails, socials, messengers)
static void edit_list(Contact* c, // контакт, который редактируем
                      char*** arr, size_t* count, // указатели на массив и его размер
                      int (*adder)(Contact*, const char*), // функция, которая добавляет элемент в массив
                      int (*remover)(Contact*, size_t), // функция, которая удаляет элемент из массива по индексу
                      const char* label) // метка для вывода пользователю
{
    while (1) {
        printf("\n%s (%zu)\n", label, *count);
        if (*count == 0) {
            puts("  (пусто)");
        } else {
            for (size_t i = 0; i < *count; i++)
                printf("  %zu. %s\n", i + 1, (*arr)[i]);
        }
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

// Заполняет обязательное поле. -1 при EOF/OOM.
static int fill_required(Contact* c, // контакт, который заполняем
                         int (*setter)(Contact*, const char*), // функция, которая устанавливает значение поля
                         const char* prompt) // строка для вывода пользователю
{
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

// Необязательное: пустой ввод = очистить. -1 — EOF/OOM.
static int fill_optional(Contact* c, // контакт, который заполняем
                         int (*setter)(Contact*, const char*), // функция, которая устанавливает значение поля
                         const char* prompt)  // строка для вывода пользователю
{
    char* line = read_line_prompt(prompt);
    if (!line) return -1;
    int rc = setter(c, line);
    free(line);
    return rc;
}

// Заполняет список (телефоны, email, соцсети, мессенджеры). -1 при EOF/OOM.
static int fill_list(Contact* c, // контакт, который заполняем
                     int (*adder)(Contact*, const char*), // функция, которая добавляет элемент в массив
                     const char* item_name) // имя элемента для вывода пользователю
{
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

// Печатает строку с меткой. Если value == NULL, печатает "—"
static void print_str_or_dash(const char* label, // метка для вывода пользователю
                              const char* value) // строка для вывода пользователю
{
    printf("%s: %s\n", label, value ? value : "—");
}

// Печатает список строк с меткой. Если count == 0, печатает "—"
static void print_str_list(const char* label, // метка для вывода пользователю
                           char* const* items, // массив строк для вывода
                           size_t count) // количество строк в массиве
{
    printf("%s:\n", label);
    if (count == 0) { puts("  —"); return; }
    for (size_t i = 0; i < count; i++)
        printf("  - %s\n", items[i]);
}

// Меню печати контакта
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

// Меню печати списка контактов
void menu_print_contact_list(const PhoneBook* pb) {
    if (pb->count == 0) { puts("Список пуст."); return; }
    for (size_t i = 0; i < pb->count; i++) {
        printf("\nКОНТАКТ [%zu]\n", i + 1);
        menu_print_contact(&pb->items[i]);
    }
}

// Меню добавления контакта
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

// Меню удаления контакта
void menu_remove_contact(PhoneBook* pb) {
    puts("\nУДАЛЕНИЕ КОНТАКТА\n");
    if (pb->count == 0) { puts("Список пуст."); return; }

    menu_print_contact_list(pb);

    size_t n;
    if (read_size_t("Номер контакта для удаления: ", &n) != 0) {
        puts("Некорректный ввод."); return;
    }
    if (n < 1 || n > pb->count) {
        puts("Нет такого номера."); return;
    }
    if (remove_contact(pb, n - 1) != 0) {
        puts("Ошибка удаления."); return;
    }
    printf("Контакт #%zu удалён.\n", n);
}

// Меню редактирования контакта
void menu_edit_contact(PhoneBook* pb) {
    puts("\nРЕДАКТИРОВАНИЕ КОНТАКТА\n");
    if (pb->count == 0) { puts("Список пуст."); return; }

    menu_print_contact_list(pb);

    size_t n;
    if (read_size_t("Номер контакта для редактирования: ", &n) != 0) {
        puts("Некорректный ввод."); return;
    }
    if (n < 1 || n > pb->count) { puts("Нет такого номера."); return; }

    Contact* c = &pb->items[n - 1];

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
        if (rc < 0) return;
        if (rc > 0) { puts("Неверный ввод."); continue; }

        switch (choice) {
            case 0: return;
            case 1: edit_scalar(c, contact_set_surname, "Фамилия", c->surname, 0); break;  // нельзя очистить
            case 2: edit_scalar(c, contact_set_name, "Имя", c->name, 0); break;  // нельзя очистить
            case 3: edit_scalar(c, contact_set_patronymic, "Отчество", c->patronymic, 1); break;
            case 4: edit_scalar(c, contact_set_workplace, "Место работы", c->workplace, 1); break;
            case 5: edit_scalar(c, contact_set_position, "Должность", c->position, 1); break;
            case 6: edit_list(c, &c->phones, &c->phone_count,
                              contact_add_phone, contact_remove_phone, "Телефоны"); break;
            case 7: edit_list(c, &c->emails, &c->email_count,
                              contact_add_email, contact_remove_email, "Email"); break;
            case 8: edit_list(c, &c->socials, &c->social_count,
                              contact_add_social, contact_remove_social, "Соцсети"); break;
            case 9: edit_list(c, &c->messengers, &c->messenger_count,
                              contact_add_messenger, contact_remove_messenger, "Мессенджеры"); break;
            default: puts("Нет такого пункта.");
        }
    }
}

// Главное меню
void menu_run(PhoneBook* pb) {
    while (1) {
        printf("\nТЕЛЕФОННАЯ КНИГА\n\n");
        printf("1. Добавить контакт\n");
        printf("2. Удалить контакт\n");
        printf("3. Редактировать контакт\n");
        printf("4. Список контактов\n");
        printf("0. Выход\n");

        size_t choice;
        int rc = read_size_t("Ваш выбор: ", &choice);
        if (rc < 0) { puts("\nЗавершение работы."); return; }   // EOF/Ctrl+D
        if (rc > 0) { puts("Некорректный ввод, попробуйте снова."); continue; }

        switch (choice) {
            case 0: return;
            case 1: system("clear"); menu_add_contact(pb); break;
            case 2: system("clear"); menu_remove_contact(pb); break;
            case 3: system("clear"); menu_edit_contact(pb); break;
            case 4: system("clear"); menu_print_contact_list(pb); getchar(); break;
            default: puts("Нет такого пункта.");
        }
        system("clear");
    }
}