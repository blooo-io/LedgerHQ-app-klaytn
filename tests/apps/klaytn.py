from typing import List, Generator, Tuple
from enum import IntEnum
from contextlib import contextmanager

from ragger.backend import BackendInterface
from ragger.utils import RAPDU


class INS(IntEnum):
    # DEPRECATED - Use non "16" suffixed variants below
    INS_GET_APP_CONFIGURATION16 = 0xF1
    INS_GET_PUBKEY16 = 0xF2
    INS_SIGN_MESSAGE16 = 0xF3
    # END DEPRECATED
    INS_GET_APP_CONFIGURATION = 0x01
    INS_GET_PUBKEY = 0x02
    INS_SIGN_MESSAGE = 0xF6
    INS_SIGN_OFFCHAIN_MESSAGE = 0xF7
    INS_SIGN_LEGACY_TRANSACTION = 0x04
    INS_SIGN_VALUE_TRANSFER = 0x08
    INS_SIGN_VALUE_TRANSFER_MEMO = 0x10
    INS_SIGN_SMART_CONTRACT_DEPLOY = 0x28
    INS_SIGN_SMART_CONTRACT_EXECUTION = 0x30
    INS_SIGN_CANCEL = 0x38


CLA = 0xE0

P1_NON_CONFIRM = 0x00
P1_CONFIRM = 0x01

P1_BASIC = 0x00
P1_FEE_DELEGATED = 0x01
P1_FEE_DELEGATED_WITH_RATIO = 0x02

P2_NONE = 0x00
P2_EXTEND = 0x01
P2_MORE = 0x02

PUBLIC_KEY_LENGTH = 40

MAX_CHUNK_SIZE = 255

STATUS_OK = 0x9000


class ErrorType:
    NO_APP_RESPONSE = 0x6700
    SDK_EXCEPTION = 0x6801
    SDK_INVALID_PARAMETER = 0x6802
    SDK_EXCEPTION_OVERFLOW = 0x6803
    SDK_EXCEPTION_SECURITY = 0x6804
    SDK_INVALID_CRC = 0x6805
    SDK_INVALID_CHECKSUM = 0x6806
    SDK_INVALID_COUNTER = 0x6807
    SDK_NOT_SUPPORTED = 0x6808
    SDK_INVALID_STATE = 0x6809
    SDK_TIMEOUT = 0x6810
    SDK_EXCEPTION_PIC = 0x6811
    SDK_EXCEPTION_APP_EXIT = 0x6812
    SDK_EXCEPTION_IO_OVERFLOW = 0x6813
    SDK_EXCEPTION_IO_HEADER = 0x6814
    SDK_EXCEPTION_IO_STATE = 0x6815
    SDK_EXCEPTION_IO_RESET = 0x6816
    SDK_EXCEPTION_CX_PORT = 0x6817
    SDK_EXCEPTION_SYSTEM = 0x6818
    SDK_NOT_ENOUGH_SPACE = 0x6819
    NO_APDU_RECEIVED = 0x6982
    USER_CANCEL = 0x6985
    UNIMPLEMENTED_INSTRUCTION = 0x6d00
    INVALID_CLA = 0x6e00


def _extend_and_serialize_multiple_derivations_paths(derivations_paths: List[bytes]):
    serialized: bytes = len(derivations_paths).to_bytes(1, byteorder='little')
    for derivations_path in derivations_paths:
        serialized += derivations_path
    return serialized


class KlaytnClient:
    client: BackendInterface

    def __init__(self, client):
        self._client = client

    def get_app_configuration(self) -> bytes:
        rapdu: RAPDU = self._client.exchange(CLA, INS.INS_GET_APP_CONFIGURATION,
                                                  P1_NON_CONFIRM, P2_NONE)
        print('RAPDU: ', rapdu)
        data = rapdu.data
        return data

    def get_public_key(self, derivation_path: bytes) -> Tuple[bytes, bytes]:
        rapdu: RAPDU = self._client.exchange(CLA, INS.INS_GET_PUBKEY,
                                                  P1_NON_CONFIRM, P2_NONE,
                                                  derivation_path)
        print('RAPDU: ', rapdu)
        data = rapdu.data
        
        # Extract the public key
        length_of_public_key = data[0]
        public_key = data[1:1 + length_of_public_key]

        # Convert public key to hexadecimal format for readability
        public_key_hex = public_key.hex()
        print('Public Key:', public_key_hex)

        # Extract the address
        # Assuming the address immediately follows the public key
        offset = 1 + length_of_public_key
        length_of_address = data[offset]  # The length of the address
        address = data[offset + 1: offset + 1 + length_of_address]

        # Convert address to hexadecimal format for readability
        address_hex = address.hex()
        print('Address:', address_hex)

        return public_key, address

    def split_and_prefix_message(self, derivation_path: bytes, message: bytes) -> List[bytes]:
        assert len(message) <= 65535, "Message to send is too long"
        header: bytes = _extend_and_serialize_multiple_derivations_paths([
            derivation_path])
        # Check to see if this data needs to be split up and sent in chunks.
        max_size = MAX_CHUNK_SIZE - len(header)
        message_splited = [message[x:x + max_size]
                           for x in range(0, len(message), max_size)]
        # Add the header to every chunk
        return [header + s for s in message_splited]

    def send_first_message_batch(self, messages: List[bytes], p1: int) -> RAPDU:
        self._client.exchange(CLA, INS.INS_SIGN_MESSAGE,
                              p1, P2_MORE, messages[0])
        for m in messages[1:]:
            self._client.exchange(CLA, INS.INS_SIGN_MESSAGE,
                                  p1, P2_MORE | P2_EXTEND, m)

    @ contextmanager
    def send_async_sign_message(self,
                                derivation_path: bytes,
                                message: bytes) -> Generator[None, None, None]:
        message_splited_prefixed = self.split_and_prefix_message(
            derivation_path, message)

        # Send all chunks with P2_MORE except for the last chunk
        # Send all chunks with P2_EXTEND except for the first chunk
        if len(message_splited_prefixed) > 1:
            final_p2 = P2_EXTEND
            self.send_first_message_batch(
                message_splited_prefixed[:-1], P1_CONFIRM)
        else:
            final_p2 = 0

        with self._client.exchange_async(CLA,
                                         INS.INS_SIGN_MESSAGE,
                                         P1_CONFIRM,
                                         final_p2,
                                         message_splited_prefixed[-1]):
            yield

    @contextmanager
    def send_async_sign_transaction(self, message, ins, p1=P1_BASIC, p2=P1_BASIC):
        with self._client.exchange_async(CLA,
                                         ins,
                                         p1,
                                         p2,
                                         message):
            yield

    def get_async_response(self) -> RAPDU:
        return self._client.last_async_response

    def send_blind_sign_message(self, derivation_path: bytes, message: bytes) -> RAPDU:
        message_splited_prefixed = self.split_and_prefix_message(
            derivation_path, message)

        # Send all chunks with P2_MORE except for the last chunk
        # Send all chunks with P2_EXTEND except for the first chunk
        if len(message_splited_prefixed) > 1:
            final_p2 |= P2_EXTEND
            self.send_first_message_batch(
                message_splited_prefixed[:-1], P1_NON_CONFIRM)
        else:
            final_p2 = 0

        return self._client.exchange(CLA,
                                     INS.INS_SIGN_MESSAGE,
                                     P1_NON_CONFIRM,
                                     final_p2,
                                     message_splited_prefixed[-1])
