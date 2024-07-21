import serial
import time
import platform
import re
import sys
import tkinter as tk
from sensors import Sensors
from calibration import Calibration

class CT4(object):
    FINALIZE_MAGIC_NUMBER = "65370";
    MAX_TEMPERTURE= 6;#6;

    def __init__(self, txtBox, port):
        self.txtBox = txtBox;
        self.port   = port;
        self.ser    = serial.Serial(self.port, 115200, timeout=0.15);
        self.logFile= open("./run.log", "w", encoding="utf8");

    #--------------------------------------------------------------------------
    # CT4 Outside temperature Calibration
    #--------------------------------------------------------------------------
    def _CT4(self, temp=None, raw=None):
        if( (temp!=None) and (raw!=None) ):
            cmd = ("CT4=%d,%d\n"%(temp,raw));
            bCmd = str.encode(cmd);
            self.ser.write(bCmd);
        else:
            self.ser.write(b'ct4\n');
        lines = self.ser.readlines();
        return lines;

    # 1. First issue command to reduce volage attenuation to 0.
    # 2. After that set temperature attenuation to a random number.
    #    In reality it accepts the TTC value that gets the desired output power.
    # 3. After that value of TTC is set to the CT3 value for an index of
    #    0 to 5 corresponding to -40 to 40 Deg C.
    def run(self):
        CT4List = (401, 402, 403, 404, 405, 406);  # I want to write known numbers.

        # Init
        c = Calibration.Calibration(self.txtBox, self.port);
        c.tpb('0', '4095');
        c.tpb('1', '4095');

        c.tryTemperatureComp();

        for i in range(0,self.MAX_TEMPERTURE):
            print(self._CT4(i, CT4List[i]));

        print(c.ttc(self.FINALIZE_MAGIC_NUMBER));  # Store it

    def verify(self):
        CT4TestData = (401, 402, 403, 404, 405, 406);
        DEBUG = False;

        ct4Out = self._CT4();
        ct4List = re.findall("\d{2,4}", ct4Out[1].decode("utf-8"));

        i = 0;
        for v in ct4List:
            assert int(v) == CT4TestData[i], v;
            i = i + 1;

        print("CT4 Verified");

    def __del__(self):
        self.logFile.close();
