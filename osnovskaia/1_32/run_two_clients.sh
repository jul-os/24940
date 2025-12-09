#!/bin/bash
SERVER="./server"
CLIENT="./client"

SOCKET_PATH="/tmp/uppercase_socket"

# Функция завершения убить все процессы и удалить сокет
cleanup() {
    echo "Завершаем работу..."
    kill $SERVER_PID 2>/dev/null
    kill $CLIENT1_PID 2>/dev/null
    kill $CLIENT2_PID 2>/dev/null
    wait $SERVER_PID $CLIENT1_PID $CLIENT2_PID 2>/dev/null
    rm -f "$SOCKET_PATH"
    exit 0
}

trap cleanup SIGINT SIGTERM

# Запускаем сервер 
echo "Запуск сервера..."
$SERVER &
SERVER_PID=$!

# Ждём пока сокет появится 
for i in {1..20}; do
    if [ -S "$SOCKET_PATH" ]; then
        break
    fi
    sleep 0.1
done

if [ ! -S "$SOCKET_PATH" ]; then
    echo "Ошибка: сервер не создал сокет за разумное время"
    kill $SERVER_PID 2>/dev/null
    exit 1
fi

echo "Сервер запущен, запускаем клиентов..."

# Клиент 1 отправляет каждые 0.5 сек
(
    while true; do
        echo "Client one"
        sleep 0.5
    done
) | $CLIENT &
CLIENT1_PID=$!

# Клиент 2 отправляет каждые 0.7 сек
(
    while true; do
        echo "Client two"
        sleep 0.7
    done
) | $CLIENT &
CLIENT2_PID=$!

echo "Клиенты запущены. Нажмите Ctrl+C для остановки."

wait