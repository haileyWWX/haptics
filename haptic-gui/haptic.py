#import os
#import pickle
#from pathlib import Path

import tkinter
from tkinter import *
#from tkinter import ttk
#import tkinter.filedialog as filedialog

#import bleak
import serial
#import struct
#import time
from PIL import ImageTk, Image
import threading

#import asyncio
#from bleak import BleakClient
#from bleak import BleakScanner
#from bleak import discover
import serial.tools.list_ports
#from winrt import _winrt

#import platform
#import ast
#import logging

#Lets tkinter.filedialog.askdirectory work (No Clue Why)
#_winrt.uninit_apartment()

root = Tk()
root.geometry("860x585")

# Serial Constants 
# Servo 
CLOCKWISE = 0x31 
COUNTERCLOCKWISE = 0x32
QUARTER = 0xC8
SET = 0xC9
RESET = 0xCA
SQUEEZE = 0xCB 
LOOSEN = 0xCC
RELEASE = 0xCD 
END = 0xCE

# VTB 
START_CMD = 0xF1
END_CMD = 0xF2
START_BLOCK = 0xF3 
END_BLOCK = 0xF4
PREDEFINED = 0xF5 
REALTIME = 0xF6
PAUSE = 0xF8

serial_object = None
vibration_block_list = []
current_connections_list = []
pause_flag = 0
real_time_flag = 0
    
quarter_flag = 0
half_flag = 0
set_base_flag = 0
reset_flag = 0
squeeze_flag = 0
loosen_flag = 0
release_flag = 0
end_flag = 0

########################################################################################################################
# SERVO MODES
########################################################################################################################
""" Select direction of servo"""
def servo_type_select():
    global cw_flag, ccw_flag 
    if servo_input_type.get() == "CW":
        servo_frame.tkraise()
        cw_flag = 1
        ccw_flag = 0
        print("CW")
  
    if servo_input_type.get() == "CCW":
        servo_frame.tkraise()
        ccw_flag = 1
        cw_flag = 0
        print("CCW")

# select servo turn magnitudes 
def quarter_turn():
    global quarter_flag, cw_flag, ccw_flag
    quarter_flag = 1  

    data1 = bytearray()   
    data2 = bytearray()

    if cw_flag == 1:
        data1.append(CLOCKWISE)
        ser.write(data1)
        print("cw")
        data2.append(QUARTER)
        print(data1)
        print(data2)
        ser.write(data2)
        print("Tighten")
        
    elif ccw_flag == 1: 
        data1.append(COUNTERCLOCKWISE)
        ser.write(data1)
        print("ccw")
        data2.append(QUARTER)
        print(data1)
        print(data2)
        ser.write(data2)
        print("Tighten")

    
def set_base():
    global set_base_flag
    set_base_flag = 1
    data = bytearray()
    data.append(SET)
    print(data)
    ser.write(data)
    print("Set")


def squeeze_band():
    global squeeze_flag
    squeeze_flag = 1
    data = bytearray()
    data.append(SQUEEZE) # do squeeze 
    print(data)
    ser.write(data)
    print("Squeeze~")


def loosen_band():
    global loosen_flag
    loosen_flag = 1
    data = bytearray()
    data.append(LOOSEN) # do squeeze 
    print(data) 
    ser.write(data)
    print("Loosen~ ")


def release_squeeze():
    global release_flag 
    release_flag = 1
    data = bytearray()
    data.append(RELEASE) # release band
    print(data)
    ser.write(data)
    print("Release")


def end_squeeze():
    global end_flag 
    end_flag = 1
    data = bytearray()
    data.append(END) # end band
    print(data)
    ser.write(data)
    print("END")


########################################################################################################################
# VTF BLOCK MODES
########################################################################################################################
"""Sets the pause flag"""
def addPause():
    global pause_flag
    pause_flag = 1
    addVibrationBlock()


