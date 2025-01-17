/*****************************************************************************
 Copyright (c) 2018-2023, Intel Corporation

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

     * Redistributions of source code must retain the above copyright notice,
       this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
     * Neither the name of Intel Corporation nor the names of its contributors
       may be used to endorse or promote products derived from this software
       without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*****************************************************************************/

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <intel-ipsec-mb.h>

#include "utils.h"

#define MAX_BURST_JOBS 64

int aes_test(struct IMB_MGR *mb_mgr);

struct aes_vector {
	const uint8_t *K;          /* key */
	const uint8_t *IV;         /* initialization vector */
	const uint8_t *P;          /* plain text */
	uint64_t       Plen;       /* plain text length */
        const uint8_t *C;          /* cipher text - same length as plain text */
        uint32_t       Klen;       /* key length */
};

/* =================================================================== */
/* =================================================================== */

/*
 *  AES Test vectors from
 *  http://csrc.nist.gov/publications/fips/fips197/fips-197.pdf
 */

/*  DOCSIS: AES-128 CFB */
static const uint8_t DK1[] = {
        0xe6, 0x60, 0x0f, 0xd8, 0x85, 0x2e, 0xf5, 0xab,
        0xe6, 0x60, 0x0f, 0xd8, 0x85, 0x2e, 0xf5, 0xab
};
static const uint8_t DIV1[] = {
        0x81, 0x0e, 0x52, 0x8e, 0x1c, 0x5f, 0xda, 0x1a,
        0x81, 0x0e, 0x52, 0x8e, 0x1c, 0x5f, 0xda, 0x1a
};
static const uint8_t DP1[] = {
        0x00, 0x01, 0x02, 0x88, 0xee, 0x59, 0x7e
};
static const uint8_t DC1[] = {
        0xfc, 0x68, 0xa3, 0x55, 0x60, 0x37, 0xdc
};

/*  DOCSIS: AES-128 CBC + CFB */
static const uint8_t DK2[] = {
        0xe6, 0x60, 0x0f, 0xd8, 0x85, 0x2e, 0xf5, 0xab,
        0xe6, 0x60, 0x0f, 0xd8, 0x85, 0x2e, 0xf5, 0xab
};
static const uint8_t DIV2[] = {
        0x81, 0x0e, 0x52, 0x8e, 0x1c, 0x5f, 0xda, 0x1a,
        0x81, 0x0e, 0x52, 0x8e, 0x1c, 0x5f, 0xda, 0x1a
};
static const uint8_t DP2[] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x91,
        0xd2, 0xd1, 0x9f
};
static const uint8_t DC2[] = {
        0x9d, 0xd1, 0x67, 0x4b, 0xba, 0x61, 0x10, 0x1b,
        0x56, 0x75, 0x64, 0x74, 0x36, 0x4f, 0x10, 0x1d,
        0x44, 0xd4, 0x73
};

/*  DOCSIS: AES-128 CBC */
static const uint8_t DK3[] = {
        0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
        0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c
};
static const uint8_t DIV3[] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f
};
static const uint8_t DP3[] = {
        0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96,
        0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a,
        0xae, 0x2d, 0x8a, 0x57, 0x1e, 0x03, 0xac, 0x9c,
        0x9e, 0xb7, 0x6f, 0xac, 0x45, 0xaf, 0x8e, 0x51,
        0x30, 0xc8, 0x1c, 0x46, 0xa3, 0x5c, 0xe4, 0x11,
        0xe5, 0xfb, 0xc1, 0x19, 0x1a, 0x0a, 0x52, 0xef,
        0xf6, 0x9f, 0x24, 0x45, 0xdf, 0x4f, 0x9b, 0x17,
        0xad, 0x2b, 0x41, 0x7b, 0xe6, 0x6c, 0x37, 0x10
};
static const uint8_t DC3[] = {
        0x76, 0x49, 0xab, 0xac, 0x81, 0x19, 0xb2, 0x46,
        0xce, 0xe9, 0x8e, 0x9b, 0x12, 0xe9, 0x19, 0x7d,
        0x50, 0x86, 0xcb, 0x9b, 0x50, 0x72, 0x19, 0xee,
        0x95, 0xdb, 0x11, 0x3a, 0x91, 0x76, 0x78, 0xb2,
        0x73, 0xbe, 0xd6, 0xb8, 0xe3, 0xc1, 0x74, 0x3b,
        0x71, 0x16, 0xe6, 0x9e, 0x22, 0x22, 0x95, 0x16,
        0x3f, 0xf1, 0xca, 0xa1, 0x68, 0x1f, 0xac, 0x09,
        0x12, 0x0e, 0xca, 0x30, 0x75, 0x86, 0xe1, 0xa7
};

/*  DOCSIS: AES-256 CFB */
static const uint8_t DK4[] = {
        0xE6, 0x60, 0x0F, 0xD8, 0x85, 0x2E, 0xF5, 0xAB,
        0xE6, 0x60, 0x0F, 0xD8, 0x85, 0x2E, 0xF5, 0xAB,
        0xE6, 0x60, 0x0F, 0xD8, 0x85, 0x2E, 0xF5, 0xAB,
        0xE6, 0x60, 0x0F, 0xD8, 0x85, 0x2E, 0xF5, 0xAB
};
static const uint8_t DIV4[] = {
        0x81, 0x0E, 0x52, 0x8E, 0x1C, 0x5F, 0xDA, 0x1A,
        0x81, 0x0E, 0x52, 0x8E, 0x1C, 0x5F, 0xDA, 0x1A
};
static const uint8_t DP4[] = {
        0x00, 0x01, 0x02, 0x88, 0xEE, 0x59, 0x7E
};
static const uint8_t DC4[] = {
        0xE3, 0x75, 0xF2, 0x30, 0x1F, 0x75, 0x9A
};

/*  DOCSIS: AES-256 CBC + CFB */
static const uint8_t DK5[] = {
        0xE6, 0x60, 0x0F, 0xD8, 0x85, 0x2E, 0xF5, 0xAB,
        0xE6, 0x60, 0x0F, 0xD8, 0x85, 0x2E, 0xF5, 0xAB,
        0xE6, 0x60, 0x0F, 0xD8, 0x85, 0x2E, 0xF5, 0xAB,
        0xE6, 0x60, 0x0F, 0xD8, 0x85, 0x2E, 0xF5, 0xAB
};
static const uint8_t DIV5[] = {
        0x81, 0x0E, 0x52, 0x8E, 0x1C, 0x5F, 0xDA, 0x1A,
        0x81, 0x0E, 0x52, 0x8E, 0x1C, 0x5F, 0xDA, 0x1A
};
static const uint8_t DP5[] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x91,
        0xD2, 0xD1, 0x9F
};
static const uint8_t DC5[] = {
        0xD1, 0x28, 0x73, 0x1F, 0xB5, 0x28, 0xB5, 0x18,
        0xAB, 0x51, 0xAB, 0xC8, 0x98, 0x3D, 0xD1, 0xEE,
        0xE4, 0x43, 0x59
};

