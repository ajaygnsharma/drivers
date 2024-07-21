import serial
import time
import platform
import re
import sys
import tkinter as tk
from sensors import Sensors
from calibration import Calibration

class CT2(object):
    FINALIZE_MAGIC_NUMBER = "65370";
    MAX_FREQUENCY = 2;#11;
    MAX_TEMPERTURE= 2;#6;
    MAX_POWER     = 2;#11;

    def __init__(self, txtBox, port):
        self.txtBox = txtBox;
        self.port  = port;
        self.ser = serial.Serial(self.port, 115200, timeout=0.15);
        self.logFile = open("./run.log", "w", encoding="utf8");

    # --------------------------------------------------------------------------
    # CT2_InputPowerCalibration
    # --------------------------------------------------------------------------
    def _CT2(self, freq=None, temp=None, pwr=None, raw=None):
        if ((freq != None) and (temp != None) and (pwr != None) and (
                raw != None)):  # Set CT1
            cmd = ("CT2=%d,%d,%d,%d\n" % (freq, temp, pwr, raw));
            bCmd = str.encode(cmd);
            self.ser.write(bCmd);
        elif ((freq != None) and (temp != None) and (
                pwr != None)):  # Get a specific freq, temp and pwr
            cmd = ("CT2=%d,%d,%d\n" % (freq, temp, pwr));
            bCmd = str.encode(cmd);
            self.ser.write(bCmd);
        else:
            self.ser.write(b'CT2\n');
        lines = self.ser.readlines();
        return lines;

    # --------------------------------------------------------------------------
    # CT1_OutputPowerCalibration
    #
    # One can check the values from the following commands at the unit.
    # od -i -j 3200 -N 800 /config/ibuc_cal.dat
    # --------------------------------------------------------------------------
    def _saveRawInputPower(self, freq, temperature, power, CT2_VAL):
        s = Sensors.Sensors(self.port, "1");
        rawList = s.raw();
        if (len(rawList) > 0):
            lines = self._CT2(freq, temperature, power, CT2_VAL);
            print(lines);
            for l in lines:
                self.logFile.write(str(l) + '\n');

    def run(self):
        TPB_WRITE = False;

        CT2_VAL = 201;

        for freq in range(0, self.MAX_FREQUENCY):
            for temperature in range(0, self.MAX_TEMPERTURE):
                for power in range(0, self.MAX_POWER):
                    if (TPB_WRITE):
                        self.maxGain();
                        self.adjTemperatureCompensation(temperature);

                    self._saveRawInputPower(freq, temperature, power,
                                            CT2_VAL);
                    CT2_VAL = CT2_VAL + 1;

        c = Calibration.Calibration(self.txtBox, self.port);
        line = c.ttc(self.FINALIZE_MAGIC_NUMBER)
        print(line);  # save calibration table to the sd card.
        self.logFile.write(str(line) + '\n');
        self._verify();

    def _verify(self):
        CT2_VAL = 201;
        DEBUG = False;

        for freq in range(0, self.MAX_FREQUENCY):
            for temperature in range(0, self.MAX_TEMPERTURE):
                for power in range(0, self.MAX_POWER):
                    # Below should output ct1 value as 101... and so on.
                    ct2Out = self._CT2(freq, temperature, power);
                    ct2 = re.findall("\d{2,4}", ct2Out[1].decode("utf-8"));
                    if (DEBUG):
                        print(ct2);
                        for c in ct2:
                            self.logFile.write(str(c) + '\n');
                    assert int(ct2[0]) == CT2_VAL;
                    CT2_VAL = CT2_VAL + 1;


    def maxGain(self):
        c = Calibration.Calibration(self.txtBox, self.port);
        c.tpb('0', '4095');
        c.tpb('1', '4095');

    # Take the temperature
    def adjTemperatureCompensation(self, temperature):
        ct3Str = self.CT3();
        print(ct3Str);
        self.write(ct3Str);
        ct3List = re.findall("\d+", ct3Str[1].decode("utf-8"));
        if (len(ct3List) > temperature):
            t = ct3List[temperature];
            c = Calibration.Calibration(self.txtBox, self.port);
            c.ttc(str(t));

    def __del__(self):
        self.logFile.close();
