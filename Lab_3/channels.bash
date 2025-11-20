CXX=g++
CXXFLAGS="-std=c++17 -O3 -pthread -Wall"

OUTPUT="matrix_channel"
SRC="matrix_thread.cpp"
HEADER="buffered_channel.h"

if [ ! -f "$SRC" ]; then
    echo "Ошибка: Файл $SRC не найден."
    exit 1
fi

if [ ! -f "$HEADER" ]; then
    echo "Ошибка: Файл $HEADER не найден."
    exit 1
fi

echo "Компиляция проекта..."

$CXX $CXXFLAGS $SRC -o $OUTPUT

if [ $? -eq 0 ]; then
    echo "Сборка прошла успешно!"
    echo "Запуск программы..."
    echo "-----------------------------------------"
    ./$OUTPUT
else
    echo "Ошибка при компиляции."
    exit 1
fi