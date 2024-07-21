'''
Created on Dec 12, 2019

@author: asharma
'''
import serial
import platform
import re
import random
import tkinter as tk
from stat import S_ISCHR
import os, sys

from Login import Login

''' Covers
TAH
TAL
TAS 

TTC - is covered under calibration. Its usually not done here.
'''
class Transmitter(object):
    def __init__(self, port, logFile, debug: str=None):
        self.debug = 0;
        if(debug):
            self.debug = 1;

        self.f = open(logFile, "wt");
        self.ser = serial.Serial(port, 115200, timeout=1);
        #self.test1();

    def _getOutput(self, pattern, negative=None):
        lines = self.ser.readlines();
        for l in lines:
            x = re.findall(pattern, l.decode("utf-8"));
            if(len(x) > 0):
                return x[0];

        return "None";

    def runCmd(self, i, val=None):
        _list = [('TAH', '[+-]?\d+.\d+'),
                 ('TAL', '[+-]?\d+.\d+'),
                 ('TBH', '[+-]?\d+.\d+'),
                 ('TBL', '[+-]?\d+.\d+'),
                 ('TDT', '[+-]?\d+.\d+'),
                 ('TPO', '[+-]?\d+.\d+'),
                 ('TFB', '[+-]?\d+'),
                 ('TFI', '[+-]?\d+'),
                 ('TFR', '[+-]?\d+'),
                 ('TGC', '[+-]?\d+')];

        if(val):
            cmd = _list[i][0]+'='+val+'\n';
        else:
            cmd = _list[i][0] + '\n';

        self.ser.write(str.encode(cmd));
        return self._getOutput(_list[i][1]);

    def outputHighThreshold(self, val=None):
        return self.runCmd(0, val);

    def outputLowThreshold(self, val=None):
        return self.runCmd(1, val);

    def inputHighThreshold(self, val=None):
        return self.runCmd(2, val);

    def inputLowThreshold(self, val=None):
        return self.runCmd(3, val);

    def inputPowerLevel(self, val=None):
        return self.runCmd(4, val);

    def outputPowerLevel(self, val=None):
        return self.runCmd(5, val);

    def frequencyBand(self, val=None):
        return self.runCmd(6, val);

    def spectralInversion(self, val=None):
        return self.runCmd(7, val);

    def powerMonitorFrequency(self, val=None):
        return self.runCmd(8, val);

    def gainControlMode(self, val=None):
        return self.runCmd(9, val);


