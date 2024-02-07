from typing import List
from enum import IntEnum
from ecdsa import VerifyingKey, SECP256k1, BadSignatureError
import sha3 

PROGRAM_ID_SYSTEM = "11111111111111111111111111111111"

# Fake blockhash so this example doesn't need a network connection. It should be queried from the cluster in normal use.
FAKE_RECENT_BLOCKHASH = "11111111111111111111111111111111"

ADDRESS_SENDER = ""
ADDRESS_RECIPIENT = ""


def verify_transaction_signature_from_public_key(transaction: bytes, signature: bytes, from_public_key: bytes):
    try:
        if len(signature) == 65:
            signature = strip_v_from_signature(signature)
        verifying_key = VerifyingKey.from_string(from_public_key, curve=SECP256k1)    
        return verifying_key.verify(signature, transaction, hashfunc=sha3.keccak_256)
    except Exception as e:
        raise e 
     
def strip_v_from_signature(signature: bytes) -> bytes:
    return signature[1:]

def extract_r_and_s(signature_RS: bytes) -> List[bytes]:
    r = signature_RS[0:32]
    s = signature_RS[32:64]
    return [r, s]


class KlaytnTransaction:
    def __init__(self, value: int):
        self.type = 0x02
        self.value = value
        self.sender = ADDRESS_SENDER
        self.recipient = ADDRESS_RECIPIENT
        self.gas = 0
        self.gasPrice = 0
        self.nonce = 0
        self.chainID = 0


class SystemInstruction(IntEnum):
    SignLegacyTransaction = 0x00


class MessageHeader:
    def __init__(self, num_required_signatures: int, num_readonly_signed_accounts: int, num_readonly_unsigned_accounts: int):
        self.num_required_signatures = num_required_signatures
        self.num_readonly_signed_accounts = num_readonly_signed_accounts
        self.num_readonly_unsigned_accounts = num_readonly_unsigned_accounts

    def serialize(self) -> bytes:
        return self.num_required_signatures.to_bytes(1, byteorder='little') + \
            self.num_readonly_signed_accounts.to_bytes(1, byteorder='little') + \
            self.num_readonly_unsigned_accounts.to_bytes(1, byteorder='little')


class AccountMeta:
    pubkey: bytes
    is_signer: bool
    is_writable: bool

    def __init__(self, pubkey: bytes, is_signer: bool, is_writable: bool):
        self.pubkey = pubkey
        self.is_signer = is_signer
        self.is_writable = is_writable

# Only support Transfer instruction for now
# TODO add other instructions if the need arises


class Instruction:
    program_id: bytes
    accounts: List[AccountMeta]
    data: bytes
    from_pubkey: bytes
    to_pubkey: bytes


class SystemInstructionTransfer(Instruction):
    def __init__(self, from_pubkey: bytes, to_pubkey: bytes, amount: int):
        self.from_pubkey = from_pubkey
        self.to_pubkey = to_pubkey
        self.program_id = base58.b58decode(PROGRAM_ID_SYSTEM)
        self.accounts = [AccountMeta(
            from_pubkey, True, True), AccountMeta(to_pubkey, False, True)]
        self.data = (SystemInstruction.Transfer).to_bytes(
            4, byteorder='little') + (amount).to_bytes(8, byteorder='little')

# Cheat as we only support 1 SystemInstructionTransfer currently
# TODO add support for multiple transfers and other instructions if the needs arises


class CompiledInstruction:
    program_id_index: int
    accounts: List[int]
    data: bytes

    def __init__(self, program_id_index: int, accounts: List[int], data: bytes):
        self.program_id_index = program_id_index
        self.accounts = accounts
        self.data = data

    def serialize(self) -> bytes:
        serialized: bytes = self.program_id_index.to_bytes(
            1, byteorder='little')
        serialized += len(self.accounts).to_bytes(1, byteorder='little')
        for account in self.accounts:
            serialized += (account).to_bytes(1, byteorder='little')
        serialized += len(self.data).to_bytes(1, byteorder='little')
        serialized += self.data
        return serialized

# Solana communication message, header + list of public keys used by the instructions + instructions
# with references to the keys array


class Message:
    header: MessageHeader
    account_keys: List[AccountMeta]
    recent_blockhash: bytes
    compiled_instructions: List[CompiledInstruction]

    def __init__(self, instructions: List[Instruction]):
        # Cheat as we only support 1 SystemInstructionTransfer currently
        # TODO add support for multiple transfers and other instructions if the needs arises
        self.header = MessageHeader(2, 0, 1)
        self.account_keys = [instructions[0].from_pubkey,
                             instructions[0].to_pubkey, instructions[0].program_id]
        self.recent_blockhash = base58.b58decode(FAKE_RECENT_BLOCKHASH)
        self.compiled_instructions = [
            CompiledInstruction(2, [0, 1], instructions[0].data)]

    def serialize(self) -> bytes:
        serialized: bytes = self.header.serialize()
        serialized += len(self.account_keys).to_bytes(1, byteorder='little')
        for account_key in self.account_keys:
            serialized += account_key
        serialized += self.recent_blockhash
        serialized += len(self.compiled_instructions).to_bytes(1,
                                                               byteorder='little')
        serialized += self.compiled_instructions[0].serialize()
        return serialized
