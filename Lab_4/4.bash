if ! command -v g++ &> /dev/null; then
    echo "ошибка: g++ не установлен."
    exit 1
fi

echo "компиляция bye"
g++ -std=c++11 -pthread bye.cpp -o killer

if [ $? -ne 0 ]; then
    echo "ошибка компиляции bye"
    exit 1
fi

echo "компиляция abc"
g++ -std=c++11 -pthread abc.cpp -o user

if [ $? -ne 0 ]; then
    echo "ошибка компиляции abc"
    exit 1
fi

echo "готово"
./user