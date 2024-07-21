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

'''
> Welcome to the IBUC[CR][LF] 1      
> [CR]ibuc login:        
<< admin[CR][LF]
> admin[CR][LF]
> "message"[CR][LF]
> Password: 
<< admin[LF]
> [CR][LF]  
'''
class User(object):
    def __init__(self, loginName, oldPassword, newPassword, txtBox, port):
        self.ser         = 0;
        self.txtBox      = txtBox;
        self.port        = port;
        self.loginName   = loginName;
        self.oldPassword = oldPassword;
        self.newPassword = newPassword;

        self.ser = serial.Serial(self.port, 115200, timeout=0.5);

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

    def isOldPasswordPromptFound(self, line):
        x = re.findall('Old password: |\r', line.decode("utf-8"));
        while (len(x) > 0):
            v = x.pop();
            # print(x);
            if (v == 'Old password: '):
                return True;

        return False;

    def isNewPasswordPromptFound(self, line):
        x = re.findall('New password: |\r', line.decode("utf-8"));
        while (len(x) > 0):
            v = x.pop();
            # print(x);
            if (v == 'New password: '):
                return True;

        return False;

    def isRetypePasswordPromptFound(self, line):
        x = re.findall('Retype password: |\r', line.decode("utf-8"));
        while (len(x) > 0):
            v = x.pop();
            # print(x);
            if (v == 'Retype password: '):
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

    def addUser(self, username, password):
        cmd = ("ADU=%s,%s,admin\n"%(username,password));
        self.ser.write(str.encode(cmd));
        time.sleep(2);
        lines = self.ser.readlines(5000);
        self.txtBox.insert(tk.END, lines);
        return True;

    def setPassword(self):
        noOfTries = 0;
        self.ser.write(b'\x03');

        while(noOfTries < 5):
            line = self.ser.read(300);
            if(self.isLoginFound(line) == True):
                self.txtBox.insert(tk.END, "Entering Login name\n");
                cmd = self.loginName+'\n';
                self.ser.write(str.encode(cmd));

            elif(self.isPasswordPromptFound(line) == True):
                self.txtBox.insert(tk.END, "Entering Password\n");
                cmd = self.oldPassword + '\n';
                self.ser.write(str.encode(cmd)); # Not echoed

            elif(self.isCmdPromptFound(line) == True):
                self.txtBox.insert(tk.END, "Cmd Prompt Ready\n");
                if(debug):
                    self.ser.write(b'\x04');
                    self.txtBox.insert(tk.END, "Cmd Prompt close\n");

                self.ser.close();
                break;
            elif(self.isOldPasswordPromptFound(line) == True):
                self.txtBox.insert(tk.END, "Entering Old Password\n");
                cmd = self.oldPassword + '\n';
                self.ser.write(str.encode(cmd)); # Not echoed
            elif(self.isNewPasswordPromptFound(line) == True):
                self.txtBox.insert(tk.END, "Entering New Password\n");
                cmd = self.newPassword + '\n';
                self.ser.write(str.encode(cmd)); # Not echoed
            elif(self.isRetypePasswordPromptFound(line) == True):
                self.txtBox.insert(tk.END, "Re-Entering New Password\n");
                cmd = self.newPassword + '\n';
                self.ser.write(str.encode(cmd)); # Not echoed
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