'''
Created on Dec 12, 2019

@author: asharma
'''
import serial
import re
import platform

max = 300;

class Sensors(object):
    def __init__(self, port, debug: str=None):
        self.debug = 0;
        if(debug):
            self.debug = 1;
        self.ser = serial.Serial(port, 115200, timeout=1)
    
    def readTest(self):
        self.current();
        self.externalLED();
        self.internalTemperature();
        
    def validTest(self):
        self.internalTemperature("1");
        self.internalTemperature("2");
        self.internalTemperature("10085");
        pass;
    
    def invalidTest(self):
        pass;
    
    def parseRsp(self, pattern):
        lines = self.ser.readlines();
        ok = False;
        for l in lines:
            s = l.decode("utf-8");
            expList = re.findall(pattern, s);
            if( len(expList) > 0 ):
                return expList;
            
    def chechkRsp(self, expList, low, high):
        assert ((float(expList[0]) >  low) and (float(expList[0]) <= high)), expList;
        
        if(self.debug):
            print(expList);
        else:
            print("OK");
    # RAW
    def raw(self):
        self.ser.write(b'raw\n');
        lines = self.ser.readlines();
        for l in lines:
            s = l.decode("utf-8");
            if "ADC Values" in s:
                rawList = re.findall("\d{1,4}",s);
                return rawList;
                    
    def checkRaw(self):
        print("RAW values: ",end="");
        rawList = self.raw();
        goodValue = 0;
        for i in rawList:
            if (int(i) > 0):
                goodValue += 1;
                
        # Raw command has 7 ADC values that are not supposed to be 0
        assert (goodValue >= 7), rawList; 
                
        if(self.debug):
            print(rawList);
        print("OK");
            
    # Current
    def current(self, val=None):
        print("Current: ",end="");
        
        if(val):
            cmdStr=val+"\n";
            self.ser.write(b'CID='+str.encode(cmdStr));
        else:
            self.ser.write(b'CID\n');
    
        c = self.parseRsp("[+-]?\d+.\d+");
        return c;
    
    def checkCurrent(self):
        expList = self.current();
        self.chechkRsp(expList, 0.00, 5.00);
    
    # DRO        
    def DROTuneVolts(self):
        self.ser.write(b'CTV\n');
        expList = self.parseRsp("\d+.\d+");
        dro = float(expList[0]);
        if(dro != -1):
            if(self.debug):
                print("%s VDC"%dro);
            return dro;
            
    def checkDROTuneVolts(self):
        print("DRO Tune volts: ",end="");
        droVolts = self.DROTuneVolts();
        assert (droVolts > 3.00); # is DRO more than 0 OK? Shouldn't it be more?
        print("OK");
    
    #      Factory DRO Record
    def DROrecord(self):
        self.ser.write(b'DRO\n');
        expList = self.parseRsp("\d+.\d+");
        
        droMin      = float(expList[0]);
        droMax      = float(expList[1]);
        droOriginal = float(expList[2]);
        
        droList = [droMin, droMax, droOriginal];
        if(self.debug):
            print("DRO: min=%s, max=%s, original=%s"%(droMin, droMax, droOriginal));
        return droList;
    
    def checkDRORecord(self):
        droList = self.DROrecord();
        droMin      = droList[0];
        droMax      = droList[1];
        droOriginal = droList[2];
        assert (droMin >= 0) and (droMax < 5) and (droOriginal > 3);
    
    def setDRORecord(self, min, max, original):
        self.ser.write(b'dro='+min+","+max+","+original+"\n");
        droRecordStatus = self.parseRsp("\d+.\d+");
        print(droRecordStatus);
        
    # Voltage    
    def powerSupplyVoltage(self):
        self.ser.write(b'CVD\n');
        expList = self.parseRsp("AC|DC|\d+.\d+");
        supplyType = expList[0];
        supplyValue = expList[1];
        if(supplyType == 'AC'):
            if(self.debug):
                print("%-30s %s VAC"%("Power Supply Volts",supplyValue));
        elif(supplyType == 'DC'):
            if(self.debug):
                print("%-30s %s VDC"%("Power Supply Volts",supplyValue));
        
        return expList;
    
    def checkPowerSupplyVoltage(self):
        print("Power supply Volts: ",end="");
        expList = self.powerSupplyVoltage();
        supplyType = expList[0];
        supplyValue = float(expList[1]);
        if(supplyType == "AC"):
            assert ((supplyValue > 105) and (supplyValue <= 115));
        elif(supplyType == "DC"):
            assert ((supplyValue > 0) and (supplyValue <= 15));
        print("OK");
                
    def __del__(self):
        self.ser.close()
