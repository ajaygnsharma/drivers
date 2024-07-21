import serial
import time
import platform
import re
import sys
import tkinter as tk
from sensors import Sensors
from calibration import Calibration

class CCF(object):
    FINALIZE_MAGIC_NUMBER = "CCF_CAL";
    MAX_RANGE=11;

    def __init__(self, txtBox, port):
        self.txtBox = txtBox;
        self.port   = port;
        self.ser    = serial.Serial(self.port, 115200, timeout=0.15);
        self.logFile= open("./run.log", "w", encoding="utf8");

    #--------------------------------------------------------------------------
    # CCF Sample Cal
    #--------------------------------------------------------------------------
    def _CCF(self, idx=None, raw=None):
        if( (idx!=None) and (raw!=None) ):
            cmd = ("CCF=%u,%d\n"%(idx,raw));
            bCmd = str.encode(cmd);
            self.ser.write(bCmd);
        else:
            self.ser.write(b'CCF\n');
        lines = self.ser.readlines();
        return lines;

    def run(self):
        CCFList = (111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121);

        # Init
        c = Calibration.Calibration(self.txtBox, self.port);
        c.tpb('0', '4095');
        c.tpb('1', '4095');

        c.tryTemperatureComp();

        i = 0;
        for ccf in CCFList:
            print(self._CCF(i, ccf));
            i += 1;

        print(c.ttc(self.FINALIZE_MAGIC_NUMBER));  # Store it

    def verify(self):
        CCFList = (111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121);
        DEBUG = False;

        ccfOut = self._CCF();
        ccfList = re.findall("\d{2,4}", ccfOut[1].decode("utf-8"));
        assert ccfList[0] > 0, ccfList;

        print("CCF Verified");

    def __del__(self):
        self.logFile.close();
