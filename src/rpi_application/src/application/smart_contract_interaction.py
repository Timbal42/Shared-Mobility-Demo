from web3 import Web3, middleware
from web3.middleware import ExtraDataToPOAMiddleware # geth_poa_middleware
from web3._utils.events import IGNORE
IGNORE = "ignore"
import time
import rlp
import json
from eth_keys import keys
from eth_utils import decode_hex
## from ethereum.transactions import Transaction
from eth_hash.auto import keccak
from hexbytes import HexBytes
from eth_utils import keccak, to_checksum_address
from attributedict.collections import AttributeDict
from cytoolz import dissoc, assoc
# import ethereum

from eth_account._utils.legacy_transactions import (
    UnsignedTransaction,
    encode_transaction,
    serializable_unsigned_transaction_from_dict
)

import cmd_lib_wrapper as cmd_lib


key_slot = 1
## Signing Account Public Key and Private Key are hardcoded,
## because "w3.eth.accounts" api is not supported by infura, as infura does not store account details.

Signing_Account_pubKey = "0xfa6eABdD950B671E30C10D689Cb126f642B51091"
Signing_Account_privKey = "b7bc586aa62248020a873164bee429c844dff89ae788f2d9c7855659c9594f92"

class Wallet:
  def __init__(self, address, pub_key):
     self.address = address
     self.pub_key = pub_key

def getPermissionMsg(i):
    switcher = {
        0: "User is allowed to unlock this type of vehicle",
        1: "User not registered",
        2: "Vehicle not registerd",
        3: "User not allowed to unlock this type of vehicle",
    }
    return switcher.get(i, "Invalid response")

def setupBCNetwork():
    network_provider = "https://sepolia.infura.io/v3/a23216c694404ccdab49b8eb7cd340d1" # New link to the network provider

    contract_address = "0x79d7F0EFF2D76fA7aA2aea9244B21a88F587A185" # Old contract_address = "0x412E7eB365Cb6DaB3119ec19A5e6D541d31b03bC"
    contract_abi_file = "velink_mobility.json"

    with open(contract_abi_file, 'r') as f:
        parsed = json.load(f)

    global w3
    w3 = Web3(Web3.HTTPProvider(network_provider))

    ## w3.middleware_onion.inject(geth_poa_middleware, layer=0)
    w3.middleware_onion.inject(ExtraDataToPOAMiddleware, layer=0)

    print("[SCI] Check connection to HTTPS provider...")
    conn = w3.is_connected() #w3.isConnected()
    print("[SCI] Connection: " + str(conn))

    if(conn == False):
        return False

    global contract
    contract = w3.eth.contract(address=contract_address, abi=parsed)


def initSE():

    if(cmd_lib.init()):
        return -1
    if(cmd_lib.block2go_select()):
        return -1
    return 0


def mintVehicleToken(vehicle_wallet):
    #generate vehicle token and transfer the vehicle wallet
    print("[SCI] Generating from account: " + str(Signing_Account_pubKey))
    print("[SCI] vehicle_wallet: " + str(vehicle_wallet.address))

    contract_function_mint = contract.functions.mintVehicleToken("car")
    transaction_dictionary = contract_function_mint.build_transaction({'nonce': w3.eth.get_transaction_count(Signing_Account_pubKey), # contract_function_mint.buildTransaction, w3.eth.getTransactionCount
        "from": Signing_Account_pubKey,
        "gasPrice": (w3.eth.gas_price*2)}) #w3.eth.gasPrice*20

    print("[SCI] Transaction build done")
    # sign transaction:
    signed_transaction = w3.eth.account.sign_transaction(transaction_dictionary, Signing_Account_privKey) # w3.eth.account.signTransaction

    # send transaction
    tx_hash = w3.eth.send_raw_transaction(signed_transaction.raw_transaction) # w3.eth.sendRawTransaction(signed_transaction.rawTransaction)
    
    print("[SCI] Waiting for mint receipt...")
    receipt = w3.eth.wait_for_transaction_receipt(tx_hash)
    print("[SCI] Minted - block:", receipt.blockNumber)

    # w3.eth.wait_for_transaction_receipt(tx_hash) # waitForTransactionReceipt
    # print("[SCI] Tx done - Hash: " + str(tx_hash.hex()))
    # time.sleep(0.3)
# #######################################################################
    # token_count = contract.functions.balanceOf(Signing_Account_pubKey).call()
    # time.sleep(0.3)
    # print("token count: " + str(token_count))

    transfer_logs = contract.events.Transfer().process_receipt(receipt, errors=IGNORE)
    if not transfer_logs:
        raise RunTimeError("No Transfer event found in mint tx!")
    token_id = transfer_logs[0]["args"]["tokenId"]
    print("[SCI] Minted token id:", token_id)
    
    
    contract_function_transfer = contract.functions.transfer(vehicle_wallet.address, token_id) # token_id <- token_count - 1

    transaction_dictionary2 = contract_function_transfer.build_transaction({'nonce': w3.eth.get_transaction_count(Signing_Account_pubKey), # contract_function_transfer.buildTransaction({'nonce': w3.eth.getTransactionCount(Signing_Account_pubKey),
        "from": Signing_Account_pubKey,
        "gasPrice": (w3.eth.gas_price*2)}) # w3.eth.gasPrice*20

    print("[SCI] Transaction build done")
    # sign transaction:
    signed_transaction2 = w3.eth.account.sign_transaction(transaction_dictionary2, Signing_Account_privKey) # w3.eth.account.signTransaction

    # send transaction
    tx_hash2 = w3.eth.send_raw_transaction(signed_transaction2.raw_transaction) # w3.eth.sendRawTransaction(signed_transaction.rawTransaction)
    
    print("[SCI] Waiting for transfer receipt")
    w3.eth.wait_for_transaction_receipt(tx_hash2) # w3.eth.waitForTransactionReceipt
    print("[SCI] Token ", token_id, " succesfully transfered to vehicle wallet")
    # print("[SCI] Tx done - Hash: " + str(tx_hash2.hex()))


