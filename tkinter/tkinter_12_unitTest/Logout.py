'''
Created on Dec 12, 2019

@author: asharma
'''
import serial
import time
import platform
import re
import sys
import tkinter as tk
debug = False;

def isLoginFound(line):
    x = re.findall('ibuc login: ', line.decode("utf-8"));
    while(len(x) > 0):
        v = x.pop();
        #print(x);
        if(v == 'ibuc login: '):
            return True;
            
    return False;

class LogOut(object):
    def __init__(self, txtBox, port):
        self.ser  = 0
        self.txtBox = txtBox;
        self.port = port;
        self.ser = serial.Serial(self.port, 115200, timeout=0.5);
        
        self.ser.write(b'\x03');# ctrl c
        
        while(1):
            line = self.ser.read(300);
            if(isLoginFound(line) == True):
                txtBox.insert(tk.END, "Logged Out\n");
                self.ser.close();
                break;
            else:
                time.sleep(1);
                self.ser.write(b'\n');
