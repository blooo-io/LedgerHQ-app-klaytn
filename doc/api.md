# Klaytn application : Common Technical Specifications

## About

This application describes the APDU messages interface to communicate with the Klaytn application.

The application covers the following functionalities :

- Retrieve an address given an account number
- Sign Klaytn transaction (excluding signFeePayerTransaction)
- Sign off-chain message

The application interface can be accessed over HID (or BLE ?)

## General purpose APDUs

### GET APP CONFIGURATION

#### Description

_This command returns specific application configuration_

##### Command

| _CLA_ | _INS_ | _P1_ | _P2_ | _Lc_ |
| ----- | :---: | ---: | ---- | :--: |
| E0    |  01   |   00 | 00   |  00  |

##### Input data

_None_

##### Output data

| _Description_             | _Length_ |
| ------------------------- | :------: |
| Allow blind sign (00: false, 01: true)   |    01    |
| Application major version |    01    |
| Application minor version |    01    |
| Application patch version |    01    |
| [Status word](#status-words)  |    02    |

### GET PUBKEY

#### Description

_This command returns a Klaytn pubkey for the given BIP 32 path_

#### Command

| _CLA_ | _INS_ | _P1_ | _P2_ |   _Lc_   | _Input data_|
| ----- | :---: | ---: | ---- | :------: | :----------:|
| E0    |  02   |   00 | 00   | variable |   variable  |

##### Input data

| _Description_                                    | _Length_ |
| ------------------------------------------------ | :------: |
| Number of BIP 32 derivations to perform (5)      |    1     |
| First derivation index (big endian)              |    4     |
| ...                                              |    4     |
| Fifth derivation index (big endian)              |    4     |

##### Output data

| _Description_ | _Length_ |
| ------------- | :------: |
| Length of pubkey      |    1    |
| Pubkey        |   previous value (65)    |
| Length of address        |    1    |
| Address        |    previous value (40)    |
| [Status Word](#status-words)     |   2     |


## SIGN KLAYTN TRANSACTION

### Description

This is the command used to sign a Klaytn transaction after having the user validate the transaction-specific parametrs. You can find more info about each section using the links in the table below.

### Command Structure


#### Input structure
| _CLA_ | [_INS_](#ins) | [_P1_](#p1) | [_P2_](#p2) |   _Lc_                   |     [_Input data_](#input-data-2) |
| ----- | :--------------------: | ----------: | ----------- | :----------------------: | ------------------------------: |
| E0    |  variable              |    variable | variable    | len in byte (input data) | variable                        |

##### INS

`INS` will vary depending on the transaction you want to sign. Use the one that match the transaction you want.

| Transaction type        | INS command   |
| ----------------------- | ------------- |
| LegacyTransaction       | 0x04          | 
| ValueTransfer           | 0x08          | 
| ValueTransferMemo       | 0x10          | 
| SmartContractDeploy     | 0x28          |
| SmartContractExecution  | 0x30          |
| Cancel                  | 0x38          |

##### P1

`P1` is used to say which type of transaction the app is dealing with. There are three main types in Klaytn.

| P1_BASIC                     | P1_FEE_DELEGATED             | P1_FEE_DELEGATED_WITH_RATIO |
| ---------------------------- | ---------------------------- | --------------------------- |
| 0x00                         | 0x01                         | 0x02                        |

##### P2

`P2` is used to tell the app if the apdu sent is:
- the last one(P2_NONE), 
- the first one with more coming (P2_MORE) or
- at least the second one but not last (P2_EXTEND)

| P2_NONE                  | P2_EXTEND                | P2_MORE                  |
| ------------------------ | ------------------------ | ------------------------ |
| 0x00                     | 0x01                     | 0x02                     |


##### Input data


| _Description_                                       | _Length_ |
| --------------------------------------------------- | :------: |
| Number of BIP 32 derivations to perform (always 5 ) |    1     |
| First derivation index (big endian)                 |    4     |
| ...                                                 |    4     |
| Last derivation index (big endian)                  |    4     |
| Encoded transaction                                 | variable |

###### Encoded transaction

In the table bellow you will see what type of information need to be encoded and how. 
The encoding algorithm is [RLP](https://ethereum.org/en/developers/docs/data-structures-and-encoding/rlp). 
You can find more info about RLP encoding for signature [here](https://archive-docs.klaytn.foundation/content/klaytn/design/transactions/basic#rlp-encoding-for-signature). 

You can find information about how to encode Klaytn Transaction on [this page](https://archive-docs.klaytn.foundation/content/klaytn/design/transactions#sendertxhash). Click on the transaction that interests you and search for `RLP Encoding for Signature`

#### Output structure

| _Description_ | _Length_ |
| ------------- | :------: |
| Signature     |    64    |
| [Status word](#status-words)|   2   |


## Transport protocol

### General transport description

_Ledger APDUs requests and responses are encapsulated using a flexible protocol allowing to fragment large payloads over different underlying transport mechanisms._

The common transport header is defined as follows:

| _Description_                         | _Length_ |
| ------------------------------------- | :------: |
| Communication channel ID (big endian) |    2     |
| Command tag                           |    1     |
| Packet sequence index (big endian)    |    2     |
| Payload                               |   var    |

The Communication channel ID allows commands multiplexing over the same physical link. It is not used for the time being, and should be set to 0101 to avoid compatibility issues with implementations ignoring a leading 00 byte.

The Command tag describes the message content. Use TAG_APDU (0x05) for standard APDU payloads, or TAG_PING (0x02) for a simple link test.

The Packet sequence index describes the current sequence for fragmented payloads. The first fragment index is 0x00.

### APDU Command payload encoding

APDU Command payloads are encoded as follows :

| _Description_            | _Length_ |
| ------------------------ | :------: |
| APDU length (big endian) |    2     |
| APDU CLA                 |    1     |
| APDU INS                 |    1     |
| APDU P1                  |    1     |
| APDU P2                  |    1     |
| APDU data length         |    1     |
| Optional APDU data       |   var    |


#### Deprecation notice

The `ADPU data length` field was formerly serialized as a 16bit unsigned big endian integer. As of version 0.2.0, this has been changed to an 8bit unsigned integer to improve compatibility with client libraries. In doing so, the following instructions have been deprecated.

- 0xF1 - GET_APP_CONFIGURATION
- 0xF2 - GET_PUBKEY
- 0xF3 - SIGN_MESSAGE

### APDU Response payload encoding

APDU Response payloads are encoded as follows :

| _Description_                      | _Length_ |
| ---------------------------------- | :------: |
| APDU response length (big endian)  |    var     |
| APDU response data and Status Word |   2    |

### USB mapping

Messages are exchanged with the dongle over HID endpoints over interrupt transfers, with each chunk being 64 bytes long. The HID Report ID is ignored.

### BLE mapping

A similar encoding is used over BLE, without the Communication channel ID.

The application acts as a GATT server defining service UUID D973F2E0-B19E-11E2-9E96-0800200C9A66

When using this service, the client sends requests to the characteristic D973F2E2-B19E-11E2-9E96-0800200C9A66, and gets notified on the characteristic D973F2E1-B19E-11E2-9E96-0800200C9A66 after registering for it.

Requests are encoded using the standard BLE 20 bytes MTU size

## Status Words

The following standard Status Words are returned for all APDUs - some specific Status Words can be used for specific commands and are mentioned in the command description.

| _SW_ |                   _Description_                   |
| ---- | :-----------------------------------------------: |
| 9000 |                       Success                       |
| 6F01 |         Klaytn SummaryUpdateFailed         |
| 6F00 |         Klaytn SummaryFinalizeFailed         |
| 6A83 |         Klaytn InvalidMessageSize         |
| 6A82 |         Klaytn InvalidMessageFormat         |
| 6A81 |       Klaytn InvalidMessageHeader       |
| 6A80 |         Klaytn InvalidMessage         |
| 6982 |         NoApduReceived         |
| 6E00 |                     InvalidCla                     |
| 6D00 |             UnimplementedInstruction              |
| 6819 |         NotEnoughSpace         |
| 6818 |         ExceptionSystem         |
| 6817 |         ExceptionCxPort         |
| 6816 |         ExceptionIoReset         |
| 6815 |         ExceptionIoState         |
| 6814 |         ExceptionIoHeader         |
| 6813 |         ExceptionIoOverflow         |
| 6812 |         ExceptionAppExit         |
| 6811 |         ExceptionPIC         |
| 6810 |         Timeout         |
| 6809 |         InvalidState         |
| 6808 |         NotSupported         |
| 6807 |         InvalidCounter         |
| 6806 |         InvalidChecksum         |
| 6805 |         InvalidCrc         |
| 6804 |         ExceptionSecurity         |
| 6803 |         ExceptionOverflow         |
| 6802 |         InvalidParameter         |
| 6801 |         SdkException         |
