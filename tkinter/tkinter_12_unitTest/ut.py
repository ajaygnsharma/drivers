'''
Created on Jan 29, 2020

@author: asharma
'''
import Login
import Logout
from eventlog    import EventLog
from statlog     import StatLog
from alarms      import Alarms
from temperature import Temperature
from transmitter import Transmitter
from snmp        import SNMP
from sensors     import Sensors
from network     import Network
from metrics     import Metrics
from calibration import Calibration
from system      import User
from tenMHz      import TenMHz
from system      import System

import time
import re

class UT(object):
    def __init__(self, 
                 port: str=None):
        self.port        = ''
        if( port ):
            self.port = port;
            
    def stress_test(self):
        Login.Login(self.port);
        tx = Transmitter.Transmitter(self.port);
        tx.stress_test_log();
        Logout.LogOut(self.port);
            
    def testLogin(self, n):
        for i in range(0, n):
            Login.Login(self.port);
            time.sleep(3 * 60); # 3 minutes
            #Logout.LogOut(self.port);
            
    def readTest(self):
        s = Sensors.Sensors(self.port);
        s.readTest();
        
        tenMHz = TenMHz.TenMHz(self.port);
        tenMHz.readTest();
        
    def calibration(self):
        temperature = (-40,-20,0,20,40,60);
        
        for i in range(0,6):
            print("Temperature: %d\n"%temperature[i]);
            
            Login.Login(self.port);
            calTest = Calibration.Calibration(self.port);
            calTest.CT3_TemperatureCompensation(i);
            Logout.LogOut(self.port);
            Login.Login(self.port); # Disable Calibration
            Logout.LogOut(self.port);
            
            Login.Login(self.port);
            calTest = Calibration.Calibration(self.port);
            calTest.CT1_OutputPowerCalibration(i);
            Logout.LogOut(self.port);
            Login.Login(self.port); # Disable Calibration
            Logout.LogOut(self.port);

            '''
            Login.Login(self.port);
            calTest = Calibration.Calibration(self.port);
            calTest.CT2_InputPowerCalibration();
            Logout.LogOut(self.port);
            Login.Login(self.port); # Disable Calibration
            Logout.LogOut(self.port);
            
            Login.Login(self.port);
            calTest = Calibration.Calibration(self.port);
            calTest.CT4_TemperatureCalibration();
            Logout.LogOut(self.port);
            Login.Login(self.port); # Disable Calibration
            Logout.LogOut(self.port);
            
            Login.Login(self.port);
            calTest = Calibration.Calibration(self.port);
            calTest.CT5_10MHzCalibration();
            Logout.LogOut(self.port);
            Login.Login(self.port); # Disable Calibration
            Logout.LogOut(self.port);
            
            Login.Login(self.port);
            calTest = Calibration.Calibration(self.port);
            calTest.CT9_VVACalibration();
            Logout.LogOut(self.port);
            Login.Login(self.port); # Disable Calibration
            Logout.LogOut(self.port);
            
            Login.Login(self.port);
            calTest = Calibration.Calibration(self.port);
            calTest.CT8_CurrentCalibration();
            Logout.LogOut(self.port);
            Login.Login(self.port); # Disable Calibration
            Logout.LogOut(self.port);
            '''
        
    def Cmds(self, cmdType):
        failTheTest = False;
        Login.Login(self.port); # Disable Calibration
        
        if(cmdType == "tenMHz"):
            tenMHz = TenMHz.TenMHz(self.port);
            tenMHz.runTest();
        elif(cmdType == "transmitter"):
            tx = Transmitter.Transmitter(self.port);
            tx.outputHighThreshold();
            tx.outputLowThreshold();
            tx.inputHighThreshold();
            tx.inputLowThreshold();
            tx.inputPowerLevel();
            tx.outputPowerLevel();
            tx.frequencyBand();
            tx.spectralInversion();
            tx.powerMonitorFreq();
            tx.gainControlMode();
            tx.gainTargetLevel();
            tx.gainControlReset();
            tx.powerMonitorType();
            tx.gainAttenuator();
            tx.powerUpDelay();
            tx.powerUpState();
            tx.outputBurstThreshold();
            tx.state();
            tx.temperatureCompensation();
            tx.temperatureShutdown();
            tx.loID();
            tx.checkTXHours();
            tx.checkBurstThreshold();
        elif(cmdType == "alarms"):
            a = Alarms.Alarms(self.port);
            print("Alarms: %s"%(a.get()));
            a.getMask("major");
            a.getMask("minor");
            a.getMask("suppress");
            a.getSimulated();
            a.getSuppression();
            #if(self.setAlarms):
            #    a.setMask("major","0xff")
            #    a.setMask("minor","0xff")
            #    a.setMask("suppress","0xff")
            # 
            #    a.setSimulated("0xff")
            #    a.setSuppression("0xff")
            del a;
        elif(cmdType == "temperature"):
            t = Temperature.Temperature(self.port);
            #t.getMinMax();
            #t.getInternal();
            t.getOutside();
            #t.getHighTemperatureShutdown();
            del t
        elif(cmdType == "sensors"):
            snsrs = Sensors.Sensors(self.port,0);
            snsrs.checkRaw();
            snsrs.checkCurrent();
            snsrs.checkDROTuneVolts();
            snsrs.checkPowerSupplyVoltage();
            
        elif(cmdType == "system"):    
            _sys = System.System(self.port); 
            _sys.checkExternalLED();
            _sys.check50HzCorrection();
            _sys.checkVersion(0.43);
            _sys.checkSSH();
            _sys.checkWebserver();
            del _sys;
            
        # if(cmdName == "cas"):
        #     casTest = cas.Cas(self.port);
        #     #casTest.get("1");
        #     casTest.set("2dsdsdsdsdsd-dsd-==-ds=dsds=ds-=d-s=d");
        #     casTest.stop();
        #
        # elif(cmdName == "cbt"): # Command Burst threshold
        #     cbtTest = cbt.Cbt(self.port);
        #     cbtTest.test1();
        #     cbtTest.test2();
        #     cbtTest.stop();
        #
        # elif(cmdName == "cc1"): # Modelname command command 1?
        #     cc1Test = cc1.Cc1(self.port);
        #     cc1Test.test1();
        #     cc1Test.test3();
        #     cc1Test.stop();
        # elif(cmdName == "cc2"): # Modelname command command 1?
        #     cc2Test = cc2.Cc2(self.port);
        #     cc2Test.test1();
        #     cc2Test.test2();
        #     cc2Test.stop();
        # elif(cmdName == "cc3"): # Modelname command command 1?
        #     cc3Test = cc3.Cc3(self.port);
        #     cc3Test.test1();
        #     #cc3Test.test2();
        #     cc3Test.stop();
        # elif(cmdName == "cc4"): # Modelname command command 1?
        #     cc4Test = cc4.Cc4(self.port);
        #     cc4Test.test1();
        #     cc4Test.test2();
        #     cc4Test.stop();
        # elif(cmdName == "cc5"): # Modelname command command 1?
        #     cc5Test = cc5.Cc5(self.port);
        #     cc5Test.test1();
        #     cc5Test.test2();
        #     cc5Test.stop();
        # elif(cmdName == "ccf"):
        #     ccfTest = ccf.Ccf(self.port);
        #     ccfTest.test1();
        #     ccfTest.test2();
        #     ccfTest.stop();
        # elif(cmdName == "ccm"):
        #     ccmTest = ccm.Ccm(self.port);
        #     ccmTest.test1();
        #     ccmTest.test2();
        #     ccmTest.stop();
        # elif(cmdName == "ccs"):
        #     ccsTest = ccs.Ccs(self.port);
        #     ccsTest.test1();
        #     ccsTest.test2();
        #     ccsTest.stop();
        # elif(cmdName == "cfd"):
        #     # Don't run this. Fails!
        #     cfdTest = cfd.Cfd(self.port);
        #     cfdTest.test1();
        #     #cfdTest.test2();
        #     cfdTest.stop();
        # elif(cmdName == "cia"):
        #     ciaTest = cia.Cia(self.port);
        #     ciaTest.test1();
        #     ciaTest.test2();
        #     ciaTest.stop();
        # elif(cmdName == "cic"):
        #     cicTest = cic.Cic(self.port);
        #     cicTest.test1();
        #     #cicTest.test2();
        #     cicTest.stop();
        # elif(cmdName == "cid"):
        #     cidTest = cid.Cid(self.port);
        #     cidTest.test1();
        #     #cidTest.test2();
        #     cidTest.stop();
        # elif(cmdName == "cie"):
        #     cieTest = cie.Cie(self.port);
        #     cieTest.test1();
        #     cieTest.test2();
        #     cieTest.test3();
        #     cieTest.stop();
        # elif(cmdName == "ciha"):
        #     cihaTest = ciha.Ciha(self.port);
        #     cihaTest.test1();
        #     #cihaTest.test2();
        #     #cihaTest.test3();
        #     cihaTest.stop();
        # elif(cmdName == "cihd"):
        #     cihdTest = cihd.Cihd(self.port);
        #     cihdTest.test1();
        #     #cihdTest.test2();
        #     #cihdTest.test3();
        #     cihdTest.stop();
        # elif(cmdName == "cim"):
        #     cimTest = cim.Cim(self.port);
        #     cimTest.test1();
        #     cimTest.test2();
        #     cimTest.stop();
        # elif(cmdName == "cis"):
        #     cisTest = cis.Cis(self.port);
        #     cisTest.test1();
        #     cisTest.test2();
        #     #cisTest.test3();
        #     cisTest.stop();
        # elif(cmdName == "cm1"):
        #     cm1Test = cm1.Cm1(self.port);
        # elif(cmdName == "cm2"):
        #     cm2Test = cm2.Cm2(self.port);
        # elif(cmdName == "cms"):
        #     cmsTest = cms.Cms(self.port);
        # elif(cmdName == "coi"):
        #     coiTest = coi.Coi(self.port);
        # elif(cmdName == "cox"):
        #     coxTest = cox.Cox(self.port);
        # elif(cmdName == "cpl"):
        #     cplTest = cpl.Cpl(self.port);
        # elif(cmdName == "crt"):
        #     crtTest = crt.Crt(self.port);
        # elif(cmdName == "csf"):
        #     csfTest = csf.Csf(self.port);
        # elif(cmdName == "ctd"):
        #     tenMHz = TenMHz.TenMHz(self.port);
        #     tenMHz.level();
        # elif(cmdName == "cti"):
        #     pass;
        # elif(cmdName == "ctm"):
        #     itime = Time.Time(self.port, "1");
        # elif(cmdName == "ehi"):
        #     evt = EventLog.EventLog(self.port);
        #     #itime.get();
        # elif(cmdName == "user"):
        #     user = User.User(self.port,"1");
        #     user.list();
        #
        #     user.new("user1","passwd","user");
        #     user.list();
        #
        #     user.delete("user1");
        #     user.list();
        # elif(cmdName == "dro"):
        #     dro = DRO.DRO(self.port);
        # elif(cmdName == "ibs"):
        #     system = System.System(self.port);
        # elif(cmdName == "snmp"):
        #     snmp = SNMP.SNMP(self.port);
        # elif(cmdName == "statlog"):
        #     stl = StatLog.StatLog(self.port);
        # elif(cmdName == "tx"):
        #     tx = Transmitter.Transmitter(self.port);
        #Logout.LogOut(self.port);    
        
        '''
        debug=0;
        if(debug):
            a = adu.Adu("/dev/ttyV0","debug");
        else:
            a = adu.Adu("/dev/ttyV0");
        a.list();
        #a.new("user3","password","user");
        #a.new("user4","password","user");
        #a.new("user5","password","user");
        #a.new("user6","password","user");
        #a.new("user7","password","user");
        #a.new("user8","password","user");
        #a.list();
        a.stop();
        
        '''
    '''
    A test to see when the attenuators come back from calibration to normal
    '''
    def utAttenuatorComeback(self):
        calTest = Calibration.Calibration(self.port);
        print(calTest.tpb());
        calTest.enable();
        calTest.tpb(0,0);
        calTest.tpb(1,0);
        calTest.ttc("4095");
        calTest.disable();
        for i in range(1, 10):
            print(calTest.tpb());
            time.sleep(1);
        calTest.stop();
        
   
            
    def utNetwork(self):
        nw = Network.Network()
        nw.getIPAddress()
        nw.getIPGateway()
        nw.getSubnetMask()
    
    def utNTP(self):
        ntp = NTP.NTP()
        ntp.getIPAddress()
        ntp.getCurrentTime()
        ntp.getTimeZone()
        
        ntp.setTimeZone('8')
        ntp.getCurrentTime()
        
        ntp.setIPAddress('132.163.96.5')
                                
    def utSetTx(self):
        tx = Transmitter.Transmitter()
        tx.setOutputHighThreshold("39")
        tx.setOutputLowThreshold("38.5")
        tx.setInputHighThreshold("-20")
        tx.setInputLowThreshold("-45")
        tx.setOutputBurstThreshold("40")
        tx.setInputPowerLevel("-50")
        tx.setOutputPowerLevel("-20")
        tx.setPowerMonitorFreq("6000")
        tx.setGainMode("2")
        tx.setGainControl("0")
        tx.setPowerMonitorType("1")
        tx.setPowerUpDelay("100")
        tx.setPowerUpState("0")
        tx.setOutput("1")
        del tx
    
    def utStatusLog(self):
        s = StatLog.StatLog()
        s.get()
        s.getTime()
        s.clear()
        s.clearAlt()
        s.setTime("1440")
        del s
    
    def utEventLog(self):
        e = EventLog.EventLog()
        e.get()
        e.clear()
        e.clearAlt()
        del e
    
    def utMetrics(self):
        mt = Metrics.Metrics()
        mt.getFactoryData()
        mt.getTxOnTime()
        mt.getSUFactoryData()
        del mt
