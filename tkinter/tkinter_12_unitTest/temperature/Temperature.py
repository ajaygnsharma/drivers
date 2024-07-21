'''
Created on Dec 12, 2019

@author: asharma
'''
import serial
import platform
import re
import tkinter as tk
from Login import Login
from calibration import Calibration
from stat import S_ISCHR
import os, sys

max = 300;

class Temperature(object):
    def __init__(self, port, logFile, debug: str=None):
        self.debug = 0;
        self.f = "";
        if(debug):
            self.debug = 1;
        
        self.f   = open(logFile, "wt");
        self.ser = serial.Serial(port, 115200, timeout=0.2);

    def getMinMax(self):
        print("Temperature",end="");
        self.ser.write(b'ATL\n');
        lines = self.ser.readlines();
        for l in lines:
            x = re.findall("[+-]?\d+", l.decode("utf-8"));
            if(len(x) > 0):
                outStr=("%s,%s,%s"%(x[0],x[1],x[2]));
                return outStr;
    
    def setMinMax(self):
        self.ser.write(b'ATL=1\n')
        line = self.ser.readline()
        try:
            x = re.findall("\d+", line.decode("utf-8") )
            print("min\tmax\tentries")
            print("%s\t%s\t%s"%(x[0],x[1],x[2]))
        except:
            print(line)
                
    def getInternal(self):
        print("Internal Temperature: ",end="");
        self.ser.write(b'COX\n');
        lines = self.ser.readlines();
        for l in lines:
            x = re.findall("[+-]?\d+", l.decode("utf-8"));
            if(len(x) > 0):
                outStr=("%s"%x[0]);
                return outStr;
    
    def getOutside(self):
        print("Outside Temperature: ",end="");
        self.ser.write(b'COX=1\n');
        lines = self.ser.readlines();
        print(lines);
        for l in lines:
            x = re.findall("[+-]?\d{2,5}", l.decode("utf-8"));
            if (len(x) > 0):
                outStr = ("%s" % x[0]);
                return outStr;

    def getHighTemperatureThreshold(self):
        print("High Temperature Threshold: ",end="");
        self.ser.write(b'COX=2\n');
        lines = self.ser.readlines();
        print(lines);
        for l in lines:
            x = re.findall("[+-]?\d{2,5}", l.decode("utf-8"));
            if (len(x) > 0):
                outStr = ("%s" % x[0]);
                return outStr;

    def setHighTemperatureThreshold(self, val):
        print("High Temperature Threshold: ",end="");
        cmd=("COX=%d\n"%(int(val)+10000));
        self.ser.write(cmd.encode());
        lines = self.ser.readlines();
        print(lines);
        for l in lines:
            x = re.findall("[+-]?\d{1,5}", l.decode("utf-8"));
            if (len(x) > 0):
                if(int(x[0]) < 10000):
                    outStr = ("%s" % x[0]);
                    return outStr;

    def checkOutside(self):
        # -40 and 60 deg is the range but here in lab, 20-30 deg
        assert ((self.getOutside() > 20) and (self.getOutside() < 30));
        
    
    def setInternal(self, val):
        self.ser.write(b'COX='+str.encode(val)+b'\n')
        line = self.ser.readline()
        try:
            x = re.findall("\d+", line.decode("utf-8") )
            print("=%s"%x[0]);
        except:
            print(line)
        
    def getHighTemperatureShutdown(self):
        self.ser.write(b'TTS\n')
        line = self.ser.read(max);
        x = re.findall("\d+", line.decode("utf-8") )
        if(x[0] == '0'):
            print("High Temperature Shutdown: Disabled");
        else:
            print("High Temperature Shutdown: Enabled");

    def setHighTemperatureShutdown(self, val):
        self.ser.write(b'TTS='+str.encode(val)+b'\n')
        line = self.ser.readline()
        x = re.findall("\d+", line.decode("utf-8") )
        if(x[0] == '0'):
            print("High Temperature Shutdown=Disabled")
        else:
            print("High Temperature Shutdown=Enabled")

    def current(self, val=None):
        if (val):
            cmdStr = val + "\n";
            self.ser.write(b'CID=' + str.encode(cmdStr));
        else:
            self.ser.write(b'CID\n');

        rsp = self.parseRsp("[+-]?\d+.\d+");
        if (len(rsp) > 0):
            outStr = ("%s" % rsp[0]);
            return outStr;
        else:
            return 'No current';
                    
    def __del__(self):
        self.ser.close()

