// Эти строки отключают проверки безопасности в Visual Studio (MSVC)
#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NONSTDC_NO_DEPRECATE

#include <iostream>
#include <cstdio>
#include <cstring>

// В новых стандартах C++11 и выше gets() удален совсем.
// Если компилятор ругается, что "gets is not declared", 
// используем старый добрый сишный прототип.


void test_g() {
    char name[64];
    printf("Введите имя пользователя: ");
    // Анализатор должен найти это:
    std::getline(std::cin, name);
    printf("Привет, %s\n", name);
}

void test_st() {
    char source[] = "Это очень длинная строка, которая точно не влезет!";
    char dest[20];
    
    // Анализатор должен предложить: dest = source;
    dest = source;
    
    char login[10];
    // Проверка на запятые (сложный случай для парсера)
    login = "admin,root,user";
}

void test_sp() {
    char buffer[50];
    const char* user = "CyberPunk2077";
    int id = 12345;

    // Анализатор должен предложить: std::string buffer_s = std::format(...);
    buffer = fmt::format("User: %s (ID: %d)", user, id);
}

int main() {
    printf("--- Запуск теста (если видишь это, значит скомпилилось) ---\n");
    
    // ВНИМАНИЕ: Если ты реально запустишь это, вводи короткое имя,
    // иначе программа вылетит с ошибкой "Stack Smashing Detected"
    test_g();
    test_st();
    test_sp();
    
    printf("Тест завершен!\n");
    return 0;
}