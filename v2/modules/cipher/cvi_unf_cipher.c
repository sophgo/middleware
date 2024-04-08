/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: sample/cvi_unf_cipher.c
 * Description:
 */

// #define DEBUG 1

#include <unistd.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/hmac.h>
#include <openssl/rand.h>
#include <errno.h>

#include <cvi_unf_cipher.h>

#pragma GCC diagnostic ignored "-Wformat-zero-length"

#define _cc_trace(...) __trace("", __FILE__, __func__, __LINE__, __VA_ARGS__)
#define _cc_error(...) __trace("ERROR:", __FILE__, __func__, __LINE__, __VA_ARGS__)

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

// clang-format off
#define GET_CVI_CIPHER(cp, hCipher) do { \
		cp = (CVI_CIPHER_CTX *)hCipher; \
		if (!cp) { \
			_cc_error("hCipher is NULL\n"); \
			return CVI_FAILURE; \
		} \
	} while (0)

#define GET_CVI_MD(md, hHashHandle) do { \
		md = (CVI_MD_CTX *)hHashHandle; \
		if (!md) { \
			_cc_error("hHashHandle is NULL\n"); \
			return CVI_FAILURE; \
		} \
	} while (0)
// clang-format on

static CVI_BOOL cipher_is_inited;

typedef struct {
	CVI_UNF_CIPHER_ATTS_S attr;
	CVI_UNF_CIPHER_CTRL_S ctrl;
	CVI_UNF_CIPHER_HASH_ATTS_S hash_attr;

	CVI_U32 u32OddKey[8];
	CVI_U32 u32IVLen;
	CVI_U32 u32TagLen;
	CVI_U32 u32ALen;
	CVI_SIZE_T szAdataAddr;

	EVP_CIPHER_CTX *cp_ctx;

	const EVP_CIPHER *evc_cipher;
} CVI_CIPHER_CTX;

typedef struct {
	CVI_UNF_CIPHER_HASH_ATTS_S attr;

	EVP_MD_CTX *md_ctx;
	HMAC_CTX *hmac_ctx;
	CVI_BOOL is_hmac;

	const EVP_MD *evc_md;
	const EVP_MD *(*md_func)(void);
} CVI_MD_CTX;

static int __trace(const char *prefix, const char *path, const char *func, int lineno, const char *fmt, ...)
	__attribute__((__format__(__printf__, 5, 6)));
static void cvi_hexdump(const void *buf, size_t size, const char *name) __attribute__((unused));

#ifdef DEBUG
static int __trace(const char *prefix, const char *path, const char *func, int lineno, const char *fmt, ...)
{
	va_list ap;
	int ret;

	printf("[%s%s:%s:%d] ", prefix, path, func, lineno);
	if (!fmt || fmt[0] == '\0') {
		ret = printf("\n");
	} else {
		va_start(ap, fmt);
		ret = vprintf(fmt, ap);
		va_end(ap);
	}

	fflush(NULL);
	tcdrain(1);

	return ret;
}

static void cvi_hexdump(const void *buf, size_t size, const char *name)
{
	const uint8_t *p = buf;
	size_t i;

	if (name)
		printf("%s ", name);

	printf("(%lu):\n", size);

	for (i = 0; i < size; i++) {
		if (i % 16 == 0)
			printf("%04lx: ", i);
		printf("%02x ", p[i]);
		if (i % 16 == 15)
			printf("\n");
	}
	if (i % 16 != 15)
		printf("\n");
}
#else
static int __trace(const char *prefix, const char *path, const char *func, int lineno, const char *fmt, ...)
{
	prefix = prefix;
	path = path;
	func = func;
	lineno = lineno;
	fmt = fmt;
	return 0;
}

static void cvi_hexdump(const void *buf, size_t size, const char *name)
{
	buf = buf;
	size = size;
	name = name;
}
#endif /* DEBUG */

static void *cvi_cipher_malloc(size_t size)
{
	void *p = malloc(size);

	if (!p)
		_cc_error("malloc()");
	return p;
}

static void cvi_cipher_free(void *p)
{
	free(p);
}

// clang-format off
static struct {
	CVI_UNF_CIPHER_ALG_E alg;
	const char *name;
} alg_names[] = {
	{CVI_UNF_CIPHER_ALG_DES,  "des"},
	{CVI_UNF_CIPHER_ALG_3DES, "des"},
	{CVI_UNF_CIPHER_ALG_AES,  "aes"},
	{CVI_UNF_CIPHER_ALG_SM4,  "sm4"},
};

static struct {
	CVI_UNF_CIPHER_KEY_LENGTH_E len;
	const char *name;
} klen_names[] = {
	{CVI_UNF_CIPHER_KEY_AES_128BIT, "128"},
	{CVI_UNF_CIPHER_KEY_AES_192BIT, "192"},
	{CVI_UNF_CIPHER_KEY_AES_256BIT, "256"},
	{CVI_UNF_CIPHER_KEY_DES_3KEY,   "ede3"},
	{CVI_UNF_CIPHER_KEY_DES_2KEY,   "ede"},
};

static struct {
	CVI_UNF_CIPHER_WORK_MODE_E mode;
	const char *name;
} mode_names[] = {
	{CVI_UNF_CIPHER_WORK_MODE_ECB,     "ecb"},
	{CVI_UNF_CIPHER_WORK_MODE_CBC,     "cbc"},
	{CVI_UNF_CIPHER_WORK_MODE_CFB,     "cfb"},
	{CVI_UNF_CIPHER_WORK_MODE_OFB,     "ofb"},
	{CVI_UNF_CIPHER_WORK_MODE_CTR,     "ctr"},
	{CVI_UNF_CIPHER_WORK_MODE_CCM,     "ccm"},
	{CVI_UNF_CIPHER_WORK_MODE_GCM,     "gcm"},
	{CVI_UNF_CIPHER_WORK_MODE_CBC_CTS, "cts"},
};