/*  DOCSIS: AES-256 CBC */
static const uint8_t DK6[] = {
        0xE4, 0x23, 0x33, 0x8A, 0x35, 0x64, 0x61, 0xE2,
        0x49, 0x03, 0xDD, 0xC6, 0xB8, 0xCA, 0x55, 0x7A,
        0xD4, 0xC3, 0xA3, 0xAA, 0x33, 0x62, 0x61, 0xE0,
        0x37, 0x07, 0xB8, 0x23, 0xA2, 0xA3, 0xB5, 0x8D
};
static const uint8_t DIV6[] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F
};
static const uint8_t DP6[] = {
        0x57, 0x68, 0x61, 0x74, 0x20, 0x61, 0x20, 0x6c,
        0x6f, 0x75, 0x73, 0x79, 0x20, 0x65, 0x61, 0x72,
        0x74, 0x68, 0x21, 0x20, 0x48, 0x65, 0x20, 0x77,
        0x6f, 0x6e, 0x64, 0x65, 0x72, 0x65, 0x64, 0x20,
        0x68, 0x6f, 0x77, 0x20, 0x6d, 0x61, 0x6e, 0x79,
        0x20, 0x70, 0x65, 0x6f, 0x70, 0x6c, 0x65, 0x20,
        0x77, 0x65, 0x72, 0x65, 0x20, 0x64, 0x65, 0x73,
        0x74, 0x69, 0x74, 0x75, 0x74, 0x65, 0x20, 0x74,
        0x68, 0x61, 0x74, 0x20, 0x73, 0x61, 0x6d, 0x65,
        0x20, 0x6e, 0x69, 0x67, 0x68, 0x74, 0x20, 0x65,
        0x76, 0x65, 0x6e, 0x20, 0x69, 0x6e, 0x20, 0x68,
        0x69, 0x73, 0x20, 0x6f, 0x77, 0x6e, 0x20, 0x70,
        0x72, 0x6f, 0x73, 0x70, 0x65, 0x72, 0x6f, 0x75,
        0x73, 0x20, 0x63, 0x6f, 0x75, 0x6e, 0x74, 0x72,
        0x79, 0x2c, 0x20, 0x68, 0x6f, 0x77, 0x20, 0x6d,
        0x61, 0x6e, 0x79, 0x20, 0x68, 0x6f, 0x6d, 0x65,
        0x73, 0x20, 0x77, 0x65, 0x72, 0x65, 0x20, 0x73,
        0x68, 0x61, 0x6e, 0x74, 0x69, 0x65, 0x73, 0x2c,
        0x20, 0x68, 0x6f, 0x77, 0x20, 0x6d, 0x61, 0x6e,
        0x79, 0x20, 0x68, 0x75, 0x73, 0x62, 0x61, 0x6e,
        0x64, 0x73, 0x20, 0x77, 0x65, 0x72, 0x65, 0x20,
        0x64, 0x72, 0x75, 0x6e, 0x6b, 0x20, 0x61, 0x6e,
        0x64, 0x20, 0x77, 0x69, 0x76, 0x65, 0x73, 0x20,
        0x73, 0x6f, 0x63, 0x6b, 0x65, 0x64, 0x2c, 0x20,
        0x61, 0x6e, 0x64, 0x20, 0x68, 0x6f, 0x77, 0x20,
        0x6d, 0x61, 0x6e, 0x79, 0x20, 0x63, 0x68, 0x69,
        0x6c, 0x64, 0x72, 0x65, 0x6e, 0x20, 0x77, 0x65,
        0x72, 0x65, 0x20, 0x62, 0x75, 0x6c, 0x6c, 0x69,
        0x65, 0x64, 0x2c, 0x20, 0x61, 0x62, 0x75, 0x73,
        0x65, 0x64, 0x2c, 0x20, 0x6f, 0x72, 0x20, 0x61,
        0x62, 0x61, 0x6e, 0x64, 0x6f, 0x6e, 0x65, 0x64,
        0x2e, 0x20, 0x48, 0x6f, 0x77, 0x20, 0x6d, 0x61,
        0x6e, 0x79, 0x20, 0x66, 0x61, 0x6d, 0x69, 0x6c,
        0x69, 0x65, 0x73, 0x20, 0x68, 0x75, 0x6e, 0x67,
        0x65, 0x72, 0x65, 0x64, 0x20, 0x66, 0x6f, 0x72,
        0x20, 0x66, 0x6f, 0x6f, 0x64, 0x20, 0x74, 0x68,
        0x65, 0x79, 0x20, 0x63, 0x6f, 0x75, 0x6c, 0x64,
        0x20, 0x6e, 0x6f, 0x74, 0x20, 0x61, 0x66, 0x66,
        0x6f, 0x72, 0x64, 0x20, 0x74, 0x6f, 0x20, 0x62,
        0x75, 0x79, 0x3f, 0x20, 0x48, 0x6f, 0x77, 0x20,
        0x6d, 0x61, 0x6e, 0x79, 0x20, 0x68, 0x65, 0x61,
        0x72, 0x74, 0x73, 0x20, 0x77, 0x65, 0x72, 0x65,
        0x20, 0x62, 0x72, 0x6f, 0x6b, 0x65, 0x6e, 0x3f,
        0x20, 0x48, 0x6f, 0x77, 0x20, 0x6d, 0x61, 0x6e,
        0x79, 0x20, 0x73, 0x75, 0x69, 0x63, 0x69, 0x64,
        0x65, 0x73, 0x20, 0x77, 0x6f, 0x75, 0x6c, 0x64,
        0x20, 0x74, 0x61, 0x6b, 0x65, 0x20, 0x70, 0x6c,
        0x61, 0x63, 0x65, 0x20, 0x74, 0x68, 0x61, 0x74,
        0x20, 0x73, 0x61, 0x6d, 0x65, 0x20, 0x6e, 0x69,
        0x67, 0x68, 0x74, 0x2c, 0x20, 0x68, 0x6f, 0x77,
        0x20, 0x6d, 0x61, 0x6e, 0x79, 0x20, 0x70, 0x65,
        0x6f, 0x70, 0x6c, 0x65, 0x20, 0x77, 0x6f, 0x75,
        0x6c, 0x64, 0x20, 0x67, 0x6f, 0x20, 0x69, 0x6e,
        0x73, 0x61, 0x6e, 0x65, 0x3f, 0x20, 0x48, 0x6f,
        0x77, 0x20, 0x6d, 0x61, 0x6e, 0x79, 0x20, 0x63,
        0x6f, 0x63, 0x6b, 0x72, 0x6f, 0x61, 0x63, 0x68,
        0x65, 0x73, 0x20, 0x61, 0x6e, 0x64, 0x20, 0x6c,
        0x61, 0x6e, 0x64, 0x6c, 0x6f, 0x72, 0x64, 0x73,
        0x20, 0x77, 0x6f, 0x75, 0x6c, 0x64, 0x20, 0x74,
        0x72, 0x69, 0x75, 0x6d, 0x70, 0x68, 0x3f, 0x20,
        0x48, 0x6f, 0x77, 0x20, 0x6d, 0x61, 0x6e, 0x79,
        0x20, 0x77, 0x69, 0x6e, 0x6e, 0x65, 0x72, 0x73,
        0x20, 0x77, 0x65, 0x72, 0x65, 0x20, 0x6c, 0x6f,
        0x73, 0x65, 0x72, 0x73, 0x2c, 0x20, 0x73, 0x75
};
static const uint8_t DC6[] = {
        0xF3, 0xDD, 0xF0, 0x0B, 0xFF, 0xA2, 0x6A, 0x04,
        0xBE, 0xDA, 0x52, 0xA6, 0xFE, 0x6B, 0xA6, 0xA7,
        0x48, 0x1D, 0x7D, 0x98, 0x65, 0xDB, 0xEF, 0x06,
        0x26, 0xB5, 0x8E, 0xEB, 0x05, 0x0E, 0x77, 0x98,
        0x17, 0x8E, 0xD0, 0xD4, 0x7B, 0x92, 0x8F, 0x5C,
        0xD0, 0x74, 0x5C, 0xA8, 0x4B, 0x54, 0xB6, 0x2F,
        0x83, 0x72, 0x2C, 0xFF, 0x72, 0xE9, 0xE4, 0x15,
        0x4C, 0x32, 0xAF, 0xC8, 0xC9, 0x89, 0x3C, 0x6E,
        0x31, 0xD5, 0xC0, 0x16, 0xC0, 0x31, 0x7D, 0x11,
        0xAB, 0xCB, 0xDE, 0xD2, 0xD6, 0xAA, 0x76, 0x5E,
        0xBA, 0xF6, 0xE2, 0x92, 0xCB, 0x86, 0x07, 0xFA,
        0xD4, 0x9E, 0x83, 0xED, 0xFD, 0xB8, 0x70, 0x54,
        0x6B, 0xBE, 0xEC, 0x72, 0xDD, 0x28, 0x5E, 0x95,
        0x78, 0xA5, 0x28, 0x43, 0x3D, 0x6D, 0xB1, 0xD9,
        0x69, 0x1F, 0xC9, 0x66, 0x0E, 0x32, 0x44, 0x08,
        0xD2, 0xAE, 0x2C, 0x43, 0xF2, 0xD0, 0x7D, 0x26,
        0x70, 0xE5, 0xA1, 0xCA, 0x37, 0xE9, 0x7D, 0xC7,
        0xA3, 0xFA, 0x81, 0x91, 0x64, 0xAA, 0x64, 0x91,
        0x9A, 0x95, 0x2D, 0xC9, 0xF9, 0xCE, 0xFE, 0x9F,
        0xC4, 0xD8, 0x81, 0xBE, 0x57, 0x84, 0xC5, 0x02,
        0xDB, 0x30, 0xC1, 0xD9, 0x0E, 0xA0, 0xA6, 0x00,
        0xD6, 0xF3, 0x52, 0x7E, 0x0D, 0x23, 0x6B, 0x2B,
        0x34, 0x99, 0x1F, 0x70, 0x27, 0x6D, 0x58, 0x84,
        0x93, 0x77, 0xB8, 0x3E, 0xF1, 0x71, 0x58, 0x42,
        0x8B, 0x2B, 0xC8, 0x6D, 0x05, 0x84, 0xFF, 0x4E,
        0x85, 0xEF, 0x4A, 0x9D, 0x91, 0x6A, 0xD5, 0xE1,
        0xAF, 0x01, 0xEB, 0x83, 0x8F, 0x23, 0x7C, 0x7F,
        0x12, 0x91, 0x05, 0xF0, 0x4E, 0xD9, 0x17, 0x62,
        0x75, 0xBB, 0xAC, 0x97, 0xEE, 0x3B, 0x4E, 0xC7,
        0xE5, 0x92, 0xF8, 0x9D, 0x4C, 0xF9, 0xEE, 0x55,
        0x18, 0xBB, 0xCC, 0xB4, 0xF2, 0x59, 0xB9, 0xFC,
        0x7A, 0x0F, 0x98, 0xD4, 0x8B, 0xFE, 0xF7, 0x83,
        0x46, 0xE2, 0x83, 0x33, 0x3E, 0x95, 0x8D, 0x17,
        0x1E, 0x85, 0xF8, 0x8C, 0x51, 0xB0, 0x6C, 0xB5,
        0x5E, 0x95, 0xBA, 0x4B, 0x69, 0x1B, 0x48, 0x69,
        0x0B, 0x8F, 0xA5, 0x18, 0x13, 0xB9, 0x77, 0xD1,
        0x80, 0x32, 0x32, 0x6D, 0x53, 0xA1, 0x95, 0x40,
        0x96, 0x8A, 0xCC, 0xA3, 0x69, 0xF8, 0x9F, 0xB5,
        0x8E, 0xD2, 0x68, 0x07, 0x4F, 0xA7, 0xEC, 0xF8,
        0x20, 0x21, 0x58, 0xF8, 0xD8, 0x9E, 0x5F, 0x40,
        0xBA, 0xB9, 0x76, 0x57, 0x3B, 0x17, 0xAD, 0xEE,
        0xCB, 0xDF, 0x07, 0xC1, 0xDF, 0x66, 0xA8, 0x0D,
        0xC2, 0xCE, 0x8F, 0x79, 0xC3, 0x32, 0xE0, 0x8C,
        0xFE, 0x5A, 0xF3, 0x55, 0x27, 0x73, 0x6F, 0xA1,
        0x54, 0xC6, 0xFC, 0x28, 0x9D, 0xBE, 0x97, 0xB9,
        0x54, 0x97, 0x72, 0x3A, 0x61, 0xAF, 0x6F, 0xDE,
        0xF8, 0x0E, 0xBB, 0x6B, 0x96, 0x84, 0xDD, 0x9B,
        0x62, 0xBA, 0x47, 0xB5, 0xC9, 0x3B, 0x4E, 0x8C,
        0x78, 0x2A, 0xCC, 0x0A, 0x69, 0x54, 0x25, 0x5E,
        0x8B, 0xAC, 0x56, 0xD9, 0xFE, 0x48, 0xBA, 0xCE,
        0xA9, 0xCE, 0xA6, 0x1D, 0xBF, 0x3E, 0x3C, 0x66,
        0x40, 0x71, 0x79, 0xAD, 0x5B, 0x26, 0xAD, 0xBE,
        0x58, 0x13, 0x64, 0x60, 0x7C, 0x05, 0xFC, 0xE3,
        0x51, 0x7A, 0xF2, 0xCC, 0x54, 0x16, 0x2C, 0xA4,
        0xCE, 0x5F, 0x59, 0x12, 0x77, 0xEB, 0xD9, 0x23,
        0xE3, 0x86, 0xFB, 0xD7, 0x48, 0x76, 0x9D, 0xE3,
        0x89, 0x87, 0x39, 0xFA, 0x7B, 0x21, 0x0B, 0x76,
        0xB2, 0xED, 0x1C, 0x27, 0x4B, 0xD5, 0x27, 0x05,
        0x8C, 0x7D, 0x58, 0x6C, 0xCA, 0xA5, 0x54, 0x9A,
        0x0F, 0xCB, 0xE9, 0x88, 0x31, 0xAD, 0x49, 0xEE,
        0x38, 0xFB, 0xC9, 0xFB, 0xB4, 0x7A, 0x00, 0x58,
        0x20, 0x32, 0xD3, 0x53, 0x5A, 0xDD, 0x74, 0x95,
        0x60, 0x59, 0x09, 0xAE, 0x7E, 0xEC, 0x74, 0xA3,
        0xB7, 0x1C, 0x6D, 0xF2, 0xAE, 0x79, 0xA4, 0x7C
};

