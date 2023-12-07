from ragger.backend import RaisePolicy
from ragger.navigator import NavInsID
from ragger.utils import RAPDU

from .apps.solana import SolanaClient, ErrorType, INS, P1_FEE_DELEGATED, P1_FEE_DELEGATED_WITH_RATIO
from .apps.solana_cmd_builder import SystemInstructionTransfer, Message
from .apps.klaytn_cmd_builder import verify_transaction_signature_from_public_key
from .apps.solana_utils import FOREIGN_PUBLIC_KEY, FOREIGN_PUBLIC_KEY_2, AMOUNT, AMOUNT_2, SOL_PACKED_DERIVATION_PATH, SOL_PACKED_DERIVATION_PATH_2, KLAYTN_DERIVATION_PATH

from .utils import ROOT_SCREENSHOT_PATH
from binascii import hexlify


# def test_klaytn_get_public_key(backend, navigator, test_name):
#     sol = SolanaClient(backend)
#     from_public_key, address = sol.get_public_key(KLAYTN_DERIVATION_PATH)

#     print("Address 0x", address.decode(), sep='')

#     assert address == "6E93a3ACfbaDF457F29fb0E57FA42274004c32EA".encode(
#     ), "Public key is not the expected one"


# def test_klaytn_signLegacyTransaction(backend, navigator, test_name):
#     sol = SolanaClient(backend) # might not be needed
#     from_public_key, address = sol.get_public_key(KLAYTN_DERIVATION_PATH)
#     print("from_public_key", list(from_public_key))
#     data: bytes = bytearray.fromhex(
#         "058000002c80002019800000008000000080000000e719850ba43b7400830493e0940ee56b604c869e3792c99e35c1c424f88f87dc8a01808220198080")
#     with sol.send_async_sign_transaction(data, INS.INS_SIGN_LEGACY_TRANSACTION):
#         navigator.navigate_until_text_and_compare(NavInsID.RIGHT_CLICK,
#                                                   [NavInsID.BOTH_CLICK],
#                                                   "Approve",
#                                                   ROOT_SCREENSHOT_PATH,
#                                                   test_name)

#     signature: bytes = sol.get_async_response().data
#     # print("signature: ", signature)
#     # verify_signature(from_public_key, data, signature)


def test_klaytn_signValueTransfer(backend, navigator, test_name):
    sol = SolanaClient(backend)
    from_public_key, address = sol.get_public_key(KLAYTN_DERIVATION_PATH)
    print("from_public_key", list(from_public_key))
    data: bytes = bytearray.fromhex(
        "058000002c80002019800000000000000000000000f83fb838f70819850ba43b7400830493e0940ee56b604c869e3792c99e35c1c424f88f87dc8a01946e93a3acfbadf457f29fb0e57fa42274004c32ea8220198080")
    with sol.send_async_sign_transaction(data, INS.INS_SIGN_VALUE_TRANSFER):
        navigator.navigate_until_text_and_compare(NavInsID.RIGHT_CLICK,
                                                  [NavInsID.BOTH_CLICK],
                                                  "Approve",
                                                  ROOT_SCREENSHOT_PATH,
                                                  test_name)
    
    signature: bytes = sol.get_async_response().data
    address_decoded = address.decode()
    signature_decoded = hexlify(signature).decode()
    from_public_key_decoded = hexlify(from_public_key).decode()
    data_decoded = hexlify(data).decode()
    print('-------------------------------------------------------')
    print('address_decoded: ', address_decoded)
    print('signature_decoded: ', signature_decoded)
    print('from_public_key_decoded: ', from_public_key_decoded)
    print('data_decoded: ', data_decoded)
    print('-------------------------------------------------------')

    result = verify_transaction_signature_from_public_key(data_decoded, signature, from_public_key)
    print("-----------------RESULT-----------------")
    print(result)


# def test_klaytn_signValueTransferMemo(backend, navigator, test_name):
#     sol = SolanaClient(backend)
#     from_public_key, address = sol.get_public_key(KLAYTN_DERIVATION_PATH)
#     print("from_public_key", list(from_public_key))
#     data: bytes = bytearray.fromhex(
#         "058000002c80002019800000008000000080000000f846b83ff83d1019850ba43b7400830493e0940ee56b604c869e3792c99e35c1c424f88f87dc8a01946694d467b419b36fb719e851cd65d54205df75558568656c6c6f8220198080")
#     with sol.send_async_sign_transaction(data, INS.INS_SIGN_VALUE_TRANSFER_MEMO):
#         navigator.navigate_until_text_and_compare(NavInsID.RIGHT_CLICK,
#                                                   [NavInsID.BOTH_CLICK],
#                                                   "Approve",
#                                                   ROOT_SCREENSHOT_PATH,
#                                                   test_name)

