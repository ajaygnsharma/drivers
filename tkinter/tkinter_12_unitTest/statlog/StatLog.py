'''
Created on Dec 12, 2019

@author: asharma
'''
import serial
import platform
import re

class StatLog(object):
    def __init__(self, port, debug: str=None):
        self.debug = 0;
        if(debug):
            self.debug = 1;
            
        self.ser = serial.Serial(port, 115200, timeout=1);
        #self.test2();
    
    def test1(self):
        self.get();
        print(self.period());
        self.period("1"); # Minutes
        self.get();
        print(self.period());
        
    def test2(self):
        print("Clearing log");
        self.clear();
        #print("Getting log");
        #self.get();
        
    def period(self, val=None):
        if(val):
            self.ser.write(b'shp='+str.encode(val)+b'\n');
        else:
            self.ser.write(b'shp\n');
        lines = self.ser.readlines()
        shp = 0;
        for l in lines:
            x = re.findall("\d{1,4}", l.decode("utf-8"));
            if(len(x) > 0):
                shp = x[0];
        return shp;
    
    def clearAlt(self):
        self.ser.write(b'shl=0\n')
        line = self.ser.readline()
        print(line)
        
    # Get Stat log table
    def get(self):       
        self.ser.write(b'shl\n')
        lines = self.ser.readlines()
        for l in lines:
            print("%s"%l);
    
    def clear(self):
        self.ser.write(b'shz\n')
        lines = self.ser.readlines();
        for l in lines:
            s = l.decode("utf-8");
            if("cleared" in s):
                print(s);
                
    def __del__(self):        
        self.ser.close()
        
