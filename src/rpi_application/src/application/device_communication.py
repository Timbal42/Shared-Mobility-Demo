from pybleno import *
import array
import sys
import status
import re

class EchoCharacteristic(Characteristic):
    
    def __init__(self, uuid):
        Characteristic.__init__(self, {
            'uuid': uuid,
            'properties': ['read', 'write', 'writeWithoutResponse', 'notify'],
            'value': None
          })
          
        self._value = array.array('B', [0] * 0)
        self._updateValueCallback = None
          
    def onReadRequest(self, offset, callback):
        print("[DEV] read request")
        if offset:
            callback(Characteristic.RESULT_ATTR_NOT_LONG, None)
        else:
            ret_val = [1, 1] #unlocked
            if(status.lock_status == status.LOCKED):
                ret_val = [0, 0] #locked
                print("[DEV] return locked")
            elif(status.lock_status == status.UNLOCKED):
                print("[DEV] return unlocked")
            
            callback(Characteristic.RESULT_SUCCESS, array.array('B', ret_val))

    def onWriteRequest(self, data, offset, withoutResponse, callback):
        global action
        
        self._value = data
        encoding = 'utf-8'
        
        data = str(data, encoding)
        if(data.startswith("un", 0, 2)):
            status.action = status.DO_UNLOCK
        elif(data.startswith("lo", 0, 2)):
            status.action = status.DO_LOCK
            
        print("[DEV] received data: " + data)
        
        # status.user_address = data[data.find("0x"):]
        match = re.search(r"0x[a-fA-f0-9]{40}", data)
        if match:
            status.user_address = match.group(0)
            print("Extracted user address:", status.user_address)
        else:
            print("Invalid address format!")
            callback(Characteristic.RESULT_UNLIKELY_ERROR)
            return
        
        print("address: " + status.user_address)
        
        if self._updateValueCallback:
            print('[DEV] EchoCharacteristic - onWriteRequest: notifying')
            self._updateValueCallback(self._value)
            
        callback(Characteristic.RESULT_SUCCESS)
        
    def onSubscribe(self, maxValueSize, updateValueCallback):
        print('[DEV] EchoCharacteristic - onSubscribe')
        
        self._updateValueCallback = updateValueCallback

    def onUnsubscribe(self):
        print("[DEV] EchoCharacteristic - onUnsubscribe");
        
        self._updateValueCallback = None


def onStateChange(state):
   print("[DEV] on -> stateChange: " + state);

   if (state == "poweredOn"):
     
     bleno.startAdvertising("Raspberry Car", ["0000ec00-0000-1000-8000-00805f9b34fb"]) # ec00
   else:
     bleno.stopAdvertising();


def onAdvertisingStart(error):
    print("[DEV] on -> advertisingStart: " + ("error " + error if error else "success"));
    print("[DEV] Waiting for my next ride...")

    if not error:
        bleno.setServices([
            BlenoPrimaryService({
                "uuid": "0000ec00-0000-1000-8000-00805f9b34fb", #ec00
                "characteristics": [ 
                    EchoCharacteristic("0000ec0F-00001000-8000-00805f9b34fb") # ec0E or ec0F
                    ]
            })
        ])

def start():
    global bleno
    bleno = Bleno()
    bleno.on("stateChange", onStateChange)
    bleno.on("advertisingStart", onAdvertisingStart)
    bleno.start()

    if (sys.version_info > (3, 0)):
        input()
    else:
        raw_input()

    bleno.stopAdvertising()
    bleno.disconnect()