"""Raises the programed or real"""
def typeSelect():
    global real_time_flag
    if input_type.get() == "Programed":
        control_frame.tkraise()
        real_time_flag = 0

    elif input_type.get() == "Real":
        real_time_frame.tkraise()
        real_time_flag = 1
        

"""Predefined Mode"""
def programedType():
    if programed_type.get() == "predef":
        predef_list.configure(state=NORMAL)


"""Reset Real Time Mode Intensity"""
def resetRealTimeScale():
    motor0_scale.set(0)
    motor1_scale.set(0)
    motor2_scale.set(0)
    motor3_scale.set(0)
    motor4_scale.set(0)
    motor5_scale.set(0)
    motor6_scale.set(0)
    motor7_scale.set(0)


"""Clear the vibration boxes on the screen"""
def clearVibrationBlock():
    vibration_block_list.clear()
    drawBlocks()


"""Will add a given vibration block command to the vib_dict"""
def addVibrationBlock():
    global pause_flag

    # Directory of all the commands to be displayed / executed
    vib_dict = {'Motors': [], 'Type': [], 'PredefName': [], 'PredefNum': [],'Pause': [], 'labelnum': []}

    if pause_flag == 1:
        pause_flag = 0
        vib_dict['Type'].append('Pause')
        vib_dict['Pause'].append(pause_time.get())
        pause_time.set(0)
        pause_time_enter.delete(0, 'end')

    else:
        for i in motor_select.curselection():
            vib_dict['Motors'].append(motor_select.get(i))

        vib_dict['Type'].append(programed_type.get())

        if programed_type.get() == 'predef':
            vib_dict['PredefName'].append(predef_list.get(predef_list.curselection()))
            vib_dict['PredefNum'].append(predef_list.curselection()[0])


    vibration_block_list.append(vib_dict)

    drawBlocks()


"""Enables a block to be deleted by clicking on it twice"""
def deleteBlock(info):
    for widget in block_frame.winfo_children():
        if widget is info.widget:
            i = len(vibration_block_list)-1
            while i >= 0:
                if vibration_block_list[i]['labelnum'] == widget.winfo_id():
                    vibration_block_list.pop(i)
                i = i - 1

    drawBlocks()


"""Will draw all the current commands on the screen """
def drawBlocks():
    x_pos = 5
    y_pos = 5
    y_pos_max = 5
    i = 0

    for widget in block_frame.winfo_children():
        if isinstance(widget, tkinter.Label):
            widget.destroy()

    while i < len(vibration_block_list):

        if vibration_block_list[i]['Type'] == ['Pause']:
            block_info = "Pause for " + vibration_block_list[i]['Pause'][0] + " Second/s"

        elif vibration_block_list[i]['Type'][0] == 'created':
            block_info = "Saved Block: "
            block_info += vibration_block_list[i]['SavedName'][0]

        else:
            # Get Motors
            block_info = " "
            for j in range(len(vibration_block_list[i]['Motors'])):
                block_info += vibration_block_list[i]['Motors'][j] + ' '

            block_info += '\n'

            if vibration_block_list[i]['Type'] == ['predef']:
                block_info += "Vibration Type: "
                block_info += vibration_block_list[i]['PredefName'][0]

        block_label = Label(block_frame, text=block_info, borderwidth=2, relief="solid", wraplength=200)

        block_label.place(x=x_pos, y=y_pos)
        block_label.bind("<Double-Button-1>", deleteBlock)
        vibration_block_list[i]['labelnum'] = block_label.winfo_id()
        block_frame.update()

        x_pos = x_pos + block_label.winfo_width() + 5

        if (block_label.winfo_height() > y_pos_max):
            y_pos_max = block_label.winfo_y()

        if x_pos > 600:
            block_label.destroy()
            x_pos = 5
            y_pos = y_pos_max + 50
            i = i - 1

        i = i + 1


