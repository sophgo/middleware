/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: include/cvi_unf_cipher.h
 * Description:
 */

#ifndef CVI_UNF_CIPHER_H_FYUVBDPZ
#define CVI_UNF_CIPHER_H_FYUVBDPZ

#include <cvi_type.h>
#include <cvi_sys.h>

/*Error code*/
#define CVI_ERR_CIPHER_NOT_INIT 0x804D0001 // Cipher uninitialized
#define CVI_ERR_CIPHER_INVALID_HANDLE 0x804D0002 // Handle invalid
#define CVI_ERR_CIPHER_INVALID_POINT 0x804D0003 // Parameter has NULL point
#define CVI_ERR_CIPHER_INVALID_PARA 0x804D0004 // Parameter is invalid
#define CVI_ERR_CIPHER_FAILED_INIT 0x804D0005 // Init failed
#define CVI_ERR_CIPHER_FAILED_GETHANDLE 0x804D0006 // Get handle failed
#define CVI_ERR_CIPHER_FAILED_RELEASEHANDLE 0x804D0007 // Free handle failed
#define CVI_ERR_CIPHER_FAILED_CONFIGAES                                        \
	0x804D0008 // AES configuration is invalid
#define CVI_ERR_CIPHER_FAILED_CONFIGDES                                        \
	0x804D0009 // DES configuration is invalid
#define CVI_ERR_CIPHER_FAILED_ENCRYPT 0x804D000A // Encrypt failed
#define CVI_ERR_CIPHER_FAILED_DECRYPT 0x804D000B // Decrypt failed
#define CVI_ERR_CIPHER_BUSY 0x804D000C // Cipher is busy
#define CVI_ERR_CIPHER_NO_AVAILABLE_RNG 0x804D000D // No available RNG
#define CVI_ERR_CIPHER_FAILED_MEM 0x804D000E // Memory allocation failed
#define CVI_ERR_CIPHER_UNAVAILABLE 0x804D000F // Service unavailable
#define CVI_ERR_CIPHER_OVERFLOW 0x804D0010 // Data overflow
#define CVI_ERR_CIPHER_HARD_STATUS 0x804D0011 // Hardware error
#define CVI_ERR_CIPHER_TIMEOUT 0x804D0012 // Timeout
#define CVI_ERR_CIPHER_UNSUPPORTED 0x804D0013 // Unsupported
#define CVI_ERR_CIPHER_REGISTER_IRQ 0x804D0014 // IRQ registration failed
#define CVI_ERR_CIPHER_ILLEGAL_UUID 0x804D0015 // Illegal UUID
#define CVI_ERR_CIPHER_ILLEGAL_KEY 0x804D0016 // Illegal key
#define CVI_ERR_CIPHER_INVALID_ADDR 0x804D0017 // Invalid address
#define CVI_ERR_CIPHER_INVALID_LENGTH 0x804D0018 // Invalid length
#define CVI_ERR_CIPHER_ILLEGAL_DATA 0x804D0019 // Illegal data
#define CVI_ERR_CIPHER_RSA_SIGN 0x804D001A // RSA signature failed
#define CVI_ERR_CIPHER_RSA_VERIFY 0x804D001B // RSA verification failed
#define CVI_ERR_CIPHER_RSA_CRYPT_FAILED 0x804D001E // RSA cryptography failed

#define CVI_CIPHER_HANDLE uintptr_t // indicates the Handle of CIPHER operate.

#define CIPHER_IV_CHANGE_ONE_PKG                                               \
	(1) // CIPHER updates the IV of only one packet when setting a vector for a packet.
#define CIPHER_IV_CHANGE_ALL_PKG                                               \
	(2) // CIPHER updates the IV of all packets when the vector is set for packets.

typedef enum {
	CVI_UNF_CIPHER_WORK_MODE_ECB, // Electronic Codebook mode
	CVI_UNF_CIPHER_WORK_MODE_CBC, // Cipher Block Chaining mode
	CVI_UNF_CIPHER_WORK_MODE_CFB, // Cipher Feedback mode
	CVI_UNF_CIPHER_WORK_MODE_OFB, // Output Feedback mode
	CVI_UNF_CIPHER_WORK_MODE_CTR, // Counter mode
	CVI_UNF_CIPHER_WORK_MODE_CCM, // Counter with CBC-MAC mode
	CVI_UNF_CIPHER_WORK_MODE_GCM, // Galois/Counter mode
	CVI_UNF_CIPHER_WORK_MODE_CBC_CTS, // CBC Ciphertext Stealing mode
	CVI_UNF_CIPHER_WORK_MODE_BUTT, // End of work modes
	CVI_UNF_CIPHER_WORK_MODE_INVALID = -1, // Invalid work mode
} CVI_UNF_CIPHER_WORK_MODE_E; // CIPHER working mode

