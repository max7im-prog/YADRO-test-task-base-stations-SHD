# Система учета компьютерного клуба

В проекте используется google test как подмодуль

## Сборка и запуск

1. Клонировать с подмодулями:
```bash
git clone --recurse-submodules https://github.com/max7im-prog/YADRO-test-task-base-stations-SHD
```

2. Собрать проект:
```bash
cmake -B build
cmake --build build
```
3.Запуск программы
```bash
build/YADRO-test-task входной_файл.txt
```

4.Тесты
```bash
cmake --build build --target tests_runner
build/tests_runner
```

5. Пример входного файла
```bash
3               # Количество столов
09:00 19:00     # Часы работы
10              # Цена за час
09:00 1 client1 # События...
```