class Telegram:
    def __init__(self):
        from telebot import types
        self.types = types

    def create_keyboard_greeting(self):
        markup = self.types.ReplyKeyboardMarkup(resize_keyboard=True, one_time_keyboard=True)
        buttons = self.types.KeyboardButton
        item1 = buttons("/Список_датчиков")
        item2 = buttons("/Информация")
        markup.add(item1, item2)
        return markup


class Data:
    def __init__(self):
        import json
        import os
        self.json = json
        self.os = os

    def list_of_sensor(self):
        list_of_directories = self.os.listdir("data\\sensors")
        list_of_sensors = []
        for item in list_of_directories:
            list_of_sensors.append(item.replace(".json", ""))
        sensors = '\n'.join(list_of_sensors)
        file = open("data\\sensors.txt", "w")
        file.write(sensors)
        file.close()
        if len(list_of_sensors) < 15:
            return f"Список датчиков: \n{sensors}"
        else:
            sensors = '\n'.join(list_of_sensors[:15])
            return f"Список датчиков (только первые 15 вхождений): \n{sensors}"

    # def get_sensor_data(self, name):
    #     json_data = self.requests.get(f"{self.address}/{name}").json()
    #     data = []
    #     for i in range(0, len(json_data.data)):
    #         data.append(f"{json_data.field[i]} - {json_data.data[i]}")
    #     message = "\n".join(data)
    #     return message


greeting = (f"Здравствуйте! Спасибо за то, что воспользовались нашим Телеграм ботом погоды.\n\n"
            f"Чтобы отобразить список датчиков, нажмите кнопку 'Список_датчиков'. \n\n"
            f"Чтобы узнать больше о боте, нажмите кнопку 'Информация'")
information1 = (f"Этот бот был разработан командой 'Прогозавры' в рамках хакатона 'Cyber Garden 2023' \n\n"
                f"Команда состоит из трех человек: \n"
                f"1. Дутов Илья Валерьевич - командир команды, ответственен за железо, \n"
                f"2. Чидлеев Дмитрий Сергеевич - ответственный за сайт и серверную часть,\n"
                f"3. Мумладзе Александр Сергоевич - ответственный за этого бота.\n\n")
main_info = f"Чтобы отобразить список датчиков, нажмите кнопку 'Список_датчиков'"
