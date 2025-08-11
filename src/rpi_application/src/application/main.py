import os
from web3 import Web3
import threading

import smart_contract_interaction as sc_int
import device_communication as dev_com
import time
import requests
import visualization
import status


def visualize():
    
    visualization.showReady()
    
    #if(status.lock_status == status.LOCKED):
    #    visualization.showLocked()
    #else:
    #    visualization.showUnlocked()
    while(1):

        if(status.action != status.IDLE):
            visualization.showWorking()            
        

def locking():
    
    vehicle_wallet = sc_int.getVehicleWallet()
    
    print("Vehicle wallet address: " + str(vehicle_wallet.address))
    print("Request lock status...")
    lock_status = sc_int.getVehicleLockStatus(vehicle_wallet)
    print("Vehicle currently " + lock_status)
    print("Waiting for commands...")
    
    while(1):
        if(status.action == status.DO_UNLOCK):
            while(status.user_address == ""):
                time.sleep(0.1)
            lock_status = sc_int.unlockVehicle(status.user_address, vehicle_wallet)
            if(lock_status == status.UNLOCKED):
                visualization.openDoors()
        elif(status.action == status.DO_LOCK):
            while(status.user_address == ""):
                time.sleep(0.1)
            lock_status = sc_int.lockVehicle(status.user_address, vehicle_wallet)
            if(lock_status == status.LOCKED):
                visualization.closeDoors()
        else:
            continue
        
        status.action = status.IDLE
        status.lock_status = lock_status
        print("Vehicle currently " + str(lock_status))
        print("Waiting for commands...")
     

def haveInternet(url='http://www.google.com/', timeout=5):
    
    try:
        os.system("sudo ip route del default dev eth0") 
        requests.get(url, timeout=timeout)
        return True
    except:
        print("no conn")
        return False

def main():    
    print("Wait for internet connection...");
    
    #check internet connection
    while(haveInternet() == False):
        time.sleep(1)
    print("Internet connection esablished")
    
    #check private BC running
    while(sc_int.setupBCNetwork() == False):
        time.sleep(1)
    
    #init secure element
    print("Init SE and select scorpion...")  
    if(sc_int.initSE()):
        return -1
    print("Scroption selected")  
    
    #init gpio
    print("Init raspberry gpios...")  
    if(visualization.gpioInit()):
        return -1
    print("Done")  
    
    
    ###### testing
    
    
    
    
    ######### testing end
    
    threadDeviceCommunication = threading.Thread(target=dev_com.start)
    threadSmartContractInteraction = threading.Thread(target=locking)
    threadVisualization = threading.Thread(target=visualize)
    
    threadDeviceCommunication.start()
    threadSmartContractInteraction.start()
    threadVisualization.start()
    
    threadDeviceCommunication.join()
    threadSmartContractInteraction.join() 
    threadVisualization.join()       

if __name__ == '__main__':
    main()
    
        
        
        
        
        
      
    
