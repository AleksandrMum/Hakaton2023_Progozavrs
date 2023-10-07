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


class Requests:
    def __init__(self):
        import requests
        self.requests = requests
        self.address = "https://сайт"

    def get_list_sensors(self):
        json_sensors = self.requests.get(f"{self.address}/list_of_sensors").json()
        number_of_sensors = int(json_sensors["number_of_sensors"])
        list_of_sensors = json_sensors["sensors"]
        names_of_sensors_good = []
        names_of_sensors_bad = []
        for number in range(number_of_sensors):
            if list_of_sensors[number][1] == "ok":
                names_of_sensors_good.append(list_of_sensors[number][0])
            else:
                names_of_sensors_bad.append(list_of_sensors[number][0])
        str_names_of_sensors_good = '\n'.join(names_of_sensors_good)
        str_names_of_sensors_bad = '\n'.join(names_of_sensors_bad)
        message = (f"Список активных датчиков: \n"
                   f"{str_names_of_sensors_good} \n\n"
                   f"Список неактивных датчиков: \n"
                   f"{str_names_of_sensors_bad} \n\n"
                   f"Чтобы получить данные с активного датчика, введите '/Данные ИмяСенсора'")
        return message

    def get_sensor_data(self, name):
        json_data = self.requests.get(f"{self.address}/{name}").json()
        data = []
        for i in range(0, len(json_data.data)):
            data.append(f"{json_data.field[i]} - {json_data.data[i]}")
        message = "\n".join(data)
        return message


greeting = (f"Здравствуйте! Спасибо за то, что воспользовались нашим Телеграм ботом погоды.\n\n"
            f"Чтобы отобразить список датчиков, нажмите кнопку 'Список_датчиков'. \n\n"
            f"Чтобы узнать больше о боте, нажмите кнопку 'Информация'")
information1 = (f"Этот бот был разработан командой 'Прогозавры' в рамках хакатона 'Cyber Garden 2023' \n\n"
                f"Команда состоит из трех человек: \n"
                f"1. Дутов Илья Валерьевич - командир команды, ответственен за железо, \n"
                f"2. Чидлеев Дмитрий Сергеевич - ответственный за сайт и серверную часть,\n"
                f"3. Мумладзе Александр Сергоевич - ответственный за этого бота.\n\n")
main_info = f"Чтобы отобразить список датчиков, нажмите кнопку 'Список_датчиков'"
