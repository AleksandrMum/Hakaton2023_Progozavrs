class Server:
    def __init__(self):
        import socket
        import json
        import os
        import time
        self.socket = socket
        self.json = json
        self.os = os
        self.time = time
        self.HOST = "127.0.0.1"
        self.PORT = 65432

    def start_server(self):
        with self.socket.socket(self.socket.AF_INET, self.socket.SOCK_STREAM) as s:
            s.bind((self.HOST, self.PORT))
            s.listen(5)
            conn, addr = s.accept()
            while True:
                data = conn.recv(1024)
                if not data:
                    break
                message = data.decode()
                print(f"{addr} отправил пакет данных - {message}")
                self.data_processing(addr, message)

    def data_processing(self, addr, message):
        file_name = f"{addr}.json"
        file_directory = f"data\\sensors\\{file_name}.json"
        if file_name not in self.os.listdir('\\metric'):
            temp = open(file_directory)
            temp.close()
        file = self.json.load(open(file_directory, "a+"))
        data = {message}
        metric = {self.time.time(): data}
        file.update(metric)
        file.dump(file_directory)
