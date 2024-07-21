'''
Created on Jan 29, 2020

@author: asharma
'''
import Login as login
import Logout as logout
from transmitter import Transmitter
from calibration import Calibration
from statlog import StatLog
from factory import factory
from eventlog import EventLog

import time
import re
import tkinter as tk




class UTFifo(object):
    def __init__(self, txtBox, testName, port: str = None):
        self.port = "";
        if (port):
            self.port = port;
        if(txtBox):
            self.txtBox = txtBox;
        if(testName):
            self.testName = testName;

    def _tpbCalibrationState(self):
        cal = Calibration.Calibration(self.port);

        # In this test, first donot enable the calibration mode.
        # It will print calibration not enabled.
        # After that Enable it and set it again. It should not complain this
        # time around.
        s = cal.tpb("0", "4095");
        if (b"calibration not enabled" in s):
            self.txtBox.insert(tk.END, "Calibration not enabled: OK\n");
        else:
            self.txtBox.insert(tk.END, s);

        s = cal.enable(self.txtBox);
        s = cal.tpb("0", "4095");
        if (b"calibration not enabled" in s):
            self.txtBox.insert(tk.END, "Calibration not enabled: OK");
        else:
            self.txtBox.insert(tk.END, "Calibration enabled: OK");
            self.txtBox.insert(tk.END, s);

    def _tfr(self):
        tx = Transmitter.Transmitter(self.txtBox, self.port, "1");
        tx.powerMonitorFreq("5850");

    def _tgc(self):
        tx = Transmitter.Transmitter(self.txtBox, self.port, "1");
        tx.gainControlMode("0");
        tx.gainControlMode("1");
        tx.gainControlMode("2");
        # Reset it back to normal
        tx.gainControlMode("0");

    def _eventLogClear(self):
        e = EventLog.EventLog(self.txtBox, self.port);
        e.clear();

    #--------------------------------------------------------------------------
    # Run FIFO tests
    # --------------------------------------------------------------------------
    def fifo_test(self):
        login.Login(self.txtBox, self.port);

        if (self.testName == "Fifo-TPB-CalibrationState"):
            self._tpbCalibrationState();

        if (self.testName =="Fifo-TFR"):
            self._tfr();

        if (self.testName =="Fifo-TGC"):
            self._tgc();

        if (self.testName =="EHZ-Clear-Alarm"):
            self._eventLogClear();

        logout.LogOut(self.txtBox, self.port);