static struct {
	CVI_UNF_CIPHER_HASH_TYPE_E eShaType;
	const EVP_MD *(*md_func)(void);
	CVI_BOOL is_hmac;
} md_map[] = {
	{CVI_UNF_CIPHER_HASH_TYPE_SHA1,          EVP_sha1, 0},
	{CVI_UNF_CIPHER_HASH_TYPE_SHA224,      EVP_sha224, 0},
	{CVI_UNF_CIPHER_HASH_TYPE_SHA256,      EVP_sha256, 0},
	{CVI_UNF_CIPHER_HASH_TYPE_SHA384,      EVP_sha384, 0},
	{CVI_UNF_CIPHER_HASH_TYPE_SHA512,      EVP_sha512, 0},
	{CVI_UNF_CIPHER_HASH_TYPE_HMAC_SHA1,     EVP_sha1, 1},
	{CVI_UNF_CIPHER_HASH_TYPE_HMAC_SHA224, EVP_sha224, 1},
	{CVI_UNF_CIPHER_HASH_TYPE_HMAC_SHA256, EVP_sha256, 1},
	{CVI_UNF_CIPHER_HASH_TYPE_HMAC_SHA384, EVP_sha384, 1},
	{CVI_UNF_CIPHER_HASH_TYPE_HMAC_SHA512, EVP_sha512, 1},
	{CVI_UNF_CIPHER_HASH_TYPE_SM3,            EVP_sm3, 0},
};
// clang-format on

CVI_S32 CVI_UNF_CIPHER_Init(void)
{
	_cc_trace("");
	cipher_is_inited = CVI_TRUE;
	return CVI_SUCCESS;
}

CVI_S32 CVI_UNF_CIPHER_DeInit(void)
{
	_cc_trace("");

	if (!cipher_is_inited)
		return CVI_ERR_CIPHER_NOT_INIT;

	cipher_is_inited = CVI_FALSE;
	return CVI_SUCCESS;
}

CVI_S32 CVI_UNF_CIPHER_CreateHandle(CVI_CIPHER_HANDLE *phCipher, const CVI_UNF_CIPHER_ATTS_S *pstCipherAttr)
{
	_cc_trace("");

	CVI_CIPHER_CTX *cp;

	if (!phCipher || !pstCipherAttr) {
		_cc_error("!phCipher || !pstCipherAttr\n");
		return CVI_FAILURE;
	}

	switch (pstCipherAttr->enCipherType) {
	case CVI_UNF_CIPHER_TYPE_NORMAL:
	case CVI_UNF_CIPHER_TYPE_COPY_AVOID:
		break;
	default:
		_cc_error("unknown stCipherAttr->enCipherType\n");
		return CVI_FAILURE;
	}

	cp = cvi_cipher_malloc(sizeof(CVI_CIPHER_CTX));
	if (!cp)
		return CVI_FAILURE;

	memset(cp, 0, sizeof(CVI_CIPHER_CTX));

	*phCipher = (CVI_CIPHER_HANDLE)cp;

	cp->attr = *pstCipherAttr;
	cp->cp_ctx = EVP_CIPHER_CTX_new();

	return CVI_SUCCESS;
}

CVI_S32 CVI_UNF_CIPHER_DestroyHandle(CVI_CIPHER_HANDLE hCipher)
{
	_cc_trace("");

	CVI_CIPHER_CTX *cp;

	GET_CVI_CIPHER(cp, hCipher);

	if (cp->cp_ctx)
		EVP_CIPHER_CTX_free(cp->cp_ctx);

	cvi_cipher_free(cp);
	return CVI_SUCCESS;
}

static const EVP_CIPHER *cvi_get_cipher(CVI_CIPHER_CTX *cp)
{
	char name[128], *pname;
	const char *alg = NULL, *klen = NULL, *mode = NULL;
	size_t i;
	int n;

	for (i = 0; i < ARRAY_SIZE(alg_names); i++) {
		if (alg_names[i].alg == cp->ctrl.enAlg) {
			alg = alg_names[i].name;
			break;
		}
	}
	if (!alg) {
		_cc_error("unknown alg %d\n", cp->ctrl.enAlg);
		return NULL;
	}

	if (cp->ctrl.enAlg == CVI_UNF_CIPHER_ALG_DES) {
		cp->ctrl.enKeyLen = CVI_UNF_CIPHER_KEY_DEFAULT;
	}

	if (cp->ctrl.enKeyLen == CVI_UNF_CIPHER_KEY_DEFAULT) {
		switch (cp->ctrl.enAlg) {
		case CVI_UNF_CIPHER_ALG_DES:
			klen = "";
			break;
		case CVI_UNF_CIPHER_ALG_3DES:
			klen = "ede3";
			break;
		case CVI_UNF_CIPHER_ALG_AES:
			klen = "128";
			break;
		case CVI_UNF_CIPHER_ALG_SM4:
			klen = "";
			break;
		default:
			_cc_error("unknown keylen %d\n", cp->ctrl.enKeyLen);
			return NULL;
		}
	} else {
		for (i = 0; i < ARRAY_SIZE(klen_names); i++) {
			if (klen_names[i].len == cp->ctrl.enKeyLen) {
				klen = klen_names[i].name;
				break;
			}
		}
	}

	if (!klen) {
		_cc_error("unknown keylen %d\n", cp->ctrl.enKeyLen);
		return NULL;
	}

	for (i = 0; i < ARRAY_SIZE(mode_names); i++) {
		if (mode_names[i].mode == cp->ctrl.enWorkMode) {
			mode = mode_names[i].name;
			break;
		}
	}
	if (!mode) {
		_cc_error("unknown mode %d\n", cp->ctrl.enWorkMode);
		return NULL;
	}

	pname = name;
	n = snprintf(pname, sizeof(name), "%s", alg);
	pname += n;
	if (klen[0] != '\0') {
		n = snprintf(pname, sizeof(name) - (pname - name), "-%s", klen);
		pname += n;
	}
	if (mode[0] != '\0') {
		n = snprintf(pname, sizeof(name) - (pname - name), "-%s", mode);
		pname += n;
	}
	_cc_trace("name=%s\n", name);

	const EVP_CIPHER *evc_cipher = EVP_get_cipherbyname(name);

	if (!evc_cipher)
		_cc_error("cannot find %s\n", name);

	return evc_cipher;
}

CVI_S32 CVI_UNF_CIPHER_ConfigHandle(CVI_CIPHER_HANDLE hCipher, const CVI_UNF_CIPHER_CTRL_S *pstCtrl)
{
	_cc_trace("");

	CVI_CIPHER_CTX *cp;

	GET_CVI_CIPHER(cp, hCipher);

	if (!pstCtrl) {
		_cc_error("!pstCtrl\n");
		return CVI_FAILURE;
	}

	cp->ctrl = *pstCtrl;

	switch (cp->ctrl.enAlg) {
	case CVI_UNF_CIPHER_ALG_DMA:
		break;
	case CVI_UNF_CIPHER_ALG_SM1:
		_cc_error("CVI_UNF_CIPHER_ALG_SM1 is not supported\n");
		return CVI_FAILURE;
	default:
		cp->evc_cipher = cvi_get_cipher(cp);
		if (!cp->evc_cipher) {
			return CVI_FAILURE;
		}
	}

	return CVI_SUCCESS;
}