typedef enum {
	CVI_UNF_CIPHER_ALG_DES = 0x0, // Data Encryption Standard
	CVI_UNF_CIPHER_ALG_3DES = 0x1, // Triple Data Encryption Standard
	CVI_UNF_CIPHER_ALG_AES = 0x2, // Advanced Encryption Standard
	CVI_UNF_CIPHER_ALG_SM1 = 0x3, // SM1 algorithm
	CVI_UNF_CIPHER_ALG_SM4 = 0x4, // SM4 algorithm
	CVI_UNF_CIPHER_ALG_DMA = 0x5, // Direct Memory Access
	CVI_UNF_CIPHER_ALG_BUTT = 0x6, // End of algorithms
	CVI_UNF_CIPHER_ALG_INVALID = -1, // Invalid algorithm
} CVI_UNF_CIPHER_ALG_E; // CIPHER encryption algorithm

typedef enum {
	CVI_UNF_CIPHER_KEY_AES_128BIT = 0x0, // The AES key length is 128bit
	CVI_UNF_CIPHER_KEY_AES_192BIT = 0x1, // The AES key length is 192bit
	CVI_UNF_CIPHER_KEY_AES_256BIT = 0x2, // The AES key length is 256bit
	CVI_UNF_CIPHER_KEY_DES_3KEY = 0x2, // The DES key length is 3 keys
	CVI_UNF_CIPHER_KEY_DES_2KEY = 0x3, // The DES key length is 2 keys
	CVI_UNF_CIPHER_KEY_DEFAULT = 0x0, // Default key length
	CVI_UNF_CIPHER_KEY_INVALID = -1, // Invalid key length
} CVI_UNF_CIPHER_KEY_LENGTH_E; // Length of the CIPHER key

typedef enum {
	CVI_UNF_CIPHER_BIT_WIDTH_64BIT = 0x0, // 64-bit width
	CVI_UNF_CIPHER_BIT_WIDTH_8BIT = 0x1, // 8-bit width
	CVI_UNF_CIPHER_BIT_WIDTH_1BIT = 0x2, // 1-bit width
	CVI_UNF_CIPHER_BIT_WIDTH_128BIT = 0x3, // 128-bit width
	CVI_UNF_CIPHER_BIT_WIDTH_INVALID = -1, // Invalid bit width
} CVI_UNF_CIPHER_BIT_WIDTH_E; // CIPHER bit width

typedef struct {
	CVI_U32 bit1IV : 2; // IV bit
	CVI_U32 bitsResv : 30; // Reserved bits
} CVI_UNF_CIPHER_CTRL_CHANGE_FLAG_S; // Message structure of the CIPHER CCM mode

typedef enum {
	CVI_UNF_CIPHER_TYPE_NORMAL = 0x0, // Normal type
	CVI_UNF_CIPHER_TYPE_COPY_AVOID, // Copy avoid type
	CVI_UNF_CIPHER_TYPE_BUTT, // End of types
	CVI_UNF_CIPHER_TYPE_INVALID = -1, // Invalid type
} CVI_UNF_CIPHER_TYPE_E; // CIPHER encryption and decryption type

typedef struct {
	CVI_UNF_CIPHER_TYPE_E enCipherType; // CIPHER cipher type structure
} CVI_UNF_CIPHER_ATTS_S;

typedef enum {
	CVI_UNF_CIPHER_SM1_ROUND_08 = 0x00, // 8 rounds
	CVI_UNF_CIPHER_SM1_ROUND_10 = 0x01, // 10 rounds
	CVI_UNF_CIPHER_SM1_ROUND_12 = 0x02, // 12 rounds
	CVI_UNF_CIPHER_SM1_ROUND_14 = 0x03, // 14 rounds
	CVI_UNF_CIPHER_SM1_ROUND_BUTT, // End of rounds
	CVI_UNF_CIPHER_SM1_ROUND_INVALID = -1, // Invalid round
} CVI_UNF_CIPHER_SM1_ROUND_E; // Struct of SM1 algorithm rounds

