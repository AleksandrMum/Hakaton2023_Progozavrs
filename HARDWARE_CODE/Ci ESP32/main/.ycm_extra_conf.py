import ycm_core
import os

FLAGS = [
        "-x", "c",
        "-isystem", os.path.abspath("./build/include")
        ]

# Return a list of all folders with name "include"
def ret_all_include_pathes(some_path):
    ret_list = []
    checkouts = os.listdir(some_path)
    for checkName in checkouts:
        testPath =  f"{some_path}/{checkName}"
        if os.path.isdir(testPath):
            if checkName == "include":
                ret_list.append(os.path.abspath(testPath))
            else:
                ret_list += ret_all_include_pathes(testPath)
    return ret_list

def Settings(**kwargs):
    global FLAGS
    includePathes = ret_all_include_pathes("/home/kapitan/Progects/AVRStuff/esp/esp-idf")


    for path in includePathes:
        FLAGS += ["-isystem", path]

        for name in os.listdir(path):
            dirName = f"{path}/{name}"
            if os.path.isdir(dirName):
                FLAGS += ["-isystem", dirName]


    return {
            "flags":FLAGS,
            "do_cache":True,
            }
