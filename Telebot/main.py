import telebot
import functions
import Server.s_flask as Flask
import Server.s_socket as Socket
import asyncio

Telegram = functions.Telegram()
Data = functions.Data()
socket = Socket.ServerToSensors()
Loop = asyncio.get_event_loop()

bot = telebot.TeleBot(open("data\\TOKEN.txt").read())
keyboard_greeting = Telegram.create_keyboard_greeting()


@bot.message_handler(commands=["start"])
def greeting(message):
    bot.send_message(message.chat.id, functions.greeting, reply_markup=keyboard_greeting)


@bot.message_handler(commands=["Информация"])
def information(message):
    bot.send_message(message.chat.id, functions.information1)
    bot.send_message(message.chat.id, functions.main_info)


@bot.message_handler(commands=["Назад"])
def back_to_main(message):
    bot.send_message(message.chat.id, functions.main_info)


@bot.message_handler(commands=["Список_датчиков"])
def list_of_sensors(message):
    send_message = Data.list_of_sensor()
    file = open("data\\sensors.txt", "rb").read()
    bot.send_message(message.chat.id, send_message)
    bot.send_document(message.chat.id, file)


# @bot.message_handler(content_types=["Данные"])
# def list_of_data(message):
#     input_text = message.text.replace("/Данные ", "")
#     data = Requests.get_sensor_data(input_text)
#     bot.send_message(message.chat.id, data)


@bot.message_handler(content_types=["text"])
def another_text(message):  # Название функции не играет никакой роли
    bot.send_message(message.chat.id, "Ошибка, повторите ввод")


Loop.run_until_complete(Flask.start_app())
Loop.run_until_complete(bot.polling())
Loop.run_until_complete(socket.start_server())
Loop.run_forever()
