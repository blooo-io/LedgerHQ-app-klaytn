from ragger.backend import RaisePolicy
from ragger.navigator import NavInsID
from ragger.utils import RAPDU

from .apps.solana import SolanaClient, ErrorType, INS, P1_FEE_DELEGATED, P1_FEE_DELEGATED_WITH_RATIO, P1_BASIC
from .apps.solana_cmd_builder import SystemInstructionTransfer, Message
from .apps.klaytn_cmd_builder import verify_transaction_signature_from_public_key
from .apps.solana_utils import FOREIGN_PUBLIC_KEY, FOREIGN_PUBLIC_KEY_2, AMOUNT, AMOUNT_2, SOL_PACKED_DERIVATION_PATH, SOL_PACKED_DERIVATION_PATH_2, KLAYTN_DERIVATION_PATH

from .utils import ROOT_SCREENSHOT_PATH
from binascii import hexlify


def perform_test_that_verifies_signature(backend, navigator, test_name, message_hex, ins_value, p1_value=P1_BASIC):
    sol = SolanaClient(backend)
    from_public_key, address = sol.get_public_key(KLAYTN_DERIVATION_PATH)
    print("from_public_key", list(from_public_key))
    derivation_path_hex = '058000002c80002019800000000000000000000000'
    derivation_path_bytes = bytearray.fromhex(derivation_path_hex)
    message_bytes = bytearray.fromhex(message_hex)
    data: bytes = derivation_path_bytes + message_bytes

    with sol.send_async_sign_transaction(data, ins_value, p1_value):
        navigator.navigate_until_text_and_compare(NavInsID.RIGHT_CLICK,
                                                    [NavInsID.BOTH_CLICK],
                                                    "Approve",
                                                    ROOT_SCREENSHOT_PATH,
                                                    test_name)

    signature: bytes = sol.get_async_response().data
    result = verify_transaction_signature_from_public_key(message_bytes, signature, from_public_key)
    print("-----------------RESULT-----------------", result)
    assert result, "Signature is not valid"

def test_klaytn_get_app_configuration(backend,navigator, test_name):
    sol = SolanaClient(backend)
    app_config = sol.get_app_configuration() # (allow_blind_sign, major_version, minor_version, patch_version)
    allow_blind_sign = app_config[0]
    version = f"{app_config[1]}.{app_config[2]}.{app_config[3]}"
    assert allow_blind_sign == False, "Blind sign is not the expected one"
    assert version == "1.0.0", "Version is not the expected one"

def test_klaytn_get_public_key(backend, navigator, test_name):
    sol = SolanaClient(backend)
    from_public_key, address = sol.get_public_key(KLAYTN_DERIVATION_PATH)

    print("Address 0x", address.decode(), sep='')

    assert address == "6E93a3ACfbaDF457F29fb0E57FA42274004c32EA".encode(
    ), "Public key is not the expected one"

def test_klaytn_signLegacyTransaction(backend, navigator, test_name):
    message_hex = 'e719850ba43b7400830493e0940ee56b604c869e3792c99e35c1c424f88f87dc8a01808203e98080'
    perform_test_that_verifies_signature(backend, navigator, test_name, message_hex, INS.INS_SIGN_LEGACY_TRANSACTION)

def test_klaytn_signValueTransfer(backend, navigator, test_name):
    message_hex = 'f84eb847f8450882115c850ba43b7400830493e0940ee56b604c869e3792c99e35c1c424f88f87dc8a8ca18f07d736b90be550000001946e93a3acfbadf457f29fb0e57fa42274004c32ea8203e98080'
    perform_test_that_verifies_signature(backend, navigator, test_name, message_hex, INS.INS_SIGN_VALUE_TRANSFER)

def test_klaytn_signValueTransferMemo(backend, navigator, test_name):
    message_hex = 'f846b83ff83d1019850ba43b7400830493e0940ee56b604c869e3792c99e35c1c424f88f87dc8a01946e93a3acfbadf457f29fb0e57fa42274004c32ea8568656c6c6f8203e98080'
    perform_test_that_verifies_signature(backend, navigator, test_name, message_hex, INS.INS_SIGN_VALUE_TRANSFER_MEMO)

def test_klaytn_signSmartContractDeploy(backend, navigator, test_name):
    message_hex = ("f2aceb2819850ba43b7400830493e08001946e93a3acfbadf457f29fb0e57fa42274004c32ea8568656c6c6f80808203e98080")
    perform_test_that_verifies_signature(backend, navigator, test_name, message_hex, INS.INS_SIGN_SMART_CONTRACT_DEPLOY)

def test_klaytn_signSmartContractExecution(backend, navigator, test_name):
    message_hex = ("f846b83ff83d3019850ba43b7400830493e0940ee56b604c869e3792c99e35c1c424f88f87dc8a01946e93a3acfbadf457f29fb0e57fa42274004c32ea8568656c6c6f8203e98080")
    perform_test_that_verifies_signature(backend, navigator, test_name, message_hex, INS.INS_SIGN_SMART_CONTRACT_EXECUTION)

