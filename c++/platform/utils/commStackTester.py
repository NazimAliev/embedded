import tkinter
import sys
import os
from datetime import datetime
import time
from tkinter.constants import COMMAND
from tkinter import *
import threading
import subprocess

import threading
import subprocess

# class MyClass(threading.Thread):
#     def __init__(self, command):
#         self.stdout = None
#         self.stderr = None
#         threading.Thread.__init__(self)
#         self.command = command

#     def run(self):
#         # p = subprocess.Popen(self.command.split(),
#         #                      shell=False,
#         #                      stdout=subprocess.PIPE,
#         #                      stderr=subprocess.PIPE)
#         p = subprocess.run(["adb", "shell", "gateway-client-shell", "-r", variableValue, "-s", "5"] ,stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
#         self.stdout = p.stdout #, self.stderr = p.communicate()


# myclass.join()
# print myclass.stdout

# from __future__ import print_function # Only Python 2.x
import subprocess
from subprocess import Popen, PIPE, CalledProcessError
def executecmd(cmd):
    print(cmd)
    with Popen(cmd, stdout=PIPE, bufsize=1, universal_newlines=True) as p:
        while AppRunning:
            for line in p.stdout:
                print(line, end='') # process line here
def execute(cmd):
    print(cmd)
    process = subprocess.Popen(cmd, stdout=subprocess.PIPE, universal_newlines=True)
    while AppRunning:
        nextline = process.stdout.readline()
        if nextline == '' and process.poll() is not None:
            break
        sys.stdout.write(nextline)
        sys.stdout.flush()
    print("")

mainTk     = None
simpleTk   = None
AppRunning = True

OPTIONS_CONTENT_TYPE = [
"ATP",
"SXR",
"SETUP",
"UNKNOWN"
]
OPTIONS = [
"COMMON",
"CBMC",
"REMOTEDOOR",
"ARTEMIS10",
"REGISTRATION",
"REMOTEDIAGNOSIS",
"REMOTESTART",
"RCS",
"WEEKDEPTSET",
"PRECOND",
"AUXHEAT",
"MPS",
"REMOTEVTA",
"REMOTETEMPSETTING",
"REMOTEVEHICLEFINDER",
"REMOTEWINDOW",
"REMOTEHU",
"REMOTECARSHARING",
"DATACOLLECTOR2",
"ONBOARDGEOFENCE",
"CHAROPTLOCATIONBASED",
"SPEEDFENCE",
"CHARCONF",
"RTMDL",
"APPLICATIONCONFIG",
"MECALL",
"MECALLTRIGGER",
"AVP",
"PNC",
"C3P0",
"CLOCKTIMER",
"REMOTEUPDATECONTROL",
"HVBATT",
"CROWDDATAIDC",
"CROWDDATALRR",
"MAP",
"OFFBOARDCALC",
"PARKAVP",
"PARKRPA",
"POS_BURST",
"TMPT",
"UNKNOWN",
"SOFTWAREDLL"
] #etc
OPTIONS.sort()
def mosqThread():
    command = "ssh -p 22 root@bb mosquitto -c /etc/mosquitto/mosquitto.conf.example"
    execute(command.split())

def mosqSubThread():
    command = "ssh -p 22 root@bb mosquitto_sub -h 127.0.0.1 -p 1883 -t 100/Inf/WDD2231631Z000043/Con/Sta --cafile /etc/gateway/test_cert/ca_cert.pem --cert /etc/gateway/client_cert.pem --key /etc/gateway/client_key.pem"
    execute(command.split())
    
def startingMosquitto():
    threading.Thread(target=mosqThread).start()
    threading.Thread(target=mosqSubThread).start()