static const struct aes_vector docsis_vectors[] = {
        {DK1, DIV1, DP1, sizeof(DP1), DC1, sizeof(DK1)},
        {DK2, DIV2, DP2, sizeof(DP2), DC2, sizeof(DK2)},
        {DK3, DIV3, DP3, sizeof(DP3), DC3, sizeof(DK3)},
        {DK4, DIV4, DP4, sizeof(DP4), DC4, sizeof(DK4)},
        {DK5, DIV5, DP5, sizeof(DP5), DC5, sizeof(DK5)},
        {DK6, DIV6, DP6, sizeof(DP6), DC6, sizeof(DK6)},
};

/* =================================================================== */
/* =================================================================== */
/* DOCSIS with CRC: AES CBC + CFB + CRC32 */

/* 128-bit keys */
static const uint8_t DOCRC1_KEY[] = {
        0x00, 0x00, 0x00, 0x00, 0xaa, 0xbb, 0xcc, 0xdd,
        0xee, 0xff, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55
};
static const uint8_t DOCRC1_IV[] = {
        0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
        0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11
};
static const uint8_t DOCRC1_PT[] = {
        /* DOCSIS Header (6 bytes) */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        /* PDU without CRC (14 bytes) */
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
        0x06, 0x05, 0x04, 0x03, 0x02, 0x01,
        0x08, 0x00,
        /* CRC (4 bytes) */
        0x14, 0x08, 0xe8, 0x55
};
static const uint8_t DOCRC1_CT[] = {
        /* DOCSIS Header (6 bytes) */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        /* PDU DA / SA (12 bytes) */
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x06, 0x05,
        0x04, 0x03, 0x02, 0x01,
        /* PDU Type/Len (encrypted) */
        0x7A, 0xF0,
        /* CRC (encrypted) */
        0x61, 0xF8, 0x63, 0x42
};
#define DOCRC1_KEY_LEN       16
#define DOCRC1_HASH_OFFSET   6
#define DOCRC1_HASH_LENGTH   14
#define DOCRC1_CIPHER_OFFSET 18
#define DOCRC1_CIPHER_LENGTH 6
#define DOCRC1_CRC           0x55e80814 /* LE */
#define DOCRC1_FRAME_LEN     DIM(DOCRC1_PT)

static const uint8_t DOCRC2_PT[] = {
        /* DOCSIS Header (6 bytes) */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        /* PDU DA / SA (12 bytes) */
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
        0x06, 0x05, 0x04, 0x03, 0x02, 0x01,
        /* PDU Type/Len */
        0x08, 0x00,
        /* PDU payload */
        0xAA,
        /* CRC (4 bytes) */
        0x0E, 0x99, 0x8E, 0xFE
};
static const uint8_t DOCRC2_CT[] = {
        /* DOCSIS Header (6 bytes) */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        /* PDU DA / SA (12 bytes) */
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
        0x06, 0x05, 0x04, 0x03, 0x02, 0x01,
        /* PDU Type/Len (encrypted) */
        0x7A, 0xF0,
        /* PDU payload */
        0xDF,
        /* CRC (encrypted) */
        0xFE, 0x12, 0x99, 0xE5
};
#define DOCRC2_KEY           DOCRC1_KEY
#define DOCRC2_KEY_LEN       DOCRC1_KEY_LEN
#define DOCRC2_IV            DOCRC1_IV
#define DOCRC2_HASH_OFFSET   6
#define DOCRC2_HASH_LENGTH   15
#define DOCRC2_CIPHER_OFFSET 18
#define DOCRC2_CIPHER_LENGTH 7
#define DOCRC2_CRC           0xFE8E990E
#define DOCRC2_FRAME_LEN     DIM(DOCRC2_PT)

static const uint8_t DOCRC3_PT[] = {
        /* DOCSIS Header (6 bytes) */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        /* PDU DA / SA (12 bytes) */
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
        0x06, 0x05, 0x04, 0x03, 0x02, 0x01,
        /* PDU Type/Len */
        0x08, 0x00,
        /* PDU payload */
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA,
        /* CRC (4 bytes) */
        0xCB, 0x7C, 0xAB, 0x56
};
static const uint8_t DOCRC3_CT[] = {
        /* DOCSIS Header (6 bytes) */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        /* PDU DA / SA (12 bytes) */
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
        0x06, 0x05, 0x04, 0x03, 0x02, 0x01,
        /* PDU Type/Len (encrypted) */
        0xD6, 0xE2,
        /* PDU payload */
        0x70, 0x5C, 0xE6, 0x4D, 0xCC, 0x8C, 0x47, 0xB7,
        0x09, 0xD6,
        /* CRC (encrypted) */
        0x54, 0x85, 0xF8, 0x32
};
#define DOCRC3_KEY           DOCRC1_KEY
#define DOCRC3_KEY_LEN       DOCRC1_KEY_LEN
#define DOCRC3_IV            DOCRC1_IV
#define DOCRC3_HASH_OFFSET   6
#define DOCRC3_HASH_LENGTH   24
#define DOCRC3_CIPHER_OFFSET 18
#define DOCRC3_CIPHER_LENGTH 16
#define DOCRC3_CRC           0x56AB7CCB
#define DOCRC3_FRAME_LEN     DIM(DOCRC3_PT)

static const uint8_t DOCRC4_PT[] = {
        /* DOCSIS Header (6 bytes) */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        /* PDU DA / SA (12 bytes) */
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
        0x06, 0x05, 0x04, 0x03, 0x02, 0x01,
        /* PDU Type/Len */
        0x08, 0x00,
        /* PDU payload */
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA, 0xAA,
        /* CRC (4 bytes) */
        0x3F, 0x15, 0xE1, 0xE8
};
static const uint8_t DOCRC4_CT[] = {
        /* DOCSIS Header (6 bytes) */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        /* PDU DA / SA (12 bytes) */
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
        0x06, 0x05, 0x04, 0x03, 0x02, 0x01,
        /* PDU Type/Len (encrypted) */
        0x92, 0x6A,
        /* PDU payload */
        0xC2, 0xDC, 0xEE, 0x3B, 0x31, 0xEC, 0x03, 0xDE,
        0x95, 0x33, 0x5E,
        /* CRC (encrypted) */
        0xFE, 0x47, 0x3E, 0x22
};
#define DOCRC4_KEY           DOCRC1_KEY
#define DOCRC4_KEY_LEN       DOCRC1_KEY_LEN
#define DOCRC4_IV            DOCRC1_IV
#define DOCRC4_HASH_OFFSET   6
#define DOCRC4_HASH_LENGTH   25
#define DOCRC4_CIPHER_OFFSET 18
#define DOCRC4_CIPHER_LENGTH 17
#define DOCRC4_CRC           0xE8E1153F
#define DOCRC4_FRAME_LEN     DIM(DOCRC4_PT)

static const uint8_t DOCRC5_PT[] = {
        /* DOCSIS Header (6 bytes) */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        /* PDU DA / SA (12 bytes) */
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
        0x06, 0x05, 0x04, 0x03, 0x02, 0x01,
        /* PDU Type/Len */
        0x08, 0x00,
        /* PDU payload */
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA,
        /* CRC (4 bytes) */
        0x2E, 0x07, 0xC8, 0x3C
};
static const uint8_t DOCRC5_CT[] = {
        /* DOCSIS Header (6 bytes) */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        /* PDU DA / SA (12 bytes) */
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
        0x06, 0x05, 0x04, 0x03, 0x02, 0x01,
        /* PDU Type/Len (encrypted) */
        0x77, 0x74,
        /* PDU payload */
        0x56, 0x05, 0xD1, 0x14, 0xA2, 0x8D, 0x2C, 0x9A,
        0x11, 0xFC, 0x7D, 0xB0, 0xE7, 0x18, 0xCE, 0x75,
        0x7C, 0x89, 0x14, 0x56, 0xE2, 0xF2, 0xB7, 0x47,
        0x08, 0x27, 0xF7, 0x08, 0x7A, 0x13, 0x90, 0x81,
        0x75, 0xB0, 0xC7, 0x91, 0x04, 0x83, 0xAD, 0x11,
        0x46, 0x46, 0xF8, 0x54, 0x87, 0xA0, 0x42, 0xF3,
        0x71, 0xA9, 0x8A, 0xCD, 0x59, 0x77, 0x67, 0x11,
        0x1A, 0x87,
        /* CRC (encrypted) */
        0xAB, 0xED, 0x2C, 0x26
};
#define DOCRC5_KEY           DOCRC1_KEY
#define DOCRC5_KEY_LEN       DOCRC1_KEY_LEN
#define DOCRC5_IV            DOCRC1_IV
#define DOCRC5_HASH_OFFSET   6
#define DOCRC5_HASH_LENGTH   72
#define DOCRC5_CIPHER_OFFSET 18
#define DOCRC5_CIPHER_LENGTH 64
#define DOCRC5_CRC           0x3CC8072E
#define DOCRC5_FRAME_LEN     DIM(DOCRC5_PT)

