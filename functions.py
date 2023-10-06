class Telegram:
    def __init__(self):
        from telebot import types
        self.types = types

    def create_keyboard(self):
        markup = self.types.ReplyKeyboardMarkup(resize_keyboard=True, one_time_keyboard=True)
        buttons = self.types.KeyboardButton
        item1 = buttons("/Показания")
        item2 = buttons("/Информация")
        markup.add(item1, item2)
        return markup


greeting = (f"Здравствуйте! Спасибо за то, что воспользовались нашим Телеграм ботом погоды.\n\n"
            f"Чтобы получить текущие погодные данные с датчиков, нажмите кнопку 'Показания'. \n\n"
            f"Чтобы узнать больше о боте, нажмите кнопку 'Информация'")
information1 = (f"Этот бот был разработан командой 'Прогозавры' в рамках хакатона 'Cyber Garden 2023' \n\n"
                f"Команда состоит из трех человек: \n"
                f"1. Дутов Илья Валерьевич - командир команды, ответственен за железо, \n"
                f"2. Чидлеев Дмитрий Сергеевич - ответственный за сайт и серверную часть,\n"
                f"3. Мумладзе Александр Сергоевич - ответственный за этого бота.\n\n")
information2 = f"Чтобы получить данные с датчиков, нажмите кнопку 'Получить данные'"