typedef struct {
	CVI_U32 u32Key[8]; // Key
	CVI_U32 u32IV[4]; // Initialization Vector
	CVI_UNF_CIPHER_ALG_E enAlg; // Algorithm
	CVI_UNF_CIPHER_BIT_WIDTH_E enBitWidth; // Bit width
	CVI_UNF_CIPHER_WORK_MODE_E enWorkMode; // Work mode
	CVI_UNF_CIPHER_KEY_LENGTH_E enKeyLen; // Key length
	CVI_UNF_CIPHER_CTRL_CHANGE_FLAG_S stChangeFlags; // Change flags
} CVI_UNF_CIPHER_CTRL_S; // defines the CIPHER control information structure.

typedef struct {
	CVI_U32 u32EvenKey[8]; // Even key
	CVI_U32 u32OddKey[8]; // Odd key
	CVI_U32 u32IV[4]; // Initialization Vector
	CVI_UNF_CIPHER_BIT_WIDTH_E enBitWidth; // Bit width
	CVI_UNF_CIPHER_KEY_LENGTH_E enKeyLen; // Key length
	CVI_UNF_CIPHER_CTRL_CHANGE_FLAG_S stChangeFlags; // Change flags
} CVI_UNF_CIPHER_CTRL_AES_S; // indicates the extension of AES encryption control information structure

typedef struct {
	CVI_U32 u32Key[8]; // Key
	CVI_U32 u32IV[4]; // Initialization Vector
	CVI_UNF_CIPHER_KEY_LENGTH_E enKeyLen; // Key length
	CVI_U32 u32IVLen; // IV length
	CVI_U32 u32TagLen; // Tag length
	CVI_U32 u32ALen; // A length
	CVI_SIZE_T szAdataAddr; // A data address
} CVI_UNF_CIPHER_CTRL_AES_CCM_GCM_S; // indicates the AES-CCM and AES-GCM encryption control information structure

typedef struct {
	CVI_U32 u32Key[2]; // Key
	CVI_U32 u32IV[2]; // Initialization Vector
	CVI_UNF_CIPHER_BIT_WIDTH_E enBitWidth; // Bit width
	CVI_UNF_CIPHER_CTRL_CHANGE_FLAG_S stChangeFlags; // Change flags
} CVI_UNF_CIPHER_CTRL_DES_S; // indicates the extension of the DES encryption control information structure

typedef struct {
	CVI_U32 u32Key[6]; // Key
	CVI_U32 u32IV[2]; // Initialization Vector
	CVI_UNF_CIPHER_BIT_WIDTH_E enBitWidth; // Bit width
	CVI_UNF_CIPHER_KEY_LENGTH_E enKeyLen; // Key length
	CVI_UNF_CIPHER_CTRL_CHANGE_FLAG_S stChangeFlags; // Change flags
} CVI_UNF_CIPHER_CTRL_3DES_S; // 3DES Encryption control information structure

typedef struct {
	CVI_U32 u32EK[4]; // EK
	CVI_U32 u32AK[4]; // AK
	CVI_U32 u32SK[4]; // SK
	CVI_U32 u32IV[4]; // Initialization Vector
	CVI_UNF_CIPHER_BIT_WIDTH_E enBitWidth; // Bit width
	CVI_UNF_CIPHER_SM1_ROUND_E enSm1Round; // SM1 round
	CVI_UNF_CIPHER_CTRL_CHANGE_FLAG_S stChangeFlags; // Change flags
} CVI_UNF_CIPHER_CTRL_SM1_S; // defines the Struct of SM1 algorithm controler

typedef struct {
	CVI_U32 u32Key[4]; // Key
	CVI_U32 u32IV[4]; // Initialization Vector
	CVI_UNF_CIPHER_CTRL_CHANGE_FLAG_S stChangeFlags; // Change flags
} CVI_UNF_CIPHER_CTRL_SM4_S; // defines the Struct of SM4 algorithm controler

typedef struct {
	CVI_UNF_CIPHER_ALG_E enAlg; // Algorithm
	CVI_UNF_CIPHER_WORK_MODE_E enWorkMode; // Work mode
	CVI_VOID *pParam; // Parameter
} CVI_UNF_CIPHER_CTRL_EX_S; // specifies the extended structure of the encryption control information as a special parameter for the algorithm

typedef struct {
	CVI_SIZE_T szSrcAddr; // Source address
	CVI_SIZE_T szDestAddr; // Destination address
	CVI_U32 u32ByteLength; // Byte length
	CVI_BOOL bOddKey; // Odd key
} CVI_UNF_CIPHER_DATA_S; // specifies the CIPHER encryption and decryption data

