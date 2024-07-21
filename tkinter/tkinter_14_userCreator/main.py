from Login import Login
from Logout import LogOut
from User import User

import sys
import time
import platform
import getopt
import tkinter as tk
import threading as thr

import os, sys
from stat import S_ISCHR

window = tk.Tk();
window.rowconfigure(0, minsize=100);
window.columnconfigure([0, 1], minsize=50);

# use threading
def threadChkAdminUser():
    # Call work function
    t1 = thr.Thread(target=workCheckUserName, args=("admin","FactoryUser!123"));
    t1.start();

def threadChkFactoryUser():
    # Call work function
    t1 = thr.Thread(target=workCheckUserName, args=("factory","Calibration#1#1"));
    t1.start();

def threadCreateAdminUser():
    # Call work function
    t1 = thr.Thread(target=workCreateAdminUser, args=("admin","admin","FactoryUser!123"));
    t1.start();

def threadCreateFactoryUser():
    # Call work function
    t1 = thr.Thread(target=workCreateFactoryUser);
    t1.start();

frame1=tk.Frame(master=window);

CheckAdminUserBtn = tk.Button(master=frame1, bg="#17A589",text="Check Admin User", command=threadChkAdminUser);
CheckAdminUserBtn.grid(row=0, column=0);

CreateAdminUserBtn = tk.Button(master=frame1, bg="#17A589",text="Create Admin User", command=threadCreateAdminUser);
CreateAdminUserBtn.grid(row=0, column=1);

CheckFactoryUserBtn = tk.Button(master=frame1, text="Check Factory User", command=threadChkFactoryUser);
CheckFactoryUserBtn.grid(row=1, column=0);

CreateFactoryUserBtn = tk.Button(master=frame1, text="Create Factory User", command=threadCreateFactoryUser);
CreateFactoryUserBtn.grid(row=1, column=1);

frame1.grid(row=0, column=0);
# ListBox for Serial Ports List
serialPort = tk.StringVar();
listBoxPort = tk.Listbox(master=window, listvariable=serialPort);
listBoxPort.grid(row=1, column=0, stick="e");

txtBox = tk.Text(master=window, height=20, width=40);
txtBox.grid(row=2, column=0, sticky="e");


def checkfile(fileName):
    try:
        mode = os.lstat(fileName).st_mode;
        if S_ISCHR(mode):
            # It's a character special file, call the callback function
            return True;
    except OSError:
        print('Skipping %s' % fileName);
        return False;

# work function
def workCheckUserName(userName, password):
    msg = ("Checking User: %s\n"%(userName));
    txtBox.insert(tk.END, msg);

    # Clear text box first
    txtBox.delete("1.0", "end");

    idx = listBoxPort.curselection(); # The curselection() returns a Tuple
    if (len(idx) == 0):
        txtBox.insert(tk.END, "Select a Serial port First\n");
        return 0;
    else:
        port = listBoxPort.get(idx); # "/dev/ttyUSB0";
        l1 = Login(userName, password, txtBox, port);
        if(l1.login() == True):
            msg = ('User: %s Exists\n'%(userName));
            txtBox.insert(tk.END, msg);

        l2 = LogOut(txtBox,port);
        txtBox.insert(tk.END,"Done");


# work function
def workCreateAdminUser(userName, oldPassword, newPassword):
    msg = ("Creating User: %s\n"%(userName));
    txtBox.insert(tk.END, msg);

    # Clear text box first
    txtBox.delete("1.0", "end");

    idx = listBoxPort.curselection(); # The curselection() returns a Tuple
    if (len(idx) == 0):
        txtBox.insert(tk.END, "Select a Serial port First\n");
        return 0;
    else:
        port = listBoxPort.get(idx); # "/dev/ttyUSB0";
        u = User(userName, oldPassword, newPassword, txtBox, port);
        if(u.setPassword() == True):
            msg = ('User: %s password changed\n'%(userName));
            txtBox.insert(tk.END, msg);

        l2 = LogOut(txtBox,port);
        txtBox.insert(tk.END,"Done");

# work function
def workCreateFactoryUser():
    msg = ("Creating User: Factory\n");
    txtBox.insert(tk.END, msg);

    # Clear text box first
    txtBox.delete("1.0", "end");

    idx = listBoxPort.curselection(); # The curselection() returns a Tuple
    if (len(idx) == 0):
        txtBox.insert(tk.END, "Select a Serial port First\n");
        return 0;
    else:
        port = listBoxPort.get(idx); # "/dev/ttyUSB0";
        l1 = Login("admin","FactoryUser!123",txtBox,port);
        l1.login();

        u = User("factory","admin","Calibration#1#1",txtBox, port);
        if(u.addUser("factory","admin") == True):
            msg = ('User: factory created\n');
            txtBox.insert(tk.END, msg);
            if (u.setPassword() == True):
                msg = ('User: factory password changed\n');
                txtBox.insert(tk.END, msg);

        l2 = LogOut(txtBox,port);
        txtBox.insert(tk.END,"Done");



serialPorts = ("COM1","COM2","/dev/ttyS0", "/dev/ttyUSB0", "/dev/ttyUSB1");
for s in serialPorts:
    if checkfile(s):
        listBoxPort.insert('end', s);

window.mainloop();