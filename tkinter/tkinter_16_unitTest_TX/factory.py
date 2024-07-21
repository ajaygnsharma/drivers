import serial
import re
import platform
from lib2to3.pgen2.token import OP
from Login import Login
import tkinter as tk
import lib as ibbLib
import os, sys
from stat import S_ISCHR

max = 300;
minSerialNumberLength = 25;
maxSerialNumberLength = 25;
minHWRevLength        = 2;
maxHWRevLength        = 5;

class Factory(object):
    def __init__(self, port, logFile, debug: str=None):
        self.debug = 0;
        if(debug):
            self.debug = 1;
        self.ser = serial.Serial(port, 115200, timeout=1)

    def __isSerialNumValid(self, serialNum):
        if(("TE" in serialNum)  
           and ((len(serialNum)) > minSerialNumberLength) 
           and ((len(serialNum)) < maxSerialNumberLength)):
            return True;
        else:    
            return False;
    
    def __isHWRevValid(self, hwRev):
        if(((len(hwRev)) > minHWRevLength) and ((len(hwRev)) < maxHWRevLength)):
            return True;
        else:    
            return False;
    
    def parseFactoryInfo(self, lines):
        for l in lines:
            s = l.decode("utf-8");
            expList = re.findall("\d+.\d+", s);
            if(len(expList) > 1):
                if("AMPS" in s):
                    assert(float(expList[0]) >=0 and float(expList[1]) <= 5), s;
                elif("VOLTS" in s):
                    assert(float(expList[0]) >=0 and float(expList[1]) <= 250), s;
                elif("WATTS" in s):
                    assert(float(expList[0]) >=0 and float(expList[1]) <= 900), s;
                elif("TOTAL HRS" in s):
                    assert(float(expList[0]) >=0), s;
                
        return True;

    # Bear in mind that the CBT=Number followed by String will convert
    # the number and ignore the string. For example cbt=3fdfdf will make cbt=3
    def cc1(self, val=None, verbose=None):
        if( val != None ):
            cmd=('CC1=%s\n'%val);
            self.ser.write(cmd.encode());
        else:
            self.ser.write(b"CC1\n");

        opList = self.ser.readlines();
        for opStr in opList:
            modelNameList = opStr.decode('utf-8').split("=");
            if ((len(modelNameList) == 2) and ("IBH" in modelNameList[1])):
                modelNameList[1] = modelNameList[1].rstrip('\r\n');
                return modelNameList[1];
        # Reached end
        return "Undefined";

    def cc2(self, val=None):
        if( val != None ):
            cmd = ('CC2=%s\n' % val);
            self.ser.write(cmd.encode());
        else:
            self.ser.write(b"CC2\n");
        
        opList = self.ser.readlines();
        for opStr in opList:
            serialNumList = opStr.decode('utf-8').split("=");
            print(serialNumList);
            if( (len(serialNumList) == 2) and ("TE" in serialNumList[1]) ):
                serialNumList[1] = serialNumList[1].rstrip('\r\n');
                return serialNumList[1];

        # Reached end
        return "Undefined";

    def cc3(self, val=None):
        if (val != None):
            cmd = ('CC3=%s\n' % val);
            self.ser.write(cmd.encode());
        else:
            self.ser.write(b'CC3\n');

        opList = self.ser.readlines();
        for opStr in opList:
            HWAssNumList = opStr.decode('utf-8').split("=");
            print(HWAssNumList);
            if ((len(HWAssNumList) == 2) and ("00" in HWAssNumList[1])):
                HWAssNumList[1] = HWAssNumList[1].rstrip('\r\n');
                return HWAssNumList[1];

        # Reached end
        return "Undefined";

    def cc4(self, val=None):
        if (val != None):
            cmd = ('CC4=%s\n' % val);
            self.ser.write(cmd.encode());
        else:
            self.ser.write(b'CC4\n');

        opList = self.ser.readlines();
        for opStr in opList:
            _list = opStr.decode('utf-8').split("=");
            print(_list);
            if (len(_list) == 2):
                _list[1] = _list[1].rstrip('\r\n');
                return _list[1];

        # Reached end
        return "Undefined";

    def cc5(self, val=None):
        if (val != None):
            cmd = ('CC5=%s\n' % val);
            self.ser.write(cmd.encode());
        else:
            self.ser.write(b'CC5\n');

        opList = self.ser.readlines();
        for opStr in opList:
            _list = opStr.decode('utf-8').split("=");
            print(_list);
            if (len(_list) == 2):
                _list[1] = _list[1].rstrip('\r\n');
                return _list[1];

        # Reached end
        return "Undefined";

    def ccm(self, val=None):
        if (val != None):
            cmd = ('CCM=%s\n' % val);
            self.ser.write(cmd.encode());
        else:
            self.ser.write(b'CCM\n');

        opList = self.ser.readlines();
        for opStr in opList:
            _list = opStr.decode('utf-8').split("=");
            print(_list);
            if (len(_list) == 2):
                _list[1] = _list[1].rstrip('\r\n');
                return _list[1];

        # Reached end. Return whatever is read from serial port.
        return opList;

    def ccs(self, val=None):
        if (val != None):
            cmd = ('CCS=%s\n' % val);
            self.ser.write(cmd.encode());
        else:
            self.ser.write(b'CCS\n');

        opList = self.ser.readlines();
        for opStr in opList:
            _list = opStr.decode('utf-8').split("=");
            print(_list);
            if (len(_list) == 2):
                _list[1] = _list[1].rstrip('\r\n');
                return _list[1];

        # Reached end. Return whatever is read from serial port.
        return opList;

    def cfd(self, val=None):
        if (val != None):
            cmd = ('CFD=%s\n' % val);
            self.ser.write(cmd.encode());
        else:
            self.ser.write(b'CFD\n');

        opList = self.ser.readlines();

        outStr = "";
        for opStr in opList:
            _list = opStr.decode('utf-8').split("\r\n");
            print(_list);
            if ( ("AMPS" in _list[0]) or ("VOLTS" in _list[0]) or\
                 ("WATTS" in _list[0]) or ("HRS" in _list[0]) ):
                _list[0] = _list[0].rstrip('\r\n');
                outStr += str(_list[0]);
                outStr += '\n';

        # Reached end. Return whatever is read from serial port.
        if(len(outStr) > 0):
            return outStr;
        else:
            return opList;

    def coi(self, val=None):
        if (val != None):
            cmd = ('COI=%s\n' % val);
            self.ser.write(cmd.encode());
        else:
            self.ser.write(b'COI\n');

        opList = self.ser.readlines();

        outStr = "";
        for opStr in opList:
            _list = opStr.decode('utf-8').split("\r\n");
            print(_list);
            if ( ("Model" in _list[0]) or ("SN" in _list[0]) or\
                 ("FW" in _list[0]) or ("LO" in _list[0]) ):
                _list[0] = _list[0].rstrip('\r\n');
                outStr += str(_list[0]);
                outStr += '\n';

        # Reached end. Return whatever is read from serial port.
        if(len(outStr) > 0):
            return outStr;
        else:
            return opList;

    def coi_2(self, val=None):
        self.ser.write(b'COI=1\n');

        opList = self.ser.readlines();

        outStr = "";
        for opStr in opList:
            _list = opStr.decode('utf-8').split("\r\n");
            print(_list);
            if ( ("ODU" in _list[0]) or ("Band" in _list[0]) or\
                 ("PwrClass" in _list[0]) or ("PLDRO" in _list[0]) or\
                 ("FW" in _list[0]) or ("Buc" in _list[0]) or\
                 ("Housing" in _list[0]) ):
                _list[0] = _list[0].rstrip('\r\n');
                outStr += str(_list[0]);
                outStr += '\n';

        # Reached end. Return whatever is read from serial port.
        if(len(outStr) > 0):
            return outStr;
        else:
            return opList;

    '''
    # Bear in mind that the CBT=Number followed by String will convert
    # the number and ignore the string. For example cbt=3fdfdf will make cbt=3
    def cc3(self, val=None):
        if( val != None ):
            self.ser.write(b"CC3="+str.encode(val)+"\n");
        else:
            self.ser.write(b"CC3=\n");
            
        opList = self.ser.readlines();
        ok = False;
        for opStr in opList:
            hwRev = opStr.decode("utf-8");
            if("invalid value" in hwRev):
                ok = True;
            elif(self.__isHWRevValid(hwRev)):
                ok = True;
            
        assert (ok == True), opList;
    
        # Bear in mind that the CBT=Number followed by String will convert
    # the number and ignore the string. For example cbt=3fdfdf will make cbt=3
    def cc4(self, val=None):
        if( val != None ):
            self.ser.write(b"CC3="+str.encode(val)+"\n");
        else:
            self.ser.write(b"CC3=\n");
            
        opList = self.ser.readlines();
        ok = False;
        expList = re.findall("\d{2,}", line.decode("utf-8") )
        for ratedPower in expList:
            if(int(ratedPower) > 0):
                print("Rated Power OK");
                ok = True;
            
        assert (ok == True), expList;
        
    # Bear in mind that the CBT=Number followed by String will convert
    # the number and ignore the string. For example cbt=3fdfdf will make cbt=3
    def cc5(self, val1=None, val2=None):
        if( (val1 != None) and (val2 != None) ):
            self.ser.write(b"CC5="+str.encode(val1)+","+str.encode(val2)+"\n");
        else:
            self.ser.write(b"CC5\n");
        line = self.ser.read(max);
        
        ok = ibbLib.isNegativeNumber(line);
        
        assert (ok == True), line;
        print("Input Value OK");
            
    def ccf(self, index=None, level=None):
        if((index != None) and (level != None)):
            self.ser.write(b"CCF="+str.encode(index)+b","+str.encode(level)+b"\n");
        else:
            self.ser.write(b"CCF\n");
        line = self.ser.read(max);
        ok = ibbLib.hasANumber(line);
        
        assert (ok == True), line;
        print("Coupler Loss: OK");
    
    def ccm(self, val=None):
        if( val != None ):
            self.ser.write(b"CCM="+str.encode(val)+"\n");
        else:
            self.ser.write(b"CCM=\n");
        
        found = ibbLib.hasString(line, "IBB");
        assert (found == True), line;
        print("ModelName and Serial Number: OK");
    
    def cfd(self, val=None):
        if( val != None ):
            self.ser.write(b"CFD="+str.encode(val)+"\n");
        else:
            self.ser.write(b"CFD=\n");
            
        lines = self.ser.readlines();
        ok = self.parseFactoryInfo(lines);
        assert (found == True), line;
        print("ModelName and Serial Number: OK");
    
    def coi(self):
        self.ser.write(b"COI=\n");
        lines = self.ser.readlines();
        print(lines);

    # CPL 
    def privateLabel(self, label=None):
        if(label):
            self.ser.write(b"CPL="+str.encode(label)+"\n");
        else:
            self.ser.write(b'CPL\n');
        
        lines = self.ser.readlines();
        print(lines);
                
    def test1(self):
        self.cc1("IBB058067-1NA200QSWW-0000");
    
    def test2(self):
        self.cc2("TE817680");
        
    def test3(self):
        self.cc3("00001");

    def test4(self):
        self.cc4("5400");

    def test5(self):
        self.cc5("-6000","-1500");
    
    def test6(self):
        self.ccf("0","100");
    '''

    def stop(self):
        self.ser.close();


