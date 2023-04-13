from ragger.backend import RaisePolicy
from ragger.navigator import NavInsID
from ragger.utils import RAPDU

from .apps.solana import SolanaClient, ErrorType
from .apps.solana_cmd_builder import SystemInstructionTransfer, Message, verify_signature
from .apps.solana_utils import FOREIGN_PUBLIC_KEY, FOREIGN_PUBLIC_KEY_2, AMOUNT, AMOUNT_2, SOL_PACKED_DERIVATION_PATH, SOL_PACKED_DERIVATION_PATH_2, KLAYTN_DERIVATION_PATH

from .utils import ROOT_SCREENSHOT_PATH
from binascii import hexlify


def test_klaytn_get_public_key(backend, navigator, test_name):
    sol = SolanaClient(backend)
    from_public_key = sol.get_public_key(KLAYTN_DERIVATION_PATH)

    offset = 1 + from_public_key[0]
    address = from_public_key[offset + 1: offset + 1 + from_public_key[offset]]

    print("Address 0x", address.decode(), sep='')

    assert address == "6E93a3ACfbaDF457F29fb0E57FA42274004c32EA".encode(
    ), "Public key is not the expected one"
