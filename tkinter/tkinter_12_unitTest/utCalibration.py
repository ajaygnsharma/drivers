import tkinter

import Login
import Logout
from calibration import Calibration
from calibration import CT0
from calibration import CT1
from calibration import CT2
from calibration import CT3
from calibration import CT4
from calibration import CT5
from calibration import CT8
from calibration import CT9
from calibration import CCF

class UTCalibration(object):
    def __init__(self,
                 txtBox,
                 port: str = None):
        self.port = ''
        if (port):
            self.port = port;

        if(txtBox):
            self.txtBox = txtBox;


    def disableCal(self):
        self.txtBox.insert(tkinter.END, "Disable Cal\n", "ok");
        Logout.LogOut(self.txtBox, self.port);
        Login.Login(self.txtBox, self.port);  # Disable Calibration
        Logout.LogOut(self.txtBox, self.port);

    def utCT0(self):
        temperature = (-40, -20, 0, 20, 40, 60);

        for i in range(0, 1):
            #print("Temperature: %d\n" % temperature[i]);
            Login.Login(self.txtBox, self.port);
            c = Calibration.Calibration(self.txtBox, self.port);
            c.enable();

            ct0 = CT0.CT0(self.txtBox, self.port);
            ct0.run();

            self.disableCal();

    def utCT1(self):
        temperature = (-40, -20, 0, 20, 40, 60);

        for i in range(0, 1):
            #print("Temperature: %d\n" % temperature[i]);
            Login.Login(self.txtBox, self.port);
            c = Calibration.Calibration(self.txtBox, self.port);
            c.enable();

            ct1 = CT1.CT1(self.txtBox, self.port);
            ct1.run();

            self.disableCal();


            '''
            Login.Login(self.txtBox, self.port);
            calTest = Calibration.Calibration(self.txtBox,self.port);
            calTest.CT3_TemperatureCompensation(i);
            Logout.LogOut(self.txtBox, self.port);
            Login.Login(self.txtBox, self.port);  # Disable Calibration
            Logout.LogOut(self.txtBox, self.port);

            '''
            '''
            Login.Login(self.txtBox, self.port);
            calTest = Calibration.Calibration(self.txtBox,self.port);
            ct2Val = calTest.CT2_InputPowerCalibration(i, ct2Val);
            self.disableCal();

            Login.Login(self.txtBox, self.port);
            calTest = Calibration.Calibration(self.txtBox,self.port);
            calTest.CT4_TemperatureCalibration();
            self.disableCal();
            '''
    def utCT2(self):
        temperature = (-40, -20, 0, 20, 40, 60);

        for i in range(0, 1):
            #print("Temperature: %d\n" % temperature[i]);
            Login.Login(self.txtBox, self.port);
            c = Calibration.Calibration(self.txtBox, self.port);
            c.enable();

            ct2 = CT2.CT2(self.txtBox, self.port);
            ct2.run();

            self.disableCal();

    def utCT3(self):
        temperature = (-40, -20, 0, 20, 40, 60);

        for i in range(0, 1):
            #print("Temperature: %d\n" % temperature[i]);
            Login.Login(self.txtBox, self.port);
            c = Calibration.Calibration(self.txtBox, self.port);
            c.enable();

            ct3 = CT3.CT3(self.txtBox, self.port);
            ct3.run();

            ct3.verify();

            self.disableCal();


    def utCT4(self):
        temperature = (-40, -20, 0, 20, 40, 60);

        for i in range(0, 1):
            #print("Temperature: %d\n" % temperature[i]);
            Login.Login(self.txtBox, self.port);
            c = Calibration.Calibration(self.txtBox, self.port);
            c.enable();

            ct4 = CT4.CT4(self.txtBox, self.port);
            ct4.run();

            ct4.verify();

            self.disableCal();

    def utCT5(self):
        temperature = (-40, -20, 0, 20, 40, 60);

        for i in range(0, 1):
            #print("Temperature: %d\n" % temperature[i]);
            Login.Login(self.txtBox, self.port);
            c = Calibration.Calibration(self.txtBox, self.port);
            c.enable();

            ct5 = CT5.CT5(self.txtBox, self.port);
            ct5.run();

            ct5.verify();

            self.disableCal();


    def utCT8(self):
        temperature = (-40, -20, 0, 20, 40, 60);

        for i in range(0, 1):
            #print("Temperature: %d\n" % temperature[i]);
            Login.Login(self.txtBox, self.port);
            c = Calibration.Calibration(self.txtBox, self.port);
            c.enable();

            ct8 = CT8.CT8(self.txtBox, self.port);
            ct8.run();
            ct8.verify();

            self.disableCal();

    def utCT9(self):
        temperature = (-40, -20, 0, 20, 40, 60);

        for i in range(0, 1):
            #print("Temperature: %d\n" % temperature[i]);
            Login.Login(self.txtBox, self.port);
            c = Calibration.Calibration(self.txtBox, self.port);
            c.enable();

            ct9 = CT9.CT9(self.txtBox, self.port);
            ct9.run();
            ct9.verify();

            self.disableCal();


    def utCCF(self):
        temperature = (-40, -20, 0, 20, 40, 60);

        for i in range(0, 1):
            #print("Temperature: %d\n" % temperature[i]);
            Login.Login(self.txtBox, self.port);
            c = Calibration.Calibration(self.txtBox, self.port);
            c.enable();

            ccf = CCF.CCF(self.txtBox, self.port);
            ccf.run();

            ccf.verify();

            self.disableCal();
