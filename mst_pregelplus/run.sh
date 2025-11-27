#!/bin/bash

# Проверяем, что передано минимум два аргумента
if [ $# -lt 2 ]; then
    echo "Использование: $0 <путь_к_файлу> <количество_запусков> [reqresp]"
    exit 1
fi

INPUT_FILE="$1"
RUNS="$2"
REQRESP="$3"  # Третий опциональный аргумент
FULL_PATH="$(realpath "$0")"
PP_DIR="$(dirname "$FULL_PATH")"

# Выбираем версию boruvka в зависимости от наличия reqresp
if [ -n "$REQRESP" ] && [ "$REQRESP" = "reqresp" ]; then
    BORUVKA_EXEC="$PP_DIR/boruvka_reqresp.out"
    echo "Используется версия с reqresp"
else
    BORUVKA_EXEC="$PP_DIR/boruvka.out"
    echo "Используется стандартная версия"
fi

# Проверяем существование выбранного исполняемого файла
if [ ! -f "$BORUVKA_EXEC" ]; then
    echo "Ошибка: Исполняемый файл '$BORUVKA_EXEC' не найден" >&2
    exit 1
fi

# Получаем имя файла без пути для использования в имени директории
INPUT_FILENAME=$(basename "$INPUT_FILE")

# Проверяем, что файл существует
if [ ! -f "$INPUT_FILE" ]; then
    echo "Ошибка: Файл '$INPUT_FILE' не существует или недоступен" >&2
    exit 1
fi

# Проверяем, что количество запусков - целое число больше 0
if ! [[ "$RUNS" =~ ^[1-9][0-9]*$ ]]; then
    echo "Ошибка: Количество запусков должно быть целым числом больше 0" >&2
    exit 1
fi

# Устанавливаем N_JOBS: либо из окружения, либо значение по умолчанию 1
: "${N_JOBS:=1}"

# Проверяем существует ли директория /input и очищаем
echo "Очистка директории /input в HDFS..."
if hadoop fs -test -d /input 2>/dev/null; then
    hadoop fs -rm -r -f /input/*
    echo "Директория /input очищена"
else
    echo "Директория /input не существует, будет создана"
fi

# Создаем директорию
hadoop fs -mkdir -p /input

# Загружаем файл
echo "Загрузка $INPUT_FILE в HDFS /input/"
hadoop fs -put "$INPUT_FILE" /input/

if [ $? -eq 0 ]; then
    echo "Файл успешно загружен в HDFS"
else
    echo "Ошибка при загрузке в HDFS" >&2
    exit 1
fi

# Создаем директорию для логов с именем файла
LOG_DIR="logs_${INPUT_FILENAME}"
mkdir -p "$LOG_DIR"
echo "Логи будут сохранены в директорию: $LOG_DIR"

# Запускаем программу указанное количество раз
echo "Запуск $BORUVKA_EXEC $RUNS раз(а) с $N_JOBS процессами"

for ((i=1; i<=RUNS; i++)); do
    echo "=== Запуск $i/$RUNS ==="
    
    # Формируем имя файла для вывода: номер_запуска_потоков.log
    OUTPUT_FILE="${LOG_DIR}/run${i}_${N_JOBS}proc.log"
    
    # Запускаем и перенаправляем вывод в файл
    mpiexec -n "$N_JOBS" "$BORUVKA_EXEC" > "$OUTPUT_FILE" 2>&1
    
    # Проверяем успешность выполнения
    if [ $? -eq 0 ]; then
        echo "Запуск $i завершен успешно, вывод сохранен в $OUTPUT_FILE"
    else
        echo "Ошибка в запуске $i, проверьте файл $OUTPUT_FILE" >&2
        exit 1
    fi
done

echo "Все $RUNS запусков завершены успешно"