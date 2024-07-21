import serial
import time
import platform
import re
import sys
import tkinter as tk
from sensors import Sensors
from calibration import Calibration

class CT1(object):
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
    # CT1_OutputPowerCalibration
    # --------------------------------------------------------------------------
    def _CT1(self, freq=None, temp=None, pwr=None, raw=None):
        if ((freq != None) and (temp != None) and (pwr != None) and (
                raw != None)):  # Set CT1
            cmd = ("CT1=%d,%d,%d,%d\n" % (freq, temp, pwr, raw));
            bCmd = str.encode(cmd);
            self.ser.write(bCmd);
        elif ((freq != None) and (temp != None) and (
                pwr != None)):  # Get a specific freq, temp and pwr
            cmd = ("CT1=%d,%d,%d\n" % (freq, temp, pwr));
            bCmd = str.encode(cmd);
            self.ser.write(bCmd);
        else:  # All ct1
            self.ser.write(b'CT1\n');
        lines = self.ser.readlines();
        return lines;

    # --------------------------------------------------------------------------
    # CT1_OutputPowerCalibration
    #
    # One can check the values from the following commands at the unit.
    # od -i -j 3200 -N 800 /config/ibuc_cal.dat
    # --------------------------------------------------------------------------
    def _saveRawOutputPower(self, freq, temperature, power, CT1_VAL):
        s = Sensors.Sensors(self.port, "1");
        rawList = s.raw();
        if (len(rawList) > 0):
            # Write CT1 value
            lines = self._CT1(freq, temperature, power, CT1_VAL);
            print(lines);
            for l in lines:
                self.logFile.write(str(l) + '\n');

    def run(self):
        TPB_WRITE = False;

        CT1_VAL = 101;

        for freq in range(0, self.MAX_FREQUENCY):
            for temperature in range(0, self.MAX_TEMPERTURE):
                for power in range(0, self.MAX_POWER):
                    if (TPB_WRITE):
                        self.maxGain();
                        self.adjTemperatureCompensation(temperature);

                    self._saveRawOutputPower(freq, temperature, power,
                                            CT1_VAL);
                    CT1_VAL = CT1_VAL + 1;

        c = Calibration.Calibration(self.txtBox, self.port);
        line = c.ttc(self.FINALIZE_MAGIC_NUMBER)
        print(line);  # save calibration table to the sd card.
        self.logFile.write(str(line) + '\n');
        self._verify();

    def _verify(self):
        CT1_VAL = 101;
        DEBUG = False;

        for freq in range(0, self.MAX_FREQUENCY):
            for temperature in range(0, self.MAX_TEMPERTURE):
                for power in range(0, self.MAX_POWER):
                    # Below should output ct1 value as 101... and so on.
                    ct1Out = self._CT1(freq, temperature, power);
                    ct1 = re.findall("\d{2,4}", ct1Out[1].decode("utf-8"));
                    if (DEBUG):
                        print(ct1);
                        for c in ct1:
                            self.logFile.write(str(c) + '\n');
                    assert int(ct1[0]) == CT1_VAL;
                    CT1_VAL = CT1_VAL + 1;


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
