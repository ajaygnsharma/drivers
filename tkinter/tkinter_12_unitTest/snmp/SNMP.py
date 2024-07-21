'''
Created on Dec 12, 2019

@author: asharma
'''
import serial
import re
import platform
import time

validResponses = ["Use this for SNMPv1/2 only",  "SNMP is disabled"]; 

'''
  SNMP class
  
  @note: the serial command a bit longer than usual so a 10 s timeout is given
   
'''
class SNMP(object):
    def __init__(self, port, debug: str=None):
        self.debug = 0;
        if(debug):
            self.debug = 1;
            
        self.ser = serial.Serial(port, 115200, timeout=1);
        self.testView();
    
    def parseRsp(self, lines):
        ok = False;
        for l in lines:
            s = l.decode("utf-8");
            for resp in validResponses:
                if(resp in s):
                    ok = True;
        
        assert (ok == True), lines;
        if(debug):
            print(lines,end="");
            
        return True;
    
    # Add an acces and remove one
    def test1(self):
        s = self.access();
        print(s);
        
        self.access("iBUCGroup", "priv", "systemview", "systemview", "none");
        
        time.sleep(5); # It takes few seconds for the snmp service to restart
        
        s = self.access();
        print(s);
        
        s = self.rmAccess("iBUCGroup", "priv");
        print(s);
        
        time.sleep(5); # It takes few seconds for the snmp service to restart
        
        s = self.access();
        print(s);
        
    def test2(self):
        n = self.rmNotification("warm_boot", ".1.3.6.1.4.1.21369");
        print(n);
        time.sleep(5); # It takes few seconds for the snmp service to restart
        n = self.rmNotification("warm_boot", ".1.3.6.1.4.1.21369");
        print(n);
        time.sleep(5);
        n = self.rmNotification("warm_boot", ".1.3.6.1.4.1.21369");
        print(n);
        time.sleep(5);

        n = self.notification();
        print(n);
        
        n = self.notification("warm_boot", ".1.3.6.1.4.1.21369", "included", "0x00");
        print(n);
        time.sleep(5);
        
        n = self.notification();
        print(n);
        
        n = self.rmNotification("warm_boot", ".1.3.6.1.4.1.21369");
        print(n);
        time.sleep(5);
        
        n = self.notification();
        print(n);
        
    def printlines(self, lines):
        for l in lines:
            print(l.decode("utf-8"), end="");
        
    def testTrapSession(self):
        #lines = self.rmTrapSession("no_coldstart", "10.10.19.108");
        #lines = self.rmTrapSession("no_coldstart", "10.10.19.108");
        #lines = self.rmTrapSession("no_coldstart", "10.10.19.108");
        #lines = self.rmTrapSession("no_coldstart", "10.10.19.108");
        #time.sleep(5);
        
        lines = self.trapSession();
        self.printlines(lines);
        
        n = self.trapSession("no_coldstart", "auth_priv", "priv", "SHA", "12345678", "AES", "12345678", "10.10.19.108");
        print(n);
        
        time.sleep(5);
        
        lines = self.trapSession();
        self.printlines(lines);
        
        lines = self.rmTrapSession("no_coldstart", "10.10.19.108");
        time.sleep(5);

        lines = self.trapSession();
        self.printlines(lines);
        
    def testUser(self):
        lines = self.user();
        self.printlines(lines);
        lines = self.user("user1", "auth", "SHA", "snmpv31234", "AES", "snmpv31234", "iBUCGroup");
        
        time.sleep(5); # <---- Put this time delay in the user function itself.
        
        lines = self.user();
        self.printlines(lines);
        
        lines = self.rmUser("user1");
        time.sleep(5);
        
        self.printlines(lines);
        time.sleep(5);
        
        lines = self.user();
        self.printlines(lines);
        
    def testView(self):
        lines = self.view();
        self.printlines(lines);
        
        lines = self.view("systemview", "included", ".1.3.6.1.4.1.21369.1", "0xff");
        self.printlines(lines);
        
        lines = self.rmView("systemview",".1.3.6.1.4.1.21369.1");
        self.printlines(lines);
        
        lines = self.view();
        self.printlines(lines);
            
    def access(self,name=None,level=None,roView=None,rwView=None,notifyView=None):
        if(name and level and roView and rwView and notifyView):
            print("+++++ Adding SNMP Groups");
            cmdStr = str.encode(name)+b','+str.encode(level)+b','+str.encode(roView)+b','+str.encode(rwView)+b','+str.encode(notifyView)+b'\n';
            print(cmdStr);
            self.ser.write(b'SGA='+cmdStr);
        else:
            print("***** SNMP Groups");
            self.ser.write(b'SGA\n');
        lines = self.ser.readlines();
        return lines;

    def rmAccess(self, name, level):
        print("------ Removing SNMP Group: %s"%name);
        cmdStr = str.encode(name)+b','+str.encode(level)+b'\n';
        self.ser.write(b'SGD='+cmdStr);
        lines = self.ser.readlines();
        return lines;
    
    def notification(self,name=None,subtree=None,flag=None,mask=None):
        if(name and subtree and flag and mask):
            print("+++++ Adding SNMP Notification");
            cmdStr = str.encode(name)+b','+str.encode(subtree)+b','+str.encode(flag)+b','+str.encode(mask)+b'\n';
            self.ser.write(b'SNA='+cmdStr);
        else:
            print("***** SNMP Notification");
            self.ser.write(b'SNA\n');
        lines = self.ser.readlines();
        return lines;
    
    def rmNotification(self, name, oidTree):
        print("------ Removing SNMP Notification: %s"%name);
        cmdStr = str.encode(name)+b','+str.encode(oidTree)+b'\n';
        self.ser.write(b'SND='+cmdStr);
        lines = self.ser.readlines();
        return lines;

    def trapSession(self,profileName=None,userName=None,privilege=None,auth=None,authPass=None,enc=None,encPass=None,ipAddr=None):
        if(profileName and userName and privilege and auth and authPass and enc and encPass and ipAddr):
            print("+++++ Adding SNMP Trap session");
            cmdStr = str.encode(profileName)+b','+str.encode(userName)+b','+str.encode(privilege)+b','+str.encode(auth)+b','+str.encode(authPass)+b','+str.encode(enc)+b','+str.encode(encPass)+ b','+str.encode(ipAddr)+b'\n';
            self.ser.write(b'STA='+cmdStr);
        else:
            print("***** SNMP Trap session");
            self.ser.write(b'STA\n');
        lines = self.ser.readlines();
        ok = self.parseRsp(lines);
        assert (ok == True);
        return lines;

    def rmTrapSession(self, profileName, trapIP):
        print("------ Removing SNMP Trap Session: %s"%profileName);
        cmdStr = str.encode(profileName)+b','+str.encode(trapIP)+b'\n';
        self.ser.write(b'STD='+cmdStr);
        lines = self.ser.readlines();
        return lines;
    
    def user(self, name=None, level=None, auth=None, authPass=None, enc=None, encPass=None, group=None):
        if(name and level and auth and authPass and enc and encPass and group):
            print("Adding user: "+name);
            cmd = 'SUA='+name+','+level+','+auth+','+authPass+','+enc+','+encPass+','+group+'\n';
        else:
            print("***Printing users:");
            cmd = "SUA\n";
            
        byte_arr = str.encode(cmd)
        self.ser.write(byte_arr)
        lines = self.ser.readlines()
        return lines; 
    
    def rmUser(self, userName):
        print("------ Removing SNMP Users: %s"%userName);
        cmdStr = str.encode(userName)+b'\n';
        self.ser.write(b'SUD='+cmdStr);
        lines = self.ser.readlines();
        return lines;
    
    def view(self, name=None, flag=None, oid=None, mask=None):
        if(name and flag and oid and mask):
            print("++++++++Adding view");
            cmd = 'SVA='+name+','+flag+','+oid+','+mask+'\n';
            byte_arr = str.encode(cmd);
            self.ser.write(byte_arr);
            time.sleep(5);
        else:
            print("********Listing views");
            cmd = "SVA\n";
            byte_arr = str.encode(cmd);
            self.ser.write(byte_arr);
            
        lines = self.ser.readlines()
        return lines;  
    
    def rmView(self, name, oid):
        cmdStr   = "SVD="+name+","+oid+"\n";
        byte_arr = str.encode(cmdStr);
        self.ser.write(byte_arr)
        time.sleep(10);
        lines = self.ser.readlines();
        return lines;
                     
    def getTrapIP(self, which):
        if( which == "1"):
            cmd = 'cih1\n'
        else:
            cmd = 'cih2\n'
        byte_arr = str.encode(cmd)
        self.ser.write(byte_arr)
        line = self.ser.readline()
        print("Trap IP"+which+":\n"+line.decode('utf=8'))
    
    def setTrapIP(self, which, ipaddr):
        if( which == "1"):
            cmd = 'cih1='+ipaddr+'\n'
        else:
            cmd = 'cih2='+ipaddr+'\n'
        byte_arr = str.encode(cmd)
        self.ser.write(byte_arr)
        line = self.ser.readline()
        print("Trap IP"+which+":\n"+line.decode('utf=8'))
        
    def getTrapStatus(self, which):
        if( which == "1"):
            cmd = 'cit1\n'
        else:
            cmd = 'cit2\n'
        byte_arr = str.encode(cmd)
        self.ser.write(byte_arr)
        line = self.ser.readline()
        print("Trap Status\n"+line.decode('utf=8'))    

    def setTrapStatus(self, which, val):
        if(which == '1'):
            if(val == "on"):
                print("Enabling Trap 1")
                cmd = 'cit1=1\n'
            else:
                print("Disabling Trap 1")
                cmd = 'cit1=0\n'                
        else:
            if(val == "on"):
                print("Enabling Trap 2")
                cmd = 'cit2=1\n'
            else:
                print("Disabling Trap 2")
                cmd = 'cit2=0\n'                
        
        byte_arr = str.encode(cmd)
        self.ser.write(byte_arr)
        line = self.ser.readline()
        print("Trap Status\n"+line.decode('utf=8'))    
    
    def getCommunity(self):        
        print("Getting community")
        cmd = 'cic\n'
        byte_arr = str.encode(cmd)
        self.ser.write(byte_arr)
        line = self.ser.readline()
        print(line.decode('utf=8'))   
        
    def setCommunity(self, val):        
        print("Setting community")
        cmd = 'cic='+val+'\n'
        byte_arr = str.encode(cmd)
        self.ser.write(byte_arr)
        line = self.ser.readline()
        print(line.decode('utf=8'))
    
    # Make sure version is 0/1/2/3 and nothing else.
    def testCISRsp(self, lines):
        ok = False;
        for l in lines:
            s = l.decode("utf-8");
            expList = re.findall("\d", s);
            if(debug):
                print(expList);
            if( len(expList) > 0 ):
                assert ((float(expList[0]) >= 0) and ((float(expList[0]) <= 3))), expList;
        
        if(debug):
            print(lines,end="");
            
        return True;
    
    def version(self, num=None):
        if(num):
            print("+++++ Setting SNMP version: %s"%num);
            cmd = 'CIS='+num+'\n'
        else:
            print("***** SNMP version:");
            cmd = "CIS\n";
        
        byte_arr = str.encode(cmd);
        self.ser.write(byte_arr);
        lines = self.ser.readlines();
        testCISRsp(lines);
        
    def __del__(self):
        self.ser.close()   
