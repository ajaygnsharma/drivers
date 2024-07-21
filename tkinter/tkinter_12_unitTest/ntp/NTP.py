'''
Created on Dec 12, 2019

@author: asharma
'''
import serial
import platform
import re

class NTP(object):
    def __init__(self):
        osType = platform.system()
        if(osType == "Windows"):
            self.ser = serial.Serial('COM1', 115200, timeout=1)
        elif(osType == "Linux"):        
            self.ser = serial.Serial('/dev/ttyUSB0', 115200, timeout=1)
    
        
    def server(self, val=None):
        if(val):
            print("++++Adding NTP server: %s"%val);
            self.ser.write(b'NTP='+str.encode(val)+b'\n');
        else:
            print("*****NTP servers:");
            self.ser.write(b'NTP\n');
            
        server = "";
        lines = self.ser.readlines();
        for l in lines:
            x = re.findall("\d+.\D+.\D+.\D+", l.decode("utf-8") )
            if (len(x) > 0):
                print(x);
                server = x[0];
                
        assert (server != ""), lines;
        return server;
    
    def rmServer(self, val):
        print("Removing NTP server: %s"%val);
        self.ser.write(b'NTPDEL='+str.encode(val)+b'\n');
        server = "";
        lines = self.ser.readlines();
        for l in lines:
            x = re.findall("\d+.\D+.\D+.\D+", l.decode("utf-8") )
            if (len(x) > 0):
                server = x[0];
        
        return server;
    
    def getTimeZone(self):
        self.ser.write(b'TZO\n')
        line = self.ser.readline()
        x = re.findall("\d+", line.decode("utf-8") )
        print("Timezone offset: %s"%x[0])
        
    def setTimeZone(self, val):
        bytes_ = str.encode(val)
        self.ser.write(b'TZO=-0'+bytes_+b':00\n')
        line = self.ser.readline()
        x = re.findall("\d+", line.decode("utf-8") )
        print("Timezone offset: %s"%x[0])
        
    def getCurrentTime(self):
        self.ser.write(b'CTM\n')
        line = self.ser.readline()
        x = re.findall("\d+/\d+/\d+ \d+:\d+:\d+", line.decode("utf-8") )
        print("Current Time: %s"%x[0])
               
    def __del__(self):
        self.ser.close()      