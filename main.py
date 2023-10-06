import telebot
import functions
Telegram = functions.Telegram()
bot = telebot.TeleBot('6359375478:AAFF2M0TBQd-6Yj4TjnJS1GxT-3nh7eXczE')
bot_keyboard = Telegram.create_keyboard()


@bot.message_handler(commands=["start"])
def greeting(message):
    bot.send_message(message.chat.id, functions.greeting, reply_markup=bot_keyboard)


@bot.message_handler(commands=["Показания"])
def send_data(message):
    bot.send_message(message.chat.id, "Данных нет но вы держитесь")


@bot.message_handler(commands=["Информация"])
def send_data(message):
    bot.send_message(message.chat.id, functions.information1)
    bot.send_message(message.chat.id, functions.information2)


@bot.message_handler(content_types=["text"])
def repeat_all_messages(message):  # Название функции не играет никакой роли
    bot.send_message(message.chat.id, "Зачем ты сюда что-то пишешь?")


print("Бот заработал")
bot.polling()
