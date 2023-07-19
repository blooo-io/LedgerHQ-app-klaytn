from ragger.backend import RaisePolicy
from ragger.navigator import NavInsID
from ragger.utils import RAPDU

from .apps.solana import SolanaClient, ErrorType, INS
from .apps.solana_cmd_builder import SystemInstructionTransfer, Message, verify_signature
from .apps.solana_utils import FOREIGN_PUBLIC_KEY, FOREIGN_PUBLIC_KEY_2, AMOUNT, AMOUNT_2, SOL_PACKED_DERIVATION_PATH, SOL_PACKED_DERIVATION_PATH_2, KLAYTN_DERIVATION_PATH

from .utils import ROOT_SCREENSHOT_PATH
from binascii import hexlify


def test_klaytn_get_public_key(backend, navigator, test_name):
    sol = SolanaClient(backend)
    from_public_key, address = sol.get_public_key(KLAYTN_DERIVATION_PATH)

    print("Address 0x", address.decode(), sep='')

    assert address == "6E93a3ACfbaDF457F29fb0E57FA42274004c32EA".encode(
    ), "Public key is not the expected one"


def test_klaytn_signLegacyTransaction(backend, navigator, test_name):
    sol = SolanaClient(backend)
    from_public_key, address = sol.get_public_key(KLAYTN_DERIVATION_PATH)
    print("from_public_key", list(from_public_key))
    message: bytes = bytearray.fromhex(
        "058000002c80002019800000008000000080000000e719850ba43b7400830493e0940ee56b604c869e3792c99e35c1c424f88f87dc8a01808220198080")
    with sol.send_async_sign_transaction(message, INS.INS_SIGN_LEGACY_TRANSACTION):
        navigator.navigate_until_text_and_compare(NavInsID.RIGHT_CLICK,
                                                  [NavInsID.BOTH_CLICK],
                                                  "Approve",
                                                  ROOT_SCREENSHOT_PATH,
                                                  test_name)

    signature: bytes = sol.get_async_response().data
    # print("signature: ", signature)
    verify_signature(from_public_key, message, signature)


def test_klaytn_signValueTransfer(backend, navigator, test_name):
    sol = SolanaClient(backend)
    from_public_key, address = sol.get_public_key(KLAYTN_DERIVATION_PATH)
    print("from_public_key", list(from_public_key))
    message: bytes = bytearray.fromhex(
        "058000002c8000201980000000800000008000000008f83b19850ba43b7400830493e0940ee56b604c869e3792c99e35c1c424f88f87dc8a01946694d467b419b36fb719e851cd65d54205df7555c4c3018080")
    with sol.send_async_sign_transaction(message, INS.INS_SIGN_VALUE_TRANSFER):
        navigator.navigate_until_text_and_compare(NavInsID.RIGHT_CLICK,
                                                  [NavInsID.BOTH_CLICK],
                                                  "Approve",
                                                  ROOT_SCREENSHOT_PATH,
                                                  test_name)

    signature: bytes = sol.get_async_response().data
    # print("signature: ", signature)
    verify_signature(from_public_key, message, signature)