static const uint8_t DOCRC6_PT[] = {
        /* DOCSIS Header (6 bytes) */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        /* PDU DA / SA (12 bytes) */
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
        0x06, 0x05, 0x04, 0x03, 0x02, 0x01,
        /* PDU Type/Len */
        0x08, 0x00,
        /* PDU payload */
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA, 0xAA,
        /* CRC (4 bytes) */
        0xB3, 0x60, 0xEB, 0x38
};
static const uint8_t DOCRC6_CT[] = {
        /* DOCSIS Header (6 bytes) */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        /* PDU DA / SA (12 bytes) */
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
        0x06, 0x05, 0x04, 0x03, 0x02, 0x01,
        /* PDU Type/Len (encrypted) */
        0x77, 0x74,
        /* PDU payload */
        0x56, 0x05, 0xD1, 0x14, 0xA2, 0x8D, 0x2C, 0x9A,
        0x11, 0xFC, 0x7D, 0xB0, 0xE7, 0x18, 0xCE, 0x75,
        0x7C, 0x89, 0x14, 0x56, 0xE2, 0xF2, 0xB7, 0x47,
        0x08, 0x27, 0xF7, 0x08, 0x7A, 0x13, 0x90, 0x81,
        0x75, 0xB0, 0xC7, 0x91, 0x04, 0x83, 0xAD, 0x11,
        0x46, 0x46, 0xF8, 0x54, 0x87, 0xA0, 0xA4, 0x0C,
        0xC2, 0xF0, 0x81, 0x49, 0xA8, 0xA6, 0x6C, 0x48,
        0xEB, 0x1F, 0x4B,
        /* CRC (encrypted) */
        0x2F, 0xD4, 0x48, 0x18
};
#define DOCRC6_KEY           DOCRC1_KEY
#define DOCRC6_KEY_LEN       DOCRC1_KEY_LEN
#define DOCRC6_IV            DOCRC1_IV
#define DOCRC6_HASH_OFFSET   6
#define DOCRC6_HASH_LENGTH   73
#define DOCRC6_CIPHER_OFFSET 18
#define DOCRC6_CIPHER_LENGTH 65
#define DOCRC6_CRC           0x38EB60B3
#define DOCRC6_FRAME_LEN     DIM(DOCRC6_PT)

static const uint8_t DOCRC7_PT[] = {
        /* DOCSIS Header (6 bytes) */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        /* PDU DA / SA (12 bytes) */
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
        0x06, 0x05, 0x04, 0x03, 0x02, 0x01,
        /* PDU Type/Len */
        0x08, 0x00,
        /* PDU payload */
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA, 0xAA,
        /* CRC (4 bytes) */
        0xB3, 0x60, 0xEB, 0x38
};
static const uint8_t DOCRC7_CT[] = {
        /* DOCSIS Header (6 bytes) */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        /* PDU DA / SA (12 bytes) */
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
        0x06, 0x05, 0x04, 0x03, 0x02, 0x01,
        /* PDU Type/Len (encrypted) */
        0x08, 0x00,
        /* PDU payload */
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA, 0xAA, 0xAA, 0x3B, 0x9F, 0x72, 0x4C,
        0xB5, 0x72, 0x3E, 0x56, 0x54, 0x49, 0x13, 0x53,
        0xC4, 0xAA, 0xCD, 0xEA, 0x6A, 0x88, 0x99, 0x07,
        0x86, 0xF4, 0xCF, 0x03, 0x4E, 0xDF, 0x65, 0x61,
        0x47, 0x5B, 0x2F, 0x81, 0x09, 0x12, 0x9A, 0xC2,
        0x24, 0x8C, 0x09,
        /* CRC (encrypted) */
        0x11, 0xB4, 0x06, 0x33
};
#define DOCRC7_KEY           DOCRC1_KEY
#define DOCRC7_KEY_LEN       DOCRC1_KEY_LEN
#define DOCRC7_IV            DOCRC1_IV
#define DOCRC7_HASH_OFFSET   6
#define DOCRC7_HASH_LENGTH   73
#define DOCRC7_CIPHER_OFFSET 40
#define DOCRC7_CIPHER_LENGTH 43
#define DOCRC7_CRC           0x38EB60B3
#define DOCRC7_FRAME_LEN     DIM(DOCRC7_PT)

/* no cipher / CRC only */
static const uint8_t DOCRC8_PT[] = {
        /* DOCSIS Header (6 bytes) */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        /* PDU DA / SA (12 bytes) */
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
        0x06, 0x05, 0x04, 0x03, 0x02, 0x01,
        /* PDU Type/Len */
        0x08, 0x00,
        /* PDU payload */
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA,
        /* CRC (4 bytes) */
        0xCB, 0x7C, 0xAB, 0x56
};
static const uint8_t DOCRC8_CT[] = {
        /* DOCSIS Header (6 bytes) */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        /* PDU DA / SA (12 bytes) */
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
        0x06, 0x05, 0x04, 0x03, 0x02, 0x01,
        /* PDU Type/Len */
        0x08, 0x00,
        /* PDU payload */
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA,
        /* CRC (4 bytes) */
        0xCB, 0x7C, 0xAB, 0x56
};
#define DOCRC8_KEY           DOCRC1_KEY
#define DOCRC8_KEY_LEN       DOCRC1_KEY_LEN
#define DOCRC8_IV            DOCRC1_IV
#define DOCRC8_HASH_OFFSET   6
#define DOCRC8_HASH_LENGTH   24
#define DOCRC8_CIPHER_OFFSET 18
#define DOCRC8_CIPHER_LENGTH 0
#define DOCRC8_CRC           0x56AB7CCB
#define DOCRC8_FRAME_LEN     DIM(DOCRC8_PT)

/* cipher only / no CRC */
static const uint8_t DOCRC9_PT[] = {
        /* DOCSIS Header (6 bytes) */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        /* PDU DA / SA (12 bytes) */
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
        0x06, 0x05, 0x04, 0x03, 0x02, 0x01,
        /* PDU Type/Len */
        0x08, 0x00,
        /* PDU payload */
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA,
        /* instead of CRC (4 bytes) */
        0xAA, 0xAA, 0xAA, 0xAA
};
static const uint8_t DOCRC9_CT[] = {
        /* DOCSIS Header (6 bytes) */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        /* DA/ SA */
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
        0x06, 0x05, 0x04, 0x03, 0x02, 0x01,
        /* PDU Type/Len (encrypted) */
        0x77, 0x74,
        /* PDU payload */
        0x56, 0x05, 0xD1, 0x14, 0xA2, 0x8D, 0x2C, 0x9A,
        0x11, 0xFC,
        /* instead of CRC (4 bytes) */
        0x7D, 0xB0, 0xE7, 0x18
};
#define DOCRC9_KEY           DOCRC1_KEY
#define DOCRC9_KEY_LEN       DOCRC1_KEY_LEN
#define DOCRC9_IV            DOCRC1_IV
#define DOCRC9_HASH_OFFSET   6
#define DOCRC9_HASH_LENGTH   0
#define DOCRC9_CIPHER_OFFSET 18
#define DOCRC9_CIPHER_LENGTH 16
#define DOCRC9_CRC           0x0
#define DOCRC9_FRAME_LEN     DIM(DOCRC9_PT)

/* no cipher / no CRC */
static const uint8_t DOCRC10_PT[] = {
        /* DOCSIS Header (6 bytes) */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        /* PDU DA / SA (12 bytes) */
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
        0x06, 0x05, 0x04, 0x03, 0x02, 0x01,
        /* PDU Type/Len */
        0x08, 0x00,
        /* PDU payload */
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA,
        /* CRC (4 bytes) */
        0xFF, 0xFF, 0xFF, 0x55
};
static const uint8_t DOCRC10_CT[] = {
        /* DOCSIS Header (6 bytes) */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        /* PDU DA / SA (12 bytes) */
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
        0x06, 0x05, 0x04, 0x03, 0x02, 0x01,
        /* PDU Type/Len */
        0x08, 0x00,
        /* PDU payload */
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA,
        /* CRC (4 bytes) */
        0xFF, 0xFF, 0xFF, 0x55
};
#define DOCRC10_KEY           DOCRC1_KEY
#define DOCRC10_KEY_LEN       DOCRC1_KEY_LEN
#define DOCRC10_IV            DOCRC1_IV
#define DOCRC10_HASH_OFFSET   6
#define DOCRC10_HASH_LENGTH   0
#define DOCRC10_CIPHER_OFFSET 18
#define DOCRC10_CIPHER_LENGTH 0
#define DOCRC10_CRC           0x0
#define DOCRC10_FRAME_LEN     DIM(DOCRC10_PT)

