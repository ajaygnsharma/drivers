'''
Created on Dec 12, 2019

@author: asharma
'''
import serial
import platform
import re
import random
import tkinter as tk

''' Covers
TAH
TAL
TAS 

TTC - is covered under calibration. Its usually not done here.
'''
class Transmitter(object):
    def __init__(self, txtBox, port, debug: str=None):
        self.debug = 0;
        if(debug):
            self.debug = 1;
        if(txtBox):
            self.txtBox = txtBox;

        self.ser = serial.Serial(port, 115200, timeout=1);
        #self.test1();
        
    def test1(self):
        '''self.outputLowThreshold();
        self.outputHighThreshold();
        self.inputHighThreshold();
        self.inputLowThreshold();
        self.inputPowerLevel();
        self.outputPowerLevel();
        self.frequencyBand();
        self.spectralInversion();
        self.powerMonitorFreq();
        self.gainControlMode();
        self.gainTargetLevel();
        self.gainControlReset();
        self.powerMonitorType();
        self.gainAttenuator();
        self.powerUpDelay();
        self.powerUpState();
        self.outputBurstThreshold();
        self.state();
        self.temperatureCompensation();
        self.temperatureShutdown();
        '''
        self.loID();
        '''
        self.outputBurstThreshold();
        
        '''
    def stress_test_log(self):
        # Init. Min Gain 
        for i in range(1, 20000):
            frandomVal = random.uniform(1, 30);
            randomVal = str(int(frandomVal));
            self.gainAttenuator(randomVal); # VVA Attenuators GS
        
    def checkThresholds(self, negative=None):
        threshold = 0;
        lines = self.ser.readlines();
        for l in lines:
            x = re.findall("[+-]?\d+.\d+", l.decode("utf-8"));
            if(len(x) > 0):
                threshold = float(x[0]);
            
        if(self.debug):
            print("%d"%threshold);
        else:
            if(negative):
                assert (threshold < 0), lines;
            else:
                assert (threshold > 0), lines;
            print("OK");
    
    def printLines(self, cmd):
        lines = self.ser.readlines();
        for l in lines:
            self.txtBox.insert(tk.END, l.decode("utf-8"));
        
    def checkVal(self, flag=None):
        val = 0;
        lines = self.ser.readlines();
        for l in lines:
            x = re.findall("[+-]?\d+", l.decode("utf-8"));
            if(len(x) > 0):
                val = int(x[0]);
            
        if(self.debug):
            self.txtBox.insert(tk.END,str(val));
            return val;
        else:
            if(flag=="negative"):
                assert (val < 0), lines;
            elif(flag=="zero_OK"):
                assert (val >= 0),lines;
            else:
                assert (val > 0), lines;
            self.txtBox.insert(tk.END,"OK");
            
    def outputHighThreshold(self, val=None):
        print("Output High Threshold: ",end="");
        if(val):
            self.ser.write(b'TAH='+str.encode(val)+b'\n');
        else:
            self.ser.write(b'TAH\n');
        self.checkThresholds();
        
    def outputLowThreshold(self, val=None):
        print("Output Low Threshold: ",end=""); 
        if(val):
            self.ser.write(b'TAL='+str.encode(val)+b'\n');
        else:
            self.ser.write(b'TAL\n');
        self.checkThresholds();
    
    def inputHighThreshold(self, val=None):
        print("Input High Threshold: ", end="");
        if(val):
            self.ser.write(b'TBH='+str.encode(val)+b'\n');
        else:
            self.ser.write(b'TBH\n');
        self.checkThresholds("negative");
        
    def inputLowThreshold(self, val=None):
        print("Input Low Threshold: ", end="");
        if(val):
            self.ser.write(b'TBL='+str.encode(val)+b'\n');
        else:
            self.ser.write(b'TBL\n');
        self.checkThresholds(1);
    
    def inputPowerLevel(self, val=None):
        print("Input Power Level: ", end="");
        if(val):
            self.ser.write(b'TDT='+str.encode(val)+b'\n');
        else:
            self.ser.write(b'TDT\n');
        self.checkThresholds(1);

    def outputPowerLevel(self, val=None):
        print("Output Power Level: ", end="");
        if(val):
            self.ser.write(b'TPO='+str.encode(val)+b'\n');
        else:
            self.ser.write(b'TPO\n');
        self.checkThresholds();
    
    def frequencyBand(self, val=None):
        print("Frequency Band: ", end="");
        if(val):
            self.ser.write(b'TFB='+str.encode(val)+b'\n');
        else:
            self.ser.write(b'TFB\n');
        
        self.checkVal();    
        
    def spectralInversion(self, val=None):
        print("Spectral Inversion: ", end="");
        if(val):
            self.ser.write(b'TFI='+str.encode(val)+b'\n');
        else:    
            self.ser.write(b'TFI\n');
        self.checkVal();
        
    ''' TFR '''
    def powerMonitorFreq(self, val=None):
        self.txtBox.insert(tk.END,"Power Monitor Freq: ");
        if(val):
            self.ser.write(b'TFR='+str.encode(val)+b'\n');
        else:
            self.ser.write(b'TFR\n');
        self.checkVal();
        
    def gainControlMode(self, val=None):
        print("Gain control mode: ", end="");
        cmd = "";
        if(val):
            cmd = b'TGC='+str.encode(val)+b'\n'
        else:
            cmd = b'TGC\n';
        
        self.ser.write(cmd);
        self.printLines(cmd);








    def gainTargetLevel(self, val=None):
        print("Gain level target: ", end="");
        if(val):
            self.ser.write(b'TGL='+str.encode(val)+b'\n');
        else:
            self.ser.write(b'TGL\n');
            
        lines = self.ser.readlines();
        for l in lines:
            x = re.findall("[+-]?\d+.\d+", l.decode("utf-8"));
            if(len(x) > 1):
                if(self.debug):
                    print("Gain Target=%s"%x[0],end="");
                    print(" Level Target=%s"%x[1]);
                else:
                    
                    assert ((float(x[0]) > 0) and (float(x[1]) > 0));
		
    def gainControlReset(self, val=None):
        print("Gain Control Reset: ",end="");
        if(val):
            self.ser.write(b'TGR='+str.encode(val)+b'\n');
        else:
            self.ser.write(b'TGR\n');
        self.checkVal("zero_OK");
        
    def powerMonitorType(self, val=None):
        print("Power Monitor: ",end="");
        if(val):
            print("Set ",end="");
            self.ser.write(b'TPM='+str.encode(val)+b'\n');
        else:
            self.ser.write(b'TPM\n');
        self.checkVal("zero_OK");
    
    def gainAttenuator(self, val=None):
        print("Gain Attenuator: ",end="");
        if(val):
            cmd=b'TPT='+str.encode(val)+b'\n';
        else:
            cmd=b'TPT\n';
        self.ser.write(cmd);
        self.checkVal("zero_OK");
         
    def powerUpDelay(self, val=None):
        print("Power Up Transmitter Delay: ",end="");
        cmd='';
        if(val):
            cmd="TSD="+val+"\n";
        else:
            cmd="TSD\n";
        self.ser.write(cmd.encode());
        self.checkVal("zero_OK");
        
    def powerUpState(self, val=None):
        print("Power Up Transmit State: ",end="");
        cmd="";
        if(val):
            cmd="TSP="+val+"\n";
        else:
            cmd="TSP\n";
        self.ser.write(cmd.encode());
        self.checkVal("zero_OK");
    
    def outputBurstThreshold(self, val=None):
        print("Output Burst Threshold: ",end="");
        if(val):
            self.ser.write(b'TBT='+str.encode(val)+b'\n');
        else:
            self.ser.write(b'TBT\n');
        self.checkVal("zero_OK");    
    
    def state(self, val=None):
        print("TX State: ",end="");
        cmd="";
        if(val):
            cmd='TST='+val+'\n';
        else:
            cmd='TST\n';
        self.ser.write(cmd.encode());
        self.checkVal("zero_OK");
    
    def temperatureCompensation(self, val=None):
        print("Temperature Compensation: ",end="");
        cmd="";
        if(val):
            cmd='TTC='+val+'\n';
        else:
            cmd='TTC\n';
        self.ser.write(cmd.encode());
        self.checkVal();
    
    def temperatureShutdown(self, val=None):
        print("High temperature shutdown: ",end="");
        cmd="";
        if(val):
            cmd='TTS='+val+'\n';
        else:
            cmd='TTS\n';
        self.ser.write(cmd.encode());
        self.checkVal("zero_OK");
       
    def loID(self, val=None):
        print("LO Id: ", end="");
        if(val):
            self.ser.write(b'TFB='+str.encode(val)+b'\n');
        else:
            self.ser.write(b'TFB\n');
        self.checkVal("zero_OK");
            
    def getTXHours(self):
        self.ser.write(b'TTT\n')
        lines = self.ser.readlines()
        for l in lines:
            x = re.findall("[+]?\d+", l.decode("utf-8") )
            if(len(x) > 0):
                return int(x[0]);
    
    def checkTXHours(self):
        hours = self.getTXHours();
        assert (hours >= 0);
        print("Total Transmit Hours=%s"%hours)
    
    def getBurstThreshold(self, verbose=None):
        self.ser.write(b'CBT\n');
        lines = self.ser.readlines();
        for l in lines:
            expList = re.findall("\d+", l.decode("utf-8") );
            if(len(expList) > 0):
                return int(expList[0]);
                
    def checkBurstThreshold(self):
        print("Burst Threshold: ", end="");
        burstThreshold = self.getBurstThreshold();
        assert (burstThreshold >= 0);
        print("OK");
            
    # Bear in mind that the CBT=Number followed by String will convert
    # the number and ignore the string. For example cbt=3fdfdf will make cbt=3
    def setBurstThreshold(self, val=None):
        cmdStr = '';
        if( val != None ):
            cmdStr += val;
        
        cmdStr += '\n';
        self.ser.write(b'CBT='+str.encode(cmdStr));
        lines = self.ser.readlines();
        ok = False;
        for l in lines:
            s = l.decode("utf-8");
            if("invalid value" in s):
                return -1;
            elif("CBT=" in s):
                return 0;
    
    def __del__(self):
        self.ser.close()