def adaptEnvironemnt():
    command = "scp -P 22 ./comm/ca_cert.pem root@bb:/etc/gateway/"
    execute(command.split())
    command = "scp -P 22 ./comm/client_cert.pem root@bb:/etc/gateway/"
    execute(command.split())

    command = "scp -P 22 ./comm/client_key.pem root@bb:/etc/gateway/"
    execute(command.split())

    command = "scp -P 22 ./comm/GateWay_RootConfig.json root@bb:/etc/gateway"
    execute(command.split())

    command = "scp -P 22 ./comm/GateWay_Settings.json root@bb:/etc/gateway"
    execute(command.split())

    command = "scp -r -P 22 ./comm/test_cert root@bb:/etc/gateway/test_cert"
    execute(command.split())

    command = "scp -r -P 22 ./comm/mosquitto root@bb:/etc/mosquitto"
    execute(command.split())

    command = "scp -r -P 22 ./comm/original root@bb:/etc/original"
    execute(command.split())

    command = "scp -P 22 ./comm/mosquitto_dir/mosquitto root@bb:/bin"
    execute(command.split())

    command = "scp -P 22 ./comm/mosquitto_dir/mosquitto_passwd root@bb:/bin"
    execute(command.split())

    command = "scp -P 22 ./comm/mosquitto_dir/mosquitto_pub root@bb:/bin"
    execute(command.split())

    command = "scp -P 22 ./comm/mosquitto_dir/mosquitto_sub root@bb:/bin"
    execute(command.split())

    command = "scp -P 22 ./comm/mosquitto_lib/libmosquitto.so.1 ./comm/mosquitto_lib/libmosquittopp.so.1 root@bb:/lib/"
    execute(command.split())

    command = "ssh -p 22 root@bb chmod +x /lib/libmosquitto.so.1 /lib/libmosquittopp.so.1"
    result = os.popen(command).read()
    print(command)
    print(result)

    command = "ssh -p 22 root@bb chmod +x /bin/mosquitto /bin/mosquitto_passwd /bin/mosquitto_pub /bin/mosquitto_sub"
    result = os.popen(command).read()
    print(command)
    print(result)

    command = "scp -P 22 ./comm/gateway-client-shell root@bb:/bin"
    result = os.popen(command).read()
    print(command)
    print(result)

    command = "ssh -p 22 root@bb chmod +x /bin/gateway-client-shell"
    result = os.popen(command).read()
    print(command)
    print(result)

    command = "ssh -p 22 root@bb sync"
    result = os.popen(command).read()
    print(command)
    print(result)

    command = "ssh -p 22 root@bb adduser mosquitto -D Test"
    result = os.popen(command).read()
    print(command)
    print(result)

    command = "ssh -p 22 root@bb sync"
    result = os.popen(command).read()
    print(command)
    print(result)

def createCommandToExecute(filePath):
    if os.path.isfile(filePath):
        pass
        # print(dir_path)
    else:
        print("file does not exists will run with fixed message")
        return "ssh -p 22 root@bb 'echo -ne \'\\x81\x01\\x31\\x0A\\x00\\x00\\x80\\x02\\x00\\x00\\x00\\x0d\\x2c\\x0e\\x0d\\x60\\x6b\\x59\\xd5\\x59\\xe1\\x6c\\xc1\\xb9\\x00\\xaf\\x12\\x23\\x26\\x4c\\xd8\\xb6\\x66\\xc6\\xd3\\x06\\x0c\\x18\\x34\\x66\\x00\\x00\\x00\\x00\\x61\\x9a\\x33\\xa5\\x02\\x43\\x00\\x05\\x56\\x48\\x65\\x73\\x68\\x61\\x6d\\x20\\x73\\x61\\x66\\x77\\x66\\x77\' > /input'"
    f = open(filePath, "r")
    dataToSend = f.read()
    dataToSend = dataToSend.replace("0x","\\x")
    dataToSend = dataToSend.replace(" ","")
    dataToSend = dataToSend.replace("\n","")
    dataToSend = dataToSend.replace("\r","")
    print(dataToSend)
    str_echo = 'echo -ne ' + dataToSend + ' > /input'
    print(str_echo)
    command = "ssh -p 22 root@bb " + "\"" + str_echo + "\""
    print(command)
    return command

def publishMessage():
    global iidEntry
    global sidEntry
    global cidEntry
    global TranscIdEntry
    global bufferEntry
    global CONTENT_TYPE_VARIABLE
    contentType = CONTENT_TYPE_VARIABLE.get()
    iid = " 0"
    sid = " 0"
    cid = " 0"
    transId = " 0"
    buff = " 0x00"
    if iidEntry.get():
        iid = " " + iidEntry.get() 
    if sidEntry.get():
        sid = " " + sidEntry.get() 
    if cidEntry.get():
        cid = " " + cidEntry.get() 
    if TranscIdEntry.get():
        transId = " " + TranscIdEntry.get()
    if bufferEntry.get():
        buff = " " + bufferEntry.get() 

    command = "ssh -p 22 root@bb /bin/gateway-client-shell -p"
    command = command.split()
    command.append("\"" + contentType + iid + sid + cid + transId + buff + "\"")
    print(command)
    execute(command)