static const uint8_t DOCRC18_KEY[] = {
        0x00, 0x00, 0x00, 0x00, 0xaa, 0xbb, 0xcc, 0xdd,
        0xee, 0xff, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55
};
static const uint8_t DOCRC18_IV[] = {
        0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
        0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11
};
static const uint8_t DOCRC18_PT[] = {
        /* DOCSIS Header (6 bytes) */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        /* PDU without CRC (14 bytes) */
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
        0x06, 0x05, 0x04, 0x03, 0x02, 0x01,
        0x08, 0x00,
        /* PDU payload */
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xFF,
        0xFF, 0xFF, 0xFF
};
static const uint8_t DOCRC18_CT[] = {
        /* DOCSIS Header (6 bytes) */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        /* PDU DA / SA (12 bytes) */
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
        0x06, 0x05, 0x04, 0x03, 0x02, 0x01,
        /* PDU payload */
        0x08, 0x00, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA, 0xAA, 0xAA,
        0x3B, 0x9F, 0x72, 0x4C, 0xB5, 0x72,
        0x3E, 0x56, 0x54, 0x49, 0x13, 0x53,
        0xC4, 0xAA, 0xCD, 0xEA, 0x6A, 0x88,
        0x99, 0x07, 0x86, 0xF4, 0xCF, 0x03,
        0x4E, 0xDF, 0x65, 0x61, 0x47, 0x5B,
        0x2F, 0x81, 0x09, 0x12, 0x9A, 0xC2,
        0x24, 0x8C, 0x09, 0x5D, 0x2B, 0x12,
        0xF4
};
#define DOCRC18_KEY_LEN       16
#define DOCRC18_HASH_OFFSET   6
#define DOCRC18_HASH_LENGTH   0
#define DOCRC18_CIPHER_OFFSET 40
#define DOCRC18_CIPHER_LENGTH 43
#define DOCRC18_CRC           0x0
#define DOCRC18_FRAME_LEN     DIM(DOCRC18_PT)

/* 256-bit keys */
static const uint8_t DOCRC11_KEY[] = {
        0x00, 0x00, 0x00, 0x00, 0xAA, 0xBB, 0xCC, 0xDD,
        0xEE, 0xFF, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55,
        0x00, 0x00, 0x00, 0x00, 0xAA, 0xBB, 0xCC, 0xDD,
        0xEE, 0xFF, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55
};
static const uint8_t DOCRC11_IV[] = {
        0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
        0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11
};
static const uint8_t DOCRC11_PT[] = {
        /* DOCSIS Header (6 bytes) */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        /* PDU without CRC (14 bytes) */
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
        0x06, 0x05, 0x04, 0x03, 0x02, 0x01,
        0x08, 0x00,
        /* CRC (4 bytes) */
        0x14, 0x08, 0xe8, 0x55
};
static const uint8_t DOCRC11_CT[] = {
        /* DOCSIS Header (6 bytes) */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        /* PDU DA / SA (12 bytes) */
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
        0x06, 0x05, 0x04, 0x03, 0x02, 0x01,
        /* PDU Type/Len (encrypted) */
        0x08, 0x46,
        /* CRC (encrypted) */
        0x98, 0x98, 0x47, 0x76
};

#define DOCRC11_KEY_LEN       32
#define DOCRC11_HASH_OFFSET   6
#define DOCRC11_HASH_LENGTH   14
#define DOCRC11_CIPHER_OFFSET 18
#define DOCRC11_CIPHER_LENGTH 6
#define DOCRC11_CRC           0x55e80814 /* LE */
#define DOCRC11_FRAME_LEN     DIM(DOCRC11_PT)

static const uint8_t DOCRC12_PT[] = {
        /* DOCSIS Header (6 bytes) */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        /* PDU without CRC (15 bytes) */
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
        0x06, 0x05, 0x04, 0x03, 0x02, 0x01,
        0x08, 0x00, 0xAA,
        /* CRC (4 bytes) */
        0x0E, 0x99, 0x8E, 0xFE
};
static const uint8_t DOCRC12_CT[] = {
        /* DOCSIS Header (6 bytes) */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        /* PDU DA / SA (12 bytes) */
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
        0x06, 0x05, 0x04, 0x03, 0x02, 0x01,
        /* PDU Type/Len (encrypted) */
        0x08, 0x46, 0x26,
        /* CRC (encrypted) */
        0x9E, 0x36, 0xAD, 0xC7
};

#define DOCRC12_KEY           DOCRC11_KEY
#define DOCRC12_KEY_LEN       DOCRC11_KEY_LEN
#define DOCRC12_IV            DOCRC11_IV
#define DOCRC12_HASH_OFFSET   6
#define DOCRC12_HASH_LENGTH   15
#define DOCRC12_CIPHER_OFFSET 18
#define DOCRC12_CIPHER_LENGTH 7
#define DOCRC12_CRC           0xFE8E990E /* LE */
#define DOCRC12_FRAME_LEN     DIM(DOCRC12_PT)

static const uint8_t DOCRC13_PT[] = {
        /* DOCSIS Header (6 bytes) */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        /* PDU without CRC (24 bytes) */
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
        0x06, 0x05, 0x04, 0x03, 0x02, 0x01,
        0x08, 0x00, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        /* CRC (4 bytes) */
        0xCB, 0x7C, 0xAB, 0x56
};
static const uint8_t DOCRC13_CT[] = {
        /* DOCSIS Header (6 bytes) */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        /* PDU DA / SA (12 bytes) */
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
        0x06, 0x05, 0x04, 0x03, 0x02, 0x01,
        /* PDU Type/Len (encrypted) */
        0xA1, 0xFF, 0x45, 0x21, 0x43, 0xC7,
        0x5B, 0x3C, 0xAB, 0x7A, 0x80, 0xC9,
        /* CRC (encrypted) */
        0x70, 0x51, 0x54, 0xD0
};

#define DOCRC13_KEY           DOCRC11_KEY
#define DOCRC13_KEY_LEN       DOCRC11_KEY_LEN
#define DOCRC13_IV            DOCRC11_IV
#define DOCRC13_HASH_OFFSET   6
#define DOCRC13_HASH_LENGTH   24
#define DOCRC13_CIPHER_OFFSET 18
#define DOCRC13_CIPHER_LENGTH 16
#define DOCRC13_CRC           0x56AB7CCB /* LE */
#define DOCRC13_FRAME_LEN     DIM(DOCRC13_PT)

static const uint8_t DOCRC14_PT[] = {
        /* DOCSIS Header (6 bytes) */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        /* PDU without CRC (24 bytes) */
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
        0x06, 0x05, 0x04, 0x03, 0x02, 0x01,
        0x08, 0x00, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA,
        /* CRC (4 bytes) */
        0x3F, 0x15, 0xE1, 0xE8
};
static const uint8_t DOCRC14_CT[] = {
        /* DOCSIS Header (6 bytes) */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        /* PDU DA / SA (12 bytes) */
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
        0x06, 0x05, 0x04, 0x03, 0x02, 0x01,
        /* PDU Type/Len (encrypted) */
        0xA0, 0x88, 0x19, 0x50, 0x3B, 0x70,
        0x91, 0x3C, 0x2A, 0x46, 0x74, 0x45,
        0x03,
        /* CRC (encrypted) */
        0x30, 0xC8, 0xA9, 0xCC
};

#define DOCRC14_KEY           DOCRC11_KEY
#define DOCRC14_KEY_LEN       DOCRC11_KEY_LEN
#define DOCRC14_IV            DOCRC11_IV
#define DOCRC14_HASH_OFFSET   6
#define DOCRC14_HASH_LENGTH   25
#define DOCRC14_CIPHER_OFFSET 18
#define DOCRC14_CIPHER_LENGTH 17
#define DOCRC14_CRC           0xE8E1153F /* LE */
#define DOCRC14_FRAME_LEN     DIM(DOCRC14_PT)

static const uint8_t DOCRC15_PT[] = {
        /* DOCSIS Header (6 bytes) */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        /* PDU without CRC (72 bytes) */
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
        0x06, 0x05, 0x04, 0x03, 0x02, 0x01,
        0x08, 0x00, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        /* CRC (4 bytes) */
        0x2E, 0x07, 0xC8, 0x3C
};

static const uint8_t DOCRC15_CT[] = {
        /* DOCSIS Header (6 bytes) */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        /* PDU DA / SA (12 bytes) */
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
        0x06, 0x05, 0x04, 0x03, 0x02, 0x01,
        /* PDU Type/Len (encrypted) */
        0x5D, 0xB3, 0xD1, 0xF4, 0x6C, 0x65,
        0xCA, 0x3B, 0x5C, 0x16, 0xB3, 0xA4,
        0x23, 0x74, 0x81, 0x5E, 0x12, 0x03,
        0x7B, 0x3F, 0xBB, 0x62, 0x1D, 0x29,
        0x66, 0x60, 0x1B, 0x6E, 0x01, 0xFE,
        0x6F, 0x40, 0x12, 0xE6, 0x20, 0xE6,
        0x10, 0xBE, 0x5B, 0xF2, 0x7E, 0x7F,
        0x43, 0x53, 0x66, 0x38, 0xA6, 0x4D,
        0xF3, 0x66, 0x84, 0x9F, 0xE2, 0xEC,
        0x9F, 0xBC, 0xD4, 0x38, 0xDB, 0x33,
        /* CRC (encrypted) */
        0x4E, 0x12, 0xB3, 0xB4
};

#define DOCRC15_KEY           DOCRC11_KEY
#define DOCRC15_KEY_LEN       DOCRC11_KEY_LEN
#define DOCRC15_IV            DOCRC11_IV
#define DOCRC15_HASH_OFFSET   6
#define DOCRC15_HASH_LENGTH   72
#define DOCRC15_CIPHER_OFFSET 18
#define DOCRC15_CIPHER_LENGTH 64
#define DOCRC15_CRC           0x3CC8072E /* LE */
#define DOCRC15_FRAME_LEN     DIM(DOCRC15_PT)

static const uint8_t DOCRC16_PT[] = {
        /* DOCSIS Header (6 bytes) */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        /* PDU without CRC (72 bytes) */
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
        0x06, 0x05, 0x04, 0x03, 0x02, 0x01,
        0x08, 0x00, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA,
        /* CRC (4 bytes) */
        0xB3, 0x60, 0xEB, 0x38
};

