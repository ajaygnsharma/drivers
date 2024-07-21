import serial
import re
import platform
from lib2to3.pgen2.token import OP
import lib as ibbLib

max = 300;

class factory(object):
    def __init__(self, port, debug: str=None):
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
            self.ser.write(b'CC1='+str.encode(val)+"\n");
        else:
            self.ser.write(b"CC1=\n");
        opList = self.ser.readlines();
        found = False;
        for opStr in opList:
            modelNameList = opStr.split("=");
            if(len(modelNameList) > 0):
                for model in modelNameList:
                    if("IBB" in model):
                        print("ModelName OK");
                        found = True;
        
            
        assert (found == True), modelNameList;
    
    # Bear in mind that the CBT=Number followed by String will convert
    # the number and ignore the string. For example cbt=3fdfdf will make cbt=3
    def cc2(self, val=None):
        if( val != None ):
            self.ser.write(b'CC2='+str.encode(val)+"\n");
        else:
            self.ser.write(b"CC2=\n");
        
        opList = self.ser.readlines();
        ok = False;
        for opStr in opList:
            serialNum = opStr.decode("utf-8");
            if("invalid value" in serialNum):
                ok = True;
            elif("TE" in serialNum):
                ok = True;
        
        assert (ok == True), opList;        
    
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
            elif(__isHWRevValid(hwRev)):
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
        if( val != None ):
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
        
    def stop(self):
        self.ser.close();
    