def simulateMessage():
    global publishFileEntry
    fileLocation = publishFileEntry.get()
    command = ""
    if fileLocation:
        print(fileLocation)
        command = createCommandToExecute(fileLocation)
    else:
        print("file is not provided will run with fixed message")

#CBMC
        command = "ssh -p 22 root@bb \"echo -ne \'\\x81\x01\\x31\\x0A\\x00\\x00\\x80\\x02\\x00\\x00\\x00\\x0d\\x2c\\x0e\\x0d\\x60\\x6b\\x59\\xd5\\x59\\xe1\\x6c\\xc1\\xb9\\x00\\xaf\\x12\\x23\\x26\\x4c\\xd8\\xb6\\x66\\xc6\\xd3\\x06\\x0c\\x18\\x34\\x66\\x00\\x00\\x00\\x00\\x61\\x9a\\x33\\xa5\\x02\\x43\\x00\\x05\\x56\\x48\\x65\\x73\\x68\\x61\\x6d\\x20\\x73\\x61\\x66\\x77\\x66\\x77\' > /input\""

#PARKAVP
        command = "ssh -p 22 root@bb \"echo -ne \'\\x81\x01\\x46\\x92\\x00\\x00\\x80\\x02\\x00\\x00\\x00\\x0d\\x2c\\x0e\\x0d\\x60\\x6b\\x59\\xd5\\x59\\xe1\\x6c\\xc1\\xb9\\x00\\xaf\\x12\\x23\\x26\\x4c\\xd8\\xb6\\x66\\xc6\\xd3\\x06\\x0c\\x18\\x34\\x66\\x00\\x00\\x00\\x00\\x61\\x9a\\x33\\xa5\\x02\\x43\\x00\\x05\\x56\\x48\\x65\\x73\\x68\\x61\\x6d\\x20\\x73\\x61\\x66\\x77\\x66\\x77\' > /input\""


    result = os.popen(command).read()
    print(command)
    print(result)
    
    command = "ssh -p 22 root@bb mosquitto_pub -h 127.0.0.1 -p 1883 -t 100/ToV/A/WDD2231631Z000043/C --cafile /etc/gateway/test_cert/ca_cert.pem --cert /etc/gateway/client_cert.pem --key /etc/gateway/client_key.pem -f /input"
    result = os.popen(command).read()
    print(command)
    print(result)
def donothing():
    pass
def subthread():
    # global simpleTk
    global variableValue
    global variableNumberofSub
    if not variableNumberofSub :
        variableNumberofSub = "5"
    command = "ssh -p 22 root@bb gateway-client-shell -r "+ variableValue +" -s "+ variableNumberofSub + " "
    print(variableNumberofSub)
    execute(command.split())
        
def subscribeSimple():
    global simpleTk
    global variableValue
    global variableNumberofSub
    global variable
    global numberOfSubscribtionEntry
    variableNumberofSub = numberOfSubscribtionEntry.get()
    variableValue = variable.get() 
    threading.Thread(target=subthread).start()
    
    
    # rr = popen(["ifconfig"], stdout=PIPE)
def registerationSimple():
    global simpleTk
    global variable
    print(variable.get())
    command = "ssh -p 22 root@bb gateway-client-shell -r "+ variable.get() +""
    result = os.popen(command).read()
    print(command)
    print(result)

def quit():
    global mainTk
    global simpleTk
    global AppRunning
    if mainTk != None:
        mainTk.destroy()
        mainTk = None
    if simpleTk != None:
        simpleTk.destroy()
        simpleTk = None
    AppRunning = False