"""Generates the binary information for the motors selected"""
def getMotorBinarySecond(motors):
    binary = 0
    for i in range(len(motors)):
        if motors[i] == "Motor 0":
            binary += (1 << 0)

        elif motors[i] == "Motor 1":
            binary += (1 << 1)

        elif motors[i] == "Motor 2":
            binary += (1 << 2)

        elif motors[i] == "Motor 3":
            binary += (1 << 3)

        elif motors[i] == "Motor 4":
            binary += (1 << 4)

        elif motors[i] == "Motor 5":
            binary += (1 << 5)

        elif motors[i] == "Motor 6":
            binary += (1 << 6)

        elif motors[i] == "Motor 7":
            binary += (1 << 7)

    return binary


"""When the send data button is pushed this takes the data in the vib_dict
    Packages it appropriately and send is via serial com"""
def sendBlocks():
  
    data = bytearray()

    data.append(START_BLOCK)
    data.append(START_BLOCK)
    for i in range(len(vibration_block_list)):
        # Data Type
        vibe_type = vibration_block_list[i]['Type']

        data.append(START_CMD)
        data.append(START_CMD)

        if vibe_type == ['predef']:
            data.append(PREDEFINED)
            data.append((vibration_block_list[i]['PredefNum'][0]))
            data.append((getMotorBinarySecond(vibration_block_list[i]['Motors'])))

        elif vibe_type == ['Pause']:
            data.append(PAUSE)
            pause_float = float(vibration_block_list[i]['Pause'][0])
            pause_int_ms = int(round(pause_float * 1000))
            pause_bytes = pause_int_ms.to_bytes(4, byteorder='little')
            data.append(pause_bytes[0])
            data.append(pause_bytes[1])
            data.append(pause_bytes[2])
            data.append(pause_bytes[3])

        data.append(END_CMD)
        data.append(END_CMD)


    data.append(END_BLOCK)
    data.append(END_BLOCK)

    for byte in data:
        print(byte)

    ser.write(data)

    data.clear()


"""Sends the realtime data for a given motor each time a slider is changed"""
def sendRealTime(motor_num, percentage):
    data = bytearray()

    data.append(START_BLOCK)
    data.append(START_BLOCK)
    data.append(START_CMD)
    data.append(START_CMD)

    data.append(REALTIME)

    motor_val = motor_num.to_bytes(1, byteorder='big')

    data.append(motor_val[0])

    value = round(((int(percentage) / 200) * 255))

    if value == 1:
        value = 0

    intensity = value.to_bytes(1, byteorder='big')
    # print(test[0])

    data.append(intensity[0])
    data.append(END_CMD)
    data.append(END_CMD)
    data.append(END_BLOCK)
    data.append(END_BLOCK)
    
    for byte in data:
        print(byte)
        
    ser.write(data)
    data.clear()

########################################################################################################################
# Serial Setup
########################################################################################################################
serial_monitor_on = 0

"""Allows the serial monitor to scroll no matter where is is"""
def _on_mousewheel(event):
    global dataCanvas
    dataCanvas.yview_scroll(int(-1*(event.delta/120)), "units")


"""Creates a new frame that displays incoming serial data"""
def startMonitor():
    global serial_monitor_on
    global dataFrame, dataCanvas
    serial_monitor_on = 1
    monitor = tkinter.Toplevel()
    monitor.wm_title("Serial Monitor")
    monitor_frame = Frame(monitor)
    monitor_frame.pack(fill=BOTH, expand=YES)
    dataCanvas = Canvas(monitor_frame, width=850, height=400, bg="white", highlightthickness=0)
    dataCanvas.pack(fill=BOTH, expand=YES, side=LEFT)

    vsb = Scrollbar(monitor_frame, orient='vertical', command=dataCanvas.yview)
    vsb.pack(fill=BOTH, expand=NO, side=RIGHT)

    dataCanvas.config(yscrollcommand=vsb.set)
    dataCanvas.bind("<Enter>", lambda _: dataCanvas.bind_all('<MouseWheel>', _on_mousewheel))
    dataCanvas.bind("<Leave>", lambda _: dataCanvas.unbind_all('<MouseWheel>'))

    dataFrame = Frame(dataCanvas, bg='white')
    dataCanvas.create_window((10,0), window=dataFrame, anchor='nw')


