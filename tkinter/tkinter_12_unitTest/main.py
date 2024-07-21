'''
Created on Dec 6, 2019

@author: asharma
'''
import utFifo
import utCalibration
from temperature import Temperature
import Login

import sys
import time
import platform
import getopt
import tkinter as tk
from tkinter import ttk as ttk

import threading as thr
import os, sys
from stat import S_ISCHR


class UnitTest(object):
    def __init__(self):
        self.testName = "None";

        self.window = tk.Tk();

        # Contains two widgets: textwindow, button to start the test.
        self.fifoTest = tk.IntVar();
        '''
        self.test3 = tk.IntVar();
        self.test4 = tk.IntVar();
        self.test5 = tk.IntVar();
        self.test6 = tk.IntVar();
        self.test7 = tk.IntVar();
        self.test8 = tk.IntVar();
        self.test9 = tk.IntVar();
        '''

    def createConsoleOut(self):
        self.txtBox = tk.Text(self.window, height=10, width=80);
        self.txtBox.pack();
        self.txtBox.tag_config('warning', background="yellow",
                               foreground="red");
        self.txtBox.tag_config('ok', background="white",
                               foreground="green");

    def createTabs(self):
        self.tabFrame = ttk.Notebook(self.window);

        self.fifoTab = ttk.Frame(self.tabFrame);
        self.tabFrame.add(self.fifoTab, text="Fifo Test");

        self.calibrationTab = ttk.Frame(self.tabFrame);
        self.tabFrame.add(self.calibrationTab, text="Calibration");

        self.temperatureTab = ttk.Frame(self.tabFrame);
        self.tabFrame.add(self.temperatureTab, text="Temperature");

        self.tabFrame.pack(expand=1, fill="both");

    def createFifoCheckBoxes(self):
        c1 = tk.Checkbutton(self.fifoTab,
                            text='Fifo',
                            variable=self.fifoTest,
                            onvalue=1,
                            offvalue=0,
                            command=self.selectTest);
        c1.pack();

    def addCalibrationChxBox(self):
        self.ct0Test = tk.IntVar();
        c0 = tk.Checkbutton(self.calibrationTab,
                            text='CT0 (10MHz)',
                            variable=self.ct0Test,
                            onvalue=1,
                            offvalue=0,
                            command=self.selectTest);
        c0.pack();

        self.ct1Test = tk.IntVar();
        c1 = tk.Checkbutton(self.calibrationTab,
                            text='CT1',
                            variable=self.ct1Test,
                            onvalue=1,
                            offvalue=0,
                            command=self.selectTest);
        c1.pack();

        self.ct2Test = tk.IntVar();
        c2 = tk.Checkbutton(self.calibrationTab,
                            text='CT2',
                            variable=self.ct2Test,
                            onvalue=1,
                            offvalue=0,
                            command=self.selectTest);
        c2.pack();

        self.ct3Test = tk.IntVar();
        c3 = tk.Checkbutton(self.calibrationTab,
                            text='CT3',
                            variable=self.ct3Test,
                            onvalue=1,
                            offvalue=0,
                            command=self.selectTest);
        c3.pack();

        self.ct4Test = tk.IntVar();
        c4 = tk.Checkbutton(self.calibrationTab,
                            text='CT4',
                            variable=self.ct4Test,
                            onvalue=1,
                            offvalue=0,
                            command=self.selectTest);
        c4.pack();

        self.ct5Test = tk.IntVar();
        c5 = tk.Checkbutton(self.calibrationTab,
                            text='CT5',
                            variable=self.ct5Test,
                            onvalue=1,
                            offvalue=0,
                            command=self.selectTest);
        c5.pack();

        self.ct8Test = tk.IntVar();
        c8 = tk.Checkbutton(self.calibrationTab,
                            text='CT8',
                            variable=self.ct8Test,
                            onvalue=1,
                            offvalue=0,
                            command=self.selectTest);
        c8.pack();

        self.ct9Test = tk.IntVar();
        c9 = tk.Checkbutton(self.calibrationTab,
                            text='CT9',
                            variable=self.ct9Test,
                            onvalue=1,
                            offvalue=0,
                            command=self.selectTest);
        c9.pack();

        self.ccfTest = tk.IntVar();
        ccf = tk.Checkbutton(self.calibrationTab,
                            text='CCF (Sample Cal Table)',
                            variable=self.ccfTest,
                            onvalue=1,
                            offvalue=0,
                            command=self.selectTest);
        ccf.pack();


    def createSerialPortList(self):
        # ListBox for Serial Ports List
        self.serialPort = tk.StringVar();
        self.listBoxPort = tk.Listbox(self.window, listvariable=self.serialPort);
        self.listBoxPort.pack();

    def temperatureFrame(self):
        t = Temperature.Temperature(self.temperatureTab,
                                    self.listBoxPort,
                                    self.txtBox,
                                    "./out.log");
        t.entryATL();
        t.entryCOX();


    def run(self):
        # use threading
        self.threading();

    def threading(self):
        # Call work function
        t1 = thr.Thread(target=self.work);
        t1.start();

    def runTestButton(self):
        btn = tk.Button(self.window, text="Run Test", command=self.threading);
        btn.pack();

    def selectTest(self):
        if (self.fifoTest.get() == 1):
            self.testName = "Fifo";
        elif (self.ct0Test.get() == 1):
            self.testName = "CT0";
        elif (self.ct1Test.get() == 1):
            self.testName = "CT1";
        elif (self.ct2Test.get() == 1):
            self.testName = "CT2";
        elif (self.ct3Test.get() == 1):
            self.testName = "CT3";
        elif (self.ct4Test.get() == 1):
            self.testName = "CT4";
        elif (self.ct5Test.get() == 1):
            self.testName = "CT5";
        elif (self.ct8Test.get() == 1):
            self.testName = "CT8";
        elif (self.ct9Test.get() == 1):
            self.testName = "CT9";
        elif (self.ccfTest.get() == 1):
            self.testName = "CCF";
        else:# Can't have both or none selected.
            self.testName = 'None';
        '''
        elif (self.test3.get() == 1):
            testName = "Fifo-TFR";
        elif (self.test4.get() == 1):
            testName = "Fifo-TGC";
        elif (self.test8.get() == 1):
            testName = "Fifo-TPB-CalibrationState";
        elif (self.test9.get() == 1):
            testName = "EHZ-Clear-Alarm";
        '''

    def allTestsCheckboxes(self):
        '''
        c3 = tk.Checkbutton(self.window, text='Fifo-TFR', variable=self.test3,
                            onvalue=1, offvalue=0, command=self.selectTest);
        c3.pack();
        c4 = tk.Checkbutton(self.window, text='Fifo-TGC', variable=self.test4,
                            onvalue=1, offvalue=0, command=self.selectTest);
        c4.pack();
        c5 = tk.Checkbutton(self.window, text='Fifo-TTC-EnableCal', variable=self.test5,
                            onvalue=1, offvalue=0, command=self.selectTest);
        c5.pack();
        c6 = tk.Checkbutton(self.window, text='Fifo-TPT-CCF-Cal', variable=self.test6,
                            onvalue=1, offvalue=0, command=self.selectTest);
        c6.pack();
        c7 = tk.Checkbutton(self.window, text='Fifo-TPT-Attenuate', variable=self.test7,
                            onvalue=1, offvalue=0, command=self.selectTest);
        c7.pack();
        c8 = tk.Checkbutton(self.window, text='Fifo-TPB-CalibrationState',
                            variable=self.test8, onvalue=1, offvalue=0, command=self.selectTest);
        c8.pack();
        c9 = tk.Checkbutton(self.window, text='EHZ-Clear-Alarm', variable=self.test9,
                            onvalue=1, offvalue=0, command=self.selectTest);
        c9.pack();
        '''

    def checkfile(self,fileName):
        try:
            mode = os.lstat(fileName).st_mode;
            if S_ISCHR(mode):
                # It's a character special file, call the callback function
                return True;
        except OSError:
            print('Skipping %s' % fileName);
            return False;

    # work function
    def work(self):
        print("Starting Fifo Test");

        # Clear text box first
        self.txtBox.delete("1.0", "end");

        idx = self.listBoxPort.curselection(); # The curselection() returns a Tuple
        if (len(idx) == 0):
            self.txtBox.insert(tk.END, "Select a Serial port First\n");
            return 0;
        else:
            port = self.listBoxPort.get(idx); # "/dev/ttyUSB0";

            if (self.testName == "CT0"):
                c = utCalibration.UTCalibration(self.txtBox, port);
                c.utCT0();
            elif(self.testName == "CT1"):
                c = utCalibration.UTCalibration(self.txtBox,port);
                c.utCT1();
            elif (self.testName == "CT2"):
                c = utCalibration.UTCalibration(self.txtBox, port);
                c.utCT2();
            elif (self.testName == "CT3"):
                c = utCalibration.UTCalibration(self.txtBox, port);
                c.utCT3();
            elif (self.testName == "CT4"):
                c = utCalibration.UTCalibration(self.txtBox, port);
                c.utCT4();
            elif (self.testName == "CT5"):
                c = utCalibration.UTCalibration(self.txtBox, port);
                c.utCT5();
            elif (self.testName == "CT8"):
                c = utCalibration.UTCalibration(self.txtBox, port);
                c.utCT8();
            elif (self.testName == "CT9"):
                c = utCalibration.UTCalibration(self.txtBox, port);
                c.utCT9();
            elif (self.testName == "CCF"):
                c = utCalibration.UTCalibration(self.txtBox, port);
                c.utCCF();
            else:
                f = utFifo.UTFifo(self.txtBox,self.testName,port);
                f.fifo_test();

            print("sleep time stop")


    def fillSerialPorts(self):
        serialPorts = ("/dev/ttyS0", "/dev/ttyUSB0", "/dev/ttyUSB1");
        for s in serialPorts:
            if self.checkfile(s):
                self.listBoxPort.insert('end', s);


    def run(self):
        self.window.mainloop();



if __name__ == "__main__":
    u = UnitTest();
    u.createTabs();
    u.createFifoCheckBoxes();
    u.addCalibrationChxBox();
    u.createConsoleOut()
    u.createSerialPortList();
    u.temperatureFrame();
    u.runTestButton();
    u.allTestsCheckboxes();
    u.fillSerialPorts();
    u.run();