typedef enum {
	CVI_UNF_CIPHER_HASH_TYPE_SHA1, // SHA-1 hash type
	CVI_UNF_CIPHER_HASH_TYPE_SHA224, // SHA-224 hash type
	CVI_UNF_CIPHER_HASH_TYPE_SHA256, // SHA-256 hash type
	CVI_UNF_CIPHER_HASH_TYPE_SHA384, // SHA-384 hash type
	CVI_UNF_CIPHER_HASH_TYPE_SHA512, // SHA-512 hash type
	CVI_UNF_CIPHER_HASH_TYPE_HMAC_SHA1, // HMAC with SHA-1 hash type
	CVI_UNF_CIPHER_HASH_TYPE_HMAC_SHA224, // HMAC with SHA-224 hash type
	CVI_UNF_CIPHER_HASH_TYPE_HMAC_SHA256, // HMAC with SHA-256 hash type
	CVI_UNF_CIPHER_HASH_TYPE_HMAC_SHA384, // HMAC with SHA-384 hash type
	CVI_UNF_CIPHER_HASH_TYPE_HMAC_SHA512, // HMAC with SHA-512 hash type
	CVI_UNF_CIPHER_HASH_TYPE_SM3, // SM3 hash type
	CVI_UNF_CIPHER_HASH_TYPE_BUTT, // End of hash types
	CVI_UNF_CIPHER_HASH_TYPE_INVALID = -1, // Invalid hash type
} CVI_UNF_CIPHER_HASH_TYPE_E; // defines the CIPHER hash algorithm type

typedef struct {
	CVI_U8 *pu8HMACKey; // HMAC key
	CVI_U32 u32HMACKeyLen; // HMAC key length
	CVI_UNF_CIPHER_HASH_TYPE_E eShaType; // SHA type
} CVI_UNF_CIPHER_HASH_ATTS_S; // defines the input structure initialized by the CIPHER hash algorithm

typedef enum {
	CVI_UNF_CIPHER_RSA_ENC_SCHEME_NO_PADDING, // No padding
	CVI_UNF_CIPHER_RSA_ENC_SCHEME_BLOCK_TYPE_0, // Block type 0
	CVI_UNF_CIPHER_RSA_ENC_SCHEME_BLOCK_TYPE_1, // Block type 1
	CVI_UNF_CIPHER_RSA_ENC_SCHEME_BLOCK_TYPE_2, // Block type 2
	CVI_UNF_CIPHER_RSA_ENC_SCHEME_RSAES_OAEP_SHA1, // RSAES-OAEP with SHA-1
	CVI_UNF_CIPHER_RSA_ENC_SCHEME_RSAES_OAEP_SHA224, // RSAES-OAEP with SHA-224
	CVI_UNF_CIPHER_RSA_ENC_SCHEME_RSAES_OAEP_SHA256, // RSAES-OAEP with SHA-256
	CVI_UNF_CIPHER_RSA_ENC_SCHEME_RSAES_OAEP_SHA384, // RSAES-OAEP with SHA-384
	CVI_UNF_CIPHER_RSA_ENC_SCHEME_RSAES_OAEP_SHA512, // RSAES-OAEP with SHA-512
	CVI_UNF_CIPHER_RSA_ENC_SCHEME_RSAES_PKCS1_V1_5, // RSAES-PKCS1 v1.5
	CVI_UNF_CIPHER_RSA_ENC_SCHEME_BUTT, // End of encryption schemes
	CVI_UNF_CIPHER_RSA_ENC_SCHEME_INVALID = -1, // Invalid encryption scheme
} CVI_UNF_CIPHER_RSA_ENC_SCHEME_E; // Defines the data encryption and filling method of the RSA algorithm