#     signature: bytes = sol.get_async_response().data
#     # print("signature: ", signature)
#     # verify_signature(from_public_key, data, signature)


# def test_klaytn_signSmartContractDeploy(backend, navigator, test_name):
#     sol = SolanaClient(backend)
#     from_public_key, address = sol.get_public_key(KLAYTN_DERIVATION_PATH)
#     print("from_public_key", list(from_public_key))
#     data: bytes = bytearray.fromhex(
#         "058000002c80002019800000008000000080000000f2aceb2819850ba43b7400830493e08001946694d467b419b36fb719e851cd65d54205df75558568656c6c6f80808220198080")
#     with sol.send_async_sign_transaction(data, INS.INS_SIGN_SMART_CONTRACT_DEPLOY):
#         navigator.navigate_until_text_and_compare(NavInsID.RIGHT_CLICK,
#                                                   [NavInsID.BOTH_CLICK],
#                                                   "Approve",
#                                                   ROOT_SCREENSHOT_PATH,
#                                                   test_name)

#     signature: bytes = sol.get_async_response().data
#     # print("signature: ", signature)
#     # verify_signature(from_public_key, data, signature)


# def test_klaytn_signSmartContractExecution(backend, navigator, test_name):
#     sol = SolanaClient(backend)
#     from_public_key, address = sol.get_public_key(KLAYTN_DERIVATION_PATH)
#     print("from_public_key", list(from_public_key))
#     data: bytes = bytearray.fromhex(
#         "058000002c80002019800000008000000080000000f846b83ff83d3019850ba43b7400830493e0940ee56b604c869e3792c99e35c1c424f88f87dc8a01946694d467b419b36fb719e851cd65d54205df75558568656c6c6f8220198080")
#     with sol.send_async_sign_transaction(data, INS.INS_SIGN_SMART_CONTRACT_EXECUTION):
#         navigator.navigate_until_text_and_compare(NavInsID.RIGHT_CLICK,
#                                                   [NavInsID.BOTH_CLICK],
#                                                   "Approve",
#                                                   ROOT_SCREENSHOT_PATH,
#                                                   test_name)

#     signature: bytes = sol.get_async_response().data
#     # print("signature: ", signature)
#     # verify_signature(from_public_key, data, signature)


# def test_klaytn_signCancel(backend, navigator, test_name):
#     sol = SolanaClient(backend)
#     from_public_key, address = sol.get_public_key(KLAYTN_DERIVATION_PATH)
#     print("from_public_key", list(from_public_key))
#     data: bytes = bytearray.fromhex(
#         "058000002c80002019800000008000000080000000e8a2e13819850ba43b7400830493e0946694d467b419b36fb719e851cd65d54205df75558220198080")
#     with sol.send_async_sign_transaction(data, INS.INS_SIGN_CANCEL):
#         navigator.navigate_until_text_and_compare(NavInsID.RIGHT_CLICK,
#                                                   [NavInsID.BOTH_CLICK],
#                                                   "Approve",
#                                                   ROOT_SCREENSHOT_PATH,
#                                                   test_name)

#     signature: bytes = sol.get_async_response().data
#     # print("signature: ", signature)
#     # verify_signature(from_public_key, data, signature)


# def test_klaytn_signFeeDelegatedValueTransfer(backend, navigator, test_name):
#     sol = SolanaClient(backend)
#     from_public_key, address = sol.get_public_key(KLAYTN_DERIVATION_PATH)
#     print("from_public_key", list(from_public_key))
#     data: bytes = bytearray.fromhex(
#         "058000002c80002019800000008000000080000000f83fb838f70919850ba43b7400830493e0940ee56b604c869e3792c99e35c1c424f88f87dc8a01946694d467b419b36fb719e851cd65d54205df75558220198080")
#     with sol.send_async_sign_transaction(data, INS.INS_SIGN_VALUE_TRANSFER, P1_FEE_DELEGATED):
#         navigator.navigate_until_text_and_compare(NavInsID.RIGHT_CLICK,
#                                                   [NavInsID.BOTH_CLICK],
#                                                   "Approve",
#                                                   ROOT_SCREENSHOT_PATH,
#                                                   test_name)

#     signature: bytes = sol.get_async_response().data
#     # print("signature: ", signature)
#     # verify_signature(from_public_key, data, signature)


# def test_klaytn_signFeeDelegatedValueTransferMemo(backend, navigator, test_name):
#     sol = SolanaClient(backend)
#     from_public_key, address = sol.get_public_key(KLAYTN_DERIVATION_PATH)
#     print("from_public_key", list(from_public_key))
#     data: bytes = bytearray.fromhex(
#         "058000002c80002019800000008000000080000000f846b83ff83d1119850ba43b7400830493e0940ee56b604c869e3792c99e35c1c424f88f87dc8a01946694d467b419b36fb719e851cd65d54205df75558568656c6c6f8220198080")
#     with sol.send_async_sign_transaction(data, INS.INS_SIGN_VALUE_TRANSFER_MEMO, P1_FEE_DELEGATED):
#         navigator.navigate_until_text_and_compare(NavInsID.RIGHT_CLICK,
#                                                   [NavInsID.BOTH_CLICK],
#                                                   "Approve",
#                                                   ROOT_SCREENSHOT_PATH,
#                                                   test_name)

