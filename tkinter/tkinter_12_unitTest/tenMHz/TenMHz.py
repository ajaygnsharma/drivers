'''
Created on Dec 12, 2019

@author: asharma
'''
import serial
import platform
import re

''' invalid command is a good response. That means the program actually detects errors '''
validResponses = ["invalid command", "CTI=0", "CTI=1", "DTM=0", "DTM=1"];
inValidResponses = ["Undefined"]; 

class TenMHz(object):
    def __init__(self, port, debug: str=None):
        self.debug = 0;
        if(debug):
            self.debug = 1;
            
        self.ser = serial.Serial(port, 115200, timeout=1);

    def runTest(self):
        print("---------------------------------");
        print("             10MHz               ");
        print("---------------------------------");
        
        self.refCurrentState();
        print("External Current state: OK"); # Assert will fail it
        self.tenMHzTrim();
        
        self.validCalibration();
        self.disabled();
        #self.disable(1);
        self.internalSupported();
    
    '''
    If incoming reference 10MHz level is currently active/valid, this returns 1. 
    0 otherwise. Generally units support external 10MHz and internal is 
    a feature. So check that first. It it fails, check if internal is 
    supported.
    '''
    def refCurrentState(self, val=None):
        if( val != None ):
            self.ser.write(b'C10='+str.encode(val)+b'\n');
            lines = self.ser.readlines();
            ok = False;
            for l in lines:
                s = l.decode("utf-8");
                if("SET is not valid with C10" in s):
                    ok = True;
        else:
            self.ser.write(b'C10\n');
            lines = self.ser.readlines();
            ok = False;
            for l in lines:
                s = l.decode("utf-8");
                if("C10=1" in s):
                    ok = True;
                    break;
        
        ''' If external 10MHz is not available, but unit supports internal
        10MHz, then it is OK 
        '''
        if(ok == False):
            if(self.internalSupported() == True):
                ok = True;
                        
        assert (ok == True), lines;
            
    def tenMHzTrim(self):
        print("10MHz Trim: ",end="");
        if(self.internalSupported() == True):
            if(val):
                cmdStr=val+"\n";
                self.ser.write(b'CRT='+str.encode(cmdStr));
            else:
                self.ser.write(b'CRT\n');
            
            self.parseRsp("\d+", 0, 1);
        else:
            ''' 
            If no internal 10MHz, 10MHz trim is not valid.
            cmd responds with "invalid command" which is OK.
            '''
            self.ser.write(b'CRT\n');
            self.parseRsp("invalid command"); 
        print("OK");
    
    ''' I am not sure I understand this. Shouldn't this be valid? '''
    def validCalibration(self):
        print("Valid Calibration: ",end="");
        self.ser.write(b'ctd\n');
        self.parseRsp("invalid command");
        print("OK");

    def setValid10MHZCal(self):
        print("Setting valid 10MHz cal: ",end="");
        self.ser.write(b'TTC=10MHZ_CAL\n');
        print("OK");

    ''' DTM allows user to disable ref 10MHz "manually/override". This returns 
    if it currently disabled or not? '''
    def disabled(self):
        print("Ref 10MHz disabled: ",end="");
        self.ser.write(b'dtm\n');
        lines = self.ser.readlines();
        ref10MHzDisabled = False;
        for l in lines:
            s = l.decode("utf-8");
            if("DTM=0" in s):
                ref10MHzDisabled = False;
                ok = True;
                break;
            elif("DTM=1" in s):
                ref10MHzDisabled = True;
                ok = True;
                break;
            
        assert (ok == True), lines;
        print("OK");
        return ref10MHzDisabled;
    
    ''' Continuing above, this disables it '''
    def disable(self, newState):
        print("Ref 10MHz disabled: ",end="");
        self.ser.write(b'dtm='+newState.encode()+b'\n');
        lines = self.ser.readlines();
        self.parseRsp(lines);
        print("OK");
    
    '''
    If the unit is supposed to have "internal" 10MHz reference this returns 1.
    0 Otherwise. Its based off part number.
    '''    
    def internalSupported(self):
        print("Internal 10MHz supported: ",end="");
        self.ser.write(b'cti\n');
        lines = self.ser.readlines();
        ok = False;
        internal10MHz = False;
        for l in lines:
            s = l.decode("utf-8");
            if("CTI=0" in s):
                internal10MHz = False;
                ok = True;
                break;
            elif("CTI=1" in s):
                internal10MHz = True;
                ok = True;
                break;
            
        assert (ok == True), lines;    
        print("OK");
        return internal10MHz;
    
    def parseRsp(self, pattern, low=None, high=None):
        lines = self.ser.readlines();
        ok = False;
        expList = '';
        for l in lines:
            s = l.decode("utf-8");
            expList = re.findall(pattern, s);
            if(low and high):
                if( len(expList) > 0 ):
                    assert ((float(expList[0]) > low) and ((float(expList[0]) <= high))), expList;
                    ok = True;
                    break;
            else:
                if( len(expList) > 0 ): # Found the pattern and so OK.
                    ok = True;
                    break;  
                
        if(self.debug):
            print(lines);
        else:
            assert (ok == True), lines;
            return expList[0];
        
    def test1(self):
        self.internalSupported();
        self.disabled();
        self.state("0");

    def __del__(self):    
        self.ser.close()
