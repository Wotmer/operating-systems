#include <iostream>
#include "number.h"
#include "vector.h"

int main() {
    std::cout << "=== Демонстрация работы системы библиотек ===" << std::endl;

    std::cout << "\n1. Демонстрация библиотеки Number (статическая):" << std::endl;
    std::cout << "=============================================" << std::endl;

    std::cout << "Глобальные переменные:" << std::endl;
    std::cout << "Number::ZERO = " << Number::ZERO.getValue() << std::endl;
    std::cout << "Number::ONE = " << Number::ONE.getValue() << std::endl;

    std::cout << "\nСоздание чисел через Number::Create():" << std::endl;
    const Number a = Number::Create(5.0);
    const Number b = Number::Create(3.0);
    const Number c = Number::Create(2.0);

    std::cout << "a = " << a.getValue() << std::endl;
    std::cout << "b = " << b.getValue() << std::endl;
    std::cout << "c = " << c.getValue() << std::endl;

    std::cout << "\nАрифметические операции:" << std::endl;
    std::cout << "a + b = " << (a + b).getValue() << std::endl;
    std::cout << "a - b = " << (a - b).getValue() << std::endl;
    std::cout << "a * b = " << (a * b).getValue() << std::endl;
    std::cout << "a / b = " << (a / b).getValue() << std::endl;

    std::cout << "\nПроверка обработки ошибок:" << std::endl;
    try {
        const Number result = a / Number::ZERO;
        std::cout << "a / ZERO = " << result.getValue() << std::endl;
    } catch (const std::runtime_error &e) {
        std::cout << "Ошибка при делении на ноль: " << e.what() << std::endl;
    }

    std::cout << "\n\n2. Демонстрация библиотеки Vector (динамическая):" << std::endl;
    std::cout << "===============================================" << std::endl;

    std::cout << "Глобальные векторы:" << std::endl;
    std::cout << "Vector::ZERO = (" << Vector::ZERO.getX().getValue()
            << ", " << Vector::ZERO.getY().getValue() << ")" << std::endl;
    std::cout << "Vector::ONE = (" << Vector::ONE.getX().getValue()
            << ", " << Vector::ONE.getY().getValue() << ")" << std::endl;

    const Vector v1(Number::Create(3.0), Number::Create(4.0)); // (3, 4)
    const Vector v2(Number::Create(1.0), Number::Create(2.0)); // (1, 2)
    const Vector v3(Number::Create(-2.0), Number::Create(2.0)); // (-2, 2)

    std::cout << "v1 = (" << v1.getX().getValue() << ", " << v1.getY().getValue() << ")" << std::endl;
    std::cout << "v2 = (" << v2.getX().getValue() << ", " << v2.getY().getValue() << ")" << std::endl;
    std::cout << "v3 = (" << v3.getX().getValue() << ", " << v3.getY().getValue() << ")" << std::endl;

    std::cout << "\nПолярные координаты (отдельные методы):" << std::endl;
    std::cout << "v1: rho = " << v1.getRho().getValue()
            << ", theta = " << v1.getTheta().getValue() << " радиан" << std::endl;
    std::cout << "v2: rho = " << v2.getRho().getValue()
            << ", theta = " << v2.getTheta().getValue() << " радиан" << std::endl;
    std::cout << "v3: rho = " << v3.getRho().getValue()
            << ", theta = " << v3.getTheta().getValue() << " радиан" << std::endl;

    std::cout << "\nПроверка корректности вычислений:" << std::endl;
    std::cout << "Длина вектора (3,4) = " << v1.getRho().getValue()
            << " (ожидается 5)" << std::endl;
    std::cout << "Угол вектора (1,2) = " << v2.getTheta().getValue()
            << " радиан" << std::endl;

    std::cout << "\nСложение векторов:" << std::endl;
    const Vector sum1 = v1 + v2;
    const Vector sum2 = v2 + v3;

    std::cout << "v1 + v2 = (" << sum1.getX().getValue()
            << ", " << sum1.getY().getValue() << ")" << std::endl;
    std::cout << "v2 + v3 = (" << sum2.getX().getValue()
            << ", " << sum2.getY().getValue() << ")" << std::endl;

    std::cout << "\nПроверка использования библиотеки Number:" << std::endl;
    std::cout << "Тип координат: " << typeid(v1.getX()).name() << std::endl;
    std::cout << "Тип полярных координат: " << typeid(v1.getRho()).name() << std::endl;

    std::cout << "\nКомплексный пример:" << std::endl;
    const Vector complex = (v1 + v2) + v3;
    std::cout << "(v1 + v2) + v3 = (" << complex.getX().getValue()
            << ", " << complex.getY().getValue() << ")" << std::endl;
    std::cout << "Его полярные координаты: rho = " << complex.getRho().getValue()
            << ", theta = " << complex.getTheta().getValue() << " радиан" << std::endl;

    std::cout << "\n=== Все функции работают как ожидается! ===" << std::endl;

    return 0;
}