CVI_BOOL cvi_is_aead(CVI_UNF_CIPHER_WORK_MODE_E mode)
{
	switch (mode) {
	case CVI_UNF_CIPHER_WORK_MODE_CCM:
	case CVI_UNF_CIPHER_WORK_MODE_GCM:
		return CVI_TRUE;
	default:
		return CVI_FALSE;
	}
}

CVI_S32 CVI_UNF_CIPHER_ConfigHandleEx(CVI_CIPHER_HANDLE hCipher, const CVI_UNF_CIPHER_CTRL_EX_S *pstExCtrl)
{
	_cc_trace("");

	CVI_CIPHER_CTX *cp;

	GET_CVI_CIPHER(cp, hCipher);

	if (!pstExCtrl) {
		_cc_error("!pstExCtrl\n");
		return CVI_FAILURE;
	}

	cp->ctrl.enAlg = pstExCtrl->enAlg;
	cp->ctrl.enWorkMode = pstExCtrl->enWorkMode;

	typedef struct {
		CVI_UNF_CIPHER_CTRL_DES_S des;
		CVI_UNF_CIPHER_CTRL_3DES_S tdes;
		CVI_UNF_CIPHER_CTRL_AES_S aes;
		CVI_UNF_CIPHER_CTRL_AES_CCM_GCM_S aes_aead;
		CVI_UNF_CIPHER_CTRL_SM4_S sm4;
	} CVI_CIPHER_PARAM;

	CVI_CIPHER_PARAM params;

	switch (cp->ctrl.enAlg) {
	case CVI_UNF_CIPHER_ALG_DES:
		params.des = *(CVI_UNF_CIPHER_CTRL_DES_S *)(pstExCtrl->pParam);
		memcpy(cp->ctrl.u32Key, params.des.u32Key, sizeof(params.des.u32Key));
		memcpy(cp->ctrl.u32IV, params.des.u32IV, sizeof(params.des.u32IV));
		cp->ctrl.enBitWidth = params.des.enBitWidth;
		cp->ctrl.enKeyLen = CVI_UNF_CIPHER_KEY_DEFAULT;
		cp->ctrl.stChangeFlags = params.des.stChangeFlags;
		break;
	case CVI_UNF_CIPHER_ALG_3DES:
		params.tdes = *(CVI_UNF_CIPHER_CTRL_3DES_S *)(pstExCtrl->pParam);
		memcpy(cp->ctrl.u32Key, params.tdes.u32Key, sizeof(params.tdes.u32Key));
		memcpy(cp->ctrl.u32IV, params.tdes.u32IV, sizeof(params.tdes.u32IV));
		cp->ctrl.enBitWidth = params.tdes.enBitWidth;
		cp->ctrl.enKeyLen = params.tdes.enKeyLen;
		cp->ctrl.stChangeFlags = params.tdes.stChangeFlags;
		break;
	case CVI_UNF_CIPHER_ALG_AES:
		if (cvi_is_aead(cp->ctrl.enWorkMode)) {
			params.aes_aead = *(CVI_UNF_CIPHER_CTRL_AES_CCM_GCM_S *)(pstExCtrl->pParam);
			memcpy(cp->ctrl.u32Key, params.aes_aead.u32Key, sizeof(params.aes_aead.u32Key));
			memcpy(cp->ctrl.u32IV, params.aes_aead.u32IV, sizeof(params.aes_aead.u32IV));
			cp->ctrl.enKeyLen = params.aes_aead.enKeyLen;

			cp->u32IVLen = params.aes_aead.u32IVLen;
			cp->u32TagLen = params.aes_aead.u32TagLen;
			cp->u32ALen = params.aes_aead.u32ALen;
			cp->szAdataAddr = params.aes_aead.szAdataAddr;
		} else {
			params.aes = *(CVI_UNF_CIPHER_CTRL_AES_S *)(pstExCtrl->pParam);
			memcpy(cp->ctrl.u32Key, params.aes.u32EvenKey, sizeof(params.aes.u32EvenKey));
			memcpy(cp->ctrl.u32IV, params.aes.u32IV, sizeof(params.aes.u32IV));
			cp->ctrl.enBitWidth = params.aes.enBitWidth;
			cp->ctrl.enKeyLen = params.aes.enKeyLen;
			cp->ctrl.stChangeFlags = params.aes.stChangeFlags;

			memcpy(cp->u32OddKey, params.aes.u32OddKey, sizeof(params.aes.u32OddKey));
		}
		break;
	case CVI_UNF_CIPHER_ALG_SM1:
		_cc_error("CVI_UNF_CIPHER_ALG_SM1 is not supported\n");
		return CVI_FAILURE;
	case CVI_UNF_CIPHER_ALG_SM4:
		params.sm4 = *(CVI_UNF_CIPHER_CTRL_SM4_S *)(pstExCtrl->pParam);
		memcpy(cp->ctrl.u32Key, params.sm4.u32Key, sizeof(params.sm4.u32Key));
		memcpy(cp->ctrl.u32IV, params.sm4.u32IV, sizeof(params.sm4.u32IV));
		cp->ctrl.stChangeFlags = params.sm4.stChangeFlags;
		break;
	case CVI_UNF_CIPHER_ALG_DMA:
		break;
	default:
		break;
	}

	cp->evc_cipher = cvi_get_cipher(cp);
	if (!cp->evc_cipher) {
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

static CVI_S32 _CVI_UNF_CIPHER_EncryptVir(CVI_CIPHER_HANDLE hCipher, const CVI_U8 *pu8SrcData, CVI_U8 *pu8DestData,
					  CVI_U32 u32ByteLength, CVI_BOOL bOddKey)
{
	_cc_trace("");

	CVI_CIPHER_CTX *cp;

	GET_CVI_CIPHER(cp, hCipher);

	const unsigned char *key;
	const unsigned char *iv = (unsigned char *)cp->ctrl.u32IV;

	unsigned char *out = pu8DestData;
	const unsigned char *in = pu8SrcData;
	const unsigned char *adata = (const unsigned char *)cp->szAdataAddr;
	int inlen = u32ByteLength;
	int outlen = 0;

	if (bOddKey)
		key = (unsigned char *)cp->u32OddKey;
	else
		key = (unsigned char *)cp->ctrl.u32Key;

	if (!cp->evc_cipher) {
		_cc_error("CIPHER handle is not configured\n");
		return CVI_FAILURE;
	}

	if (cp->ctrl.enAlg == CVI_UNF_CIPHER_ALG_DMA) {
		memcpy(pu8DestData, pu8SrcData, u32ByteLength);
	} else if (cvi_is_aead(cp->ctrl.enWorkMode)) {
		_cc_trace("cp->u32IVLen=%u cp->u32TagLen=%u\n", cp->u32IVLen, cp->u32TagLen);

		EVP_EncryptInit_ex(cp->cp_ctx, cp->evc_cipher, NULL, NULL, NULL);

		EVP_CIPHER_CTX_ctrl(cp->cp_ctx, EVP_CTRL_AEAD_SET_IVLEN, cp->u32IVLen, NULL);
		if (cp->ctrl.enWorkMode == CVI_UNF_CIPHER_WORK_MODE_CCM) {
			EVP_CIPHER_CTX_ctrl(cp->cp_ctx, EVP_CTRL_AEAD_SET_TAG, cp->u32TagLen, NULL);
		}
		EVP_EncryptInit_ex(cp->cp_ctx, NULL, NULL, key, iv);

		if (cp->ctrl.enWorkMode == CVI_UNF_CIPHER_WORK_MODE_CCM) {
			/* Set plaintext length: only needed if AAD is used */
			EVP_EncryptUpdate(cp->cp_ctx, NULL, &outlen, NULL, u32ByteLength);
		}

		/* Zero or one call to specify any AAD */
		EVP_EncryptUpdate(cp->cp_ctx, NULL, &outlen, adata, cp->u32ALen);

		/* Encrypt plaintext: can only be called once */
		EVP_EncryptUpdate(cp->cp_ctx, out, &outlen, in, inlen);

		/* Finalise: note get no output for GCM */
		EVP_EncryptFinal_ex(cp->cp_ctx, out, &outlen);
	} else {
		EVP_EncryptInit_ex(cp->cp_ctx, cp->evc_cipher, NULL, key, iv);
		EVP_EncryptUpdate(cp->cp_ctx, out, &outlen, in, inlen);
	}

	return CVI_SUCCESS;
}

static CVI_S32 _CVI_UNF_CIPHER_DecryptVir(CVI_CIPHER_HANDLE hCipher, const CVI_U8 *pu8SrcData, CVI_U8 *pu8DestData,
					  CVI_U32 u32ByteLength, CVI_BOOL bOddKey)
{
	_cc_trace("");

	CVI_CIPHER_CTX *cp;

	GET_CVI_CIPHER(cp, hCipher);

	const unsigned char *key;
	const unsigned char *iv = (unsigned char *)cp->ctrl.u32IV;

	unsigned char *out = pu8DestData;
	const unsigned char *in = pu8SrcData;
	const unsigned char *adata = (const unsigned char *)cp->szAdataAddr;
	int inlen = u32ByteLength;
	int outlen = 0;

	if (bOddKey)
		key = (unsigned char *)cp->u32OddKey;
	else
		key = (unsigned char *)cp->ctrl.u32Key;

	if (!cp->evc_cipher) {
		_cc_error("CIPHER handle is not configured\n");
		return CVI_FAILURE;
	}

	if (cp->ctrl.enAlg == CVI_UNF_CIPHER_ALG_DMA) {
		memcpy(pu8DestData, pu8SrcData, u32ByteLength);
	} else if (cvi_is_aead(cp->ctrl.enWorkMode)) {
		_cc_trace("cp->u32IVLen=%u cp->u32TagLen=%u\n", cp->u32IVLen, cp->u32TagLen);

		EVP_DecryptInit_ex(cp->cp_ctx, cp->evc_cipher, NULL, NULL, NULL);

		EVP_CIPHER_CTX_ctrl(cp->cp_ctx, EVP_CTRL_AEAD_SET_IVLEN, cp->u32IVLen, NULL);
		if (cp->ctrl.enWorkMode == CVI_UNF_CIPHER_WORK_MODE_CCM) {
			EVP_CIPHER_CTX_ctrl(cp->cp_ctx, EVP_CTRL_AEAD_SET_TAG, cp->u32TagLen, NULL);
		}
		EVP_DecryptInit_ex(cp->cp_ctx, NULL, NULL, key, iv);

		if (cp->ctrl.enWorkMode == CVI_UNF_CIPHER_WORK_MODE_CCM) {
			/* Set plaintext length: only needed if AAD is used */
			EVP_DecryptUpdate(cp->cp_ctx, NULL, &outlen, NULL, u32ByteLength);
		}

		/* Zero or one call to specify any AAD */
		EVP_DecryptUpdate(cp->cp_ctx, NULL, &outlen, adata, cp->u32ALen);

		_cc_trace("out=%p in=%p\n", out, in);

		/* Encrypt plaintext: can only be called once */
		int rv = EVP_DecryptUpdate(cp->cp_ctx, out, &outlen, in, inlen);

		_cc_trace("rv=%d\n", rv);

		/* Finalise: note get no output for CCM */
		// EVP_EncryptFinal_ex(cp->cp_ctx, out, &outlen);
	} else {
		EVP_DecryptInit_ex(cp->cp_ctx, cp->evc_cipher, NULL, key, iv);
		EVP_DecryptUpdate(cp->cp_ctx, out, &outlen, in, inlen);
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_UNF_CIPHER_GetTag(CVI_CIPHER_HANDLE hCipher, CVI_U8 *pu8Tag, CVI_U32 *pu32TagLen)
{
	CVI_CIPHER_CTX *cp;

	GET_CVI_CIPHER(cp, hCipher);

	if (!pu8Tag) {
		_cc_error("!pu8Tag\n");
		return CVI_FAILURE;
	}

	if (!pu32TagLen) {
		_cc_error("!pu32TagLen\n");
		return CVI_FAILURE;
	}

	if (*pu32TagLen < cp->u32TagLen) {
		_cc_error("*pu32TagLen is less than %u\n", cp->u32TagLen);
		return CVI_FAILURE;
	}

	*pu32TagLen = cp->u32TagLen;

	/* Get tag */
	EVP_CIPHER_CTX_ctrl(cp->cp_ctx, EVP_CTRL_AEAD_GET_TAG, *pu32TagLen, pu8Tag);

	return CVI_SUCCESS;
}

CVI_S32 CVI_UNF_CIPHER_Encrypt(CVI_CIPHER_HANDLE hCipher, CVI_SIZE_T szSrcAddr, CVI_SIZE_T szDestAddr,
			       CVI_U32 u32ByteLength)
{
	const CVI_U8 *pu8SrcData = (const CVI_U8 *)szSrcAddr;
	CVI_U8 *pu8DestData = (CVI_U8 *)szDestAddr;

	return CVI_UNF_CIPHER_EncryptVir(hCipher, pu8SrcData, pu8DestData, u32ByteLength);
}

CVI_S32 CVI_UNF_CIPHER_Decrypt(CVI_CIPHER_HANDLE hCipher, CVI_SIZE_T szSrcAddr, CVI_SIZE_T szDestAddr,
			       CVI_U32 u32ByteLength)
{
	const CVI_U8 *pu8SrcData = (const CVI_U8 *)szSrcAddr;
	CVI_U8 *pu8DestData = (CVI_U8 *)szDestAddr;

	return CVI_UNF_CIPHER_DecryptVir(hCipher, pu8SrcData, pu8DestData, u32ByteLength);
}

CVI_S32 CVI_UNF_CIPHER_EncryptVir(CVI_CIPHER_HANDLE hCipher, const CVI_U8 *pu8SrcData, CVI_U8 *pu8DestData,
				  CVI_U32 u32ByteLength)
{
	return _CVI_UNF_CIPHER_EncryptVir(hCipher, pu8SrcData, pu8DestData, u32ByteLength, CVI_FALSE);
}

CVI_S32 CVI_UNF_CIPHER_DecryptVir(CVI_CIPHER_HANDLE hCipher, const CVI_U8 *pu8SrcData, CVI_U8 *pu8DestData,
				  CVI_U32 u32ByteLength)
{
	return _CVI_UNF_CIPHER_DecryptVir(hCipher, pu8SrcData, pu8DestData, u32ByteLength, CVI_FALSE);
}

CVI_S32 CVI_UNF_CIPHER_GetHandleConfig(CVI_CIPHER_HANDLE hCipher, CVI_UNF_CIPHER_CTRL_S *pstCtrl)
{
	CVI_CIPHER_CTX *cp;

	GET_CVI_CIPHER(cp, hCipher);

	if (!pstCtrl) {
		_cc_error("!pstExCtrl\n");
		return CVI_FAILURE;
	}

	*pstCtrl = cp->ctrl;

	return CVI_SUCCESS;
}

CVI_S32 CVI_UNF_CIPHER_EncryptMulti(CVI_CIPHER_HANDLE hCipher, const CVI_UNF_CIPHER_DATA_S *pstDataPkg,
				    CVI_U32 u32DataPkgNum)
{
	CVI_U32 i, ret;

	if (!pstDataPkg) {
		_cc_error("!pstDataPkg\n");
		return CVI_FAILURE;
	}

	for (i = 0; i < u32DataPkgNum; i++) {
		const CVI_UNF_CIPHER_DATA_S *p = pstDataPkg + i;
		const CVI_U8 *pu8SrcData = (CVI_U8 *)p->szSrcAddr;
		CVI_U8 *pu8DestData = (CVI_U8 *)p->szDestAddr;

		ret = _CVI_UNF_CIPHER_EncryptVir(hCipher, pu8SrcData, pu8DestData, p->u32ByteLength, p->bOddKey);
		if (ret != CVI_SUCCESS)
			return ret;
	}
	return CVI_SUCCESS;
}

CVI_S32 CVI_UNF_CIPHER_DecryptMulti(CVI_CIPHER_HANDLE hCipher, const CVI_UNF_CIPHER_DATA_S *pstDataPkg,
				    CVI_U32 u32DataPkgNum)
{
	CVI_U32 i, ret;

	if (!pstDataPkg) {
		_cc_error("!pstDataPkg\n");
		return CVI_FAILURE;
	}

	for (i = 0; i < u32DataPkgNum; i++) {
		const CVI_UNF_CIPHER_DATA_S *p = pstDataPkg + i;
		const CVI_U8 *pu8SrcData = (CVI_U8 *)p->szSrcAddr;
		CVI_U8 *pu8DestData = (CVI_U8 *)p->szDestAddr;

		ret = _CVI_UNF_CIPHER_DecryptVir(hCipher, pu8SrcData, pu8DestData, p->u32ByteLength, p->bOddKey);
		if (ret != CVI_SUCCESS)
			return ret;
	}
	return CVI_SUCCESS;
}

CVI_S32 CVI_UNF_CIPHER_GetRandomNumber(CVI_U32 *pu32RandomNumber)
{
	unsigned char *buf = (unsigned char *)pu32RandomNumber;

	if (buf)
		RAND_bytes(buf, sizeof(CVI_U32));
	return CVI_SUCCESS;
}

CVI_S32 CVI_UNF_CIPHER_HashInit(const CVI_UNF_CIPHER_HASH_ATTS_S *pstHashAttr, CVI_CIPHER_HANDLE *pHashHandle)
{
	_cc_trace("");

	size_t i;

	CVI_MD_CTX *md;

	if (!pHashHandle || !pstHashAttr) {
		_cc_error("!pHashHandle || !pstHashAttr\n");
		return CVI_FAILURE;
	}

	md = cvi_cipher_malloc(sizeof(CVI_MD_CTX));
	if (!md)
		return CVI_FAILURE;

	memset(md, 0, sizeof(CVI_MD_CTX));
	*pHashHandle = (CVI_CIPHER_HANDLE)md;

	for (i = 0; i < ARRAY_SIZE(md_map); i++) {
		if (md_map[i].eShaType == pstHashAttr->eShaType) {
			md->md_func = md_map[i].md_func;
			md->is_hmac = md_map[i].is_hmac;
			break;
		}
	}

	if (!md->md_func) {
		_cc_error("eShaType %d is not supported\n", pstHashAttr->eShaType);
		cvi_cipher_free(md);
		return CVI_FAILURE;
	}

	md->attr = *pstHashAttr;

	if (md->is_hmac) {
		md->hmac_ctx = HMAC_CTX_new();
		HMAC_Init_ex(md->hmac_ctx, md->attr.pu8HMACKey, md->attr.u32HMACKeyLen, md->md_func(), NULL);
	} else {
		md->md_ctx = EVP_MD_CTX_new();
		EVP_DigestInit_ex(md->md_ctx, md->md_func(), NULL);
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_UNF_CIPHER_HashUpdate(CVI_CIPHER_HANDLE hHashHandle, const CVI_U8 *pu8InputData, CVI_U32 u32InputDataLen)
{
	CVI_MD_CTX *md;

	GET_CVI_MD(md, hHashHandle);

	if (md->is_hmac) {
		HMAC_Update(md->hmac_ctx, pu8InputData, u32InputDataLen);
	} else {
		EVP_DigestUpdate(md->md_ctx, pu8InputData, u32InputDataLen);
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_UNF_CIPHER_HashFinal(CVI_CIPHER_HANDLE hHashHandle, CVI_U8 *pu8OutputHash)
{
	unsigned int md_len = 128;

	CVI_MD_CTX *md;

	GET_CVI_MD(md, hHashHandle);

	if (md->is_hmac) {
		HMAC_Final(md->hmac_ctx, pu8OutputHash, &md_len);
		HMAC_CTX_free(md->hmac_ctx);
	} else {
		EVP_DigestFinal_ex(md->md_ctx, pu8OutputHash, &md_len);
		EVP_MD_CTX_free(md->md_ctx);
	}
	return CVI_SUCCESS;
}

static RSA *cvi_public_to_rsa(CVI_UNF_CIPHER_RSA_PUB_KEY_S stPubKey)
{
	BIGNUM *n = BN_bin2bn(stPubKey.pu8N, stPubKey.u16NLen, NULL);
	BIGNUM *e = BN_bin2bn(stPubKey.pu8E, stPubKey.u16ELen, NULL);

	RSA *rsa = RSA_new();

	RSA_set0_key(rsa, n, e, NULL);

	return rsa;
}

static RSA *cvi_private_to_rsa(CVI_UNF_CIPHER_RSA_PRI_KEY_S stPriKey)
{
	const unsigned char E_F4[4] = { 0x00, 0x01, 0x00, 0x01 };

	RSA *rsa = RSA_new();

	BIGNUM *n = BN_bin2bn(stPriKey.pu8N, stPriKey.u16NLen, NULL);
	BIGNUM *d = BN_bin2bn(stPriKey.pu8D, stPriKey.u16DLen, NULL);
	BIGNUM *e;

	if (stPriKey.pu8E) {
		_cc_trace("pu8E\n");
		e = BN_bin2bn(stPriKey.pu8E, stPriKey.u16ELen, NULL);
	} else {
		_cc_trace("E_F4\n");
		e = BN_bin2bn(E_F4, sizeof(E_F4), NULL);
	}

	RSA_set0_key(rsa, n, e, d);

	if (stPriKey.pu8P && stPriKey.pu8Q) {
		_cc_trace("with P Q\n");

		BIGNUM *p = BN_bin2bn(stPriKey.pu8P, stPriKey.u16PLen, NULL);
		BIGNUM *q = BN_bin2bn(stPriKey.pu8Q, stPriKey.u16QLen, NULL);

		RSA_set0_factors(rsa, p, q);
	}

	if (stPriKey.pu8DP && stPriKey.pu8DQ && stPriKey.pu8QP) {
		_cc_trace("with CRT\n");

		BIGNUM *dp = BN_bin2bn(stPriKey.pu8DP, stPriKey.u16DPLen, NULL);
		BIGNUM *dq = BN_bin2bn(stPriKey.pu8DQ, stPriKey.u16DQLen, NULL);
		BIGNUM *qp = BN_bin2bn(stPriKey.pu8QP, stPriKey.u16QPLen, NULL);

		RSA_set0_crt_params(rsa, dp, dq, qp);
	}

	return rsa;
}

static CVI_S32 cvi_set_enc_scheme(EVP_PKEY_CTX *ctx, CVI_UNF_CIPHER_RSA_ENC_SCHEME_E scheme)
{
	switch (scheme) {
	case CVI_UNF_CIPHER_RSA_ENC_SCHEME_NO_PADDING:
		if (!EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_NO_PADDING)) {
			_cc_error("EVP_PKEY_CTX_set_rsa_padding()\n");
			return CVI_FAILURE;
		}
		break;
	case CVI_UNF_CIPHER_RSA_ENC_SCHEME_BLOCK_TYPE_0:
	case CVI_UNF_CIPHER_RSA_ENC_SCHEME_BLOCK_TYPE_1:
	case CVI_UNF_CIPHER_RSA_ENC_SCHEME_BLOCK_TYPE_2:
	case CVI_UNF_CIPHER_RSA_ENC_SCHEME_RSAES_PKCS1_V1_5:
		if (!EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_PADDING)) {
			_cc_error("EVP_PKEY_CTX_set_rsa_padding()\n");
			return CVI_FAILURE;
		}
		break;
	case CVI_UNF_CIPHER_RSA_ENC_SCHEME_RSAES_OAEP_SHA1:
		if (!EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING)) {
			_cc_error("EVP_PKEY_CTX_set_rsa_padding()\n");
			return CVI_FAILURE;
		}
		if (!EVP_PKEY_CTX_set_rsa_oaep_md(ctx, EVP_sha1())) {
			_cc_error("EVP_PKEY_CTX_set_rsa_oaep_md()\n");
			return CVI_FAILURE;
		}
		break;
	case CVI_UNF_CIPHER_RSA_ENC_SCHEME_RSAES_OAEP_SHA224:
		if (!EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING)) {
			_cc_error("EVP_PKEY_CTX_set_rsa_padding()\n");
			return CVI_FAILURE;
		}
		if (!EVP_PKEY_CTX_set_rsa_oaep_md(ctx, EVP_sha224())) {
			_cc_error("EVP_PKEY_CTX_set_rsa_oaep_md()\n");
			return CVI_FAILURE;
		}
		break;
	case CVI_UNF_CIPHER_RSA_ENC_SCHEME_RSAES_OAEP_SHA256:
		if (!EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING)) {
			_cc_error("EVP_PKEY_CTX_set_rsa_padding()\n");
			return CVI_FAILURE;
		}
		if (!EVP_PKEY_CTX_set_rsa_oaep_md(ctx, EVP_sha256())) {
			_cc_error("EVP_PKEY_CTX_set_rsa_oaep_md()\n");
			return CVI_FAILURE;
		}
		break;
	case CVI_UNF_CIPHER_RSA_ENC_SCHEME_RSAES_OAEP_SHA384:
		if (!EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING)) {
			_cc_error("EVP_PKEY_CTX_set_rsa_padding()\n");
			return CVI_FAILURE;
		}
		if (!EVP_PKEY_CTX_set_rsa_oaep_md(ctx, EVP_sha384())) {
			_cc_error("EVP_PKEY_CTX_set_rsa_oaep_md()\n");
			return CVI_FAILURE;
		}
		break;
	case CVI_UNF_CIPHER_RSA_ENC_SCHEME_RSAES_OAEP_SHA512:
		if (!EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING)) {
			_cc_error("EVP_PKEY_CTX_set_rsa_padding()\n");
			return CVI_FAILURE;
		}
		if (!EVP_PKEY_CTX_set_rsa_oaep_md(ctx, EVP_sha512())) {
			_cc_error("EVP_PKEY_CTX_set_rsa_oaep_md()\n");
			return CVI_FAILURE;
		}
		break;
	default:
		_cc_error("unknown scheme %d\n", scheme);
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

CVI_S32 CVI_UNF_CIPHER_RsaPublicEncrypt(const CVI_UNF_CIPHER_RSA_PUB_ENC_S *pstRsaEnc, const CVI_U8 *pu8Input,
					CVI_U32 u32InLen, CVI_U8 *pu8Output, CVI_U32 *pu32OutLen)
{
	int ret;

	if (!pstRsaEnc || !pu8Input) {
		_cc_error("!pstRsaEnc || !pu8Input\n");
		return CVI_FAILURE;
	}

	RSA *rsa = cvi_public_to_rsa(pstRsaEnc->stPubKey);
	EVP_PKEY *pk;

	pk = EVP_PKEY_new();
	if (!EVP_PKEY_set1_RSA(pk, rsa)) {
		_cc_error("EVP_PKEY_set1_RSA()\n");
		return CVI_FAILURE;
	}

	EVP_PKEY_CTX *ctx;

	ctx = EVP_PKEY_CTX_new(pk, NULL);
	if (!EVP_PKEY_encrypt_init(ctx)) {
		_cc_error("EVP_PKEY_encrypt_init()\n");
		return CVI_FAILURE;
	}

	ret = cvi_set_enc_scheme(ctx, pstRsaEnc->enScheme);
	if (ret != CVI_SUCCESS)
		return ret;

	const unsigned char *in = pu8Input;
	unsigned char *out = pu8Output;
	size_t inlen = u32InLen;
	size_t outlen = 0;

	ret = EVP_PKEY_encrypt(ctx, NULL, &outlen, in, inlen);
	if (ret <= 0) {
		_cc_error("EVP_PKEY_encrypt()\n");
		return CVI_FAILURE;
	}

	if (out) {
		ret = EVP_PKEY_encrypt(ctx, out, &outlen, in, inlen);
		if (ret <= 0) {
			_cc_error("EVP_PKEY_encrypt()\n");
			return CVI_FAILURE;
		}
	}

	if (pu32OutLen)
		*pu32OutLen = outlen;

	EVP_PKEY_CTX_free(ctx);

	return CVI_SUCCESS;
}

CVI_S32 CVI_UNF_CIPHER_RsaPrivateDecrypt(const CVI_UNF_CIPHER_RSA_PRI_ENC_S *pstRsaDec, const CVI_U8 *pu8Input,
					 CVI_U32 u32InLen, CVI_U8 *pu8Output, CVI_U32 *pu32OutLen)
{
	int ret;

	if (!pstRsaDec || !pu8Input) {
		_cc_error("!pstRsaDec || !pu8Input\n");
		return CVI_FAILURE;
	}

	RSA *rsa = cvi_private_to_rsa(pstRsaDec->stPriKey);
	EVP_PKEY *pk;

	pk = EVP_PKEY_new();
	if (!EVP_PKEY_set1_RSA(pk, rsa)) {
		_cc_error("EVP_PKEY_set1_RSA()\n");
		return CVI_FAILURE;
	}

	EVP_PKEY_CTX *ctx;

	ctx = EVP_PKEY_CTX_new(pk, NULL);
	if (!EVP_PKEY_decrypt_init(ctx)) {
		_cc_error("EVP_PKEY_decrypt_init()\n");
		return CVI_FAILURE;
	}

	const unsigned char *in = pu8Input;
	unsigned char *out = pu8Output;
	size_t inlen = u32InLen;
	size_t outlen = 0;

	ret = cvi_set_enc_scheme(ctx, pstRsaDec->enScheme);
	if (ret != CVI_SUCCESS)
		return ret;

	ret = EVP_PKEY_decrypt(ctx, NULL, &outlen, in, inlen);
	if (ret <= 0) {
		_cc_error("EVP_PKEY_decrypt()=%d\n", ret);
		return CVI_FAILURE;
	}

	if (out) {
		ret = EVP_PKEY_decrypt(ctx, out, &outlen, in, inlen);
		if (ret <= 0) {
			_cc_error("EVP_PKEY_decrypt()=%d\n", ret);
			return CVI_FAILURE;
		}
	}

	if (pu32OutLen)
		*pu32OutLen = outlen;

	EVP_PKEY_CTX_free(ctx);

	return CVI_SUCCESS;
}

CVI_S32 CVI_UNF_CIPHER_RsaPrivateEncrypt(const CVI_UNF_CIPHER_RSA_PRI_ENC_S *pstRsaEnc, const CVI_U8 *pu8Input,
					 CVI_U32 u32InLen, CVI_U8 *pu8Output, CVI_U32 *pu32OutLen)
{
	_cc_trace("");
	int ret;
	CVI_U32 num;

	if (!pstRsaEnc || !pu8Input) {
		_cc_error("!pstRsaEnc || !pu8Input\n");
		return CVI_FAILURE;
	}

	RSA *rsa = cvi_private_to_rsa(pstRsaEnc->stPriKey);

	num = RSA_size(rsa);

	const unsigned char *in = pu8Input;
	unsigned char *out = pu8Output;
	size_t inlen = u32InLen;

	if (u32InLen > num) {
		_cc_error("Ru32InLen > %d\n", num);
		return CVI_FAILURE;
	}

	if (out) {
		switch (pstRsaEnc->enScheme) {
		case CVI_UNF_CIPHER_RSA_ENC_SCHEME_BLOCK_TYPE_1:
			break;
		default:
			return CVI_FAILURE;
		}

		ret = RSA_private_encrypt(inlen, in, out, rsa, RSA_PKCS1_PADDING);

		if (ret <= 0) {
			_cc_error("RSA_private_encrypt()=%d\n", ret);
			RSA_free(rsa);
			return CVI_FAILURE;
		}
	}

	if (pu32OutLen)
		*pu32OutLen = num;

	RSA_free(rsa);

	return CVI_SUCCESS;
}

CVI_S32 CVI_UNF_CIPHER_RsaPublicDecrypt(const CVI_UNF_CIPHER_RSA_PUB_ENC_S *pstRsaDec, const CVI_U8 *pu8Input,
					CVI_U32 u32InLen, CVI_U8 *pu8Output, CVI_U32 *pu32OutLen)
{
	_cc_trace("");
	int ret = 0;

	if (!pstRsaDec || !pu8Input) {
		_cc_error("!pstRsaDec || !pu8Input\n");
		return CVI_FAILURE;
	}

	RSA *rsa = cvi_public_to_rsa(pstRsaDec->stPubKey);

	const unsigned char *in = pu8Input;
	unsigned char *out = pu8Output;
	size_t inlen = u32InLen;

	if (out) {
		switch (pstRsaDec->enScheme) {
		case CVI_UNF_CIPHER_RSA_ENC_SCHEME_BLOCK_TYPE_1:
			break;
		default:
			return CVI_FAILURE;
		}

		ret = RSA_public_decrypt(inlen, in, out, rsa, RSA_PKCS1_PADDING);
		if (ret <= 0) {
			_cc_error("RSA_private_encrypt()=%d\n", ret);
			RSA_free(rsa);
			return CVI_FAILURE;
		}
	}

	if (pu32OutLen)
		*pu32OutLen = ret;

	RSA_free(rsa);

	return CVI_SUCCESS;
}

CVI_S32 CVI_UNF_CIPHER_RsaSign(const CVI_UNF_CIPHER_RSA_SIGN_S *pstRsaSign, const CVI_U8 *pu8InData,
			       CVI_U32 u32InDataLen, const CVI_U8 *pu8HashData, CVI_U8 *pu8OutSign,
			       CVI_U32 *pu32OutSignLen)
{
	int ret;

	if (!pstRsaSign) {
		_cc_error("!pstRsaSign\n");
		return CVI_FAILURE;
	}

	if (!pu8InData && !pu8HashData) {
		_cc_error("!pu8InData && !pu8HashData\n");
		return CVI_FAILURE;
	}

	RSA *rsa = cvi_private_to_rsa(pstRsaSign->stPriKey);
	EVP_PKEY *pk;

	pk = EVP_PKEY_new();
	if (!EVP_PKEY_set1_RSA(pk, rsa)) {
		_cc_error("EVP_PKEY_set1_RSA()\n");
		return CVI_FAILURE;
	}

	EVP_PKEY_CTX *ctx;

	unsigned char *sig = pu8OutSign;
	size_t siglen = 0;

	if (pu8InData) {
		_cc_trace("use pu8InData\n");
		const unsigned char *in = pu8InData;
		size_t inlen = u32InDataLen;

		EVP_MD_CTX *mctx = EVP_MD_CTX_new();

		if (!EVP_DigestSignInit(mctx, &ctx, EVP_sha256(), NULL, pk)) {
			_cc_error("EVP_PKEY_sign_init()\n");
			return CVI_FAILURE;
		}

		EVP_DigestSignUpdate(mctx, in, inlen);
		EVP_DigestSignFinal(mctx, NULL, &siglen);
		EVP_DigestSignFinal(mctx, sig, &siglen);

	} else {
		_cc_trace("use pu8HashData\n");
		ctx = EVP_PKEY_CTX_new(pk, NULL);
		const unsigned char *md = pu8HashData;
		size_t mdlen = 32;

		if (!EVP_PKEY_sign_init(ctx)) {
			_cc_error("EVP_PKEY_sign_init()\n");
			return CVI_FAILURE;
		}

		EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_PADDING);
		EVP_PKEY_CTX_set_signature_md(ctx, EVP_sha256());

		ret = EVP_PKEY_sign(ctx, NULL, &siglen, md, mdlen);
		if (ret <= 0) {
			_cc_error("EVP_PKEY_sign()=%d\n", ret);
			return CVI_FAILURE;
		}

		if (sig) {
			ret = EVP_PKEY_sign(ctx, sig, &siglen, md, mdlen);
			if (ret <= 0) {
				_cc_error("EVP_PKEY_decrypt()=%d\n", ret);
				return CVI_FAILURE;
			}
		}
	}

	if (pu32OutSignLen)
		*pu32OutSignLen = siglen;

	EVP_PKEY_CTX_free(ctx);

	return CVI_SUCCESS;
}

CVI_S32 CVI_UNF_CIPHER_RsaVerify(const CVI_UNF_CIPHER_RSA_VERIFY_S *pstRsaVerify, const CVI_U8 *pu8InData,
				 CVI_U32 u32InDataLen, const CVI_U8 *pu8HashData, const CVI_U8 *pu8InSign,
				 CVI_U32 u32InSignLen)
{
	int ret = CVI_FAILURE;

	if (!pstRsaVerify) {
		_cc_error("!pstRsaVerify\n");
		return CVI_FAILURE;
	}

	if (!pu8InData && !pu8HashData) {
		_cc_error("!pu8InData && !pu8HashData\n");
		return CVI_FAILURE;
	}

	RSA *rsa = cvi_public_to_rsa(pstRsaVerify->stPubKey);
	EVP_PKEY *pk;

	pk = EVP_PKEY_new();
	if (!EVP_PKEY_set1_RSA(pk, rsa)) {
		_cc_error("EVP_PKEY_set1_RSA()\n");
		return CVI_FAILURE;
	}

	EVP_PKEY_CTX *ctx;

	const unsigned char *sig = pu8InSign;
	size_t siglen = u32InSignLen;

	if (pu8InData) {
		_cc_trace("use pu8InData\n");
		const unsigned char *in = pu8InData;
		size_t inlen = u32InDataLen;

		EVP_MD_CTX *mctx = EVP_MD_CTX_new();

		if (!EVP_DigestVerifyInit(mctx, &ctx, EVP_sha256(), NULL, pk)) {
			_cc_error("EVP_PKEY_sign_init()\n");
			return CVI_FAILURE;
		}

		EVP_DigestVerifyUpdate(mctx, in, inlen);
		if (EVP_DigestVerifyFinal(mctx, sig, siglen))
			ret = CVI_SUCCESS;

	} else {
		_cc_trace("use pu8HashData\n");
	}

	EVP_PKEY_CTX_free(ctx);

	return ret;
}
