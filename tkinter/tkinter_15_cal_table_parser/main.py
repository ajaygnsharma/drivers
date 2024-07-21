'''
Created on Dec 6, 2019

@author: asharma
'''
from calTable import calTable

import sys
import time
import platform
import getopt
import tkinter as tk
from tkinter import ttk as ttk

import threading as thr
import os, sys
from stat import S_ISCHR

from calTable import Offset
from calTable import Table

class CalTableReader(object):
    def __init__(self):
        self.window = tk.Tk();

    def createTabs(self):
        self.tabFrame = ttk.Notebook(self.window);

        self.ct1Tab = ttk.Frame(self.tabFrame);
        self.tabFrame.add(self.ct1Tab, text="CT1");

        self.ct2Tab = ttk.Frame(self.tabFrame);
        self.tabFrame.add(self.ct2Tab, text="CT2");

        self.ct3Tab = ttk.Frame(self.tabFrame);
        self.tabFrame.add(self.ct3Tab, text="CT3");

        self.ct4Tab = ttk.Frame(self.tabFrame);
        self.tabFrame.add(self.ct4Tab, text="CT4");

        self.ct5Tab = ttk.Frame(self.tabFrame);
        self.tabFrame.add(self.ct5Tab, text="CT5");

        self.ct8Tab = ttk.Frame(self.tabFrame);
        self.tabFrame.add(self.ct8Tab, text="CT8");

        self.ct9Tab = ttk.Frame(self.tabFrame);
        self.tabFrame.add(self.ct9Tab, text="CT9");

        self.ct0Tab = ttk.Frame(self.tabFrame);
        self.tabFrame.add(self.ct0Tab, text="CT0");

        self.sampleTab = ttk.Frame(self.tabFrame);
        self.tabFrame.add(self.sampleTab, text="Sample Cal");

        self.tabFrame.pack(expand=1, fill="both");

    def createCT1TextBox(self):
        self.ct1TxtBox = tk.Text(self.ct1Tab, height=50, width=80);
        self.ct1TxtBox.pack();

    def createCT2TextBox(self):
        self.ct2TxtBox = tk.Text(self.ct2Tab, height=50, width=80);
        self.ct2TxtBox.pack();

    def createCT3TextBox(self):
        self.ct3TxtBox = tk.Text(self.ct3Tab, height=50, width=80);
        self.ct3TxtBox.pack();

    def createCT4TextBox(self):
        self.ct4TxtBox = tk.Text(self.ct4Tab, height=50, width=80);
        self.ct4TxtBox.pack();

    def createCT5TextBox(self):
        self.ct5TxtBox = tk.Text(self.ct5Tab, height=50, width=80);
        self.ct5TxtBox.pack();

    def createCT8TextBox(self):
        self.ct8TxtBox = tk.Text(self.ct8Tab, height=50, width=80);
        self.ct8TxtBox.pack();

    def createCT9TextBox(self):
        self.ct9TxtBox = tk.Text(self.ct9Tab, height=50, width=80);
        self.ct9TxtBox.pack();

    def createCT0TextBox(self):
        self.ct0TxtBox = tk.Text(self.ct0Tab, height=50, width=80);
        self.ct0TxtBox.pack();

    def createSampleCalTextBox(self):
        help="The valid header is 0x01020304. In it 772 is 0x0304 and 258 is 0x0102";
        self.sampleCalLabel  = tk.Label(self.sampleTab, text=help);
        self.sampleCalLabel.pack();
        self.sampleCalTxtBox = tk.Text(self.sampleTab, height=50, width=80);
        self.sampleCalTxtBox.pack();

    def readCT1Table(self):
        self.c = calTable("./ibuc_cal.dat", self.ct1TxtBox);
        self.c.readTable(Table.CT1);

    def readCT2Table(self):
        self.c = calTable("./ibuc_cal.dat", self.ct2TxtBox);
        self.c.readTable(Table.CT2);

    def readCT3Table(self):
        self.c = calTable("./ibuc_cal.dat", self.ct3TxtBox);
        self.c.readTable(Table.CT3);

    def readCT4Table(self):
        self.c = calTable("./ibuc_cal.dat", self.ct4TxtBox);
        self.c.readTable(Table.CT4);

    def readCT5Table(self):
        self.c = calTable("./ibuc_cal.dat", self.ct5TxtBox);
        self.c.readTable(Table.CT5);

    def readCT8Table(self):
        self.c = calTable("./ibuc_cal.dat", self.ct8TxtBox);
        self.c.readTable(Table.CT8);

    def readCT9Table(self):
        self.c = calTable("./ibuc_cal.dat", self.ct9TxtBox);
        self.c.readTable(Table.CT9);

    def readCT0Table(self):
        self.c = calTable("./ibuc_cal.dat", self.ct0TxtBox);
        self.c.readTable(Table.CT0);

    def readSampleCalTable(self):
        self.c = calTable("./ibuc_cal.dat", self.sampleCalTxtBox);
        self.c.readTable(Table.SAMPLE);

    def loop(self):
        self.window.mainloop();

if __name__ == "__main__":
    c = CalTableReader();
    c.createTabs();

    c.createCT1TextBox();
    c.createCT2TextBox();
    c.createCT3TextBox();
    c.createCT4TextBox();
    c.createCT5TextBox();
    c.createCT8TextBox();
    c.createCT9TextBox();
    c.createCT0TextBox();
    c.createSampleCalTextBox();

    c.readCT1Table();
    c.readCT2Table();
    c.readCT3Table();
    c.readCT4Table();
    c.readCT5Table();
    c.readCT8Table();
    c.readCT9Table();
    c.readCT0Table();
    c.readSampleCalTable();

    c.loop();