class App(object):
    def __init__(self):
        self.window = tk.Tk();
        self.window.rowconfigure(0, minsize=100);
        self.window.columnconfigure([0, 1, 2, 3], minsize=50);

        self.frame1 = tk.Frame(master=self.window);

    def frameATL(self):

        tk.Label(self.frame1,text="Temperature(Min/Max/Entries) (ATL)").grid(row=0, column=0);

        self.txtBoxATL = tk.Text(self.frame1, height=1, width=10);
        self.txtBoxATL.grid(row=0, column=1);

        tk.Label(self.frame1, text="deg C").grid(row=0, column=2);
        tk.Button(self.frame1, text="get", command=self._getCmdMinMax).grid(row=0, column=3);

    def portsList(self):
        serialPort = tk.StringVar();
        self.listBoxPort = tk.Listbox(master=self.window, listvariable=serialPort);
        self.listBoxPort.grid(row=1, column=0);
        self.fillSerialPorts();

    def outputWindow(self):
        self.txtBox = tk.Text(master=self.window, height=20, width=40);
        self.txtBox.grid(row=2, column=0, sticky="e");

        self.frame1.grid(row=0, column=0);

    def frameCOX(self):
        tk.Label(self.frame1,text="Inside Temperature (COX)").grid(row=1, column=0);

        self.txtBoxCOX = tk.Text(self.frame1, height=1, width=10);
        self.txtBoxCOX.grid(row=1, column=1);

        tk.Label(self.frame1, text="deg C").grid(row=1, column=2);
        tk.Button(self.frame1, text="get", command=self._getCmdGetInternal).grid(row=1, column=3);

    def frameCOX1(self):
        tk.Label(self.frame1,text="Outside Temperature (COX=1)").grid(row=2, column=0);

        self.txtBoxCOX1 = tk.Text(self.frame1, height=1, width=10);
        self.txtBoxCOX1.grid(row=2, column=1);

        tk.Label(self.frame1, text="deg C").grid(row=2, column=2);
        tk.Button(self.frame1, text="get", command=self._getCmdGetExternal).grid(row=2, column=3);

    def frameCOX2(self):
        tk.Label(self.frame1,text="High Temperature Threshold (COX=2)").grid(row=3, column=0);

        self.txtBoxCOX2 = tk.Text(self.frame1, height=1, width=10);
        self.txtBoxCOX2.grid(row=3, column=1);

        tk.Label(self.frame1, text="deg C").grid(row=3, column=2);
        tk.Button(self.frame1,text="get", command=self._getCmdGetHighThr).grid(row=3, column=3);
        tk.Button(self.frame1, text="set", command=self._setCmdHighThr).grid(row=3, column=4);

    def frameCID(self):
        tk.Label(self.frame1,text="IDC Detector(CID)").grid(row=4, column=0);

        self.txtBoxCID = tk.Text(self.frame1, height=1, width=10);
        self.txtBoxCID.grid(row=4, column=1);

        tk.Label(self.frame1, text="(0.0-12.5 Amps)").grid(row=4, column=2);
        tk.Button(self.frame1,text="get", command=self._getCmdGetHighThr).grid(row=4, column=3);
        tk.Button(self.frame1, text="set", command=self._setCmdHighThr).grid(row=4, column=4);

    def loop(self):
        self.window.mainloop();

    def checkfile(self,fileName):
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

    def _getCmdMinMax(self):
        idx  = self.listBoxPort.curselection();
        port = self.listBoxPort.get(idx);

        Login(self.txtBox, port);

        t = Temperature(port,"run.log");


        outStr = t.getMinMax();
        print(outStr);
        self.txtBoxATL.insert(tk.END, outStr);

    def _getCmdGetInternal(self):
        idx  = self.listBoxPort.curselection();
        port = self.listBoxPort.get(idx);

        Login(self.txtBox, port);

        t = Temperature(port,"run.log");


        outStr = t.getInternal();
        print(outStr);
        self.txtBoxCOX.insert(tk.END, outStr);

    def _getCmdGetExternal(self):
        idx  = self.listBoxPort.curselection();
        port = self.listBoxPort.get(idx);

        Login(self.txtBox, port);

        t = Temperature(port,"run.log");


        outStr = t.getOutside();
        print(outStr);
        self.txtBoxCOX1.insert(tk.END, outStr);

    def _getCmdGetHighThr(self):
        idx  = self.listBoxPort.curselection();
        port = self.listBoxPort.get(idx);

        Login(self.txtBox, port);

        t = Temperature(port,"run.log");

        outStr = t.getHighTemperatureThreshold();
        print(outStr);
        self.txtBoxCOX2.insert(tk.END, outStr);

    def _setCmdHighThr(self):
        idx  = self.listBoxPort.curselection();
        port = self.listBoxPort.get(idx);

        Login(self.txtBox, port);

        c = Calibration.Calibration(self.txtBox, port);
        c.enable();

        t = Temperature(port,"run.log");

        thr = self.txtBoxCOX2.get("1.0", tk.END);
        outStr = t.setHighTemperatureThreshold(thr);
        print(outStr);
        self.txtBoxCOX2.delete("1.0", "end");
        self.txtBoxCOX2.insert(tk.END, outStr);

if __name__ == "__main__":
    app = App();
    app.frameATL();
    app.portsList();
    app.outputWindow();

    app.frameCOX();
    app.frameCOX1();
    app.frameCOX2();

    app.frameCID();

    app.loop()