"""Updates the serial monitor with incoming data"""
def updateDataMonitor(data):
    global dataFrame, dataCanvas
    Label(dataFrame, text=data, font=('Calibri', '13'), bg='white', justify='left').pack(anchor=NW)
    dataCanvas.config(scrollregion=dataCanvas.bbox('all'))



"""Reads any serial data sent and prints it"""
def readData():
    global serial_object, serial_monitor_on
    while 1:
        try:
            serial_data = ser.readline()
            message = serial_data.decode('utf8')

            print(message)

            if serial_monitor_on == 1:
                updateDataMonitor(message.rstrip('\n'))

        except:
            status_bar_update_ser(0, ser.name)
            ser.close()


"""Start serial communications at a given port baud is set at 9600"""
def startSerial(com):
    global ser
    ser = serial.Serial(port=com, baudrate=9600, timeout=10)
    if ser:
        status_bar_update_ser(1, com)
        threading.Thread(target=readData, daemon=True).start()


"""Updates a list of current available ports"""
def refreshDevices():
    ports = serial.tools.list_ports.comports()

    for com in ports:
        usb_menu.add_command(label=com[0], command=lambda: startSerial(com[0]))


"""Updates when a serial devices is connected/disconnected"""
def status_bar_update_ser(connected, com):
    if connected:
        menubar.add_command(label=com)
    else:
        menubar.delete(com)
    menubar.update()

########################################################################################################################
# GUI FRAMES SETUP
########################################################################################################################
# Setup Frames
band_image_frame = LabelFrame(root, text="Armband", width=250, height=156)
servo_frame = LabelFrame(root, text="Servo Setup", width=250, height=161)
block_frame = LabelFrame(root, text="Vibration Blocks", width=605, height=313)
type_frame = LabelFrame(root, text="Type", width=50, height=287)
control_frame = LabelFrame(root, text="Programmed", width=890, height=287)
real_time_frame = LabelFrame(root, text="Real Time", width=890, height=287)

# Place Frames
band_image_frame.grid(row=0, column=0, sticky="nw", padx=5)
band_image_frame.grid_propagate(False)
servo_frame.grid(row=0, column=0, sticky="nw", padx=5, pady = 150)
servo_frame.grid_propagate(False)
servo_frame.grid(row=0, column=0, sticky="nw", padx=5)
servo_frame.grid_propagate(False)
block_frame.grid(row=0, column=0, sticky="nw", padx=250)
block_frame.grid_propagate(False)
type_frame.grid(row=0, column=0, sticky="nw", padx=5, pady=313)
control_frame.grid(row=0, column=0, sticky="nw", padx=110, pady=313)
real_time_frame.grid(row=0, column=0, sticky="nw", padx=110, pady=313)

control_frame.tkraise()

#band_image = ImageTk.PhotoImage(Image.open("D:\\Thesis-code\\haptics\\haptics-gui\\armband.jpg"))
#band_image_label = Label(band_image_frame, image=band_image)
#band_image_label.pack()

########################################################################################################################
# MENU
########################################################################################################################
menubar = Menu(root)
file_menu = Menu(menubar, tearoff=0)
file_menu.add_command(label="Serial Monitor", command=startMonitor)

connect_menu = Menu(menubar, tearoff=0)
connect_menu.add_command(label="Refresh Devices", command=refreshDevices)
connect_menu.add_separator()
usb_menu = Menu(menubar, tearoff=0)

connect_menu.add_cascade(label="USB", menu=usb_menu)

menubar.add_cascade(label="File", menu=file_menu)
menubar.add_cascade(label="Connect", menu=connect_menu)
menubar.add_command(label="                  ")
menubar.add_command(label="Connected:")

root.config(menu=menubar)

########################################################################################################################
# SERVO GUI 
########################################################################################################################
# Define Radio Buttons
servo_input_type = StringVar()
cw_radio_button = Radiobutton(servo_frame, text="Clockwise", variable=servo_input_type, value='CW',
                                     command=servo_type_select)