typedef enum {
	CVI_UNF_CIPHER_RSA_SIGN_SCHEME_RSASSA_PKCS1_V15_SHA1 =
		0x100, // RSASSA-PKCS1 v1.5 with SHA-1
	CVI_UNF_CIPHER_RSA_SIGN_SCHEME_RSASSA_PKCS1_V15_SHA224, // RSASSA-PKCS1 v1.5 with SHA-224
	CVI_UNF_CIPHER_RSA_SIGN_SCHEME_RSASSA_PKCS1_V15_SHA256, // RSASSA-PKCS1 v1.5 with SHA-256
	CVI_UNF_CIPHER_RSA_SIGN_SCHEME_RSASSA_PKCS1_V15_SHA384, // RSASSA-PKCS1 v1.5 with SHA-384
	CVI_UNF_CIPHER_RSA_SIGN_SCHEME_RSASSA_PKCS1_V15_SHA512, // RSASSA-PKCS1 v1.5 with SHA-512
	CVI_UNF_CIPHER_RSA_SIGN_SCHEME_RSASSA_PKCS1_PSS_SHA1, // RSASSA-PSS with SHA-1
	CVI_UNF_CIPHER_RSA_SIGN_SCHEME_RSASSA_PKCS1_PSS_SHA224, // RSASSA-PSS with SHA-224
	CVI_UNF_CIPHER_RSA_SIGN_SCHEME_RSASSA_PKCS1_PSS_SHA256, // RSASSA-PSS with SHA-256
	CVI_UNF_CIPHER_RSA_SIGN_SCHEME_RSASSA_PKCS1_PSS_SHA384, // RSASSA-PSS with SHA-384
	CVI_UNF_CIPHER_RSA_SIGN_SCHEME_RSASSA_PKCS1_PSS_SHA512, // RSASSA-PSS with SHA-512
	CVI_UNF_CIPHER_RSA_SIGN_SCHEME_BUTT, // End of signature schemes
	CVI_UNF_CIPHER_RSA_SIGN_SCHEME_INVALID = -1, // Invalid signature scheme
} CVI_UNF_CIPHER_RSA_SIGN_SCHEME_E; // Defines an RSA data signature policy

typedef struct {
	CVI_U8 *pu8N; // Modulus
	CVI_U8 *pu8E; // Public exponent
	CVI_U16 u16NLen; // Modulus length
	CVI_U16 u16ELen; // Public exponent length
} CVI_UNF_CIPHER_RSA_PUB_KEY_S; // Defines the RSA public key structure

typedef struct {
	CVI_U8 *pu8N; // Modulus
	CVI_U8 *pu8E; // Public exponent
	CVI_U8 *pu8D; // Private exponent
	CVI_U8 *pu8P; // Prime P
	CVI_U8 *pu8Q; // Prime Q
	CVI_U8 *pu8DP; // D mod (P-1)
	CVI_U8 *pu8DQ; // D mod (Q-1)
	CVI_U8 *pu8QP; // Q^-1 mod P
	CVI_U16 u16NLen; // Modulus length
	CVI_U16 u16ELen; // Public exponent length
	CVI_U16 u16DLen; // Private exponent length
	CVI_U16 u16PLen; // Prime P length
	CVI_U16 u16QLen; // Prime Q length
	CVI_U16 u16DPLen; // D mod (P-1) length
	CVI_U16 u16DQLen; // D mod (Q-1) length
	CVI_U16 u16QPLen; // Q^-1 mod P length
} CVI_UNF_CIPHER_RSA_PRI_KEY_S; // defines the RSA private key structure

typedef struct {
	CVI_UNF_CIPHER_RSA_ENC_SCHEME_E enScheme; // Encryption scheme
	CVI_UNF_CIPHER_RSA_PUB_KEY_S stPubKey; // Public key
} CVI_UNF_CIPHER_RSA_PUB_ENC_S; // Defines the parameter set of the RSA public key encryption and decryption algorithm

typedef struct {
	CVI_UNF_CIPHER_RSA_ENC_SCHEME_E enScheme; // Encryption scheme
	CVI_UNF_CIPHER_RSA_PRI_KEY_S stPriKey; // Private key
} CVI_UNF_CIPHER_RSA_PRI_ENC_S; // Defines the parameter structure of the RSA private key decryption algorithm

typedef struct {
	CVI_UNF_CIPHER_RSA_SIGN_SCHEME_E enScheme; // Signature scheme
	CVI_UNF_CIPHER_RSA_PRI_KEY_S stPriKey; // Private key
} CVI_UNF_CIPHER_RSA_SIGN_S; // Defines the parameter input structure of the RSA signature algorithm

typedef struct {
	CVI_UNF_CIPHER_RSA_SIGN_SCHEME_E enScheme; // Signature scheme
	CVI_UNF_CIPHER_RSA_PUB_KEY_S stPubKey; // Public key
} CVI_UNF_CIPHER_RSA_VERIFY_S; // Defines the parameter input structure of the RSA signature verification algorithm
/** <!-- ==== Structure Definition End ==== */

#define CVI_UNF_CIPHER_Open CVI_UNF_CIPHER_Init
#define CVI_UNF_CIPHER_Close CVI_UNF_CIPHER_DeInit

/** <!-- [CIPHER] */

/** <!-- [CIPHER] */

/**
 * @brief Initialize the CIPHER module.
 *
 * @return 0 on success, non-zero error code on failure.
 */
