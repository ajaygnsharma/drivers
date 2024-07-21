'''
Created on Feb 3, 2020

@author: asharma
'''
import serial
import platform
import re

class Metrics(object):
    def __init__(self):
        osType = platform.system()
        if(osType == "Windows"):
            self.ser = serial.Serial('COM1', 115200, timeout=1)
        elif(osType == "Linux"):        
            self.ser = serial.Serial('/dev/ttyUSB0', 115200, timeout=1)
    
    def getFactoryData(self):
        self.ser.write(b'CFD\n')
        line = self.ser.readline()
        x = re.findall("\d+", line.decode("utf-8") )
        print("Total Hours: %s"%x[0])
    
    def getSUFactoryData(self):
        self.ser.write(b'CPE=1943\n')
        self.ser.readline()
        
        self.ser.write(b'CFD\n')
        line = self.ser.readline()
        print(line)
        line = self.ser.readline()
        print(line)
        line = self.ser.readline()
        print(line)
        line = self.ser.readline()
        print(line)
    
    def getTxOnTime(self):
        self.ser.write(b'TTT\n')
        line = self.ser.readline()
        x = re.findall("\d+", line.decode("utf-8") )
        print("TX On Time: %s"%x[0])
               
    def __del__(self):
        self.ser.close()      