'''
Created on Dec 12, 2019

@author: asharma
'''
import serial
import platform
import re

validResponses = ["?"];
inValidResponses = ["?"]; 

class System(object):
    def __init__(self, port, debug: str=None):
        self.debug = 0;
        if(debug):
            self.debug = 1;
            
        self.ser = serial.Serial(port, 115200, timeout=1);
    
    def parseRsp(self, pattern):
        lines = self.ser.readlines();
        ok = False;
        for l in lines:
            s = l.decode("utf-8");
            expList = re.findall(pattern, s);
            if( len(expList) > 0 ):
                return expList;
    
    # Services status    
    def status(self):
        self.ser.write(b'ibs\n')
        lines = self.ser.readlines()
        for l in lines:
            if "STOPPED" in l.decode("utf-8"):
                assert False, l;
            if(self.debug):
                print(l);
                
    # External LED 
    def externalLED(self, val=None):
        if(val):
            cmdStr=val+"\n";
            self.ser.write(b'CLE='+str.encode(cmdStr));
        else:
            self.ser.write(b'CLE\n');
            
        reList = self.parseRsp("\d+");
        return reList[0];
    
    def checkExternalLED(self):
        print("External LED: ",end="");
        reList = self.externalLED();
        exLED = int(reList[0]);
        assert ((exLED == 0) or (exLED == 1));
        print("OK");
    
    # 50Hz correction
    def get50HzCorrection(self):
        self.ser.write(b'CSF\n')
        reList = self.parseRsp("\d");
        if(self.debug):
            if(reList[0] == '0'):
                print("%-30s %s"%("AC Correction","60 Hz"))
            elif(reList[0] == '1'):
                print("%-30s %s"%("AC Correction","50 Hz"))
            else:
                print("AC Correction: What?"+reList);
                
        return reList[0];
    
    def check50HzCorrection(self):
        print("50 Hz correction: ",end="");
        reList = self.get50HzCorrection();
        _50HzCorrection = int(reList[0]);
        assert ((_50HzCorrection == 0) or (_50HzCorrection == 1));
        print("OK");
    
    # Version
    def version(self):
        self.ser.write(b'CCS\n');
        reList = self.parseRsp("\d{1}.\d{2}");
        if(self.debug):
            print(reList);
            
        return float(reList[0]);
    
    def checkVersion(self, version):
        print("Firmware version: ",end="");
        assert (self.version() >= version);
        print("OK");
        
    # SSH/Web server change
    def setServiceState(self, service, state):
        str=service+","+state+"\n";
        if(debug):
            print(str,end="");
        
        self.ser.write(b'CIE='+str.encode(str));
    
    # SSH    
    def sshState(self):
        self.ser.write(b'CIE\n');
        reList = self.parseRsp("\d");
        if(self.debug):
            print(reList);
        return reList[0];
        
    def checkSSH(self):
        print("SSH: ",end="");
        ssh = int(self.sshState());
        assert (ssh == 1), ssh;
        print("OK");

    # Web Server
    def webServerState(self):
        self.ser.write(b'CIE\n');
        reList = self.parseRsp("\d");
        if(self.debug):
            print(reList);
        return reList[1];
        
    def checkWebserver(self):
        print("Web Server: ",end="");
        webServer = int(self.webServerState());
        assert (webServer == 1), webServer;
        print("OK");
    
    def test1(self):
        self.status();

    def __del__(self):    
        self.ser.close()