CVI_S32 CVI_UNF_CIPHER_Init(void);

/**
 * @brief Deinitialize the CIPHER module.
 *
 * @return 0 on success, non-zero error code on failure.
 */
CVI_S32 CVI_UNF_CIPHER_DeInit(void);

/**
 * @brief Create a Cipher handle.
 *
 * @param phCipher [Out] CIPHER handle pointer.
 * @param pstCipherAttr [In] Pointer to the attribute.
 *
 * @return 0 on success, non-zero error code on failure.
 */
CVI_S32 CVI_UNF_CIPHER_CreateHandle(CVI_CIPHER_HANDLE *phCipher,
				    const CVI_UNF_CIPHER_ATTS_S *pstCipherAttr);

/**
 * @brief Destroy the existing CIPHER handle.
 *
 * @param hCipher [In] CIPHER handle.
 *
 * @return 0 on success, non-zero error code on failure.
 */
CVI_S32 CVI_UNF_CIPHER_DestroyHandle(CVI_CIPHER_HANDLE hCipher);

/**
 * @brief Configure the CIPHER control information.
 *
 * @param hCipher [In] CIPHER handle.
 * @param pstCtrl [In] Control information pointer.
 *
 * @return 0 on success, non-zero error code on failure.
 */
CVI_S32 CVI_UNF_CIPHER_ConfigHandle(CVI_CIPHER_HANDLE hCipher,
				    const CVI_UNF_CIPHER_CTRL_S *pstCtrl);

/**
 * @brief Configure the CIPHER control information (extended).
 *
 * @param hCipher [In] CIPHER handle.
 * @param pstExCtrl [In] Control extended information pointer.
 *
 * @return 0 on success, non-zero error code on failure.
 */
CVI_S32
CVI_UNF_CIPHER_ConfigHandleEx(CVI_CIPHER_HANDLE hCipher,
			      const CVI_UNF_CIPHER_CTRL_EX_S *pstExCtrl);

/**
 * @brief Obtain the CIPHER configuration information.
 *
 * @param hCipher [In] CIPHER handle.
 * @param pstCtrl [Out] CIPHER channel configuration.
 *
 * @return 0 on success, non-zero error code on failure.
 */
CVI_S32 CVI_UNF_CIPHER_GetHandleConfig(CVI_CIPHER_HANDLE hCipher,
				       CVI_UNF_CIPHER_CTRL_S *pstCtrl);

/**
 * @brief Encrypt the data.
 *
 * @param hCipher [In] CIPHER handle.
 * @param u32SrcPhyAddr [In] Physical address of the source data.
 * @param u32DestPhyAddr [In] Physical address for storing encryption results.
 * @param u32ByteLength [In] Length of data (in bytes).
 *
 * @return 0 on success, non-zero error code on failure.
 */
CVI_S32 CVI_UNF_CIPHER_Encrypt(CVI_CIPHER_HANDLE hCipher,
			       CVI_SIZE_T u32SrcPhyAddr,
			       CVI_SIZE_T u32DestPhyAddr,
			       CVI_U32 u32ByteLength);

/**
 * @brief Decrypt the data.
 *
 * @param hCipher [In] CIPHER handle.
 * @param u32SrcPhyAddr [In] Physical address of source data.
 * @param u32DestPhyAddr [In] Physical address for storing decryption results.
 * @param u32ByteLength [In] Length of data (in bytes).
 *
 * @return 0 on success, non-zero error code on failure.
 */
CVI_S32 CVI_UNF_CIPHER_Decrypt(CVI_CIPHER_HANDLE hCipher,
			       CVI_SIZE_T u32SrcPhyAddr,
			       CVI_SIZE_T u32DestPhyAddr,
			       CVI_U32 u32ByteLength);

/**
 * @brief Encrypt the data.
 *
 * @param hCipher [In] CIPHER handle.
 * @param pu8SrcData [In] Virtual address of the source data.
 * @param pu8DestData [Out] Virtual address for storing encryption results.
 * @param u32ByteLength [In] Length of data (in bytes).
 *
 * @return 0 on success, non-zero error code on failure.
 */
CVI_S32 CVI_UNF_CIPHER_EncryptVir(CVI_CIPHER_HANDLE hCipher,
				  const CVI_U8 *pu8SrcData, CVI_U8 *pu8DestData,
				  CVI_U32 u32ByteLength);