def test_klaytn_signCancel(backend, navigator, test_name):
    message_hex = ("e8a2e13819850ba43b7400830493e0946e93a3acfbadf457f29fb0e57fa42274004c32ea8203e98080")
    perform_test_that_verifies_signature(backend, navigator, test_name, message_hex, INS.INS_SIGN_CANCEL)

def test_klaytn_signFeeDelegatedValueTransfer(backend, navigator, test_name):
    message_hex = ("f83fb838f70919850ba43b7400830493e0940ee56b604c869e3792c99e35c1c424f88f87dc8a01946e93a3acfbadf457f29fb0e57fa42274004c32ea8203e98080")
    perform_test_that_verifies_signature(backend, navigator, test_name, message_hex, INS.INS_SIGN_VALUE_TRANSFER, P1_FEE_DELEGATED)

def test_klaytn_signFeeDelegatedValueTransferMemo(backend, navigator, test_name):
    message_hex = ("f846b83ff83d1119850ba43b7400830493e0940ee56b604c869e3792c99e35c1c424f88f87dc8a01946e93a3acfbadf457f29fb0e57fa42274004c32ea8568656c6c6f8203e98080")
    perform_test_that_verifies_signature(backend, navigator, test_name, message_hex, INS.INS_SIGN_VALUE_TRANSFER_MEMO, P1_FEE_DELEGATED)

def test_klaytn_signFeeDelegatedSmartContractDeploy(backend, navigator, test_name):
    message_hex = ("f2aceb2919850ba43b7400830493e08001946e93a3acfbadf457f29fb0e57fa42274004c32ea8568656c6c6f80808203e98080")
    perform_test_that_verifies_signature(backend, navigator, test_name, message_hex, INS.INS_SIGN_SMART_CONTRACT_DEPLOY, P1_FEE_DELEGATED)

def test_klaytn_signFeeDelegatedSmartContractExecution(backend, navigator, test_name):
    message_hex = ("f846b83ff83d3119850ba43b7400830493e0940ee56b604c869e3792c99e35c1c424f88f87dc8a01946e93a3acfbadf457f29fb0e57fa42274004c32ea8568656c6c6f8203e98080")
    perform_test_that_verifies_signature(backend, navigator, test_name, message_hex, INS.INS_SIGN_SMART_CONTRACT_EXECUTION, P1_FEE_DELEGATED)

def test_klaytn_signFeeDelegatedCancel(backend, navigator, test_name):
    message_hex = ("e8a2e13919850ba43b7400830493e0946e93a3acfbadf457f29fb0e57fa42274004c32ea8203e98080")
    perform_test_that_verifies_signature(backend, navigator, test_name, message_hex, INS.INS_SIGN_CANCEL, P1_FEE_DELEGATED)

def test_klaytn_signFeeDelegatedValueTransferWithRatio(backend, navigator, test_name):
    message_hex = ("f841b83af8380a19850ba43b7400830493e0940ee56b604c869e3792c99e35c1c424f88f87dc8a01946e93a3acfbadf457f29fb0e57fa42274004c32ea1e8203e98080")
    perform_test_that_verifies_signature(backend, navigator, test_name, message_hex, INS.INS_SIGN_VALUE_TRANSFER, P1_FEE_DELEGATED_WITH_RATIO)

def test_klaytn_signFeeDelegatedValueTransferMemoWithRatio(backend, navigator, test_name):
    message_hex = ("f847b840f83e1219850ba43b7400830493e0940ee56b604c869e3792c99e35c1c424f88f87dc8a01946e93a3acfbadf457f29fb0e57fa42274004c32ea8568656c6c6f1e8203e98080")
    perform_test_that_verifies_signature(backend, navigator, test_name, message_hex, INS.INS_SIGN_VALUE_TRANSFER_MEMO, P1_FEE_DELEGATED_WITH_RATIO)

def test_klaytn_signFeeDelegatedSmartContractDeployWithRatio(backend, navigator, test_name):
    message_hex = ("f3adec2a19850ba43b7400830493e08001946e93a3acfbadf457f29fb0e57fa42274004c32ea8568656c6c6f801e808203e98080")
    perform_test_that_verifies_signature(backend, navigator, test_name, message_hex, INS.INS_SIGN_SMART_CONTRACT_DEPLOY, P1_FEE_DELEGATED_WITH_RATIO)

def test_klaytn_signFeeDelegatedSmartContractExecutionWithRatio(backend, navigator, test_name):
    message_hex = ("f847b840f83e3219850ba43b7400830493e0940ee56b604c869e3792c99e35c1c424f88f87dc8a01946e93a3acfbadf457f29fb0e57fa42274004c32ea8568656c6c6f1e8203e98080")
    perform_test_that_verifies_signature(backend, navigator, test_name, message_hex, INS.INS_SIGN_SMART_CONTRACT_EXECUTION, P1_FEE_DELEGATED_WITH_RATIO)

def test_klaytn_signFeeDelegatedCancelWithRatio(backend, navigator, test_name):
    message_hex = ("e9a3e23a19850ba43b7400830493e0946e93a3acfbadf457f29fb0e57fa42274004c32ea1e8203e98080")
    perform_test_that_verifies_signature(backend, navigator, test_name, message_hex, INS.INS_SIGN_CANCEL, P1_FEE_DELEGATED_WITH_RATIO) 