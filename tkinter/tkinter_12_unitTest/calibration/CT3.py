import serial
import time
import platform
import re
import sys
import tkinter as tk
from sensors import Sensors
from calibration import Calibration

class CT3(object):
    FINALIZE_MAGIC_NUMBER = "65370";
    MAX_FREQUENCY = 2;#11;
    MAX_TEMPERTURE= 2;#6;
    MAX_POWER     = 2;#11;

    def __init__(self, txtBox, port):
        self.txtBox = txtBox;
        self.port   = port;
        self.ser    = serial.Serial(self.port, 115200, timeout=0.15);
        self.logFile= open("./run.log", "w", encoding="utf8");

    # Temperature compensation begins with writing some value to the
    # using TTC and then reading it back and then
    def tryTemperatureComp(self, c):
        ttcVal = 3072
        c.ttc(str(ttcVal));  # First value

        for i in range(0, 10):
            ttcVal = ttcVal - 1;
            c.ttc(str(ttcVal));  # First value

        line = c.ttc();
        ttcVal = re.findall("\d+", line.decode("utf-8"));

    # --------------------------------------------------------------------------
    # CT3 Temperature compensation Calibration
    # --------------------------------------------------------------------------
    def _CT3(self, temp=None, raw=None):
        if ((temp != None) and (raw != None)):
            cmd = ("CT3=%d,%d\n" % (temp, raw));
            bCmd = str.encode(cmd);
            self.ser.write(bCmd);
        else:
            self.ser.write(b'CT3\n');
        lines = self.ser.readlines();
        return lines;

    # 1. First issue command to reduce volage attenuation to 0.
    # 2. After that set temperature attenuation to a random number.
    #    In reality it accepts the TTC value that gets the desired output power.
    # 3. After that value of TTC is set to the CT3 value for an index of
    #    0 to 5 corresponding to -40 to 40 Deg C.
    def run(self):
        CT3List = (301, 302, 303, 304, 305, 306);  # I want to write known numbers.

        # Init
        c = Calibration.Calibration(self.txtBox, self.port);
        c.tpb('0', '4095');
        c.tpb('1', '4095');

        self.tryTemperatureComp(c);

        for i in range(0,6):
            print(self._CT3(i, CT3List[i]));

        print(c.ttc(self.FINALIZE_MAGIC_NUMBER));  # Store it

    def verify(self):
        CT3TestData = (301, 302, 303, 304, 305, 306);
        DEBUG = False;

        ct3Out = self._CT3();
        ct3List = re.findall("\d{2,4}", ct3Out[1].decode("utf-8"));

        i = 0;
        for v in ct3List:
            assert int(v) == CT3TestData[i];
            i = i + 1;

        print("CT3 Verified");

    def __del__(self):
        self.logFile.close();