ccw_time_radio_button = Radiobutton(servo_frame, text="Counter Clockwise", variable=servo_input_type, value='CCW',
                                     command=servo_type_select)

# Place Radio buttons
cw_radio_button.grid(row=0, column=0, padx=2, pady=2, stick="w")
ccw_time_radio_button.grid(row=0, column=1, padx=2, pady=2, stick="w")

#######################################################################################################################
# Servo Frame
########################################################################################################################
servo_quarter_button = Button(servo_frame, text="Tighten", command=quarter_turn)
servo_quarter_button.place(x=10, y=30)

servo_set_button = Button(servo_frame, text="Set Base Band", command=set_base)
servo_set_button.place(x=100, y=30)

servo_squeeze_button = Button(servo_frame, text="Squeeze!", command=squeeze_band)
servo_squeeze_button.place(x=10, y=70)

servo_loose_button = Button(servo_frame, text="Loosen!", command=loosen_band)
servo_loose_button.place(x=100, y=70)

servo_release_button = Button(servo_frame, text="Release", command=release_squeeze)
servo_release_button.place(x=10, y=110)

servo_end_button = Button(servo_frame, text="End", command=end_squeeze)
servo_end_button.place(x=100, y=110)



########################################################################################################################
# VTF Frame
########################################################################################################################
# BLOCK FRAME #
send_data_button = Button(block_frame, text="Send Blocks", command=sendBlocks)
send_data_button.place(x=310, y=260)

clear_data_button = Button(block_frame, text="Clear Blocks", command=clearVibrationBlock)
clear_data_button.place(x=430, y=260)

########################################################################################################################
# Control Frame
########################################################################################################################
# RADIO BUTTONS #
# Define Radio Buttons
input_type = StringVar()
programed_radio_button = Radiobutton(type_frame, text="Programed", variable=input_type, value='Programed',
                                     command=typeSelect)
real_time_radio_button = Radiobutton(type_frame, text="Real Time", variable=input_type, value='Real',
                                     command=typeSelect)

# Place Radio buttons
programed_radio_button.grid(row=0, column=0, padx=2, pady=2, stick="w")
real_time_radio_button.grid(row=1, column=0, padx=2, pady=2, stick="w")

# Motor select
choices = ["Motor 0", "Motor 1", "Motor 2", "Motor 3", "Motor 4", "Motor 5", "Motor 6", "Motor 7"]
choicesvar = StringVar(value=choices)
motor_select = Listbox(control_frame, height=13, listvariable=choicesvar, selectmode="multiple", exportselection=False)
motor_select.grid(row=0, rowspan=13, column=1, stick="n", padx=2, pady=2)


# Define Type Buttons
programed_type = StringVar()
predefined_radio_button = Radiobutton(control_frame, text="Pre Defined", variable=programed_type, value='predef',
                                      command=programedType)
programed_type.set('predef')

predefined_radio_button.grid(row=0, column=2, padx=2, pady=2, stick="w")



