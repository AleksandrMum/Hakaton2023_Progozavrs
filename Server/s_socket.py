from threading import Thread
from queue import Queue
from Server.s_data import add_sensor_data
import socket
import datetime
import os


class Server(Thread):
    def __init__(self, data_que:Queue):
        super().__init__()
        self.daemon = True
        self.name = "Socket server"
        self.net_conn = ("192.168.43.140", 65432)
        self.data_que = data_que

    def run(self):
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as listen_sock:
            listen_sock.bind(self.net_conn)
            listen_sock.listen(1)
            print(f"[INFO] Server listening at {self.net_conn}")
            while True:
                raw_data = []
                try:
                    conn, addr = listen_sock.accept()
                    data_chunk = conn.recv(256)
                    while data_chunk:
                        raw_data.append(data_chunk)
                        data_chunk = conn.recv(1024)

                except socket.error:
                    print(f"Connection {addr} broken")

                self.data_que.put((datetime.datetime.now(), raw_data))


class Parser(Thread):
    def __init__(self, data_que:Queue, save_dir):
        super().__init__()

        self.name = "Parser"
        self.daemon = True
        self.data_que = data_que
        self.save_dir = save_dir
        self.sensors = []
        objcts = os.listdir(self.save_dir)

        for object in objcts:
            if os.path.isfile(object):
                print(f"Find file {object}")
                sen_name = object.split(".")[0]
                print(f"Add sensor {sen_name}")
                self.sensors.append({sen_name})

    def run(self):
        while True:
            date_time, raw_data = self.data_que.get()
            str_data = []
            for bts in raw_data:
                str_data.append(bts.decode("ascii"))

            str_data = " ".join(str_data)
            str_data = str_data.split("%")
            for item in str_data:
                temp = item.split(" ")
                name, data = temp[0], temp[1:]
                name = name.strip()
                print(name, data)
                if name == "":
                    continue

                if name in self.sensors:
                    method = "a"
                else:
                    method = "w"
                    print(f"Create file {name}.txt")
                    self.sensors.append(name)
                with open(f"{self.save_dir}/{name}.txt", method) as file:
                    write =" ".join(data)
                    write = write.strip()
                    file.write(f"{date_time}:{write}\n")
                print(f"Write {name} {date_time} {write}")


if __name__ == "__main__":
    data_queue = Queue()
    server = Server(data_queue)
    parser = Parser(data_queue, "..\\data\\sensors")
    server.start()
    parser.start()
    input("Input something to exit \n")





