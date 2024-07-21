import serial
import time
import platform
import re
import sys
import tkinter as tk
from sensors import Sensors
from calibration import Calibration

class CT8(object):
    FINALIZE_MAGIC_NUMBER = "65370";
    MAX_TEMPERTURE= 6;#6;

    def __init__(self, txtBox, port):
        self.txtBox = txtBox;
        self.port   = port;
        self.ser    = serial.Serial(self.port, 115200, timeout=0.15);
        self.logFile= open("./run.log", "w", encoding="utf8");

    #--------------------------------------------------------------------------
    # CT8
    #--------------------------------------------------------------------------
    def _CT8(self, temp=None, raw=None):
        if((temp!=None) and (raw!=None)):
            cmd = ("CT8=%d,%0.2f\n"%(temp,raw));
            bCmd = str.encode(cmd);
            self.ser.write(bCmd);
        else:
            self.ser.write(b'CT8\n');
        lines = self.ser.readlines();
        return lines;

    # 1. First issue command to reduce volage attenuation to 0.
    # 2. After that set temperature attenuation to a random number.
    #    In reality it accepts the TTC value that gets the desired output power.
    # 3. After that value of TTC is set to the CT3 value for an index of
    #    0 to 5 corresponding to -40 to 40 Deg C.
    def run(self):
        CT8List = (801.0, 802.0, 803.0, 804.0, 805.0, 806.0);  # I want to write known numbers.
        # Init
        c = Calibration.Calibration(self.txtBox, self.port);
        c.tpb('0', '4095');
        c.tpb('1', '4095');
        c.ttc("3000");  # Temp Comp Attenuator

        for i in range(0, self.MAX_TEMPERTURE):
            print(self._CT8(i, CT8List[i]));

        print(c.ttc(self.FINALIZE_MAGIC_NUMBER));  # First value

    def verify(self):
        CT8TestData = (801.0, 802.0, 803.0, 804.0, 805.0, 806.0);
        DEBUG = False;

        ct8Out = self._CT8();
        ct8List = re.findall("\d{2,4}.{2,4}", ct8Out[1].decode("utf-8"));

        i = 0;
        for v in ct8List:
            assert float(v) == CT8TestData[i], v;
            i = i + 1;

        print("CT8 Verified");

    def __del__(self):
        self.logFile.close();
