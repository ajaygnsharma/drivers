import serial
import time
import platform
import re
import sys
import tkinter as tk
from sensors import Sensors
from calibration import Calibration

class CT9(object):
    FINALIZE_MAGIC_NUMBER = "65370";
    FULL_RUN = False;
    if(FULL_RUN):
        MAX_FREQUENCY = 3;#3;
        MAX_TEMPERTURE= 6;#6;
        MAX_LEVEL     = 11;#11;
    else:
        MAX_FREQUENCY = 2;#3;
        MAX_TEMPERTURE= 2;#6;
        MAX_LEVEL     = 2;#11;

    def __init__(self, txtBox, port):
        self.txtBox = txtBox;
        self.port   = port;
        self.ser    = serial.Serial(self.port, 115200, timeout=0.15);
        self.logFile= open("./run.log", "w", encoding="utf8");

    #--------------------------------------------------------------------------
    # CT9 command as such
    #--------------------------------------------------------------------------
    def _CT9(self, freq=None, temp=None, power=None, dacV=None):
        if((freq!=None) and (temp!=None) and (power!=None) and (dacV!=None)):
            cmd  = ("CT9=%d,%d,%d,%d\n"%(freq,temp,power,dacV));
            bCmd = str.encode(cmd);
            self.ser.write(bCmd);
        elif((freq!=None) and (temp!=None) and (power!=None)):
            cmd  = ("CT9=%d,%d,%d\n"%(freq,temp,power));
            bCmd = str.encode(cmd);
            self.ser.write(bCmd);
        else:
            assert False, "CT9 does not have get \n";
        lines = self.ser.readlines();
        return lines;

    # 1. First issue command to reduce volage attenuation to 0.
    # 2. After that set temperature attenuation to a random number.
    #    In reality it accepts the TTC value that gets the desired output power.
    # 3. After that value of TTC is set to the CT3 value for an index of
    #    0 to 5 corresponding to -40 to 40 Deg C.
    def run(self):
        CT9Val = 901;

        # Init
        c = Calibration.Calibration(self.txtBox, self.port);
        c.tpb('0', '4095');
        c.tpb('1', '4095');
        c.ttc("3000");  # Temp Comp Attenuator

        for freq in range(0, self.MAX_FREQUENCY):
            for temperature in range(0, self.MAX_TEMPERTURE):
                for power in range(0, self.MAX_LEVEL):
                    print(self._CT9(freq, temperature, power, CT9Val));
                    CT9Val = CT9Val + 1;

        print(c.ttc(self.FINALIZE_MAGIC_NUMBER));

    def verify(self):
        CT9Val = 901;
        DEBUG = False;

        for freq in range(0, self.MAX_FREQUENCY):
            for temperature in range(0, self.MAX_TEMPERTURE):
                for power in range(0, self.MAX_LEVEL):
                    ct9Out = self._CT9(freq, temperature, power);
                    v = re.findall("\d{2,4}",ct9Out[1].decode("utf-8"));
                    assert int(v[0]) == CT9Val, v;
                    CT9Val  = CT9Val + 1;

        print("CT9 Verified");

    def __del__(self):
        self.logFile.close();
