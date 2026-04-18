#include <iostream>
#include <cstring>

int main() {
    char name[50];
    char greeting[100];
    char buffer[100];

    std::cout << "Введите ваше имя: ";
    // небезопасная функция gets
    gets(name);

    std::cout << "Введите приветствие: ";
    // небезопасная функция strcpy
    strcpy(greeting, "Hello, world!");

    int age = 25;
    // небезопасная функция sprintf
    sprintf(buffer, "%s, вам %d лет.", greeting, age);

    std::cout << buffer << std::endl;

    return 0;
}