/**
 * @brief Decrypt the data.
 *
 * @param hCipher [In] CIPHER handle.
 * @param pu8SrcData [In] Virtual address of source data.
 * @param pu8DestData [Out] Virtual address for storing decryption results.
 * @param u32ByteLength [In] Length of data (in bytes).
 *
 * @return 0 on success, non-zero error code on failure.
 */
CVI_S32 CVI_UNF_CIPHER_DecryptVir(CVI_CIPHER_HANDLE hCipher,
				  const CVI_U8 *pu8SrcData, CVI_U8 *pu8DestData,
				  CVI_U32 u32ByteLength);

/**
 * @brief Encrypt multiple packet data.
 *
 * @param hCipher [In] CIPHER handle.
 * @param pstDataPkg [In] Packets to be encrypted.
 * @param u32DataPkgNum [In] Number of packets to be encrypted.
 *
 * @return 0 on success, non-zero error code on failure.
 */
CVI_S32 CVI_UNF_CIPHER_EncryptMulti(CVI_CIPHER_HANDLE hCipher,
				    const CVI_UNF_CIPHER_DATA_S *pstDataPkg,
				    CVI_U32 u32DataPkgNum);

/**
 * @brief Decrypt multiple packet data.
 *
 * @param hCipher [In] CIPHER handle.
 * @param pstDataPkg [In] Packets to be decrypted.
 * @param u32DataPkgNum [In] Number of packets to be decrypted.
 *
 * @return 0 on success, non-zero error code on failure.
 */
CVI_S32 CVI_UNF_CIPHER_DecryptMulti(CVI_CIPHER_HANDLE hCipher,
				    const CVI_UNF_CIPHER_DATA_S *pstDataPkg,
				    CVI_U32 u32DataPkgNum);

/**
 * @brief Initialize the HASH module.
 *
 * @param pstHashAttr [In] Structure parameters for hash calculation.
 * @param pHashHandle [Out] Output hash handle.
 *
 * @return 0 on success, non-zero error code on failure.
 */
CVI_S32 CVI_UNF_CIPHER_HashInit(const CVI_UNF_CIPHER_HASH_ATTS_S *pstHashAttr,
				CVI_CIPHER_HANDLE *pHashHandle);

/**
 * @brief Calculate the hash value.
 *
 * @param hHashHandle [In] Hash handle.
 * @param pu8InputData [In] Input data buffer.
 * @param u32InputDataLen [In] Length of the input data (in bytes).
 *
 * @return 0 on success, non-zero error code on failure.
 */
CVI_S32 CVI_UNF_CIPHER_HashUpdate(CVI_CIPHER_HANDLE hHashHandle,
				  const CVI_U8 *pu8InputData,
				  CVI_U32 u32InputDataLen);

/**
 * @brief Obtain the hash value.
 *
 * @param hHashHandle [In] Hash handle.
 * @param pu8OutputHash [Out] Output hash.
 *
 * @return 0 on success, non-zero error code on failure.
 */
CVI_S32 CVI_UNF_CIPHER_HashFinal(CVI_CIPHER_HANDLE hHashHandle,
				 CVI_U8 *pu8OutputHash);

/**
 * @brief Obtain the random number.
 *
 * @param pu32RandomNumber [Out] Output random number.
 *
 * @return 0 on success, non-zero error code on failure.
 */
CVI_S32 CVI_UNF_CIPHER_GetRandomNumber(CVI_U32 *pu32RandomNumber);

/**
 * @brief Obtain the TAG value after encryption and decryption in CCM/GCM mode.
 *
 * @param hCipher [In] CIPHER handle.
 * @param pstTag [Out] TAG value.
 * @param pu32TagLen [Out] Lenth of TAG.
 *
 * @return 0 on success, non-zero error code on failure.
 */
CVI_S32 CVI_UNF_CIPHER_GetTag(CVI_CIPHER_HANDLE hCipher, CVI_U8 *pu8Tag,
			      CVI_U32 *pu32TagLen);

/**
 * @brief Encrypt plaintext using an RSA public key.
 *
 * @param pstRsaEnc [In] Public key encryption attribute structure.
 * @param pu8Input [In] Data to be encrypted.
 * @param u32InLen [In] Length of the data to be encrypted (in bytes).
 * @param pu8Output [Out] Encrypted result data.
 * @param pu32OutLen [Out] Length of the encrypted result data (in bytes).
 *
 * @return 0 on success, non-zero error code on failure.
 */
CVI_S32
CVI_UNF_CIPHER_RsaPublicEncrypt(const CVI_UNF_CIPHER_RSA_PUB_ENC_S *pstRsaEnc,
				const CVI_U8 *pu8Input, CVI_U32 u32InLen,
				CVI_U8 *pu8Output, CVI_U32 *pu32OutLen);