#     signature: bytes = sol.get_async_response().data
#     # print("signature: ", signature)
#     # verify_signature(from_public_key, data, signature)


# def test_klaytn_signFeeDelegatedSmartContractDeploy(backend, navigator, test_name):
#     sol = SolanaClient(backend)
#     from_public_key, address = sol.get_public_key(KLAYTN_DERIVATION_PATH)
#     print("from_public_key", list(from_public_key))
#     data: bytes = bytearray.fromhex(
#         "058000002c80002019800000008000000080000000f846b83ff83d1119850ba43b7400830493e0940ee56b604c869e3792c99e35c1c424f88f87dc8a01946694d467b419b36fb719e851cd65d54205df75558568656c6c6f8220198080")
#     with sol.send_async_sign_transaction(data, INS.INS_SIGN_SMART_CONTRACT_DEPLOY, P1_FEE_DELEGATED):
#         navigator.navigate_until_text_and_compare(NavInsID.RIGHT_CLICK,
#                                                   [NavInsID.BOTH_CLICK],
#                                                   "Approve",
#                                                   ROOT_SCREENSHOT_PATH,
#                                                   test_name)

#     signature: bytes = sol.get_async_response().data
#     # print("signature: ", signature)
#     # verify_signature(from_public_key, data, signature)


# def test_klaytn_signFeeDelegatedSmartContractExecution(backend, navigator, test_name):
#     sol = SolanaClient(backend)
#     from_public_key, address = sol.get_public_key(KLAYTN_DERIVATION_PATH)
#     print("from_public_key", list(from_public_key))
#     data: bytes = bytearray.fromhex(
#         "058000002c80002019800000008000000080000000f846b83ff83d3119850ba43b7400830493e0940ee56b604c869e3792c99e35c1c424f88f87dc8a01946694d467b419b36fb719e851cd65d54205df75558568656c6c6f8220198080")
#     with sol.send_async_sign_transaction(data, INS.INS_SIGN_SMART_CONTRACT_EXECUTION, P1_FEE_DELEGATED):
#         navigator.navigate_until_text_and_compare(NavInsID.RIGHT_CLICK,
#                                                   [NavInsID.BOTH_CLICK],
#                                                   "Approve",
#                                                   ROOT_SCREENSHOT_PATH,
#                                                   test_name)

#     signature: bytes = sol.get_async_response().data
#     # print("signature: ", signature)
#     # verify_signature(from_public_key, data, signature)


# def test_klaytn_signFeeDelegatedCancel(backend, navigator, test_name):
#     sol = SolanaClient(backend)
#     from_public_key, address = sol.get_public_key(KLAYTN_DERIVATION_PATH)
#     print("from_public_key", list(from_public_key))
#     data: bytes = bytearray.fromhex(
#         "058000002c80002019800000008000000080000000e8a2e13919850ba43b7400830493e0946694d467b419b36fb719e851cd65d54205df75558220198080")
#     with sol.send_async_sign_transaction(data, INS.INS_SIGN_CANCEL, P1_FEE_DELEGATED):
#         navigator.navigate_until_text_and_compare(NavInsID.RIGHT_CLICK,
#                                                   [NavInsID.BOTH_CLICK],
#                                                   "Approve",
#                                                   ROOT_SCREENSHOT_PATH,
#                                                   test_name)

#     signature: bytes = sol.get_async_response().data
#     # print("signature: ", signature)
#     # verify_signature(from_public_key, data, signature)


# def test_klaytn_signFeeDelegatedValueTransferWithRatio(backend, navigator, test_name):
#     sol = SolanaClient(backend)
#     from_public_key, address = sol.get_public_key(KLAYTN_DERIVATION_PATH)
#     print("from_public_key", list(from_public_key))
#     data: bytes = bytearray.fromhex(
#         "058000002c80002019800000008000000080000000f841b83af8380a19850ba43b7400830493e0940ee56b604c869e3792c99e35c1c424f88f87dc8a01946694d467b419b36fb719e851cd65d54205df75551e8220198080")
#     with sol.send_async_sign_transaction(data, INS.INS_SIGN_VALUE_TRANSFER, P1_FEE_DELEGATED_WITH_RATIO):
#         navigator.navigate_until_text_and_compare(NavInsID.RIGHT_CLICK,
#                                                   [NavInsID.BOTH_CLICK],
#                                                   "Approve",
#                                                   ROOT_SCREENSHOT_PATH,
#                                                   test_name)