class App(object):
    def __init__(self):
        self.window = tk.Tk();
        self.window.rowconfigure(0, minsize=100);
        self.window.columnconfigure([0, 1, 2, 3], minsize=50);

        self.frame1 = tk.Frame(master=self.window);

    def frameCC1(self):
        tk.Label(self.frame1,text="Model Number (CC1)").grid(row=0, column=0);

        self.txtBoxCC1 = tk.Text(self.frame1, height=1, width=30);
        self.txtBoxCC1.grid(row=0, column=1);

        tk.Button(self.frame1, text="get", command=self._getCC1).grid(row=0, column=2);
        tk.Button(self.frame1, text="set", command=self._setCC1).grid(row=0, column=3);

    def frameCC2(self):
        tk.Label(self.frame1,text="Serial Number (CC2)").grid(row=1, column=0);

        self.txtBoxCC2 = tk.Text(self.frame1, height=1, width=30);
        self.txtBoxCC2.grid(row=1, column=1);

        tk.Button(self.frame1, text="get", command=self._getCC2).grid(row=1, column=2);
        tk.Button(self.frame1, text="set", command=self._setCC2).grid(row=1, column=3);

    def frameCC3(self):
        tk.Label(self.frame1,text="HW Assembly Number (CC3)").grid(row=2, column=0);

        self.txtBoxCC3 = tk.Text(self.frame1, height=1, width=30);
        self.txtBoxCC3.grid(row=2, column=1);

        tk.Button(self.frame1, text="get", command=self._getCC3).grid(row=2, column=2);
        tk.Button(self.frame1, text="set", command=self._setCC3).grid(row=2, column=3);

    def frameCC4(self):
        tk.Label(self.frame1,text="Rated Power (CC4)").grid(row=3, column=0);

        self.txtBoxCC4 = tk.Text(self.frame1, height=1, width=30);
        self.txtBoxCC4.grid(row=3, column=1);

        tk.Button(self.frame1, text="get", command=self._getCC4).grid(row=3, column=2);
        tk.Button(self.frame1, text="set", command=self._setCC4).grid(row=3, column=3);

    def frameCC5(self):
        tk.Label(self.frame1,text="Input Min/Max mB (CC5)").grid(row=4, column=0);

        self.txtBoxCC5 = tk.Text(self.frame1, height=1, width=30);
        self.txtBoxCC5.grid(row=4, column=1);

        tk.Button(self.frame1, text="get", command=self._getCC5).grid(row=4, column=2);
        tk.Button(self.frame1, text="set", command=self._setCC5).grid(row=4, column=3);

    def frameCCM(self):
        tk.Label(self.frame1,text="Model, Serial Number (CCM)").grid(row=5, column=0);

        self.txtBoxCCM = tk.Text(self.frame1, height=1, width=40);
        self.txtBoxCCM.grid(row=5, column=1);

        tk.Button(self.frame1, text="get", command=self._getCCM).grid(row=5, column=2);
        tk.Button(self.frame1, text="set", command=self._setCCM).grid(row=5, column=3);

    def frameCCS(self):
        tk.Label(self.frame1,text="Firmware Revision (CCS)").grid(row=6, column=0);

        self.txtBoxCCS = tk.Text(self.frame1, height=1, width=40);
        self.txtBoxCCS.grid(row=6, column=1);

        tk.Button(self.frame1, text="get", command=self._getCCS).grid(row=6, column=2);
        tk.Button(self.frame1, text="set", command=self._setCCS).grid(row=6, column=3);

    def frameCFD(self):
        tk.Label(self.frame1,text="Factory Data (CFD)").grid(row=7, column=0);

        self.txtBoxCFD = tk.Text(self.frame1, height=4, width=40);
        self.txtBoxCFD.grid(row=7, column=1);

        tk.Button(self.frame1, text="get", command=self._getCFD).grid(row=7, column=2);
        tk.Button(self.frame1, text="set", command=self._setCFD).grid(row=7, column=3);

    def frameCOI(self):
        tk.Label(self.frame1,text="Info (COI)").grid(row=8, column=0);

        self.txtBoxCOI = tk.Text(self.frame1, height=4, width=40);
        self.txtBoxCOI.grid(row=8, column=1);

        tk.Button(self.frame1, text="get", command=self._getCOI).grid(row=8, column=2);

    def frameCOI_2(self):
        tk.Label(self.frame1,text="Info 2(COI)").grid(row=9, column=0);

        self.txtBoxCOI_2 = tk.Text(self.frame1, height=7, width=40);
        self.txtBoxCOI_2.grid(row=9, column=1);

        tk.Button(self.frame1, text="get", command=self._getCOI_2).grid(row=9, column=2);

    '''
    def frameCOX(self):
        tk.Label(self.frame1,text="Inside Temperature (COX)").grid(row=1, column=0);

        self.txtBoxCOX = tk.Text(self.frame1, height=1, width=10);
        self.txtBoxCOX.grid(row=1, column=1);

        tk.Label(self.frame1, text="deg C").grid(row=1, column=2);
        tk.Button(self.frame1, text="get", command=self._getCmdGetInternal).grid(row=1, column=3);
    '''

    def _checkfile(self,fileName):
        try:
            mode = os.lstat(fileName).st_mode;
            if S_ISCHR(mode):
                # It's a character special file, call the callback function
                return True;
        except OSError:
            print('Skipping %s' % fileName);
            return False;

    def _fillSerialPorts(self):
        serialPorts = ("/dev/ttyS0", "/dev/ttyUSB0", "/dev/ttyUSB1");
        for s in serialPorts:
            if self._checkfile(s):
                self.listBoxPort.insert('end', s);

    def portsList(self):
        serialPort = tk.StringVar();
        self.listBoxPort = tk.Listbox(master=self.window, listvariable=serialPort);
        self.listBoxPort.grid(row=1, column=0);
        self._fillSerialPorts();

    def outputWindow(self):
        self.txtBox = tk.Text(master=self.window, height=20, width=40);
        self.txtBox.grid(row=2, column=0, sticky="e");

        self.frame1.grid(row=0, column=0);

    #---------------------------------------------------------------------------
    # Main Loop
    #---------------------------------------------------------------------------
    def loop(self):
        self.window.mainloop();


    def _getCC1(self):
        idx  = self.listBoxPort.curselection();
        port = self.listBoxPort.get(idx);

        Login(self.txtBox, port);

        f = Factory(port,"run.log");

        outStr = f.cc1();
        self.txtBoxCC1.delete("1.0", "end");
        self.txtBoxCC1.insert(tk.END, outStr);

    def _setCC1(self):
        idx  = self.listBoxPort.curselection();
        port = self.listBoxPort.get(idx);

        Login(self.txtBox, port);

        f = Factory(port,"run.log");

        cc1Val = self.txtBoxCC1.get("1.0", tk.END);
        outStr = f.cc1(cc1Val);
        self.txtBoxCC1.delete("1.0", "end");
        self.txtBoxCC1.insert(tk.END, outStr);

    def _getCC2(self):
        idx  = self.listBoxPort.curselection();
        port = self.listBoxPort.get(idx);

        Login(self.txtBox, port);

        f = Factory(port,"run.log");

        outStr = f.cc2();
        self.txtBoxCC2.delete("1.0", "end");
        self.txtBoxCC2.insert(tk.END, outStr);

    def _setCC2(self):
        idx  = self.listBoxPort.curselection();
        port = self.listBoxPort.get(idx);

        Login(self.txtBox, port);

        f = Factory(port,"run.log");

        cc2Val = self.txtBoxCC2.get("1.0", tk.END);
        outStr = f.cc2(cc2Val);
        self.txtBoxCC2.delete("1.0", "end");
        self.txtBoxCC2.insert(tk.END, outStr);

    def _getCC3(self):
        idx  = self.listBoxPort.curselection();
        port = self.listBoxPort.get(idx);

        Login(self.txtBox, port);

        f = Factory(port,"run.log");

        outStr = f.cc3();
        self.txtBoxCC3.delete("1.0", "end");
        self.txtBoxCC3.insert(tk.END, outStr);

    def _setCC3(self):
        idx  = self.listBoxPort.curselection();
        port = self.listBoxPort.get(idx);

        Login(self.txtBox, port);

        f = Factory(port,"run.log");

        cc3Val = self.txtBoxCC3.get("1.0", tk.END);
        outStr = f.cc3(cc3Val);
        self.txtBoxCC3.delete("1.0", "end");
        self.txtBoxCC3.insert(tk.END, outStr);

    def _getCC4(self):
        idx  = self.listBoxPort.curselection();
        port = self.listBoxPort.get(idx);

        Login(self.txtBox, port);

        f = Factory(port,"run.log");

        outStr = f.cc4();
        self.txtBoxCC4.delete("1.0", "end");
        self.txtBoxCC4.insert(tk.END, outStr);

    def _setCC4(self):
        idx  = self.listBoxPort.curselection();
        port = self.listBoxPort.get(idx);

        Login(self.txtBox, port);

        f = Factory(port,"run.log");

        cc4Val = self.txtBoxCC4.get("1.0", tk.END);
        outStr = f.cc4(cc4Val);
        self.txtBoxCC4.delete("1.0", "end");
        self.txtBoxCC4.insert(tk.END, outStr);


    def _getCC5(self):
        idx  = self.listBoxPort.curselection();
        port = self.listBoxPort.get(idx);

        Login(self.txtBox, port);

        f = Factory(port,"run.log");

        outStr = f.cc5();
        self.txtBoxCC5.delete("1.0", "end");
        self.txtBoxCC5.insert(tk.END, outStr);

    def _setCC5(self):
        idx  = self.listBoxPort.curselection();
        port = self.listBoxPort.get(idx);

        Login(self.txtBox, port);

        f = Factory(port,"run.log");

        cc5Val = self.txtBoxCC5.get("1.0", tk.END);
        outStr = f.cc5(cc5Val);
        self.txtBoxCC5.delete("1.0", "end");
        self.txtBoxCC5.insert(tk.END, outStr);

    def _getCCM(self):
        idx  = self.listBoxPort.curselection();
        port = self.listBoxPort.get(idx);

        Login(self.txtBox, port);

        f = Factory(port,"run.log");

        outStr = f.ccm();
        self.txtBoxCCM.delete("1.0", "end");
        self.txtBoxCCM.insert(tk.END, outStr);

    def _setCCM(self):
        idx  = self.listBoxPort.curselection();
        port = self.listBoxPort.get(idx);

        Login(self.txtBox, port);

        f = Factory(port,"run.log");

        ccmVal = self.txtBoxCCM.get("1.0", tk.END);
        outStr = f.ccm(ccmVal);
        self.txtBoxCCM.delete("1.0", "end");
        self.txtBoxCCM.insert(tk.END, outStr);

    def _getCCS(self):
        idx  = self.listBoxPort.curselection();
        port = self.listBoxPort.get(idx);

        Login(self.txtBox, port);

        f = Factory(port,"run.log");

        outStr = f.ccs();
        self.txtBoxCCS.delete("1.0", "end");
        self.txtBoxCCS.insert(tk.END, outStr);

    def _setCCS(self):
        idx  = self.listBoxPort.curselection();
        port = self.listBoxPort.get(idx);

        Login(self.txtBox, port);

        f = Factory(port,"run.log");

        ccsVal = self.txtBoxCCS.get("1.0", tk.END);
        outStr = f.ccs(ccsVal);
        self.txtBoxCCS.delete("1.0", "end");
        self.txtBoxCCS.insert(tk.END, outStr);

    def _getCFD(self):
        idx  = self.listBoxPort.curselection();
        port = self.listBoxPort.get(idx);

        Login(self.txtBox, port);

        f = Factory(port,"run.log");

        outStr = f.cfd();
        self.txtBoxCFD.delete("1.0", "end");
        self.txtBoxCFD.insert(tk.END, outStr);

    def _setCFD(self):
        idx  = self.listBoxPort.curselection();
        port = self.listBoxPort.get(idx);

        Login(self.txtBox, port);

        f = Factory(port,"run.log");

        cfdVal = self.txtBoxCFD.get("1.0", tk.END);
        outStr = f.cfd(cfdVal);
        self.txtBoxCFD.delete("1.0", "end");
        self.txtBoxCFD.insert(tk.END, outStr);

    def _getCOI(self):
        idx  = self.listBoxPort.curselection();
        port = self.listBoxPort.get(idx);

        Login(self.txtBox, port);

        f = Factory(port,"run.log");

        outStr = f.coi();
        self.txtBoxCOI.delete("1.0", "end");
        self.txtBoxCOI.insert(tk.END, outStr);

    def _getCOI_2(self):
        idx  = self.listBoxPort.curselection();
        port = self.listBoxPort.get(idx);

        Login(self.txtBox, port);

        f = Factory(port,"run.log");

        outStr = f.coi_2();
        self.txtBoxCOI_2.delete("1.0", "end");
        self.txtBoxCOI_2.insert(tk.END, outStr);

if __name__ == "__main__":
    app = App();
    app.portsList();
    app.outputWindow();

    app.frameCC1();
    app.frameCC2();
    app.frameCC3();
    app.frameCC4();
    app.frameCC5();
    app.frameCCM();
    app.frameCCS();
    app.frameCFD();
    app.frameCOI();
    app.frameCOI_2();

    app.loop()