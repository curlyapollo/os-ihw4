# Карлинский Леонид Вадимович, БПИ216
## Вариант №32
## Условие задачи:
Вторая задача об Острове Сокровищ. Шайка пиратов под
предводительством Джона Сильвера высадилась на берег Острова
Сокровищ. Не смотря на добытую карту старого Флинта, местоположение сокровищ по-прежнему остается загадкой, поэтому искать
клад приходится практически на ощупь. Так как Сильвер ходит на
деревянной ноге, то самому бродить по джунглям ему не с руки.
Джон Сильвер поделил остров на участки, а пиратов на небольшие группы. Каждой группе поручается искать клад на нескольких
участках, а сам Сильвер ждет на берегу. Группа пиратов, обшарив
один участок, переходит на следующий, еще необследованный. Закончив поиски, пираты возвращаются к Сильверу и докладывают
о результатах. Если какая–то из групп находит клад на одном из
своих участков, она возвращается к Сильверу, который шлет попугая, инициализирующего прекращение (прерывание) всех работ.
Требуется создать приложение, моделирующее действия Сильвера и пиратов.Сервер — Сильвер, Каждая из групп пиратов — клиент.

## Работа выполнена на 8 баллов
Везде даны комментарии на английском, поясняющие работу программы.
Для обеспечения корректного взаимодействия сетевых приложений
и существующих в них процессов используются мьютексы.

Даны 2 папки, в каждой лежит решение на оценку, на которую называется папка. Теперь схемы:
## на 5:
Программа pirates.c является клиентской программой, которая подключается к серверу эхо с использованием TCP и отправляет/получает данные. Программа принимает два аргумента из командной строки - IP-адрес сервера и номер порта, на котором сервер слушает.

Функция checkArguments() проверяет, была ли программа вызвана с правильным количеством аргументов. Если нет, она выводит сообщение об ошибке и завершает программу.

Функция main() создает сокет с помощью socket(), настраивает структуру адреса сервера с помощью memset() и inet_addr(), и устанавливает соединение с сервером с помощью connect().

Затем программа входит в бесконечный цикл, где она получает данные от сервера с помощью recv(), обрабатывает их и отправляет ответ обратно на сервер с помощью send(). Цикл продолжается, пока сервер не отправит определенное сообщение (-1 в данном случае), чтобы указать, что поиск закончен.

Наконец, программа закрывает клиентский сокет и завершается.
