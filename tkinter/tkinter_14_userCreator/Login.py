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
    def __init__(self, loginName, password, txtBox, port):
        self.ser       = 0;
        self.txtBox    = txtBox;
        self.port      = port;
        self.loginName = loginName;
        self.password  = password;
        self.ser = serial.Serial(self.port, 115200, timeout=0.5);

        self.debug = False;

        self.ser.write(b'\x03');

    def isLoginFound(self, line):
        x = re.findall('ibuc login: |IBUC>|\x0d', line.decode("utf-8"));
        while (len(x) > 0):
            v = x.pop();
            # print(x);
            if (v == 'ibuc login: '):
                return True;

        return False;

    def isPasswordPromptFound(self, line):
        x = re.findall('Password: |\r', line.decode("utf-8"));
        while (len(x) > 0):
            v = x.pop();
            # print(x);
            if (v == 'Password: '):
                return True;

        return False;

    def isCmdPromptFound(self, line):
        x = re.findall('IBUC>|\r', line.decode("utf-8"));
        while (len(x) > 0):
            v = x.pop();
            # print(x);
            if (v == 'IBUC>'):
                return True;

        return False;

    def login(self):
        noOfTries = 0;

        while(noOfTries < 5):
            line = self.ser.read(300);
            if(self.isLoginFound(line) == True):
                self.txtBox.insert(tk.END, "Entering Login name\n");
                cmd = self.loginName+'\n';
                self.ser.write(str.encode(cmd));

            elif(self.isPasswordPromptFound(line) == True):
                self.txtBox.insert(tk.END, "Entering Password\n");
                cmd = self.password + '\n';
                self.ser.write(str.encode(cmd)); # Not echoed

            elif(self.isCmdPromptFound(line) == True):
                self.txtBox.insert(tk.END, "Cmd Prompt Ready\n");
                if(self.debug):
                    self.ser.write(b'\x04');
                    self.txtBox.insert(tk.END, "Cmd Prompt close\n");

                self.ser.close();
                break;
            else:
                noOfTries = noOfTries + 1;
                time.sleep(1);
                self.ser.write(b'\n');

        if(noOfTries >= 5):
            errorMsg="User:%s is not created\n"%(self.loginName);
            self.txtBox.insert(tk.END, errorMsg);
            return False;
        else:
            return True;