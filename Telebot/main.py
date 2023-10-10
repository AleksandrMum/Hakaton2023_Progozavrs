import telebot
import functions
# import Server.s_flask as func_flask
# import Server.s_socket as func_socket
# import asyncio

Telegram = functions.Telegram()
Data = functions.Data()
# socket = func_socket.ServerToSensors()
# Loop = asyncio.get_event_loop()                         # Необходим для параллельной работы Flask, Sockets и Telebot

bot = telebot.TeleBot(open("..\\data\\TOKEN.txt").read())
keyboard_greeting = Telegram.create_keyboard_greeting()


@bot.message_handler(commands=["start"])                # Первый запуск бота командой старт
def greeting(message):
    bot.send_message(message.chat.id, functions.greeting, reply_markup=keyboard_greeting)


@bot.message_handler(commands=["Информация"])           # Краткая инфа о членах команды
def information(message):
    bot.send_message(message.chat.id, functions.information1)
    bot.send_message(message.chat.id, functions.main_info, reply_markup=keyboard_greeting)


@bot.message_handler(commands=["Назад"])                # Вернуться в меню вызова списка сенсоров
def back_to_main(message):
    bot.send_message(message.chat.id, functions.main_info)


@bot.message_handler(commands=["Список_датчиков"])      # Получить первые 15 вхождений датчиков и список.txt
def list_of_sensors(message):
    # send_message = Data.list_of_sensor()
    # file = open("..\\data\\sensors.txt", "rb").read()
    information = f"\n\nЧтобы получить актуальные данные по сенсору, введите '/Данные ИМЯ_СЕНСОРА' (без расширения)"
    send_message = ''.join(open("..\\data\\sensors.txt", 'r').readlines()) + information
    bot.send_message(message.chat.id, send_message)
    # bot.send_document(message.chat.id, file)


@bot.message_handler(commands=["Данные"])
def list_of_data(message):
    input_text = message.text[8:]
    data = open(f"..\\data\\sensors\\{input_text}.txt", 'r+').readline()
    bot.send_message(message.chat.id, data)


@bot.message_handler(content_types=["text"])            # Реакция на непредусмотренную команду/текст
def another_text(message):
    bot.send_message(message.chat.id, "Ошибка, повторите ввод")


bot.polling()

# Loop.run_until_complete(func_flask.start_app())         # Запускает сервера как параллельные задачи (у функций async)
# Loop.run_until_complete(bot.polling())
# Loop.run_until_complete(socket.start_server())
# Loop.run_forever()                                      # Поддерживает задачи всегда активными
