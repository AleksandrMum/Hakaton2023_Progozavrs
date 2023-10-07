import telebot
import functions
# import server

Telegram = functions.Telegram()
Data = functions.Data()
# ServerToSensors = server.ServerToSensors()

bot = telebot.TeleBot('6359375478:AAFF2M0TBQd-6Yj4TjnJS1GxT-3nh7eXczE')
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
    bot.send_message(message.chat.id, send_message)


# @bot.message_handler(content_types=["Данные"])
# def list_of_data(message):
#     input_text = message.text.replace("/Данные ", "")
#     data = Requests.get_sensor_data(input_text)
#     bot.send_message(message.chat.id, data)


@bot.message_handler(content_types=["text"])
def another_text(message):  # Название функции не играет никакой роли
    bot.send_message(message.chat.id, "Ошибка, повторите ввод")


print("Бот заработал")
bot.polling()
