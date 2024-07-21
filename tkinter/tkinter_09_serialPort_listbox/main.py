#!/usr/bin/env python
# -*- coding: utf-8 -*-
import tkinter as tk
import os, sys
from stat import S_ISCHR

window = tk.Tk();
window.title('My Window');

window.geometry('500x300');

var3 = tk.StringVar();
l = tk.Label(window, bg='green', fg='yellow', font=('Arial', 12), width=10, textvariable=var3)
l.pack()


def print_selection():
    value = lb.get(lb.curselection())
    var3.set(value)


b1 = tk.Button(window, text='print selection', width=15, height=2, command=print_selection)
b1.pack();


def checkfile(fileName):
    mode = os.lstat(fileName).st_mode;
    if S_ISCHR(mode):
        # It's a character special file, call the callback function
        return True;
    else:
        # Unknown file type, print a message
        print('Skipping %s' % fileName);
        return False;

var2 = tk.StringVar();
lb = tk.Listbox(window, listvariable=var2);

serialPorts = ("/dev/ttyS0", "/dev/ttyUSB0", "/dev/ttyUSB1");
for s in serialPorts:
    if checkfile(s):
        lb.insert('end', s);

lb.pack();

lbl = tk.Label(window, bg='white', width=20, text='empty')
lbl.pack();

def print_selection():
    if (var1.get() == 1) & (var2.get() == 0):
        lbl.config(text='Fifo Unit Test')
    elif (var1.get() == 0) & (var2.get() == 1):
        lbl.config(text='Calibration Unit Test')
    elif (var1.get() == 0) & (var2.get() == 0):
        lbl.config(text='No test selected')
    else:
        lbl.config(text='I love both')

var1 = tk.IntVar()
var2 = tk.IntVar()
c1 = tk.Checkbutton(window, text='Fifo',variable=var1, onvalue=1, offvalue=0, command=print_selection)
c1.pack()
c2 = tk.Checkbutton(window, text='Calibration',variable=var2, onvalue=1, offvalue=0, command=print_selection)
c2.pack()

window.mainloop()