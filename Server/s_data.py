import pickle
import os
import datetime


def get_time():
    time_now = datetime.datetime.now()
    time_format = "%d/%m/%Y %H:%M:%S"
    time = time_now.strftime(time_format)
    return time


def add_sensor_data(sensor_name, sensor_data):
    data_file_name = f"{sensor_name}.pickle"
    if data_file_name not in os.listdir("..\\data\\sensors"):
        with open(f"..\\data\\sensors\\{data_file_name}", 'wb') as f:
            data = {}
            pickle.dump(data, f)
            f.close()

    with open(f"..\\data\\sensors\\{data_file_name}", 'rb+') as f:
        data = pickle.load(f)

        pickle.dump(data, f)
        f.close()
