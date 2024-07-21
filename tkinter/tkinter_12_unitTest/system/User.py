import serial
import re
import platform

max = 300;

# Covers ADU/DLU
class User(object):
    def __init__(self, port, debug: str=None):
        self.debug = 0;
        if(debug):
            self.debug = 1;
        self.ser = serial.Serial(port, 115200, timeout=1)
    
    def list(self):
        print("************Listing Users");
        self.ser.write(b'adu\r');
        lines = self.ser.readlines();
        userCount = 0;
        for l in lines:
            s = l.decode("utf-8");
            if(("admin" in s) or ("user" in s)):
                userCount = userCount + 1;
                if(self.debug):
                    print(s, end="");
                
        assert (userCount > 0), lines;
    
    def new(self, userName=None, password=None, privilege=None):
        print("++++++Adding %s"%userName);
        cmdStr = '';
        if( userName != None ):
            cmdStr += userName;

        cmdStr += ',';
        if( password != None ):
            cmdStr += password;
        
        cmdStr += ',';
        if( privilege != None ):
            cmdStr += privilege;
        
        cmdStr += '\n';
        self.ser.write(b'adu='+str.encode(cmdStr));
        lines = self.ser.readlines();
        added = False;
        for l in lines:
            s = l.decode("utf-8");
            if("changed" in s):
                added = True;
        
        assert (added == True), lines;
    
    def delete(self, userName):
        print("----------Deleting %s"%userName);
        cmdStr = "";
        cmdStr += userName;
        self.ser.write(b'dlu='+str.encode(cmdStr)+b'\n');
        lines = self.ser.readlines();
        return lines;
        
    def stop(self):
        self.ser.close();
        
        
           