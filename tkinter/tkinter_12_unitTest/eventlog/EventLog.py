'''
Created on Dec 12, 2019

@author: asharma
'''
import serial
import platform
import tkinter as tk

class EventLog(object):
    def __init__(self, txtBox, port):
        self.ser = serial.Serial(port, 115200, timeout=1);
        self.txtBox = txtBox;
        #self.test1();
        #self.test2();
            
    def get(self):
        self.ser.write(b'ehi\n')
        lines = self.ser.readlines()
        return lines;
    
    def clear(self):
        self.ser.write(b'ehz\n')
        lines = self.ser.readlines()
        self.txtBox.insert(tk.END, lines);

    def clearAlt(self):
        self.ser.write(b'ehi=0\n')
        line = self.ser.readline()
        print(line)
    
    def test2(self):
        self.clear();
        
    def test1(self):
        while(1):
            lines = self.get();
            for l in lines:
                print(l);
                if "empty" in l.decode("utf-8"):
                    print("All of event log is read");
                    return;
    
    def testAllCmds(self):
        self.clear();
        nw = Network.Network();
        nw.setIPAddress();
        nw.setIPGateway();
        nw.setSubnetMask();
  
                
    def __del__(self):    
        self.ser.close()
