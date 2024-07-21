'''
Created on Dec 12, 2019

@author: asharma
'''
import serial
import re
import platform

max = 300;

class Alarms(object):
    def __init__(self, port,  debug: str=None):
        self.debug = 0;
        if(debug):
            self.debug = 1;
            
        self.ser = serial.Serial(port, 115200, timeout=1);
     
    def ok(self):
        currentState = self.get();
        if(currentState == 0):
            print("Alarms: OK");
        else:
            print("Alarms fired !!! %d", currentState);
        
    '''
    def get10MHz(self):
        self.ser.write(b'C10\n')
        line = self.ser.readline()
        x = re.findall("\d+", line.decode("utf-8") )
        if(x[1] == '1'):
            print("%-30s %s"%("10MHz","OK"))
        else:
            print("%-30s %s"%("10MHz","Alarm"))
        print("")
    '''
            
    def get(self, verbose=None):
        state = -1;
        if(verbose):
            self.ser.write(b'CAS=1\n');
            lines = self.ser.readlines();
            for i in lines:
                s = i.decode("utf-8");
                expList = re.findall("0x\d{4}|0x\w{4}", s);
                if(len(expList) > 0):
                    state = int(expList[0]);
                    break;
        else:
            self.ser.write(b'CAS\n');
            lines = self.ser.readlines();
            for l in lines:
                expList = re.findall("0x\d{4}|0x\w{4}", l.decode("utf-8") );
                if(len(expList) > 0):
                    state = expList[0];
                
        return state;
    
    # Cannot change state of alarms. Use mask instead. 
    def set(self, val=None):
        cmdStr = '';
        if( val != None ):
            cmdStr += val;
        
        cmdStr += '\n';
        self.ser.write(b'CAS='+str.encode(cmdStr));
        lines = self.ser.readlines();
        ok = False;
        for l in lines:
            s = l.decode("utf-8");
            if("invalid value" in s):
                return "invalid value";
        
                
    def getMask(self, param):
        if(param == "major"):
            self.ser.write(b'CM1\n');
            lines = self.ser.readlines();
            for l in lines:
                x = re.findall("0x\d{4}|0x\w{4}", l.decode("utf-8") );
                if(len(x) > 0):
                    print("%-30s %s"%("Major Alarm mask",x[0]))
        elif(param == "minor"):
            self.ser.write(b'CM2\n');
            lines = self.ser.readlines();
            for l in lines:
                x = re.findall("0x\d{4}|0x\w{4}", l.decode("utf-8") );
                if(len(x) > 0):
                    print("%-30s %s"%("Minor Alarm mask",x[0]))
        elif(param == "suppress"):
            self.ser.write(b'CMS\n');
            lines = self.ser.readlines();
            for l in lines:
                x = re.findall("0x\d{4}|0x\w{4}", l.decode("utf-8") );
                if(len(x) > 0):
                    print("%-30s %s"%("Suppress mask",x[0]))
        
        print("")
        
    def setMask(self, param, val):    
        if(param == "major"):
            self.ser.write(b'CM1='+str.encode(val)+b'\n')
            line = self.ser.readline()
            x = re.findall("\d+x\d", line.decode("utf-8") )
            print("Major Alarm mask=%s"%x[0])
        elif(param == "minor"):
            self.ser.write(b'CM2='+str.encode(val)+b'\n')
            line = self.ser.readline()
            x = re.findall("\d+x\d", line.decode("utf-8") )
            print("Minor Alarm mask=%s"%x[0])
        elif(param == "suppress"):
            self.ser.write(b'CMS='+str.encode(val)+b'\n')
            line = self.ser.readline()
            x = re.findall("\d+x\d", line.decode("utf-8") )
            print("Suppress mask=%s"%x[0])
        
        print("")
    
    def getSimulated(self):
        self.ser.write(b'TAS\n')
        lines = self.ser.readlines();
        for l in lines:
            x = re.findall("\d+", l.decode("utf-8"));
            if(len(x) > 0):
                if(x[0] == '0'):
                    print("%-30s %s"%("TX simulated alarm","Disable"))
                else:
                    print("%-30s %s"%("TX simulated alarm","Enable"))

    def setSimulated(self, val):
        cmd = 'TAS='+val+'\n'
        self.ser.write(cmd)
        line = self.ser.readline()
        try:
            x = re.findall("\d+", line.decode("utf-8") )
            if(x[0] == '0'):
                print("%-30s %s"%("TX simulated alarm","Disable"))
            else:
                print("%-30s %s"%("TX simulated alarm","Enable"))
        except:
            print(line)
    
    def getSuppression(self):
        self.ser.write(b'TAZ\n');
        lines = self.ser.readlines();
        for l in lines:
            x = re.findall("\d+", l.decode("utf-8"));
            if(len(x) > 0):
                if(x[0] == '0'):
                    print("%-30s %s"%("TX alarm suppression","Disable"));
                else:
                    print("%-30s %s"%("TX alarm suppression","Enable"));
    
    def setSuppression(self, val):
        self.ser.write(b'TAZ='+str.encode(val)+b'\n')
        line = self.ser.readline()
        try:
            x = re.findall("\d+", line.decode("utf-8") )
            if(x[0] == '0'):
                print("%-30s %s"%("TX alarm suppression","Disable"))
            else:
                print("%-30s %s"%("TX alarm suppression","Enable"))  
        except:
            print(line)
                   
    def __del__(self):
        self.ser.close()   