if __name__ == '__main__':
    class app():
        def checkfile(self, fileName):
            try:
                mode = os.lstat(fileName).st_mode;
                if S_ISCHR(mode):
                    # It's a character special file, call the callback function
                    return True;
            except OSError:
                print('Skipping %s' % fileName);
                return False;

        def fillSerialPorts(self):
            serialPorts = ("/dev/ttyS0", "/dev/ttyUSB0", "/dev/ttyUSB1");
            for s in serialPorts:
                if self.checkfile(s):
                    self.listBoxPort.insert('end', s);

        def _getCmdGetInternal(self):
            pass;

        def serialPortMsgWindow(self):
            serialPort = tk.StringVar();
            self.listBoxPort = tk.Listbox(master=self.window,
                                          listvariable=serialPort);
            self.listBoxPort.grid(row=1, column=0);
            self.fillSerialPorts();

            self.txtBox = tk.Text(master=self.window, height=20, width=40);
            self.txtBox.grid(row=2, column=0, sticky="e");

        def _run(self, cmd, i, val):
            idx = self.listBoxPort.curselection();
            port = self.listBoxPort.get(idx);

            Login(self.txtBox, port);
            self.t = Transmitter(port, "run.log");

            listCmd = {'outHighTh'  : self.t.outputHighThreshold,
                       'outLowTh'   : self.t.outputLowThreshold,
                       'inHighTh'   : self.t.inputHighThreshold,
                       'inLowTh'    : self.t.inputLowThreshold,
                       'inPowerLvl' : self.t.inputPowerLevel,
                       'outPowerLvl': self.t.outputPowerLevel,
                       'freqBand'   : self.t.frequencyBand,
                       'spectralInv': self.t.spectralInversion,
                       'pwrMonFreq' : self.t.powerMonitorFrequency,
                       'gainCtrlMod': self.t.gainControlMode,
                       };

            outStr = listCmd[cmd](val);
            self.txtBoxList[i].insert(tk.END, outStr);

        # If set cmd is issued.
        def _runSet(self, cmd, i):
            val = self.txtBoxList[i].get("1.0", tk.END);
            listCmd = {'outHighTh': self.t.outputHighThreshold,
                       'outLowTh':  self.t.outputLowThreshold,
                       'inHighTh':
                       'inLowTh'};

            outStr = listCmd[cmd](val);
            self.txtBoxList[i].insert(tk.END, outStr);


        def outputHighThreshold(self, val=None):
            self._run('outHighTh', 0, val);

        def outputLowThreshold(self, val=None):
            self._run('outLowTh', 1, val);

        def inputHighThreshold(self, val=None):
            self._run('inHighTh', 2, val);

        def inputLowThreshold(self, val=None):
            self._run('inLowTh', 3, val);

        def inputPowerLevel(self, val=None):
            self._run('inPowerLvl', 4, val);

        def outputPowerLevel(self, val=None):
            self._run('outPowerLvl', 5, val);

        def frequencyBand(self, val=None):
            self._run('freqBand', 6, val);

        def spectralInversion(self, val=None):
            self._run('spectralInv', 7, val);

        def powerMonitorFrequency(self, val=None):
            self._run('pwrMonFreq', 8, val);

        def gainControlMode(self, val=None):
            self._run('gainCtrlMod', 9, val);

        def __init__(self, parent=None):

            self.cmd = '';

            self.window = tk.Tk();
            self.window.rowconfigure(0, minsize=100);
            self.window.columnconfigure([0, 1, 2, 3], minsize=50);
            self.frame1 = tk.Frame(master=self.window);

            self.serialPortMsgWindow();

            self.frame1.grid(row=0, column=0);

            self.txtBoxList = [];

            Cmds = [("Output High Threshold   (TAH)",  1, 0, 'dBm', 'Get', self.outputHighThreshold),
                    ("Output Low Threshold    (TAL)" , 2, 0, 'dBm', 'Get', self.outputLowThreshold),
                    ("Input High Threshold    (TBH)" , 3, 0, 'dBm', 'Get', self.inputHighThreshold),
                    ("Input Low Threshold     (TBL)" , 4, 0, 'dBm', 'Get', self.inputLowThreshold),
                    ("Input Power Level       (TDT)" , 5, 0, 'dBm', 'Get', self.inputPowerLevel),
                    ("Output Power Level      (TPO)" , 6, 0, 'dBm', 'Get', self.outputPowerLevel),
                    ("Frequency Band          (TFB)" , 7, 0, 'dBm', 'Get', self.frequencyBand),
                    ("Spectral Inversion      (TFI)" , 8, 0, 'dBm', 'Get', self.spectralInversion),
                    ("Power Monitor Frequency (TFB)" , 9, 0, 'dBm', 'Get', self.powerMonitorFrequency),
                    ("Gain Control Mode       (TGC)" ,10, 0, 'dBm', 'Get', self.gainControlMode),];


            self.cmdIdx = [0,1,2,3,4,5,6,7,8,9];

            i = 0;
            for leftLabel, rowNo, colNo, rightLabel, buttonLabel, cmd in Cmds:
                tk.Label(self.frame1, text=leftLabel).grid(row=rowNo,
                                                           column=colNo);

                txtBox = tk.Text(self.frame1, height=1, width=10);
                txtBox.grid(row=rowNo, column=(colNo + 1));
                self.txtBoxList.append(txtBox);

                tk.Label(self.frame1, text=rightLabel).grid(row=rowNo, column=(
                            colNo + 2));
                tk.Button(self.frame1, text=buttonLabel, command=cmd).grid(
                    row=rowNo, column=(colNo + 3));

                self.cmdIdx[i] = cmd;
                i = i + 1;

        def loop(self):
            self.window.mainloop();

app().loop();