static const uint8_t DOCRC16_CT[] = {
        /* DOCSIS Header (6 bytes) */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        /* PDU DA / SA (12 bytes) */
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
        0x06, 0x05, 0x04, 0x03, 0x02, 0x01,
        /* PDU Type/Len (encrypted) */
        0x5D, 0xB3, 0xD1, 0xF4, 0x6C, 0x65,
        0xCA, 0x3B, 0x5C, 0x16, 0xB3, 0xA4,
        0x23, 0x74, 0x81, 0x5E, 0x12, 0x03,
        0x7B, 0x3F, 0xBB, 0x62, 0x1D, 0x29,
        0x66, 0x60, 0x1B, 0x6E, 0x01, 0xFE,
        0x6F, 0x40, 0x12, 0xE6, 0x20, 0xE6,
        0x10, 0xBE, 0x5B, 0xF2, 0x7E, 0x7F,
        0x43, 0x53, 0x66, 0x38, 0xA6, 0x4D,
        0xD9, 0xA6, 0x6A, 0x07, 0x6B, 0xAA,
        0x5C, 0xF6, 0xB4, 0x1D, 0xC5, 0x9A,
        0x7C,
        /* CRC (encrypted) */
        0x48, 0xDB, 0xB1, 0x74
};

#define DOCRC16_KEY           DOCRC11_KEY
#define DOCRC16_KEY_LEN       DOCRC11_KEY_LEN
#define DOCRC16_IV            DOCRC11_IV
#define DOCRC16_HASH_OFFSET   6
#define DOCRC16_HASH_LENGTH   73
#define DOCRC16_CIPHER_OFFSET 18
#define DOCRC16_CIPHER_LENGTH 65
#define DOCRC16_CRC           0x38EB60B3 /* LE */
#define DOCRC16_FRAME_LEN     DIM(DOCRC16_PT)

static const uint8_t DOCRC17_PT[] = {
        /* DOCSIS Header (6 bytes) */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        /* PDU without CRC (73 bytes) */
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
        0x06, 0x05, 0x04, 0x03, 0x02, 0x01,
        0x08, 0x00, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA,
        /* CRC (4 bytes) */
        0xB3, 0x60, 0xEB, 0x38
};

static const uint8_t DOCRC17_CT[] = {
        /* DOCSIS Header (6 bytes) */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        /* PDU DA / SA + Extra data (34 bytes) */
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
        0x06, 0x05, 0x04, 0x03, 0x02, 0x01,
        0x08, 0x00, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xAA, 0xAA, 0xAA,
        /* PDU Type/Len (encrypted) */
        0x96, 0x03, 0x94, 0x9B, 0xDA, 0x96,
        0x00, 0x42, 0x7B, 0x52, 0xD3, 0xB9,
        0xA4, 0x10, 0x7B, 0x87, 0x0B, 0xBA,
        0x41, 0x0E, 0x2B, 0x8F, 0xA6, 0xA3,
        0xF5, 0x55, 0x9C, 0x0C, 0x69, 0x7C,
        0x36, 0xD2, 0xBF, 0xA1, 0xF2, 0x2B,
        0xAB, 0x1A, 0x92,
        /* CRC (encrypted) */
        0xAF, 0x19, 0x88, 0xDD
};

#define DOCRC17_KEY           DOCRC11_KEY
#define DOCRC17_KEY_LEN       DOCRC11_KEY_LEN
#define DOCRC17_IV            DOCRC11_IV
#define DOCRC17_HASH_OFFSET   6
#define DOCRC17_HASH_LENGTH   73
#define DOCRC17_CIPHER_OFFSET 40
#define DOCRC17_CIPHER_LENGTH 43
#define DOCRC17_CRC           0x38EB60B3 /* LE */
#define DOCRC17_FRAME_LEN     DIM(DOCRC17_PT)


