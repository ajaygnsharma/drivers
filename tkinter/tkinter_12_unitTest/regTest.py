'''
Created on Jan 29, 2020

@author: asharma
'''
import Login
import EventLog
import StatLog
import Alarms
import Temperature
import Transmitter
import SNMP
import Sensors
import Network
import Time
import Metrics

import time

class RegTest(object):
    def __init__(self, 
                 setAlarm: str=None):
        setup.Setup()
        self.setAlarms = ''
        if( setAlarm ):
            self.setAlarms = setAlarm
            
    def AlarmThresholds(self):
        tx = Transmitter.Transmitter()
        tx.getInputPowerLevel()
        tx.getOutputPowerLevel()
        tx.getOutputHighThreshold()
        tx.getOutputLowThreshold()
        tx.getInputHighThreshold()
        tx.getInputLowThreshold()
        
        ''' Setting Low threshold for input '''
        tx.setInputLowThreshold("-37.5")
        del tx
        
        time.sleep(2)
        
        a = Alarms.Alarms()
        a.getAll(1)
        del a
        
        time.sleep(2)
        
        tx = Transmitter.Transmitter()
        tx.setInputLowThreshold("-60")
        del tx
        
        time.sleep(2)
        
        a = Alarms.Alarms()
        a.getAll(1)
        del a
        
        time.sleep(2)
        
        ''' Setting High threshold for input '''
        tx = Transmitter.Transmitter()
        tx.setInputHighThreshold("-48")
        del tx
        
        time.sleep(2)
        
        a = Alarms.Alarms()
        a.getAll(1)
        del a
        
        time.sleep(2)
        
        tx = Transmitter.Transmitter()
        tx.setInputHighThreshold("-15")
        del tx
        
        time.sleep(2)
        
        a = Alarms.Alarms()
        a.getAll(1)
        del a
        
        ''' Setting High threshold for output '''
        tx = Transmitter.Transmitter()
        tx.setOutputHighThreshold("35")
        del tx
        
        time.sleep(2)
        
        a = Alarms.Alarms()
        a.getAll(1)
        del a
        
        time.sleep(2)
        
        tx = Transmitter.Transmitter()
        tx.setOutputHighThreshold("52")
        del tx
        
        time.sleep(2)
        
        a = Alarms.Alarms()
        a.getAll(1)
        del a
        
        
        
        ''' Setting Low threshold for Output '''
        tx = Transmitter.Transmitter()
        tx.setOutputLowThreshold("40")
        del tx
        
        time.sleep(2)
        
        a = Alarms.Alarms()
        a.getAll(1)
        del a
        
        time.sleep(2)
        
        tx = Transmitter.Transmitter()
        tx.setOutputLowThreshold("31")
        del tx
        
        time.sleep(2)
        
        a = Alarms.Alarms()
        a.getAll(1)
        del a
        
        time.sleep(2)
        