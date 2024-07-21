'''
Created on Dec 12, 2019

@author: asharma
'''
import serial
import platform
import re
import tkinter as tk
from Login import Login
#from calibration import Calibration
from stat import S_ISCHR
import os, sys

max = 300;


class Sensors(object):
    def __init__(self, port, logFile, debug: str = None):
        self.debug = 0;
        self.f = "";
        if (debug):
            self.debug = 1;

        self.f = open(logFile, "wt");
        self.ser = serial.Serial(port, 115200, timeout=0.2);


    def DROTuneVolts(self):
        self.ser.write(b'CTV\n');
        lines = self.ser.readlines();
        for l in lines:
            s = l.decode("utf-8");
            expList = re.findall("\d+.\d+", s);
            if( len(expList) > 0 ):
                dro = float(expList[0]);
                if (dro != -1):
                    outStr = ("%s" % (dro));
                    return outStr;

    def supplyVolts(self):
        self.ser.write(b'CVD\n');
        lines = self.ser.readlines();
        for l in lines:
            s = l.decode("utf-8");
            expList = re.findall("\d+.\d+", s);
            if( len(expList) > 0 ):
                volts = float(expList[0]);
                if (volts != -1):
                    outStr = ("%s" % (volts));
                    return outStr;

    def DROStat(self):
        self.ser.write(b'DRO\n');
        lines = self.ser.readlines();
        for l in lines:
            s = l.decode("utf-8");
            expList = re.findall("\d+.\d+", s);
            if( len(expList) > 0 ):
                outStr = ("%s,%s,%s" % (expList[0],expList[1],expList[2],));
                return outStr;

    def RAW(self):
        self.ser.write(b'RAW\n');
        lines = self.ser.readlines();
        for l in lines:
            s = l.decode("utf-8");
            expList = re.findall("\d+", s);
            if( len(expList) > 0 ):
                return expList;

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

    def getCurrent(self, val=None):
        if (val):
            cmdStr = val + "\n";
            self.ser.write(b'CID=' + str.encode(cmdStr));
        else:
            self.ser.write(b'CID\n');

        lines = self.ser.readlines();
        for l in lines:
            x = re.findall("\d{1,5}.\d{1,2}", l.decode("utf-8"));
            if (len(x) > 0):
                outStr = ("%s" % x[0]);
                return outStr;

    def getACCorrection(self, val=None):
        if (val):
            cmdStr = val + "\n";
            self.ser.write(b'CSF=' + str.encode(cmdStr));
        else:
            self.ser.write(b'CSF\n');

        lines = self.ser.readlines();
        for l in lines:
            x = re.findall("\d", l.decode("utf-8"));
            if (len(x) > 0):
                outStr = ("%s" % x[0]);
                return outStr;

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
        tk.Button(self.frame1,text="get", command=self._doCmdGetCurrent).grid(row=4, column=3);

    def frameCSF(self):
        tk.Label(self.frame1,text="50Hz AC correction(CSF)").grid(row=5, column=0);

        self.txtBoxCSF = tk.Text(self.frame1, height=1, width=10);
        self.txtBoxCSF.grid(row=5, column=1);

        tk.Label(self.frame1, text="(0-Disabled, 1-Enabled)").grid(row=5, column=2);
        tk.Button(self.frame1,text="get", command=self._doCmdGet50HzCorrection).grid(row=5, column=3);
        tk.Button(self.frame1, text="set", command=self._doCmdSet50HzCorrection).grid(row=5, column=4);

    def frameCTV(self):
        tk.Label(self.frame1,text="DRO Tune volts(CTV)").grid(row=6, column=0);

        self.txtBoxCTV = tk.Text(self.frame1, height=1, width=10);
        self.txtBoxCTV.grid(row=6, column=1);

        tk.Label(self.frame1, text="(0.0-10.0 Volts)").grid(row=6, column=2);
        tk.Button(self.frame1,text="get", command=self._doCmdGetDROTuneVolt).grid(row=6, column=3);

    def frameCVD(self):
        tk.Label(self.frame1,text="Supply Voltage(VDC/VAC").grid(row=7, column=0);

        self.txtBoxCVD = tk.Text(self.frame1, height=1, width=10);
        self.txtBoxCVD.grid(row=7, column=1);

        tk.Label(self.frame1, text="(0.0-60.0 Volts)").grid(row=7, column=2);
        tk.Button(self.frame1,text="get", command=self._doCmdGetSupplyVolts).grid(row=7, column=3);

    def frameDRO(self):
        tk.Label(self.frame1,text="DRO Statistics(Min,Max,Original)").grid(row=8, column=0);

        self.txtBoxDRO = tk.Text(self.frame1, height=1, width=20);
        self.txtBoxDRO.grid(row=8, column=1);

        tk.Label(self.frame1, text="(Array of 0.0-10.0 Volts)").grid(row=8, column=2);
        tk.Button(self.frame1,text="get", command=self._doCmdGetDROStat).grid(row=8, column=3);

    def frameRAW(self):
        tk.Label(self.frame1,text="RAW: Po").grid(row=9, column=0);

        self.txtBoxPo = tk.Text(self.frame1, height=1, width=10);
        self.txtBoxPo.grid(row=9, column=1);

        tk.Label(self.frame1, text="Pi").grid(row=10, column=0);
        self.txtBoxPi = tk.Text(self.frame1, height=1, width=10);
        self.txtBoxPi.grid(row=10, column=1);

        tk.Label(self.frame1, text="Temperature").grid(row=11, column=0);
        self.txtBoxT = tk.Text(self.frame1, height=1, width=10);
        self.txtBoxT.grid(row=11, column=1);

        tk.Label(self.frame1, text="DRO").grid(row=12, column=0);
        self.txtBoxD = tk.Text(self.frame1, height=1, width=10);
        self.txtBoxD.grid(row=12, column=1);

        tk.Label(self.frame1, text="VDC").grid(row=13, column=0);
        self.txtBoxV = tk.Text(self.frame1, height=1, width=10);
        self.txtBoxV.grid(row=13, column=1);

        tk.Label(self.frame1, text="IDC").grid(row=14, column=0);
        self.txtBoxI = tk.Text(self.frame1, height=1, width=10);
        self.txtBoxI.grid(row=14, column=1);

        tk.Label(self.frame1, text="10M").grid(row=15, column=0);
        self.txtBox10M = tk.Text(self.frame1, height=1, width=10);
        self.txtBox10M.grid(row=15, column=1);

        tk.Button(self.frame1,text="get", command=self._doCmdGetRAW).grid(row=9, column=2);

        pass;

    #---------------------------------------------------------------------------
    # Main Loop
    #---------------------------------------------------------------------------
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

    def _doCmdGetCurrent(self):
        idx  = self.listBoxPort.curselection();
        port = self.listBoxPort.get(idx);

        Login(self.txtBox, port);

        t = Temperature(port,"run.log");

        outStr = t.getCurrent();
        print(outStr);
        self.txtBoxCID.insert(tk.END, outStr);

    def _doCmdGet50HzCorrection(self):
        idx  = self.listBoxPort.curselection();
        port = self.listBoxPort.get(idx);

        Login(self.txtBox, port);

        t = Temperature(port,"run.log");

        outStr = t.getACCorrection();
        print(outStr);
        self.txtBoxCSF.insert(tk.END, outStr);

    def _doCmdSet50HzCorrection(self):
        idx  = self.listBoxPort.curselection();
        port = self.listBoxPort.get(idx);

        Login(self.txtBox, port);

        t = Temperature(port,"run.log");

        val = self.txtBoxCSF.get("1.0", 'end-1c');
        outStr = t.getACCorrection(val);

        self.txtBoxCSF.delete("1.0",'end'); # Clear txtbox first. Then show the result
        self.txtBoxCSF.insert(tk.END, outStr);

    def _doCmdGetDROTuneVolt(self):
        idx  = self.listBoxPort.curselection();
        port = self.listBoxPort.get(idx);

        Login(self.txtBox, port);

        s = Sensors(port,"run.log");

        outStr = s.DROTuneVolts();
        print(outStr);
        self.txtBoxCTV.insert(tk.END, outStr);

    def _doCmdGetSupplyVolts(self):
        idx  = self.listBoxPort.curselection();
        port = self.listBoxPort.get(idx);

        Login(self.txtBox, port);

        s = Sensors(port,"run.log");

        outStr = s.supplyVolts();
        print(outStr);
        self.txtBoxCVD.insert(tk.END, outStr);

    def _doCmdGetDROStat(self):
        idx  = self.listBoxPort.curselection();
        port = self.listBoxPort.get(idx);

        Login(self.txtBox, port);

        s = Sensors(port,"run.log");

        outStr = s.DROStat();
        print(outStr);
        self.txtBoxDRO.insert(tk.END, outStr);

    def _doCmdGetRAW(self):
        idx  = self.listBoxPort.curselection();
        port = self.listBoxPort.get(idx);

        Login(self.txtBox, port);

        s = Sensors(port,"run.log");

        outStr = s.RAW();
        print(outStr);
        self.txtBoxPo.insert(tk.END, outStr[0]);
        self.txtBoxPi.insert(tk.END, outStr[1]);
        self.txtBoxT.insert(tk.END,  outStr[2]);
        self.txtBoxD.insert(tk.END, outStr[3]);
        self.txtBoxV.insert(tk.END, outStr[4]);
        self.txtBoxI.insert(tk.END, outStr[5]);
        self.txtBox10M.insert(tk.END, outStr[6]);

    def _setCmdHighThr(self):
        idx  = self.listBoxPort.curselection();
        port = self.listBoxPort.get(idx);

        Login(self.txtBox, port);

        '''
        Does calibration have to be enabled?
        c = Calibration.Calibration(self.txtBox, port);
        c.enable();
        '''

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
    app.frameCSF();
    app.frameCTV();
    app.frameCVD();
    app.frameDRO();
    app.frameRAW();
    app.loop()