#define MK_DOCRC_VEC(_n)                                                \
        { _n ## _FRAME_LEN, _n ## _KEY, _n ## _KEY_LEN, _n ## _IV,      \
                        _n ## _PT, _n ## _CT,                           \
                        _n ## _HASH_OFFSET, _n ## _HASH_LENGTH,         \
                        _n ## _CIPHER_OFFSET, _n ## _CIPHER_LENGTH,     \
                        _n ## _CRC }

struct docsis_crc_vector {
        uint64_t frame_len;
        const uint8_t *key;
        uint64_t key_len;
        const uint8_t *iv;
        const uint8_t *pt;
        const uint8_t *ct;
        uint64_t hash_offset;
        uint64_t hash_length;
        uint64_t cipher_offset;
        uint64_t cipher_length;
        uint32_t crc_hash;
};

struct docsis_crc_vector docsis_crc_tab[] = {
        /* 128-bit key */
        MK_DOCRC_VEC(DOCRC1),
        MK_DOCRC_VEC(DOCRC2),
        MK_DOCRC_VEC(DOCRC3),
        MK_DOCRC_VEC(DOCRC4),
        MK_DOCRC_VEC(DOCRC5),
        MK_DOCRC_VEC(DOCRC6),
        MK_DOCRC_VEC(DOCRC7),
        MK_DOCRC_VEC(DOCRC8),
        MK_DOCRC_VEC(DOCRC9),
        MK_DOCRC_VEC(DOCRC10),
        MK_DOCRC_VEC(DOCRC18),
        /* 256-bit key */
        MK_DOCRC_VEC(DOCRC11),
        MK_DOCRC_VEC(DOCRC12),
        MK_DOCRC_VEC(DOCRC13),
        MK_DOCRC_VEC(DOCRC14),
        MK_DOCRC_VEC(DOCRC15),
        MK_DOCRC_VEC(DOCRC16),
        MK_DOCRC_VEC(DOCRC17),
};

static int
aes_job_ok(const struct IMB_JOB *job,
           const uint8_t *out_text,
           const uint8_t *target,
           const uint8_t *padding,
           const size_t sizeof_padding,
           const unsigned text_len)
{
        const int num = (const int)((uint64_t)job->user_data2);

        if (job->status != IMB_STATUS_COMPLETED) {
                printf("%d error status:%d, job %d",
                       __LINE__, job->status, num);
                return 0;
        }
        if (memcmp(out_text, target + sizeof_padding,
                   text_len)) {
                printf("%d mismatched\n", num);
                return 0;
        }
        if (memcmp(padding, target, sizeof_padding)) {
                printf("%d overwrite head\n", num);
                return 0;
        }
        if (memcmp(padding,
                   target + sizeof_padding + text_len,
                   sizeof_padding)) {
                printf("%d overwrite tail\n", num);
                return 0;
        }
        return 1;
}

static int
test_aes_many(struct IMB_MGR *mb_mgr,
              void *enc_keys,
              void *dec_keys,
              const void *iv,
              const uint8_t *in_text,
              const uint8_t *out_text,
              const unsigned text_len,
              const int dir,
              const int order,
              const IMB_CIPHER_MODE cipher,
              const int in_place,
              const int key_len,
              const int num_jobs)
{
        struct IMB_JOB *job;
        uint8_t padding[16];
        uint8_t **targets = malloc(num_jobs * sizeof(void *));
        int i, err, jobs_rx = 0, ret = -1;

        if (targets == NULL)
                goto end_alloc;

        memset(targets, 0, num_jobs * sizeof(void *));
        memset(padding, -1, sizeof(padding));

        for (i = 0; i < num_jobs; i++) {
                targets[i] = malloc(text_len + (sizeof(padding) * 2));
                if (targets[i] == NULL)
                        goto end_alloc;
                memset(targets[i], -1, text_len + (sizeof(padding) * 2));
                if (in_place) {
                        /* copy input text to the allocated buffer */
                        memcpy(targets[i] + sizeof(padding), in_text, text_len);
                }
        }

        /* flush the scheduler */
        while (IMB_FLUSH_JOB(mb_mgr) != NULL)
                ;

        for (i = 0; i < num_jobs; i++) {
                job = IMB_GET_NEXT_JOB(mb_mgr);
                job->cipher_direction = dir;
                job->chain_order = order;
                if (!in_place) {
                        job->dst = targets[i] + sizeof(padding);
                        job->src = in_text;
                } else {
                        job->dst = targets[i] + sizeof(padding);
                        job->src = targets[i] + sizeof(padding);
                }
                job->cipher_mode = cipher;
                job->enc_keys = enc_keys;
                job->dec_keys = dec_keys;
                job->key_len_in_bytes = key_len;

                job->iv = iv;
                job->iv_len_in_bytes = 16;
                job->cipher_start_src_offset_in_bytes = 0;
                job->msg_len_to_cipher_in_bytes = text_len;
                job->user_data = targets[i];
                job->user_data2 = (void *)((uint64_t)i);

                job->hash_alg = IMB_AUTH_NULL;

                job = IMB_SUBMIT_JOB(mb_mgr);
                if (job == NULL) {
                        /* no job returned - check for error */
                        err = imb_get_errno(mb_mgr);
                        if (err != 0) {
                                printf("Error: %s!\n", imb_get_strerror(err));
                                goto end;
                        }
                } else {
                        /* got job back */
                        jobs_rx++;
                        if (!aes_job_ok(job, out_text, job->user_data, padding,
                                       sizeof(padding), text_len))
                                goto end;
                }
        }

        while ((job = IMB_FLUSH_JOB(mb_mgr)) != NULL) {
                err = imb_get_errno(mb_mgr);
                if (err != 0) {
                        printf("Error: %s!\n", imb_get_strerror(err));
                        goto end;
                }

                jobs_rx++;
                if (!aes_job_ok(job, out_text, job->user_data, padding,
                               sizeof(padding), text_len))
                        goto end;
        }

        if (jobs_rx != num_jobs) {
                printf("Expected %d jobs, received %d\n", num_jobs, jobs_rx);
                goto end;
        }
        ret = 0;

 end:
        while (IMB_FLUSH_JOB(mb_mgr) != NULL) {
                err = imb_get_errno(mb_mgr);
                if (err != 0) {
                        printf("Error: %s!\n", imb_get_strerror(err));
                        goto end;
                }
        }

end_alloc:
        if (targets != NULL) {
                for (i = 0; i < num_jobs; i++)
                        free(targets[i]);
                free(targets);
        }

        return ret;
}

static int
test_aes_many_burst(struct IMB_MGR *mb_mgr,
                    void *enc_keys,
                    void *dec_keys,
                    const void *iv,
                    const uint8_t *in_text,
                    const uint8_t *out_text,
                    const unsigned text_len,
                    const int dir,
                    const int order,
                    const IMB_CIPHER_MODE cipher,
                    const int in_place,
                    const int key_len,
                    const int num_jobs)
{
        struct IMB_JOB *job, *jobs[MAX_BURST_JOBS] = {NULL};
        uint8_t padding[16];
        uint8_t **targets = malloc(num_jobs * sizeof(void *));
        int i, completed_jobs, jobs_rx = 0, ret = -1;

        if (targets == NULL)
                goto end_alloc;

        memset(targets, 0, num_jobs * sizeof(void *));
        memset(padding, -1, sizeof(padding));

        for (i = 0; i < num_jobs; i++) {
                targets[i] = malloc(text_len + (sizeof(padding) * 2));
                if (targets[i] == NULL)
                        goto end_alloc;
                memset(targets[i], -1, text_len + (sizeof(padding) * 2));
                if (in_place) {
                        /* copy input text to the allocated buffer */
                        memcpy(targets[i] + sizeof(padding), in_text, text_len);
                }
        }

        while (IMB_GET_NEXT_BURST(mb_mgr, num_jobs, jobs) < (uint32_t)num_jobs)
                IMB_FLUSH_BURST(mb_mgr, num_jobs, jobs);

        for (i = 0; i < num_jobs; i++) {
                job = jobs[i];

                job->cipher_direction = dir;
                job->chain_order = order;
                job->key_len_in_bytes = key_len;
                job->cipher_mode = cipher;
                job->hash_alg = IMB_AUTH_NULL;

                if (!in_place) {
                        job->dst = targets[i] + sizeof(padding);
                        job->src = in_text;
                } else {
                        job->dst = targets[i] + sizeof(padding);
                        job->src = targets[i] + sizeof(padding);
                }

                job->enc_keys = enc_keys;
                job->dec_keys = dec_keys;
                job->iv = iv;
                job->iv_len_in_bytes = 16;
                job->cipher_start_src_offset_in_bytes = 0;
                job->msg_len_to_cipher_in_bytes = text_len;
                job->user_data = targets[i];
                job->user_data2 = (void *)((uint64_t)i);

                imb_set_session(mb_mgr, job);
        }

        completed_jobs = IMB_SUBMIT_BURST(mb_mgr, num_jobs, jobs);
        if (completed_jobs == 0) {
                int err = imb_get_errno(mb_mgr);

                if (err != 0) {
                        printf("submit_burst error %d : '%s'\n", err,
                               imb_get_strerror(err));
                        goto end;
                }
        }

check_burst_jobs:
        for (i = 0; i < completed_jobs; i++) {
                job = jobs[i];

                if (job->status != IMB_STATUS_COMPLETED) {
                        printf("job %d status not complete!\n", i+1);
                        goto end;
                }

                if (!aes_job_ok(job, out_text, job->user_data, padding,
                                sizeof(padding), text_len))
                        goto end;
                jobs_rx++;
        }

        if (jobs_rx != num_jobs) {
                completed_jobs = IMB_FLUSH_BURST(mb_mgr,
                                                 num_jobs - completed_jobs,
                                                 jobs);
                if (completed_jobs == 0) {
                        printf("Expected %d jobs, received %d\n",
                               num_jobs, jobs_rx);
                        goto end;
                }
                goto check_burst_jobs;
        }
        ret = 0;

 end:

 end_alloc:
        if (targets != NULL) {
                for (i = 0; i < num_jobs; i++)
                        free(targets[i]);
                free(targets);
        }

        return ret;
}

static void
test_aes_vectors(struct IMB_MGR *mb_mgr,
                 struct test_suite_context *ctx128,
                 struct test_suite_context *ctx256,
                 const int vec_cnt,
                 const struct aes_vector *vec_tab, const char *banner,
                 const IMB_CIPHER_MODE cipher, const int num_jobs)
{
	int vect;
        DECLARE_ALIGNED(uint32_t enc_keys[15*4], 16);
        DECLARE_ALIGNED(uint32_t dec_keys[15*4], 16);

        if (!quiet_mode)
	        printf("%s (N jobs = %d):\n", banner, num_jobs);
	for (vect = 0; vect < vec_cnt; vect++) {
                struct test_suite_context *ctx;

                if (!quiet_mode) {
#ifdef DEBUG
                        printf("[%d/%d] Standard vector key_len:%d\n",
                               vect + 1, vec_cnt,
                               (int) vec_tab[vect].Klen);
#else
                        printf(".");
#endif
                }

                switch (vec_tab[vect].Klen) {
                case 16:
                        IMB_AES_KEYEXP_128(mb_mgr, vec_tab[vect].K, enc_keys,
                                           dec_keys);
                        ctx = ctx128;
                        break;
                case 32:
                default:
                        IMB_AES_KEYEXP_256(mb_mgr, vec_tab[vect].K, enc_keys,
                                           dec_keys);
                        ctx = ctx256;
                        break;
                }

                if (test_aes_many(mb_mgr, enc_keys, dec_keys,
                                  vec_tab[vect].IV,
                                  vec_tab[vect].P, vec_tab[vect].C,
                                  (unsigned) vec_tab[vect].Plen,
                                  IMB_DIR_ENCRYPT, IMB_ORDER_CIPHER_HASH,
                                  cipher, 0,
                                  vec_tab[vect].Klen, num_jobs)) {
                        printf("error #%d encrypt\n", vect + 1);
                        test_suite_update(ctx, 0, 1);
                } else {
                        test_suite_update(ctx, 1, 0);
                }

                if (test_aes_many_burst(mb_mgr, enc_keys, dec_keys,
                                        vec_tab[vect].IV,
                                        vec_tab[vect].P, vec_tab[vect].C,
                                        (unsigned) vec_tab[vect].Plen,
                                        IMB_DIR_ENCRYPT, IMB_ORDER_CIPHER_HASH,
                                        cipher, 0,
                                        vec_tab[vect].Klen, num_jobs)) {
                        printf("error #%d encrypt burst\n", vect + 1);
                        test_suite_update(ctx, 0, 1);
                } else {
                        test_suite_update(ctx, 1, 0);
                }

                if (test_aes_many(mb_mgr, enc_keys, dec_keys,
                                  vec_tab[vect].IV,
                                  vec_tab[vect].C, vec_tab[vect].P,
                                  (unsigned) vec_tab[vect].Plen,
                                  IMB_DIR_DECRYPT, IMB_ORDER_HASH_CIPHER,
                                  cipher, 0,
                                  vec_tab[vect].Klen, num_jobs)) {
                        printf("error #%d decrypt\n", vect + 1);
                        test_suite_update(ctx, 0, 1);
                } else {
                        test_suite_update(ctx, 1, 0);
                }

                if (test_aes_many_burst(mb_mgr, enc_keys, dec_keys,
                                        vec_tab[vect].IV,
                                        vec_tab[vect].C, vec_tab[vect].P,
                                        (unsigned) vec_tab[vect].Plen,
                                        IMB_DIR_DECRYPT, IMB_ORDER_HASH_CIPHER,
                                        cipher, 0,
                                        vec_tab[vect].Klen, num_jobs)) {
                        printf("error #%d decrypt burst\n", vect + 1);
                        test_suite_update(ctx, 0, 1);
                } else {
                        test_suite_update(ctx, 1, 0);
                }

                if (test_aes_many(mb_mgr, enc_keys, dec_keys,
                                  vec_tab[vect].IV,
                                  vec_tab[vect].P, vec_tab[vect].C,
                                  (unsigned) vec_tab[vect].Plen,
                                  IMB_DIR_ENCRYPT, IMB_ORDER_CIPHER_HASH,
                                  cipher, 1,
                                  vec_tab[vect].Klen, num_jobs)) {
                        printf("error #%d encrypt in-place\n", vect + 1);
                        test_suite_update(ctx, 0, 1);
                } else {
                        test_suite_update(ctx, 1, 0);
                }

                if (test_aes_many_burst(mb_mgr, enc_keys, dec_keys,
                                        vec_tab[vect].IV,
                                        vec_tab[vect].P, vec_tab[vect].C,
                                        (unsigned) vec_tab[vect].Plen,
                                        IMB_DIR_ENCRYPT, IMB_ORDER_CIPHER_HASH,
                                        cipher, 1,
                                        vec_tab[vect].Klen, num_jobs)) {
                        printf("error #%d encrypt burst in-place\n", vect + 1);
                        test_suite_update(ctx, 0, 1);
                } else {
                        test_suite_update(ctx, 1, 0);
                }

                if (test_aes_many(mb_mgr, enc_keys, dec_keys,
                                  vec_tab[vect].IV,
                                  vec_tab[vect].C, vec_tab[vect].P,
                                  (unsigned) vec_tab[vect].Plen,
                                  IMB_DIR_DECRYPT, IMB_ORDER_HASH_CIPHER,
                                  cipher, 1,
                                  vec_tab[vect].Klen, num_jobs)) {
                        printf("error #%d decrypt in-place\n", vect + 1);
                        test_suite_update(ctx, 0, 1);
                } else {
                        test_suite_update(ctx, 1, 0);
                }

                if (test_aes_many_burst(mb_mgr, enc_keys, dec_keys,
                                        vec_tab[vect].IV,
                                        vec_tab[vect].C, vec_tab[vect].P,
                                        (unsigned) vec_tab[vect].Plen,
                                        IMB_DIR_DECRYPT, IMB_ORDER_HASH_CIPHER,
                                        cipher, 1,
                                        vec_tab[vect].Klen, num_jobs)) {
                        printf("error #%d decrypt burst in-place\n", vect + 1);
                        test_suite_update(ctx, 0, 1);
                } else {
                        test_suite_update(ctx, 1, 0);
                }
	}
        if (!quiet_mode)
                printf("\n");
}

static int
docrc_job_ok(const struct IMB_JOB *job,
             const struct docsis_crc_vector *p_vec,
             const uint8_t *target,
             const uint8_t *padding,
             const size_t sizeof_padding,
             const uint8_t *out_text)
{
        const int num = (const int)((uint64_t)job->user_data2);
        const unsigned text_len = (unsigned) p_vec->frame_len;

        if (job->status != IMB_STATUS_COMPLETED) {
                printf("%d error status:%d, job %d",
                       __LINE__, job->status, num);
                return 0;
        }

        if (job->msg_len_to_cipher_in_bytes > 0) {
                if (memcmp(out_text, target + sizeof_padding,
                           text_len)) {
                        printf("%d message mismatched\n", num);
                        return 0;
                }

                if (memcmp(padding, target, sizeof_padding)) {
                        printf("%d overwrite head\n", num);
                        return 0;
                }

                if (memcmp(padding,
                           target + sizeof_padding + text_len,
                           sizeof_padding)) {
                        printf("%d overwrite tail\n", num);
                        return 0;
                }
        }

        if (job->msg_len_to_hash_in_bytes >= 14) {
                if (memcmp(job->auth_tag_output, &p_vec->crc_hash,
                           sizeof(p_vec->crc_hash))) {
                        const uint32_t *p_got =
                                (const uint32_t *)job->auth_tag_output;
                        const uint32_t *p_exp =
                                (const uint32_t *)&p_vec->crc_hash;

                        printf("%d authentication tag mismatch "
                               "(got = 0x%X, exp = 0x%X)\n",
                               num, *p_got, *p_exp);
                        return 0;
                }
        }

        return 1;
}

static int
test_docrc_many(struct IMB_MGR *mb_mgr,
                void *enc_keys,
                void *dec_keys,
                const struct docsis_crc_vector *p_vec,
                const int dir,
                const int order,
                const int in_place,
                const int num_jobs)
{
        const uint64_t key_len = p_vec->key_len;
        const unsigned tag_len = 4;
        const unsigned frame_len = (unsigned) p_vec->frame_len;
        struct IMB_JOB *job;
        uint8_t padding[16];
        uint8_t **targets = malloc(num_jobs * sizeof(void *));
        uint32_t *auths = malloc(num_jobs * sizeof(uint32_t));
        int i, jobs_rx = 0, ret = -1;
        const uint8_t *in_text = NULL;
        const uint8_t *out_text = NULL;

        if (targets == NULL)
                goto end_alloc;

        memset(targets, 0, num_jobs * sizeof(void *));

        if (auths == NULL)
                goto end_alloc;

        if (dir == IMB_DIR_ENCRYPT) {
                in_text = p_vec->pt;
                out_text = p_vec->ct;
        } else {
                in_text = p_vec->ct;
                out_text = p_vec->pt;
        }

        memset(padding, -1, sizeof(padding));

        for (i = 0; i < num_jobs; i++) {
                targets[i] = malloc(frame_len + (sizeof(padding) * 2));
                if (targets[i] == NULL)
                        goto end_alloc;
                memset(targets[i], -1, frame_len + (sizeof(padding) * 2));
                if (in_place) {
                        /* copy input text to the allocated buffer */
                        memcpy(targets[i] + sizeof(padding), in_text,
                               frame_len);
                        if (dir == IMB_DIR_ENCRYPT && p_vec->hash_length >= 14)
                                memset(targets[i] + sizeof(padding) +
                                       frame_len - tag_len, 0xff, tag_len);
                }
        }

        /* flush the scheduler */
        while (IMB_FLUSH_JOB(mb_mgr) != NULL)
                ;

        for (i = 0; i < num_jobs; i++) {
                job = IMB_GET_NEXT_JOB(mb_mgr);
                job->cipher_direction = dir;
                job->chain_order = order;
                if (!in_place) {
                        job->dst = targets[i] + sizeof(padding);
                        job->src = in_text;
                } else {
                        job->dst = targets[i] + sizeof(padding) +
                                p_vec->cipher_offset;
                        job->src = targets[i] + sizeof(padding);
                }
                job->cipher_mode = IMB_CIPHER_DOCSIS_SEC_BPI;
                job->enc_keys = enc_keys;
                job->dec_keys = dec_keys;
                job->key_len_in_bytes = key_len;

                job->iv = p_vec->iv;
                job->iv_len_in_bytes = 16;
                job->cipher_start_src_offset_in_bytes = p_vec->cipher_offset;
                job->msg_len_to_cipher_in_bytes = p_vec->cipher_length;
                job->user_data = targets[i];
                job->user_data2 = (void *)((uint64_t)i);

                job->hash_alg = IMB_AUTH_DOCSIS_CRC32;
                job->hash_start_src_offset_in_bytes = p_vec->hash_offset;
                job->msg_len_to_hash_in_bytes = p_vec->hash_length;

                job->auth_tag_output = (uint8_t *) &auths[i];
                job->auth_tag_output_len_in_bytes = tag_len;
                job = IMB_SUBMIT_JOB(mb_mgr);
                if (job != NULL) {
                        jobs_rx++;
                        if (!docrc_job_ok(job, p_vec, job->user_data, padding,
                                          sizeof(padding), out_text))
                                goto end;
                }
        }

        while ((job = IMB_FLUSH_JOB(mb_mgr)) != NULL) {
                jobs_rx++;
                if (!docrc_job_ok(job, p_vec, job->user_data, padding,
                                  sizeof(padding), out_text))
                        goto end;
        }

        if (jobs_rx != num_jobs) {
                printf("Expected %d jobs, received %d\n", num_jobs, jobs_rx);
                goto end;
        }
        ret = 0;

 end:
        while (IMB_FLUSH_JOB(mb_mgr) != NULL)
                ;
 end_alloc:

        if (targets != NULL) {
                for (i = 0; i < num_jobs; i++)
                        free(targets[i]);
                free(targets);
        }
        free(auths);

        return ret;
}

static void
test_docrc_vectors(struct IMB_MGR *mb_mgr,
                   struct test_suite_context *ctx128,
                   struct test_suite_context *ctx256,
                   const int vec_cnt,
                   const struct docsis_crc_vector *vec_tab,
                   const char *banner, const int num_jobs)
{
	int vect;
        DECLARE_ALIGNED(uint32_t enc_keys[15*4], 16);
        DECLARE_ALIGNED(uint32_t dec_keys[15*4], 16);

        if (!quiet_mode)
	        printf("%s (N jobs = %d):\n", banner, num_jobs);
	for (vect = 0; vect < vec_cnt; vect++) {
                struct test_suite_context *ctx;

                if (!quiet_mode) {
#ifdef DEBUG
                        printf("[%d/%d] Standard vector\n",
                               vect + 1, vec_cnt);
#else
                        printf(".");
#endif
                }

                if (vec_tab[vect].key_len == 16) {
                        IMB_AES_KEYEXP_128(mb_mgr, vec_tab[vect].key, enc_keys,
                                           dec_keys);
                        ctx = ctx128;
                } else {
                        IMB_AES_KEYEXP_256(mb_mgr, vec_tab[vect].key, enc_keys,
                                           dec_keys);
                        ctx = ctx256;
                }


                if (test_docrc_many(mb_mgr, enc_keys, dec_keys,
                                    &vec_tab[vect],
                                    IMB_DIR_ENCRYPT, IMB_ORDER_HASH_CIPHER, 1,
                                    num_jobs)) {
                        printf("error #%d encrypt\n", vect + 1);
                        test_suite_update(ctx, 0, 1);
                } else {
                        test_suite_update(ctx, 1, 0);
                }

                if (test_docrc_many(mb_mgr, enc_keys, dec_keys,
                                    &vec_tab[vect],
                                    IMB_DIR_DECRYPT, IMB_ORDER_CIPHER_HASH, 1,
                                    num_jobs)) {
                        printf("error #%d decrypt\n", vect + 1);
                        test_suite_update(ctx, 0, 1);
                } else {
                        test_suite_update(ctx, 1, 0);
                }

	}
        if (!quiet_mode)
                printf("\n");
}

int
aes_test(struct IMB_MGR *mb_mgr)
{
        const int num_jobs_tab[] = {
                1, 3, 4, 5, 7, 8, 9, 15, 16, 17, MAX_BURST_JOBS
        };
        unsigned i;
        int errors = 0;
        struct test_suite_context ctx128;
        struct test_suite_context ctx256;

        test_suite_start(&ctx128, "DOCSIS-SEC-128");
        test_suite_start(&ctx256, "DOCSIS-SEC-256");
        for (i = 0; i < DIM(num_jobs_tab); i++)
                test_aes_vectors(mb_mgr, &ctx128, &ctx256,
                                 DIM(docsis_vectors),
                                 docsis_vectors,
                                 "AES-DOCSIS standard test vectors",
                                 IMB_CIPHER_DOCSIS_SEC_BPI,
                                 num_jobs_tab[i]);
        errors += test_suite_end(&ctx128);
        errors += test_suite_end(&ctx256);

        test_suite_start(&ctx128, "DOCSIS-SEC-128-CRC32");
        test_suite_start(&ctx256, "DOCSIS-SEC-256-CRC32");
        for (i = 0; i < DIM(num_jobs_tab); i++)
                test_docrc_vectors(mb_mgr, &ctx128, &ctx256,
                                   DIM(docsis_crc_tab),
                                   docsis_crc_tab,
                                   "AES-DOCSIS+CRC32 vectors",
                                   num_jobs_tab[i]);
        errors += test_suite_end(&ctx128);
        errors += test_suite_end(&ctx256);

        return errors;
}
