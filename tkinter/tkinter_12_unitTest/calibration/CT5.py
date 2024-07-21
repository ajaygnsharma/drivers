import serial
import time
import platform
import re
import sys
import tkinter as tk
from sensors import Sensors
from calibration import Calibration

class CT5(object):
    FINALIZE_MAGIC_NUMBER = "65370";
    MAX_TEMPERTURE= 6;#6;

    def __init__(self, txtBox, port):
        self.txtBox = txtBox;
        self.port   = port;
        self.ser    = serial.Serial(self.port, 115200, timeout=0.15);
        self.logFile= open("./run.log", "w", encoding="utf8");

    #--------------------------------------------------------------------------
    # CT5 10MHz Reference Threshold Calibration
    #--------------------------------------------------------------------------
    def _CT5(self, temp=None, fault=None, clear=None):
        if((temp!=None) and (fault!=None) and (clear!=None)):
            cmd = ("CT5=%d,%d,%d\n"%(temp,fault,clear));
            bCmd = str.encode(cmd);
            self.ser.write(bCmd);
        else:
            self.ser.write(b'CT5\n');
        lines = self.ser.readlines();
        return lines;

    # 1. First issue command to reduce volage attenuation to 0.
    # 2. After that set temperature attenuation to a random number.
    #    In reality it accepts the TTC value that gets the desired output power.
    # 3. After that value of TTC is set to the CT3 value for an index of
    #    0 to 5 corresponding to -40 to 40 Deg C.
    def run(self):
        # Init
        c = Calibration.Calibration(self.txtBox, self.port);
        c.tpb('0', '4095');
        c.tpb('1', '4095');
        c.ttc("3000");  # Temp Comp Attenuator

        # Fault and Clear ADC values for 10MHz
        fault = [1000, 1001, 1002, 1003, 1004, 1005];
        clear = [2000, 2001, 2002, 2003, 2004, 2005];

        for temperature in range(0, self.MAX_TEMPERTURE):
            print(self._CT5(temperature, fault[temperature], clear[temperature]));

        print(c.ttc(self.FINALIZE_MAGIC_NUMBER));  # First value

    def verify(self):
        fault = [1000, 1001, 1002, 1003, 1004, 1005];
        clear = [2000, 2001, 2002, 2003, 2004, 2005];
        DEBUG = False;

        ct5Out = self._CT5();
        ct5List = re.findall("\d{2,4}", ct5Out[1].decode("utf-8"));

        print(ct5List);
        x = 0;
        for i in range(0, self.MAX_TEMPERTURE):
            assert int(ct5List[x])   == fault[i], ct5List[x];
            assert int(ct5List[x+1]) == clear[i], ct5List[x+1];
            x = x + 2;

        print("CT5 Verified");

    def __del__(self):
        self.logFile.close();