/**
 * @brief Decrypt ciphertext using an RSA private key.
 *
 * @param pstRsaDec [In] Private key decryption attribute structure.
 * @param pu8Input [In] Data to be decrypted.
 * @param u32InLen [In] Length of the data to be decrypted (in bytes).
 * @param pu8Output [Out] Decrypted result data.
 * @param pu32OutLen [Out] Length of the decrypted result data (in bytes).
 *
 * @return 0 on success, non-zero error code on failure.
 */
CVI_S32
CVI_UNF_CIPHER_RsaPrivateDecrypt(const CVI_UNF_CIPHER_RSA_PRI_ENC_S *pstRsaDec,
				 const CVI_U8 *pu8Input, CVI_U32 u32InLen,
				 CVI_U8 *pu8Output, CVI_U32 *pu32OutLen);

/**
 * @brief Encrypt plaintext using an RSA private key.
 *
 * @param pstRsaEnc [In] Private key encryption attribute structure.
 * @param pu8Input [In] Data to be encrypted.
 * @param u32InLen [In] Length of the data to be encrypted (in bytes).
 * @param pu8Output [Out] Encrypted result data.
 * @param pu32OutLen [Out] Length of the encrypted result data (in bytes).
 *
 * @return 0 on success, non-zero error code on failure.
 */
CVI_S32
CVI_UNF_CIPHER_RsaPrivateEncrypt(const CVI_UNF_CIPHER_RSA_PRI_ENC_S *pstRsaEnc,
				 const CVI_U8 *pu8Input, CVI_U32 u32InLen,
				 CVI_U8 *pu8Output, CVI_U32 *pu32OutLen);

/**
 * @brief Decrypt ciphertext using an RSA public key.
 *
 * @param pstRsaDec [In] Public key decryption attribute structure.
 * @param pu8Input [In] Data to be decrypted.
 * @param u32InLen [In] Length of the data to be decrypted (in bytes).
 * @param pu8Output [Out] Decrypted result data.
 * @param pu32OutLen [Out] Length of the decrypted result data (in bytes).
 *
 * @return 0 on success, non-zero error code on failure.
 */
CVI_S32
CVI_UNF_CIPHER_RsaPublicDecrypt(const CVI_UNF_CIPHER_RSA_PUB_ENC_S *pstRsaDec,
				const CVI_U8 *pu8Input, CVI_U32 u32InLen,
				CVI_U8 *pu8Output, CVI_U32 *pu32OutLen);

/**
 * @brief Sign data using an RSA private key.
 *
 * @param pstRsaSign [In] Signature attribute structure.
 * @param pu8InData [In] Data to be signed.
 * @param u32InDataLen [In] Length of the data to be signed (in bytes).
 * @param pu8HashData [In] HASH summary of the data to be signed.
 * @param pu8OutSign [Out] Signature result data.
 * @param pu32OutSignLen [Out] Length of the signature result data (in bytes).
 *
 * @return 0 on success, non-zero error code on failure.
 */
CVI_S32 CVI_UNF_CIPHER_RsaSign(const CVI_UNF_CIPHER_RSA_SIGN_S *pstRsaSign,
			       const CVI_U8 *pu8InData, CVI_U32 u32InDataLen,
			       const CVI_U8 *pu8HashData, CVI_U8 *pu8OutSign,
			       CVI_U32 *pu32OutSignLen);

/**
 * @brief Verify the validity and integrity of data using an RSA public key.
 *
 * @param pstRsaVerify [In] Signature verification attribute structure.
 * @param pu8InData [In] Data to be verified.
 * @param u32InDataLen [In] Length of the data to be verified (in bytes).
 * @param pu8HashData [In] HASH summary of the data to be verified.
 * @param pu8InSign [In] Signature data to be verified.
 * @param u32InSignLen [In] Length of the signature data to be verified (in bytes).
 *
 * @return 0 on success, non-zero error code on failure.
 */
CVI_S32
CVI_UNF_CIPHER_RsaVerify(const CVI_UNF_CIPHER_RSA_VERIFY_S *pstRsaVerify,
			 const CVI_U8 *pu8InData, CVI_U32 u32InDataLen,
			 const CVI_U8 *pu8HashData, const CVI_U8 *pu8InSign,
			 CVI_U32 u32InSignLen);

#endif /* end of include guard: CVI_UNF_CIPHER_H_FYUVBDPZ */