def getVehicleLockStatus(vehicle_wallet):
    return contract.functions.getVehicleLockStatus(vehicle_wallet.address).call() # getVehicleLockStatus

def unlockVehicle(user_address, vehicle_wallet):

    print("[SCI] Unlocking...")
    time.sleep(0.3) # waiting until file is saved and closed

    print("[SCI] Check Permission...")
    permission = 0
    permission = contract.functions.checkPermission(Web3.to_checksum_address(user_address), vehicle_wallet.address).call() # checkPermission(Web3.toChecksumAddress(user_address)

    print("[SCI] Permission: " + str(permission) + " -> " + getPermissionMsg(permission))
    if permission != 0:
        return permission

    contract_function = contract.functions.vehicleUnlock(vehicle_wallet.address)

    buildAndSendTransaction(contract_function, vehicle_wallet)

    return getVehicleLockStatus(vehicle_wallet)


def lockVehicle(user_address, vehicle_wallet):
    print("[SCI] Locking...")

    contract_function = contract.functions.vehicleLock(vehicle_wallet.address)

    buildAndSendTransaction(contract_function, vehicle_wallet)

    return getVehicleLockStatus(vehicle_wallet)

def getVehicleWallet():
    key_info = bytearray(cmd_lib.block2go_get_pub_key(key_slot))
    pub_key = key_info[1:] # remove prefix -> 02: compressed (X coordinate, Y can be calculated, 04: uncompressed (X, Y coordinate)
    keccak_hash = keccak(pub_key)
    # Take the last 20 bytes
    wallet_len = 40
    addr = Web3.to_checksum_address('0x' + keccak_hash.hex()[-wallet_len:]) # Web3.toChecksumAddress
    return Wallet(addr, pub_key)



def getVFromRecovery(vehicle_wallet, unsigned_transaction, unsigned_transaction_hash, r, s):

    # https://github.com/ethereum/EIPs/blob/master/EIPS/eip-155.md#specification
    # detect chain
    
    
    if isinstance(unsigned_transaction, UnsignedTransaction):
        chain_id = None
        print("None")
    else:
        chain_id = unsigned_transaction.v


    for v_raw in (0,1):
        signature = keys.Signature(vrs=(v_raw, r, s))
        pub = signature.recover_public_key_from_msg_hash(unsigned_transaction_hash)
        if pub == vehicle_wallet.pub_key:
            break
    else:
        raise ValueError("Public key not matching; cannot recover v")
    

    if(chain_id is None):
        v = v_raw + 27
    else:
        v = v_raw + 35 + 2 * chain_id
    return v

def buildAndSendTransaction(contract_function, vehicle_wallet):
    print("[SCI] Build and send transaction...")
    transaction_dictionary = contract_function.build_transaction({'nonce': w3.eth.get_transaction_count(vehicle_wallet.address), # w3.eth.get_transaction_count
        "from": vehicle_wallet.address,
        "gasPrice": (w3.eth.gas_price*2)}) # w3.eth.gasPrice*20

    print("[SCI] Transaction build done")
    # sign transaction:
    signed_transaction = getSignedTransaction(vehicle_wallet, transaction_dictionary)

    # send transaction
    tx_hash = w3.eth.send_raw_transaction(signed_transaction.rawTransaction) # sendRawTransaction

    # wait for completion
    print("[SCI] Wait for transaction receipt...")
    w3.eth.wait_for_transaction_receipt(tx_hash) # w3.eth.waitForTransactionReceipt
    print("[SCI] Tx done - Hash: " + str(tx_hash.hex()))


# adopted from ...accaount.py (fct sign_transaction)
def getSignedTransaction(vehicle_wallet, transaction_dict):

    sanitized_transaction = dissoc(transaction_dict, 'from')
    print(sanitized_transaction)

    unsigned_transaction = serializable_unsigned_transaction_from_dict(sanitized_transaction)
    unsigned_transaction_hash = unsigned_transaction.hash()

    # sign with SE
    print("[SCI] Request signature from transaction hash...")
    sig = bytes(cmd_lib.block2go_sign(key_slot, unsigned_transaction_hash)).hex()
    print("[SCI] Done")

    # extract r and s
    rStart = 6
    length = 2
    rLength = int(sig[rStart:rStart + length], 16)
    rStart += 2;
    r = int(sig[rStart:rStart + rLength * 2], 16)

    sStart = rStart + rLength * 2 + 2;
    sLength = int(sig[sStart:sStart + length], 16)
    sStart += 2
    s = int(sig[sStart:sStart + sLength * 2], 16)

    v = getVFromRecovery(vehicle_wallet, unsigned_transaction, unsigned_transaction_hash, r, s)

    # serialize transaction with rlp
    encoded_transaction = encode_transaction(unsigned_transaction, vrs=(v, r, s))

    transaction_hash = keccak(encoded_transaction)

    signedDict = AttributeDict({
        'rawTransaction': HexBytes(encoded_transaction),
        'hash': HexBytes(transaction_hash),
        'r': r,
        's': s,
        'v': v,
    })

    return signedDict
