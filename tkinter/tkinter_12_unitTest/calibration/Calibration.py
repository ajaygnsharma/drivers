'''
Created on Dec 12, 2019

@author: asharma
'''
import serial
import time
import platform
import re
import sys
import tkinter as tk


class Calibration(object):
    FINALIZE_MAGIC_NUMBER = "65370";
    def __init__(self, txtBox, port=None):
        self.txtBox = txtBox;
        self.port   = port;
        self.ser    = serial.Serial(self.port, 115200, timeout=0.15);
        self.logFile = open("./run.log", "w", encoding="utf8");

    def enable(self):
        self.ser.write(b'TTC=*ENABLEDB38F\n');
        line = self.ser.read(300); #  CR CR LF
        x = re.findall(".Enabled.", line.decode("utf-8") )
        print(line);
        self.logFile.write(str(line)+'\n');

        self.txtBox.insert(tk.END, line);
        if(len(x) > 0):
            return True;
    
    # This requires enabling OPTION_GAIN_CATCHUP_TEST in ttc.cpp file.
    def disable(self):
        self.ser.write(b'TTC=*DISABLEDB38F\n');
        line = self.ser.read(300); #  CR CR LF
        x = re.findall(".Disabled.", line.decode("utf-8") )
        print(line);
        self.logFile.write(str(line)+'\n');
        if(len(x) > 0):
            return True;

    def ttc(self, line=None):
        if (line):
            self.ser.write(b'TTC=' + str.encode(line) + b'\n');
        else:
            self.ser.write(b'TTC\n');

        line = self.ser.read(300);  # CR CR LF
        return line;

    def tpb(self, index=None, val=None):
        if(index and val):
            self.ser.write(b'tpb='+str.encode(index)+b','+str.encode(val)+b'\n');
        else:
            self.ser.write(b'tpb\n');
        line = self.ser.read(300);
        return line

    ''' For some reason not implemented. '''
    def CT0(self, temp=None, power=None, level=None, raw=None):
        if(temp and power and level and raw):
            ''' Set CT0 ''' 
            cmd  = str(temp)+','+str(power)+','+str(level)+','+str(raw);
            bCmd = str.encode(cmd);
            self.ser.write(b'ct0='+bCmd+b'\n');
        else: # All ct1
            self.ser.write(b'ct0\n');
        line = self.ser.read(300);
        return line;


    # --------------------------------------------------------------------------
    # CT2 Input Power Calibration
    #
    # One can check the values from the following commands at the unit.
    # od -i -j 4672 -N 800 /config/ibuc_cal.dat
    # --------------------------------------------------------------------------
    def saveRawInputPower(self,freq, temperature, power, CT2_VAL):
        rawStr = self.raw();
        rawList = re.findall("\d+", rawStr.decode("utf-8"));
        if (len(rawList) > 0):
            # Write CT1 value
            lines=self.CT2(freq, temperature, power, CT2_VAL);
            print(lines);
            for l in lines:
                self.logFile.write(str(l)+'\n');

    def maxGain(self):
        self.tpb('0', '4095');
        self.tpb('1', '4095');

    def stop(self):
        self.ser.close()


    # Temperature compensation begins with writing some value to the
    # using TTC and then reading it back and then
    def tryTemperatureComp(self):
        ttcVal = 3072
        self.ttc(str(ttcVal));  # First value

        for i in range(0, 10):
            ttcVal = ttcVal - 1;
            self.ttc(str(ttcVal));  # First value

        line = self.ttc();
        ttcVal = re.findall("\d+", line.decode("utf-8"));

    def __del__(self):
        self.logFile.close();