########################################################################################################################
# PREDEFINED Frame
########################################################################################################################
predef_choices = ["Strong Click - 100%", "Strong Click - 60%", "Strong Click - 30%", "Sharp Click - 100%",
                  "Sharp Click - 60% ", "Sharp Click - 30%", "Soft Bump - 100%", "Soft Bump - 60%", "Soft Bump - 30%",
                  "Double Click - 100%", "Double Click - 60%", "Triple Click - 100%", "Soft Fuzz - 60%",
                  "Strong Buzz - 100%", "750 ms Alert 100%", "1000 ms Alert 100%", "Strong Click 1 - 100% ",
                  "Strong Click 2 - 80%", "Strong Click 3 - 60%", "Strong Click 4 - 30%", "Medium Click 1 - 100%",
                  "Medium Click 2 - 80%", "Medium Click 3 - 60%", "Sharp Tick 1 - 100%", "Sharp Tick 2 - 80%",
                  "Sharp Tick 3 – 60%", "Short Double Click Strong 1 – 100%", "Short Double Click Strong 2 – 80%",
                  "Short Double Click Strong 3 – 60%", "Short Double Click Strong 4 – 30%",
                  "Short Double Click Medium 1 – 100%", "Short Double Click Medium 2 – 80%",
                  "Short Double Click Medium 3 – 60%", "Short Double Sharp Tick 1 – 100%",
                  "Short Double Sharp Tick 2 – 80%", "Short Double Sharp Tick 3 – 60%",
                  "Long Double Sharp Click Strong - 100%", "Long Double Sharp Click Strong 2 - 80%",
                  "Long Double Sharp Click Strong 3 – 60%", "Long Double Sharp Click Strong 4 – 30%",
                  "Long Double Sharp Click Medium 1 – 100%", "Short Double Click Medium 2 – 80%",
                  "Long Double Sharp Click Medium 3 – 60%", "Long Double Sharp Tick 1 – 100%",
                  "Long Double Sharp Tick 2 – 80%", "Long Double Sharp Tick 3 – 60%", "Buzz 1 – 100%",
                  "Buzz 2 – 80%", "Buzz 3 – 60%", "Buzz 4 – 40%", "Buzz 5 – 20%", "Pulsing Strong 1 – 100%",
                  "Pulsing Strong 2 – 60%", "Pulsing Medium 1 – 100%", "Pulsing Medium 2 – 60%",
                  "Pulsing Sharp 1 – 100%", "Pulsing Sharp 2 – 60%", "Transition Click 1 – 100%",
                  "Transition Click 2 – 80%", "Transition Click 3 – 60%", "Transition Click 4 – 40%",
                  "Transition Click 5 – 20%", "Transition Click 6 – 10%", "Transition Hum 1 – 100%",
                  "Transition Hum 2 – 80%", "Transition Hum 3 – 60% ", "Transition Hum 4 – 40%",
                  "Transition Hum 5 – 20%", "Transition Hum 6 – 10%", "Transition Ramp Down Long Smooth 1 – 100 to 0%",
                  "Transition Ramp Down Long Smooth 2 – 100 to 0%", "Transition Ramp Down Medium Smooth 1 – 100 to 0%",
                  "Transition Ramp Down Medium Smooth 2 – 100 to 0%", "Transition Ramp Down Short Smooth 1 – 100 to 0%",
                  "Transition Ramp Down Short Smooth 2 – 100 to 0%", "Transition Ramp Down Long Sharp 1 – 100 to 0%",
                  "Transition Ramp Down Long Sharp 2 – 100 to 0% ", "Transition Ramp Down Medium Sharp 1 – 100 to 0% ",
                  "Transition Ramp Down Medium Sharp 2 – 100 to 0%", "Transition Ramp Down Short Sharp 1 – 100 to 0%",
                  "Transition Ramp Down Short Sharp 2 – 100 to 0%", "Transition Ramp Up Long Smooth 1 – 0 to 100%",
                  "Transition Ramp Up Long Smooth 2 – 0 to 100%", "Transition Ramp Up Medium Smooth 1 – 0 to 100%",
                  "Transition Ramp Up Medium Smooth 2 – 0 to 100%", "Transition Ramp Up Short Smooth 1 – 0 to 100%",
                  "Transition Ramp Up Short Smooth 2 – 0 to 100%", "Transition Ramp Up Long Sharp 1 – 0 to 100%",
                  "Transition Ramp Up Long Sharp 2 – 0 to 100%", "Transition Ramp Up Medium Sharp 1 – 0 to 100%",
                  "Transition Ramp Up Medium Sharp 2 – 0 to 100%", "Transition Ramp Up Short Sharp 1 – 0 to 100%",
                  "Transition Ramp Up Short Sharp 2 – 0 to 100%", "Transition Ramp Down Long Smooth 1 – 50 to 0%",
                  "Transition Ramp Down Long Smooth 2 – 50 to 0%", "Transition Ramp Down Medium Smooth 1 – 50 to 0%",
                  "Transition Ramp Down Medium Smooth 2 – 50 to 0%", "Transition Ramp Down Short Smooth 1 – 50 to 0%",
                  "Transition Ramp Down Short Smooth 2 – 50 to 0%", "Transition Ramp Down Long Sharp 1 – 50 to 0%",
                  "Transition Ramp Down Long Sharp 2 – 50 to 0%", "Transition Ramp Down Medium Sharp 1 – 50 to 0%",
                  "Transition Ramp Down Medium Sharp 2 – 50 to 0%", "Transition Ramp Down Short Sharp 1 – 50 to 0%",
                  "Transition Ramp Down Short Sharp 2 – 50 to 0%", "Transition Ramp Up Long Smooth 1 – 0 to 50%",
                  "Transition Ramp Up Long Smooth 2 – 0 to 50%", "Transition Ramp Up Medium Smooth 1 – 0 to 50%",
                  "Transition Ramp Up Medium Smooth 2 – 0 to 50%", "Transition Ramp Up Short Smooth 1 – 0 to 50%",
                  "Transition Ramp Up Short Smooth 2 – 0 to 50%", "Transition Ramp Up Long Sharp 1 – 0 to 50%",
                  "Transition Ramp Up Long Sharp 2 – 0 to 50%", "Transition Ramp Up Medium Sharp 1 – 0 to 50%",
                  "Transition Ramp Up Medium Sharp 2 – 0 to 50%", "Transition Ramp Up Short Sharp 1 – 0 to 50%",
                  "Transition Ramp Up Short Sharp 2 – 0 to 50%", "Long buzz for programmatic stopping – 100%",
                  "Smooth Hum 1 (No kick or brake pulse) – 50%", "Smooth Hum 2 (No kick or brake pulse) – 40%",
                  "Smooth Hum 3 (No kick or brake pulse) – 30%", "Smooth Hum 4 (No kick or brake pulse) – 20%",
                  "Smooth Hum 5 (No kick or brake pulse) – 10%"]

