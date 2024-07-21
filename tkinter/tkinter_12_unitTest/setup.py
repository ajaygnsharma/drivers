'''
Created on Dec 12, 2019

@author: asharma
'''
import serial
import time
import platform
import re
import sys

'''
> Welcome to the IBUC[CR][LF] 1      
> [CR]ibuc login:        
<< terrasat[CR][LF]
> terrasat[CR][LF]
> "Hi"[CR][LF]
> Password: 
<< passwd[LF]
> [CR][LF]  

CC1=IBB058064-0N7020DSWW-0402
CC2=TE8126026
CC3=0001
CC4=5400
CPL=Terrasat Inc
'''
class Setup(object):
    def __init__(self):
        osType = platform.system()
        self.ser = 0
        
        if(osType == "Windows"):
            self.ser = serial.Serial('COM1', 115200, timeout=1)
        elif(osType == "Linux"):        
            self.ser = serial.Serial('/dev/ttyV0', 115200, timeout=1)
        
        self.ser.write(b'\n')
        line = self.ser.readline() #  CR CR LF
        print(line)
        line = self.ser.readline() # Welcome to the iBUC CR LF  Or IBUC> 
        print(line)
            
        x = re.findall('IBUC>|Welcome', line.decode("utf-8"))
        if(x[0] == 'Welcome'):
            line = self.ser.readline() # ibuc login:
            print(line)
            line = self.ser.read(12)
            print(line)
            self.ser.write(b'terrasat\r\n')
            line = self.ser.readline() # terrasat
            print(line)
            line = self.ser.readline() # Hi CR LF
            print(line)
            line = self.ser.read(10) # Password:
            print(line)
            self.ser.write(b'passwd\n')
            line = self.ser.readline() # CR LF Last Login: ....
            print(line)            
        else:
            self.ser.write(b'CPL\n')
            line = self.ser.readline()
            print(line)
            line = self.ser.readline()
            print(line)
            line = self.ser.readline()
            print(line)
        
        x = re.findall('Terrasat|Undefined', line.decode("utf-8"))
        if((x[0] == 'Undefined')):
            self.ser.write(b'cc1=IBR058064-1NA126WW-1613\n')
            line = self.ser.readline()
            
            self.ser.write(b'cc2=TE8117680\n')
            line = self.ser.readline()
        
            self.ser.write(b'cc4=5100\n')
            line = self.ser.readline()
        
            self.ser.write(b'cpl=Terrasat Inc\n')
            line = self.ser.readline()
            
            self.ser.write(b'tzo=-07:00\n')
            line = self.ser.readline()
            
            self.ser.write(b'ctm\n')
            line = self.ser.readline()
            
            self.ser.write(b'cpl\n')
            line = self.ser.readline()
            self.ser.write(b'czz\n')
            self.ser.close()
            print("New Setup Done, Rebooting...")
            time.sleep(15)
            sys.exit()
            
        elif(x[0] == 'Terrasat'):
            self.ser.write(b'coi\n')
            line = self.ser.readline()
            print("%-30s %s"%("Private Label",line.decode("utf-8")), end='' )
            
            line = self.ser.readline()
            print("%-30s %s"%("Model",line.decode("utf-8")), end='' )
        
            line = self.ser.readline()
            print("%-30s %s"%("SN",line.decode("utf-8")), end='' )
            
            line = self.ser.readline()
            print("%-30s %s"%("FW Ver",line.decode("utf-8")), end='' )
            
            line = self.ser.readline()
            print("%-30s %s"%("Power",line.decode("utf-8")), end='' )
            
            line = self.ser.readline()
            print("%-30s %s"%("Range",line.decode("utf-8")), end='' )
            print("")
            self.ser.close()    
