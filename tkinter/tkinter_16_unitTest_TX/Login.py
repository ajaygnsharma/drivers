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
    x = re.findall('ibuc login: |IBUC>|\x0d', line.decode("utf-8"));
    while(len(x) > 0):
        v = x.pop();
        #print(x);
        if(v == 'ibuc login: '):
            return True;
            
    return False;

def isPasswordPromptFound(line):
    x = re.findall('Password: |\r', line.decode("utf-8"));
    while(len(x) > 0):
        v = x.pop();
        #print(x);
        if(v == 'Password: '):
            return True;
            
    return False;

def isCmdPromptFound(line):
    x = re.findall('IBUC>|\r', line.decode("utf-8"));
    while(len(x) > 0):
        v = x.pop();
        #print(x);
        if(v == 'IBUC>'):
            return True;
            
    return False;

'''
> Welcome to the IBUC[CR][LF] 1      
> [CR]ibuc login:        
<< terrasat[CR][LF]
> terrasat[CR][LF]
> "Hi"[CR][LF]
> Password: 
<< passwd[LF]
> [CR][LF]  
'''
class Login(object):
    def __init__(self, txtBox, port):
        self.ser    = 0
        self.txtBox = txtBox;
        self.port = port;
        self.ser = serial.Serial(self.port, 115200, timeout=0.5);
        self.ser.write(b'\x03');
        
        while(1):
            line = self.ser.read(300);
            if(isLoginFound(line) == True):
                txtBox.insert(tk.END, "Entering Login name\n");
                self.ser.write(b'factory\r');
            elif(isPasswordPromptFound(line) == True):
                txtBox.insert(tk.END, "Entering Password\n");
                self.ser.write(b'Calibration#1#1\r'); # Not echoed
            elif(isCmdPromptFound(line) == True):
                txtBox.insert(tk.END, "Cmd Prompt Ready\n");
                if(debug):
                    self.ser.write(b'\x04');
                    txtBox.insert(tk.END, "Cmd Prompt close\n");

                self.ser.close();
                break;
            else:
                time.sleep(1);
                self.ser.write(b'\n');
