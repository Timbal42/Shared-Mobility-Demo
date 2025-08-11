import smart_contract_interaction as sc_int


def main():    
    #init secure element
    print("Init SE and select scorpion...")  
    if(sc_int.initSE()):
        return -1
    print("Scroption selected")  
    
    #check private BC running
    while(sc_int.setupBCNetwork() == False):
        time.sleep(1)

    print("Generate vehicle token and transfer to vehicle wallet...")
    vehicle_wallet = sc_int.getVehicleWallet()
    sc_int.mintVehicleToken(vehicle_wallet)
    
    print("Vehicle registered - ready to use")
    

if __name__ == '__main__':
    main()