#     signature: bytes = sol.get_async_response().data
#     # print("signature: ", signature)
#     # verify_signature(from_public_key, data, signature)


# def test_klaytn_signFeeDelegatedValueTransferMemoWithRatio(backend, navigator, test_name):
#     sol = SolanaClient(backend)
#     from_public_key, address = sol.get_public_key(KLAYTN_DERIVATION_PATH)
#     print("from_public_key", list(from_public_key))
#     data: bytes = bytearray.fromhex(
#         "058000002c80002019800000008000000080000000f847b840f83e1219850ba43b7400830493e0940ee56b604c869e3792c99e35c1c424f88f87dc8a01946694d467b419b36fb719e851cd65d54205df75558568656c6c6f1e8220198080")
#     with sol.send_async_sign_transaction(data, INS.INS_SIGN_VALUE_TRANSFER_MEMO, P1_FEE_DELEGATED_WITH_RATIO):
#         navigator.navigate_until_text_and_compare(NavInsID.RIGHT_CLICK,
#                                                   [NavInsID.BOTH_CLICK],
#                                                   "Approve",
#                                                   ROOT_SCREENSHOT_PATH,
#                                                   test_name)

#     signature: bytes = sol.get_async_response().data
#     # print("signature: ", signature)
#     # verify_signature(from_public_key, data, signature)


# def test_klaytn_signFeeDelegatedSmartContractDeployWithRatio(backend, navigator, test_name):
#     sol = SolanaClient(backend)
#     from_public_key, address = sol.get_public_key(KLAYTN_DERIVATION_PATH)
#     print("from_public_key", list(from_public_key))
#     data: bytes = bytearray.fromhex(
#         "058000002c80002019800000008000000080000000f3adec2a19850ba43b7400830493e08001946694d467b419b36fb719e851cd65d54205df75558568656c6c6f801e808220198080")
#     with sol.send_async_sign_transaction(data, INS.INS_SIGN_SMART_CONTRACT_DEPLOY, P1_FEE_DELEGATED_WITH_RATIO):
#         navigator.navigate_until_text_and_compare(NavInsID.RIGHT_CLICK,
#                                                   [NavInsID.BOTH_CLICK],
#                                                   "Approve",
#                                                   ROOT_SCREENSHOT_PATH,
#                                                   test_name)

#     signature: bytes = sol.get_async_response().data
#     # print("signature: ", signature)
#     # verify_signature(from_public_key, data, signature)


# def test_klaytn_signFeeDelegatedSmartContractExecutionWithRatio(backend, navigator, test_name):
#     sol = SolanaClient(backend)
#     from_public_key, address = sol.get_public_key(KLAYTN_DERIVATION_PATH)
#     print("from_public_key", list(from_public_key))
#     data: bytes = bytearray.fromhex(
#         "058000002c80002019800000008000000080000000f847b840f83e3219850ba43b7400830493e0940ee56b604c869e3792c99e35c1c424f88f87dc8a01946694d467b419b36fb719e851cd65d54205df75558568656c6c6f1e8220198080")
#     with sol.send_async_sign_transaction(data, INS.INS_SIGN_SMART_CONTRACT_EXECUTION, P1_FEE_DELEGATED_WITH_RATIO):
#         navigator.navigate_until_text_and_compare(NavInsID.RIGHT_CLICK,
#                                                   [NavInsID.BOTH_CLICK],
#                                                   "Approve",
#                                                   ROOT_SCREENSHOT_PATH,
#                                                   test_name)

#     signature: bytes = sol.get_async_response().data
#     # print("signature: ", signature)
#     # verify_signature(from_public_key, data, signature)


# def test_klaytn_signFeeDelegatedCancelWithRatio(backend, navigator, test_name):
#     sol = SolanaClient(backend)
#     from_public_key, address = sol.get_public_key(KLAYTN_DERIVATION_PATH)
#     print("from_public_key", list(from_public_key))
#     data: bytes = bytearray.fromhex(
#         "058000002c80002019800000008000000080000000e9a3e23a19850ba43b7400830493e0946694d467b419b36fb719e851cd65d54205df75551e8220198080")
#     with sol.send_async_sign_transaction(data, INS.INS_SIGN_CANCEL, P1_FEE_DELEGATED_WITH_RATIO):
#         navigator.navigate_until_text_and_compare(NavInsID.RIGHT_CLICK,
#                                                   [NavInsID.BOTH_CLICK],
#                                                   "Approve",
#                                                   ROOT_SCREENSHOT_PATH,
#                                                   test_name)

#     signature: bytes = sol.get_async_response().data
#     # print("signature: ", signature)
#     # verify_signature(from_public_key, data, signature)
