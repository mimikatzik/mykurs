#include <iostream>
#include <cstring>

int main() {
    char name[50];
    char greeting[100];
    char buffer[100];

    std::cout << "Введите ваше имя: ";
    // небезопасная функция gets
    fgets(name, sizeof(name), stdin);

    std::cout << "Введите приветствие: ";
    // небезопасная функция strcpy
    strncpy(greeting, "Hello, world!", sizeof(greeting) - 1); greeting[sizeof(greeting) - 1] = '\0';

    int age = 25;
    // небезопасная функция sprintf
    snprintf(buffer, sizeof(buffer), "%s, вам %d лет.", greeting, age);

    std::cout << buffer << std::endl;

    return 0;
}