predef_choicesvar = StringVar(value=predef_choices)
predef_list = Listbox(control_frame, height=15, width=52, listvariable=predef_choicesvar, exportselection=False)
predef_list.grid(row=0, rowspan=15, column=3, stick="n", padx=2, pady=2)

# scroll bar
predef_scrollbar = Scrollbar(control_frame, orient="vertical", command=predef_list.yview)
predef_scrollbar.grid(row=0, rowspan=15, column=4, stick="nsw")
predef_list.config(yscrollcommand=predef_scrollbar.set)

# Add Created
add_created_button = Button(control_frame, text="Add Block", command=addVibrationBlock)
add_created_button.grid(row=1, column=7, padx=2)

# pause button 
pause_time = StringVar()
pause_time_enter = Entry(control_frame, textvariable=pause_time, width=10)
pause_time_add_button = Button(control_frame, text="Add Pause", command=addPause)

pause_time_enter.grid(row=2, column=7, padx=2)
pause_time_add_button.grid(row=2, column=8, padx=25)

########################################################################################################################
# Real Time Frame
########################################################################################################################
def motor_0_change(percentage):
    sendRealTime(0, percentage)


def motor_1_change(percentage):
    sendRealTime(1, percentage)


def motor_2_change(percentage):
    sendRealTime(2, percentage)


def motor_3_change(percentage):
    sendRealTime(3, percentage)


def motor_4_change(percentage):
    sendRealTime(4, percentage)


def motor_5_change(percentage):
    sendRealTime(5, percentage)


def motor_6_change(percentage):
    sendRealTime(6, percentage)


def motor_7_change(percentage):
    sendRealTime(7, percentage)


# Create Scales & Lables
motor_0_percentage = IntVar(value=0)
motor0_scale = Scale(real_time_frame, orient=VERTICAL, length=200, from_=100, to=0, variable=motor_0_percentage,
                     command=motor_0_change)