def simpleUsage():
    global mainTk
    global simpleTk
    global variable
    global OPTIONS
    global OPTIONS_CONTENT_TYPE
    global numberOfSubscribtionEntry
    global publishFileEntry
    global iidEntry
    global sidEntry
    global cidEntry
    global TranscIdEntry
    global bufferEntry
    global CONTENT_TYPE_VARIABLE
    
    if mainTk != None:
        mainTk.destroy()
        mainTk = None
    simpleTk = tkinter.Tk()
    variable = StringVar(simpleTk)
    variable.set(OPTIONS[0]) # default value
    CONTENT_TYPE_VARIABLE = StringVar(simpleTk)
    CONTENT_TYPE_VARIABLE.set(OPTIONS_CONTENT_TYPE[0]) # default value
    quitButton = tkinter.Button(simpleTk, text='Quit', width=33, command=quit)
    mainMenuButton = tkinter.Button(simpleTk, text='back to main menu', width=33, command=bacKtoMainMenu)
    mosquittoSendMessageButton = tkinter.Button(simpleTk, text='send message from mosquitto', width=33, command=simulateMessage)
    registerationButton = tkinter.Button(simpleTk, text='register', width=33, command=registerationSimple)
    subscribtionButton = tkinter.Button(simpleTk, text='subscribe', width=33, command=subscribeSimple)
    publishButton = tkinter.Button(simpleTk, text='publish message using gateWay-client', width=33, command=publishMessage)
    publishTextFileLocation = tkinter.Label(simpleTk, text = "message file location")
    numberOfSubscribtionEntry = tkinter.Entry(simpleTk)
    publishFileEntry = tkinter.Entry(simpleTk)
    applicationTypeMenu = OptionMenu(simpleTk, variable, *OPTIONS)
    contentTypeMenu = OptionMenu(simpleTk, CONTENT_TYPE_VARIABLE, *OPTIONS_CONTENT_TYPE)
    
    buffer = tkinter.Label(simpleTk, text = "buffer", width=33)
    iid = tkinter.Label(simpleTk, text = "iid", width=33)
    sid = tkinter.Label(simpleTk, text = "sid", width=33)
    cid = tkinter.Label(simpleTk, text = "cid", width=33)
    TranscId = tkinter.Label(simpleTk, text = "TranscId", width=10)
    
    bufferEntry = tkinter.Entry(simpleTk)
    iidEntry = tkinter.Entry(simpleTk)
    sidEntry = tkinter.Entry(simpleTk)
    cidEntry = tkinter.Entry(simpleTk)
    TranscIdEntry = tkinter.Entry(simpleTk)

    quitButton.grid(row=0, column=2)
    mainMenuButton.grid(row=0, column=0)
    publishTextFileLocation.grid(row=1, column=1)
    mosquittoSendMessageButton.grid(row=2, column=0)
    publishFileEntry.grid(row=2, column=1)
    applicationTypeMenu.grid(row=3, column=0)
    registerationButton.grid(row=3, column=1)
    subscribtionButton.grid(row=4, column=1)
    numberOfSubscribtionEntry.grid(row=4, column=0)
    contentTypeMenu.grid(row=5, column=0)
    publishButton.grid(row=5, column=1)
    buffer.grid(row=5, column=2)
    bufferEntry.grid(row=5, column=3)
    iid.grid(row=6, column=0)
    sid.grid(row=6, column=1)
    cid.grid(row=6, column=2)
    TranscId.grid(row=6, column=3)
    iidEntry.grid(row=7, column=0)
    sidEntry.grid(row=7, column=1)
    cidEntry.grid(row=7, column=2)
    TranscIdEntry.grid(row=7, column=3)
    # publishFixedMessageButton.grid(row=2, column=0)

def bacKtoMainMenu():
    global simpleTk
    if simpleTk != None:
        simpleTk.destroy()
        simpleTk = None
    mainTkinterCreator()

def mainTkinterCreator():
    global mainTk
    mainTk = tkinter.Tk()
    mainTk.title('gateWay tester')
    quitButton = tkinter.Button(mainTk, text='Quit', width=33, command=quit)
    simpleUsageButton = tkinter.Button(mainTk, text='simple tests', width=33, command=simpleUsage)
    adaptEnvironmentButton = tkinter.Button(mainTk, text='adapt the environemnt', width=33, command=adaptEnvironemnt)
    mosquittoStartButton = tkinter.Button(mainTk, text='start mosquitto', width=33, command=startingMosquitto)
    quitButton.pack()
    adaptEnvironmentButton.pack()
    mosquittoStartButton.pack()
    simpleUsageButton.pack()



def main():
    mainTkinterCreator()
    '''
    widgets are added here
    '''
    mainTk.mainloop()


main()
