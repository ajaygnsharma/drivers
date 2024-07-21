'''
Created on Dec 12, 2019

@author: asharma
'''
import serial
import re
import platform

class Network(object):
    def __init__(self, port, debug: str=None):
        self.debug = 0;
        if(debug):
            self.debug = 1;
        self.ser = serial.Serial(port, 115200, timeout=1)
        self.cmdStr = "";
        
    def parseIP(self, lines):
        for l in lines:
            s = l.decode("utf-8");
            expList = re.findall("\d+.\d+.\d+.\d+", s);
            if( len(expList) > 0 ):
                assert (str(expList[0]) != '0.0.0.0'), expList;
        return True;

    def writeCmd(self, cmdStr):
        self.ser.write(str.encode(cmdStr));
        lines = self.ser.readlines();
        ok = self.parseIP(lines);
        assert (ok == True), lines;
        
    
    def ipAddr(self, val=None):
        if(val != None):
            self.cmdStr = b"CIA="+str.encode(val)+b"\n";
        else:
            self.cmdStr = b"CIA\n";
        self.writeCmd(self.cmdStr);
          
    def gateway(self, val=None):
        if(val != None):
            self.cmdStr = b"CIG="+str.encode(val)+b"\n";
        else:
            self.cmdStr = b"CIG\n";
        self.writeCmd(self.cmdStr);

    def subnetMask(self):
        if(val != None):
            self.cmdStr = b"CIM="+str.encode(val)+b"\n";
        else:
            self.cmdStr = b"CIM\n";
        self.writeCmd(self.cmdStr);
               
    def __del__(self):
        self.ser.close();
    