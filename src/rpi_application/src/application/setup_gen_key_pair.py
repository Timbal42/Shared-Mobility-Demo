from eth_utils import keccak
import cmd_lib_wrapper as cmd_lib

def getWalletAddressFromPubKey(index):
    key_info = bytearray(cmd_lib.block2go_get_pub_key(index))
    pub_key = key_info[1:] #remove prefix -> 02: compressed (X coordinate, Y can be calculated, 04: uncompressed (X, Y coordinate)
    keccak_hash = keccak(pub_key)
    # Take the last 20 bytes
    wallet_len = 40
    addr = '0x' + keccak_hash.hex()[-wallet_len:]
    return addr

def main():
    print("Init SE and select scorpion...")
    cmd_lib.init()
    cmd_lib.block2go_select()
    print("Scroption selected")

    print("Generate key...")
    key_index = cmd_lib.block2go_gen_key()

    pub_key = cmd_lib.block2go_get_pub_key(key_index)
    print("Public key in slot " + str(key_index) + ":")

    print((bytes(pub_key).hex()) + "\n")

    print("Vehicle Wallet address: " + str(getWalletAddressFromPubKey(key_index)))

if __name__ == '__main__':
    main()
