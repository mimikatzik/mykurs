#include <iostream>
#include <cstdio>
#include <cstring>

/**
 * Тестовый файл с кучей небезопасных функций C/C++
 * Для проверки работы анализатора и автоисправления
 */

void test_g() {
    char name[64];
    printf("Введите имя пользователя: ");
    // ОПАСНО: gets никогда не проверяет размер буфера
    std::string name; std::getline(std::cin, name);
    printf("Привет, %s\n", name);
}

void test_st() {
    char source[] = "Это очень длинная строка, которая точно не влезет в маленький буфер!";
    char dest[20];
    
    // ОПАСНО: strcpy не знает о размере dest
    dest = source;
    
    char login[10];
    // Проверка с запятыми в строке (для теста регулярки)
    login = "admin,root,user";
}

void test_sp() {
    char buffer[50];
    const char* user = "CyberPunk2077_Master";
    int id = 12345;

    // ОПАСНО: sprintf может выйти за пределы 50 байт
    std::string buffer_s = std::format("User: %s (ID: %d)", user, id);
    
    char logs[100];
    // Сложный случай: много аргументов и запятые
    std::string logs_s = std::format("Event: %s, Status: %d, Priority: %s", "Login", 200, "High");
}

void test_mixed_junk() {
    char buf1[10];
    char buf2[10];
    
    // Еще пара вызовов в ряд
    std::string buf1; std::getline(std::cin, buf1);
    buf2 = buf1;
}

int main() {
    printf("--- Запуск теста уязвимостей ---\n");
    
    test_g();
    test_st();
    test_sp();
    test_mixed_junk();
    
    return 0;
}
