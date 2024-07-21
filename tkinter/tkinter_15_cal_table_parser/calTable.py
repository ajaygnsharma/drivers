
import io
import numpy as np
from enum import Enum
import tkinter as tk

class Offset(Enum):
    SAMPLE_CAL = 3172;
    CT1 = 3200;
    CT2 = 4672;
    CT3 = 6144;
    CT4 = 6208;
    CT5 = 6272;
    CT8 = 6336;
    CT9 = 6400;
    CT0 = 7872;

class Table(Enum):
    CT1     = 1;
    CT2     = 2;
    CT3     = 3;
    CT4     = 4;
    CT5     = 5;
    CT8     = 6;
    CT9     = 7;
    CT0     = 8;
    SAMPLE  = 9;

class calTable(object):
    def __init__(self, fileName, txtBox):
        self.fileName = fileName;
        self.buff     = "";
        self.txtBox   = txtBox;
        pass;

    def readTable(self, tableNum):
        if(tableNum == Table.CT1):
            dType = np.dtype([("if", np.uint16),
                              ("t",np.uint16),
                              ("p", np.uint16)]);

            totalSize = int((11*6*11) / 3);
            self.read(Offset.CT1, dType, totalSize);

        elif(tableNum == Table.CT2):
            dType = np.dtype([("if", np.uint16),
                              ("t",np.uint16),
                              ("p", np.uint16)]);

            totalSize = int((11*6*11) / 3);
            self.read(Offset.CT2, dType, totalSize);

        elif (tableNum == Table.CT3):
            dType = np.dtype([("t", np.uint16)]);
            totalSize = 6;
            self.read(Offset.CT3, dType, totalSize);

        elif (tableNum == Table.CT4):
            dType = np.dtype([("t", np.uint16)]);
            totalSize = 6;
            self.read(Offset.CT4, dType, totalSize);

        elif (tableNum == Table.CT5):
            dType = np.dtype([("Fault", np.uint16),
                              ("Clear", np.uint16)]);
            totalSize = (6 * 2);
            self.read(Offset.CT5, dType, totalSize);

        elif (tableNum == Table.CT8):
            dType = np.dtype([("t", float)]);
            totalSize = 6;
            self.read(Offset.CT8, dType, totalSize);

        elif(tableNum == Table.CT9):
            dType = np.dtype([("f", np.uint32),
                              ("t", np.uint32),
                              ("l", np.uint32)]);

            totalSize = int((3*6*11) / 3);
            self.read(Offset.CT9, dType, totalSize);

        elif(tableNum == Table.CT0):
            dType = np.dtype([("level", np.float32),
                              ("adc",   np.int32)]);

            totalSize = int((6*11) / 2);
            self.read(Offset.CT0, dType, totalSize);

        elif (tableNum == Table.SAMPLE):
            dType = np.dtype([("data", np.int16)]);
            totalSize = 11;
            self.read(Offset.SAMPLE_CAL, dType, totalSize);

    def read(self, offset, dType, n):
        f = open(self.fileName, "rb");
        f.seek(offset.value, io.SEEK_SET);
        a = np.fromfile(f, dtype=dType, count=n);
        #print(a);
        self.txtBox.insert(tk.END, a);

    def dump(self):
        print(self.buff);






