/**********************************************************************
  Copyright(c) 2023 Intel Corporation All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in
      the documentation and/or other materials provided with the
      distribution.
    * Neither the name of Intel Corporation nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**********************************************************************/

/* GMAC */
#include "mac_test.h"

const struct mac_test gmac_test_kat_json[] = {
        /*
         * Vectors from
         * http://csrc.nist.gov/groups/ST/toolkit/BCM/documents/
         * proposedmodes/gcm/gcm-revised-spec.pdf
        */
        {128, 32, 1,
         "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f",
         "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10\x11"
         "\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f\x20\x21\x22\x23"
         "\x24\x25\x26\x27\x28\x29\x2a\x2b\x2c\x2d\x2e\x2f\x30\x31\x32\x33\x34\x35"
         "\x36\x37\x38\x39\x3a\x3b\x3c\x3d\x3e\x3f",
         "\xc5\x3a\xf9\xe8", 1, 512,
         "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b", 96},
        {128, 128, 2,
         "\xfe\xff\xe9\x92\x86\x65\x73\x1c\x6d\x6a\x8f\x94\x67\x30\x83\x08",
         "\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10\x01\x02"
         "\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10\x01\x02\x03\x04"
         "\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10\x01\x02\x03\x04\x05\x06"
         "\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10\x01\x02\x03\x04\x05\x06\x07\x08"
         "\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a"
         "\x0b\x0c\x0d\x0e\x0f\x10\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c"
         "\x0d\x0e\x0f\x10\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e"
         "\x0f\x10\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10"
         "\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10",
         "\x4c\x0c\x4f\x47\x2d\x78\xf6\xd8\x03\x53\x20\x2f\x1a\xdf\x90\xd0", 1,
         1280, "\xca\xfe\xba\xbe\xfa\xce\xdb\xad\xde\xca\xf8\x88", 96},
        {192, 128, 3,
         "\xaa\x74\x0a\xbf\xad\xcd\xa7\x79\x22\x0d\x3b\x40\x6c\x5d\x7e\xc0\x9a\x77"
         "\xfe\x9d\x94\x10\x45\x39",
         "\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10\x01\x02"
         "\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10\x01\x02\x03\x04"
         "\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10\x01\x02\x03\x04\x05\x06"
         "\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10\x01\x02\x03\x04\x05\x06\x07\x08"
         "\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10",
         "\xcf\x82\x80\x64\x02\x46\xf4\xfb\x33\xae\x1d\x90\xea\x48\x83\xdb", 1, 640,
         "\xab\x22\x65\xb4\xc1\x68\x95\x55\x61\xf0\x43\x15", 96},
        {256, 128, 4,
         "\xb5\x48\xe4\x93\x4f\x5c\x64\xd3\xc0\xf0\xb7\x8f\x7b\x4d\x88\x24\xaa\xc4"
         "\x6b\x3c\x8d\x2c\xc3\x5e\xe4\xbf\xb2\x54\xe4\xfc\xba\xf7",
         "\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10\x01\x02"
         "\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10\x01\x02\x03\x04"
         "\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10\x01\x02\x03\x04\x05\x06"
         "\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10\x01",
         "\x77\x46\x0d\x6f\xb1\x87\xdb\xa9\x46\xad\xcd\xfb\xb7\xf9\x13\xa1", 1, 520,
         "\x2e\xed\xe1\xdc\x64\x47\xc7\xaf\xc4\x41\x53\x58", 96},
        {0, 0, 0, NULL, NULL, NULL, 0, 0, NULL, 0}
};