motor0_scale_label = Label(real_time_frame, text="Motor 0 %")

motor_1_percentage = IntVar(value=0)
motor1_scale = Scale(real_time_frame, orient=VERTICAL, length=200, from_=100, to=0, variable=motor_1_percentage,
                     command=motor_1_change)
motor1_scale_label = Label(real_time_frame, text="Motor 1 %")

motor_2_percentage = IntVar(value=0)
motor2_scale = Scale(real_time_frame, orient=VERTICAL, length=200, from_=100, to=0, variable=motor_2_percentage,
                     command=motor_2_change)
motor2_scale_label = Label(real_time_frame, text="Motor 2 %")

motor_3_percentage = IntVar(value=0)
motor3_scale = Scale(real_time_frame, orient=VERTICAL, length=200, from_=100, to=0, variable=motor_3_percentage,
                     command=motor_3_change)
motor3_scale_label = Label(real_time_frame, text="Motor 3 %")

motor_4_percentage = IntVar(value=0)
motor4_scale = Scale(real_time_frame, orient=VERTICAL, length=200, from_=100, to=0, variable=motor_4_percentage,
                     command=motor_4_change)
motor4_scale_label = Label(real_time_frame, text="Motor 4 %")

motor_5_percentage = IntVar(value=0)
motor5_scale = Scale(real_time_frame, orient=VERTICAL, length=200, from_=100, to=0, variable=motor_5_percentage,
                     command=motor_5_change)
motor5_scale_label = Label(real_time_frame, text="Motor 5 %")

motor_6_percentage = IntVar(value=0)
motor6_scale = Scale(real_time_frame, orient=VERTICAL, length=200, from_=100, to=0, variable=motor_6_percentage,
                     command=motor_6_change)
motor6_scale_label = Label(real_time_frame, text="Motor 6 %")

motor_7_percentage = IntVar(value=0)
motor7_scale = Scale(real_time_frame, orient=VERTICAL, length=200, from_=100, to=0, variable=motor_7_percentage,
                     command=motor_7_change)
motor7_scale_label = Label(real_time_frame, text="Motor 7 %")


# Place
motor0_scale.grid(row=1, rowspan=20, column=0, stick="nsw", padx=10, pady=5)
motor0_scale_label.grid(row=21, column=0, stick="sw", padx=10, pady=5)

motor1_scale.grid(row=1, rowspan=20, column=1, stick="nsw", padx=10, pady=5)
motor1_scale_label.grid(row=21, column=1, stick="sw", padx=10, pady=5)

motor2_scale.grid(row=1, rowspan=20, column=2, stick="nsw", padx=10, pady=5)
motor2_scale_label.grid(row=21, column=2, stick="sw", padx=10, pady=5)

motor3_scale.grid(row=1, rowspan=20, column=3, stick="nsw", padx=10, pady=5)
motor3_scale_label.grid(row=21, column=3, stick="sw", padx=10, pady=5)

motor4_scale.grid(row=1, rowspan=20, column=4, stick="nsw", padx=10, pady=5)
motor4_scale_label.grid(row=21, column=4, stick="sw", padx=10, pady=5)

motor5_scale.grid(row=1, rowspan=20, column=5, stick="nsw", padx=10, pady=5)
motor5_scale_label.grid(row=21, column=5, stick="sw", padx=10, pady=5)

motor6_scale.grid(row=1, rowspan=20, column=6, stick="nsw", padx=10, pady=5)
motor6_scale_label.grid(row=21, column=6, stick="sw", padx=10, pady=5)

motor7_scale.grid(row=1, rowspan=20, column=7, stick="nsw", padx=10, pady=5)
motor7_scale_label.grid(row=21, column=7, stick="sw", padx=10, pady=5)

# Reset Button
reset_button = Button(real_time_frame, text="Reset", command=resetRealTimeScale)
reset_button.grid(row=10, column=8, stick="s", padx=28)


refreshDevices()

root.title("Haptic Driver Control")
root.mainloop()
