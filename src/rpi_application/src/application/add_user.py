import sys, json
from web3 import Web3
from eth_account import Account
import smart_contract_interaction as sci

network_provider = "https://sepolia.infura.io/v3/a23216c694404ccdab49b8eb7cd340d1" ## RPC_URL of the network provider
contract_address = "0x79d7F0EFF2D76fA7aA2aea9244B21a88F587A185" # address of the smart contract

with open("velink_mobility.json") as f:
	ABI = json.load(f)	

if len(sys.argv) != 3:
	print("Usage: python3 add_user.py 0xUserPrivatekey \"car\"")
	sys.exit(1)

private_key = sys.argv[1]
vehicle_type = sys.argv[2]

# user_addr = Web3.to_checksum_address(sys.argv[1])

account = Web3().eth.account.from_key(private_key)
w3 = Web3(Web3.HTTPProvider(network_provider))
contract = w3.eth.contract(address=contract_address, abi=ABI)

# admin = w3.eth.account.from_key(sci.Signing_Account_private_key)

# w3, contract = sci.loadContract()
# admin = sci.getAdminWallet()

# mints a User token
tx = contract.functions.mintUserToken(vehicle_type).build_transaction({
		"from": account.address,
		"nonce": w3.eth.get_transaction_count(account.address),
		"gas": 200_000,
		"gasPrice": w3.to_wei("1", "gwei")
})

signed = account.sign_transaction(tx)
tx_hash = w3.eth.send_raw_transaction(signed.raw_transaction)

print("TX sent ->", tx_hash.hex())
print("Waiting ...")

receipt = w3.eth.wait_for_transaction_receipt(tx_hash)
print("Success" if receipt.status else "Failed")
