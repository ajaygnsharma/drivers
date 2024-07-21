import serial
import time
import platform
import re
import sys
import tkinter as tk
from sensors import Sensors
from calibration import Calibration
from tenMHz import TenMHz

class CT0(object):
    FINALIZE_MAGIC_NUMBER = "65370";
    MAX_TEMPERTURE= 6;#6;
    MAX_POWER     = 11;#11;

    def __init__(self, txtBox, port):
        self.txtBox = txtBox;
        self.port  = port;
        self.ser = serial.Serial(self.port, 115200, timeout=0.15);
        self.logFile = open("./run.log", "w", encoding="utf8");

    # --------------------------------------------------------------------------
    # CT0 10MHz level
    # --------------------------------------------------------------------------
    def _CT0(self, temp=None, power=None, level=None, raw=None):
        if ((temp!=None) and (power!=None) and (level!=None) and (raw!=None)):
            cmd = ("CT0=%u,%u,%f,%u\n" % (temp, power, level, raw));
            bCmd = str.encode(cmd);
            self.ser.write(bCmd);
        else:  # All ct1
            assert False, "Can't read CT0\n";
        lines = self.ser.readlines();
        return lines;

    # od -x -j 7872 -N 800 /config/ibuc_cal.dat
    # 0017300 0000 3f80 0001 0000 0000 4000 0002 0000
    # 0017320 0000 0000 0000 0000 0000 0000 0000 0000
    # convert hex to float : 3f800000 to float gives 1
    #
    # use od -f -w 2 -j 7872 -N 800 /config/ibuc_cal.dat
    # also to get the result. 1, 2, 3, 4 and so on to 65, 66
    # 0017300   1.0000000e+00 .. 2.0000000e+00 .. 3.0000000e+00 .. 4.0000000e+00
    # ...
    # 6.5000000e+01 ..  6.6000000e+01 ..   -nan            -nan
    def run(self):
        TPB_WRITE = False;

        CT0_VAL = 1;

        for temperature in range(0, self.MAX_TEMPERTURE):
            for power in range(0, self.MAX_POWER):
                print(self._CT0(temperature, power, CT0_VAL, CT0_VAL));
                CT0_VAL = CT0_VAL + 1;

        t = TenMHz.TenMHz(self.port, "1");
        t.setValid10MHZCal();
        c = Calibration.Calibration(self.txtBox, self.port);
        line = c.ttc(self.FINALIZE_MAGIC_NUMBER);

        print(line);  # save calibration table to the sd card.
        self.logFile.write(str(line) + '\n');

    def __del__(self):
        self.logFile.close();
