/*
* MIT License
*
* Copyright (c) 2020 Infineon Technologies AG
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

/** \file blocksec2go.c
 * C-API for implementation of Blockchain Security 2 Go Starter Kit v2 command set. 
 */

#include "ifx/blocksec2go.h"
#include "ifx/apdu.h"
#include "ifx/blocksec2go/status.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * \brief Parameter for indicating last block of update key label command
 */
#define BLOCK2GO_LAST_BLOCK 0x80

/**
 * \brief Parameter for indicating more blocks of update key label command
 */
#define BLOCK2GO_MORE_BLOCKS 0x00

/**
 * \brief Parameter for indicating first occurance of get key label command
 */
#define BLOCK2GO_FIRST_OCCURANCE 0x00

/**
 * \brief Parameter for indicating next occurances of get key label command
 */
#define BLOCK2GO_NEXT_OCCURANCE 0x01

/**
 * \brief Converts a 4 byte uint8_t array into a uint32_t.
 *
 * \param i8[in]  uint8_t array
 *
 * \return uint32_t number
 */
static uint32_t uint8_to_uint32(uint8_t *i8) {
  return (uint32_t)i8[0] << 24 | (uint32_t)i8[1] << 16 | (uint32_t)i8[2] << 8 |
         (uint32_t)i8[3];
}

/**
 * \brief Generates length indicator (BER-TLV format) for a given length.
 *
 * \param buffer[in]  buffer to write length indicator into
 * \param length[in] length for which length indicator should be generated
 *
 * \return pointer to the end of length indicator
 */
static int8_t *write_length_indicator(uint8_t *buffer, uint16_t length) {
  if (length < 128) {
    buffer[0] = length & 0x00FF;
    return buffer + 1;
  } else if (length >= 128 && length < 256) {
    buffer[0] = 0x81;
    buffer[1] = length & 0x00FF;
    return buffer + 2;
  } else {
    buffer[0] = 0x82;
    buffer[1] = length >> 8;
    buffer[2] = length & 0x00FF;
    return buffer + 3;
  }
}

/**
 * \brief Returns the length and the length (in bytes) of the length component
 *  from length indicator.
 *
 * \param data[in] BER-TLV formated data
 * \param byte_length[out] length of the length component in bytes
 * \param key_label_length[out] length of the key label
 *
 * \return pointer to the end of length indicator
 */
static void get_label_length(uint8_t *data, size_t *byte_length,
                             uint16_t *key_label_length) {
  if (data[2] == 0x82) {
    *key_label_length = (data[3] << 8) | data[4];
    *byte_length = 3;
  } else if (data[2] == 0x81) {
    *key_label_length = *(data + 3);
    *byte_length = 2;
  } else {
    *key_label_length = *(data + 2);
    *byte_length = 1;
  }
}

/**
 * \brief Sends APDU and receives response APDU.
 *
 * \param protocol[in] instance of activated protocol to use
 * \param apdu[in] APDU which is to be sent
 * \param resp[out] decoded response APDU
 *
 * \retval APDUDECODE_SUCCESS in case of success
 * \retval others indicate failures from lower layers
 */
static int exchange_apdu(Protocol *protocol, APDU *apdu, APDUResponse *resp) {
  memset(resp, 0, sizeof(APDUResponse));

  uint8_t *encoded;
  size_t encoded_len;
  int status = apdu_encode(apdu, &encoded, &encoded_len);
  if (status != APDU_ENCODE_SUCCESS) {
    return status;
  }

  uint8_t *response = 0;
  size_t response_len;

  status = protocol_transceive(protocol, encoded, encoded_len, &response,
                               &response_len);
  free(encoded);
  if (status == PROTOCOL_TRANSCEIVE_SUCCESS) {
    status = apduresponse_decode(resp, response, response_len);
  }
  free(response);
  return status;
}

// SELECT
int block2go_select(Protocol *protocol, uint8_t id[BLOCK2GO_ID_LEN],
                    char **version) {
  *version = NULL;
  uint8_t aid[13] = {0xD2, 0x76, 0x00, 0x00, 0x04, 0x15, 0x02,
                     0x00, 0x01, 0x00, 0x00, 0x00, 0x01};
  APDU apdu = {.cla = 0x00,
               .ins = 0xA4,
               .p1 = 0x04,
               .p2 = 0x00,
               .lc = 0x0D,
               .data = aid,
               .le = 0};

  APDUResponse decoded;
  int status = exchange_apdu(protocol, &apdu, &decoded);

  if (status == APDURESPONSE_DECODE_SUCCESS) {
    if (decoded.sw != 0x9000) {
      status = BLOCK2GO_SELECT_SE_FAIL;
    } else if (decoded.len < 12) {
      status = BLOCK2GO_SELECT_INVALID_DATA_LENGTH;
    } else {
      status = BLOCK2GO_SELECT_SUCCESS;
      memcpy(id, decoded.data, BLOCK2GO_ID_LEN);
      const size_t version_len = decoded.len - BLOCK2GO_ID_LEN;
      *version = (char *)malloc(version_len + 1);
      memcpy(*version, decoded.data + BLOCK2GO_ID_LEN, version_len);
      (*version)[version_len] = 0;
    }
  }
  apduresponse_destroy(&decoded);
  return status;
}

// GENERATE KEY
static int block2go_generate_key(Protocol *protocol, block2go_curve curve,
                                 block2go_key_type key_type, uint8_t *keyslot) {
  APDU apdu = {.cla = 0x00,
               .ins = 0x02,
               .p1 = curve,
               .p2 = key_type,
               .lc = 0x00,
               .data = NULL,
               .le = 0};

  APDUResponse decoded;
  int status = exchange_apdu(protocol, &apdu, &decoded);
  if (status == APDURESPONSE_DECODE_SUCCESS) {
    if (decoded.sw != 0x9000) {
      status = BLOCK2GO_GENERATE_KEY_SE_FAIL;
    } else if ((decoded.len != 0 && key_type == BLOCK2GO_KEY_TYPE_SESSION) ||
               (decoded.len != 1 && key_type == BLOCK2GO_KEY_TYPE_PERMANENT)) {
      status = BLOCK2GO_GENERATE_KEY_INVALID_DATA_LENGTH;
    } else if (key_type == BLOCK2GO_KEY_TYPE_PERMANENT) {
      status = BLOCK2GO_GENERATE_KEY_SUCCESS;
      *keyslot = decoded.data[0];
    }
  }
  apduresponse_destroy(&decoded);
  return status;
}

// GENERATE SESSION KEY
int block2go_generate_key_session(Protocol *protocol, block2go_curve curve) {
  return block2go_generate_key(protocol, curve, BLOCK2GO_KEY_TYPE_SESSION,
                               NULL);
}

// GENERATE PERMANENT KEY
int block2go_generate_key_permanent(Protocol *protocol, block2go_curve curve,
                                    uint8_t *key_slot) {
  return block2go_generate_key(protocol, curve, BLOCK2GO_KEY_TYPE_PERMANENT,
                               key_slot);
}

// GET KEY INFO

static int block2go_get_key_info(Protocol *protocol, uint8_t key_index,
                                 uint8_t key_type, block2go_curve *curve,
                                 uint32_t *global_counter, uint32_t *counter,
                                 uint8_t **public_key) {
  *public_key = NULL;
  APDU apdu = {.cla = 0x00,
               .ins = 0x16,
               .p1 = key_index,
               .p2 = key_type,
               .lc = 0x00,
               .data = NULL,
               .le = 0x00};

  APDUResponse decoded;
  int status = exchange_apdu(protocol, &apdu, &decoded);

  if (status == APDURESPONSE_DECODE_SUCCESS) {
    if (decoded.sw != 0x9000) {
      status = BLOCK2GO_GET_KEY_INFO_SE_FAIL;
    } else if (decoded.len != BLOCK2GO_PUBLIC_KEY_LEN + 9) {
      status = BLOCK2GO_GET_KEY_INFO_INVALID_DATA_LENGTH;
    } else {
      status = BLOCK2GO_GET_KEY_INFO_SUCCESS;
      *curve = decoded.data[0];
      *global_counter = uint8_to_uint32(decoded.data + 1);
      *counter = uint8_to_uint32(decoded.data + 5);
      *public_key = (uint8_t *)malloc(BLOCK2GO_PUBLIC_KEY_LEN);
      memcpy(*public_key, decoded.data + 9, BLOCK2GO_PUBLIC_KEY_LEN);
    }
  }
  apduresponse_destroy(&decoded);
  return status;
}

int block2go_get_key_info_session(Protocol *protocol, block2go_curve *curve,
                                  uint32_t *global_counter, uint32_t *counter,
                                  uint8_t **public_key) {
  return block2go_get_key_info(protocol, 0x00, BLOCK2GO_KEY_TYPE_SESSION, curve,
                               global_counter, counter, public_key);
}

int block2go_get_key_info_permanent(Protocol *protocol, uint8_t key_index,
                                    block2go_curve *curve,
                                    uint32_t *global_counter, uint32_t *counter,
                                    uint8_t **public_key) {
  return block2go_get_key_info(protocol, key_index, BLOCK2GO_KEY_TYPE_PERMANENT,
                               curve, global_counter, counter, public_key);
}

// ENCRYPTED KEYIMPORT

int block2go_encrypted_keyimport(Protocol *protocol, block2go_curve curve,
                                 uint8_t seed[BLOCK2GO_SEED_LEN]) {

  APDU apdu = {.cla = 0x00,
               .ins = 0x20,
               .p1 = curve,
               .p2 = 0x00,
               .lc = BLOCK2GO_SEED_LEN,
               .data = seed,
               .le = 0x00};

  APDUResponse decoded;
  int status = exchange_apdu(protocol, &apdu, &decoded);

  if (status == APDURESPONSE_DECODE_SUCCESS) {
    if (decoded.sw != 0x9000) {
      status = BLOCK2GO_ENCRYPTED_KEYIMPORT_FAIL;
    } else if (decoded.len != 0) {
      status = BLOCK2GO_ENCRYPTED_KEYIMPORT_INVALID_DATA_LENGTH;
    } else {
      status = BLOCK2GO_ENCRYPTED_KEYIMPORT_SUCCESS;
    }
  }
  apduresponse_destroy(&decoded);
  return status;
}

// GENERATE SIGNATURE
static int block2go_generate_signature(Protocol *protocol, uint8_t keyslot,
                                       block2go_key_type keytype,
                                       uint8_t data_to_sign[32],
                                       uint32_t *global_counter,
                                       uint32_t *counter, uint8_t **signature,
                                       size_t *signature_len) {
  *signature = NULL;
  APDU apdu = {.cla = 0x00,
               .ins = 0x18,
               .p1 = keyslot,
               .p2 = keytype,
               .lc = 0x20,
               .data = data_to_sign,
               .le = 0x00};

  APDUResponse decoded;
  int status = exchange_apdu(protocol, &apdu, &decoded);

  if (status == APDURESPONSE_DECODE_SUCCESS) {
    if (decoded.sw != 0x9000) {
      status = BLOCK2GO_GENERATE_SIGNATURE_FAIL;
    } else if (decoded.len < 16) { // counter (8) + signature (>= 8)
      status = BLOCK2GO_GENERATE_SIGNATURE_INVALID_DATA_LENGTH;
    } else {
      status = BLOCK2GO_GENERATE_SIGNATURE_SUCCESS;
      *global_counter = uint8_to_uint32(decoded.data);
      *counter = uint8_to_uint32(decoded.data + 4);
      *signature_len = decoded.len - 8;
      *signature = (uint8_t *)malloc(*signature_len);
      memcpy(*signature, decoded.data + 8, *signature_len);
    }
  }
  apduresponse_destroy(&decoded);
  return status;
}

int block2go_generate_signature_session(Protocol *protocol,
                                        uint8_t data_to_sign[32],
                                        uint32_t *global_counter,
                                        uint32_t *counter, uint8_t **signature,
                                        size_t *signature_len) {
  return block2go_generate_signature(protocol, 0x00, BLOCK2GO_KEY_TYPE_SESSION,
                                     data_to_sign, global_counter, counter,
                                     signature, signature_len);
}

int block2go_generate_signature_permanent(Protocol *protocol, uint8_t key_index,
                                          uint8_t data_to_sign[32],
                                          uint32_t *global_counter,
                                          uint32_t *counter,
                                          uint8_t **signature,
                                          size_t *signature_len) {
  return block2go_generate_signature(
      protocol, key_index, BLOCK2GO_KEY_TYPE_PERMANENT, data_to_sign,
      global_counter, counter, signature, signature_len);
}

// CREATE KEY LABEL
int block2go_create_key_label(Protocol *protocol, uint8_t key_index,
                              uint16_t key_label_size, uint32_t *memory) {

  uint8_t data[2] = {key_label_size >> 8, key_label_size & 0x00FF};
  APDU apdu = {.cla = 0x00,
               .ins = 0x1D,
               .p1 = key_index,
               .p2 = 0x00,
               .lc = 0x02,
               .data = data,
               .le = 0x04};

  APDUResponse decoded;
  int status = exchange_apdu(protocol, &apdu, &decoded);

  if (status == APDURESPONSE_DECODE_SUCCESS) {
    if (decoded.sw != 0x9000) {
      status = BLOCK2GO_CREATE_KEY_LABEL_FAIL;
    } else if (decoded.len != 4) {
      status = BLOCK2GO_CREATE_KEY_LABEL_INVALID_DATA_LENGTH;
    } else {
      status = BLOCK2GO_CREATE_KEY_LABEL_SUCCESS;
      *memory = uint8_to_uint32(decoded.data);
    }
  }
  apduresponse_destroy(&decoded);
  return status;
}

// UPDATE KEY LABEL

int block2go_update_key_label(Protocol *protocol, uint8_t key_index,
                              uint8_t *key_label, uint16_t key_label_size) {

  uint8_t block_len_max = 160; //to be determined 

  if (key_label_size + 1 >= 1024) {
    return BLOCK2GO_UPDATE_KEY_LABEL_OUT_OF_MEMORY;
  }
  int status = BLOCK2GO_UPDATE_KEY_LABEL_SUCCESS;

  uint8_t *data = malloc(key_label_size + 6);
  uint8_t *write_ptr = data;

  *write_ptr++ = 0xDF;
  *write_ptr++ = 0x1F;
  write_ptr = write_length_indicator(write_ptr, key_label_size + 1);
  *write_ptr++ = key_index;
  memcpy(write_ptr, key_label, key_label_size);
  write_ptr += key_label_size;

  uint16_t data_length = (uintptr_t)write_ptr - (uintptr_t)data;
  uint8_t num_blocks = (data_length - 1) / block_len_max + 1;
  for (uint8_t block = 1; block <= num_blocks; block++) {

    uint8_t sequence_num = block - 1;
    uint8_t p1 = BLOCK2GO_MORE_BLOCKS;
    uint8_t data_len = block_len_max;

    if (block == num_blocks) {
      p1 = BLOCK2GO_LAST_BLOCK;
      data_len = (uint8_t)data_length;
    }

    APDU apdu = {.cla = 0x00,
                 .ins = 0x1E,
                 .p1 = p1,
                 .p2 = sequence_num,
                 .lc = data_len,
                 .data = data + sequence_num * block_len_max,
                 .le = 0x00};

    if (block < num_blocks) {
      // decrease data_length accordingly
      data_length = data_length - block_len_max;
    }

    APDUResponse decoded;
    status = exchange_apdu(protocol, &apdu, &decoded);
    // printf("\nStatus word: %x", decoded.sw);
    if (status == APDURESPONSE_DECODE_SUCCESS) {
      if (decoded.sw != 0x9000) {
        status = BLOCK2GO_UPDATE_KEY_LABEL_FAIL;
      } else if (decoded.len != 0) {
        status = BLOCK2GO_UPDATE_KEY_LABEL_INVALID_DATA_LENGTH;
      }
    }
    apduresponse_destroy(&decoded);
    if (status != APDURESPONSE_DECODE_SUCCESS) {
      break;
    }
  }
  free(data);
  return status;
}

// GET KEY LABEL
int block2go_get_key_label(Protocol *protocol, uint8_t key_index,
                           uint8_t **key_label, uint16_t *key_label_length) {
  *key_label = NULL;

  APDU apdu = {.cla = 0x00,
               .ins = 0x1F,
               .p1 = key_index,
               .p2 = BLOCK2GO_FIRST_OCCURANCE,
               .lc = 0x00,
               .data = NULL,
               .le = 0x00};
  APDUResponse decoded;
  int status = exchange_apdu(protocol, &apdu, &decoded);

  uint16_t label_length = 0;
  uint16_t label_received = 0;
  if (status == APDURESPONSE_DECODE_SUCCESS) {
    if (decoded.sw == 0x9000 || decoded.sw == 0x6310) {
      if (((decoded.data[0] << 8) | decoded.data[1]) != 0xDF1F) {
        status = BLOCK2GO_GET_KEY_LABEL_KEY_LABEL_TAG_MISSING;
      } else if (decoded.len == 0) {
        status = BLOCK2GO_GET_KEY_LABEL_INVALID_DATA_LENGTH;
      } else {
        size_t byte_length = 0;
        get_label_length(decoded.data, &byte_length, &label_length);
        *key_label = (uint8_t *)malloc(label_length);
        memcpy(*key_label, decoded.data + 2 + byte_length, label_length);
        label_received += label_length;
        *key_label_length += label_length;
        while (decoded.sw == 0x9000 || decoded.sw == 0x6310) {
          if (decoded.sw == 0x9000) {
            apduresponse_destroy(&decoded);
            return BLOCK2GO_GET_KEY_LABEL_SUCCESS;
          } else if (decoded.sw == 0x6310) {
            apdu.p2 = BLOCK2GO_NEXT_OCCURANCE;
            status = exchange_apdu(protocol, &apdu, &decoded);
            if (status == APDURESPONSE_DECODE_SUCCESS) {
              get_label_length(decoded.data, &byte_length, &label_length);
              *key_label_length += label_length;
              *key_label = (uint8_t *)realloc(*key_label, *key_label_length);
              memcpy(*key_label + label_received,
                     decoded.data + 2 + byte_length, label_length);
              label_received += label_length;
            }
          } else {
            apduresponse_destroy(&decoded);
            return BLOCK2GO_GET_KEY_LABEL_FAIL;
          }
        }
      }
    } else {
      apduresponse_destroy(&decoded);
      return BLOCK2GO_GET_KEY_LABEL_FAIL;
    }
  }
  apduresponse_destroy(&decoded);
  return status;
}

// GET RANDOM
int block2go_get_random(Protocol *protocol, uint8_t length,
                        uint8_t **random_num) {

  APDU apdu = {.cla = 0x00,
               .ins = 0x1A,
               .p1 = length,
               .p2 = 0x00,
               .lc = 0x00,
               .data = NULL,
               .le = 0};
  APDUResponse decoded;
  int status = exchange_apdu(protocol, &apdu, &decoded);

  if (status == APDURESPONSE_DECODE_SUCCESS) {
    if (decoded.sw != 0x9000) {
      status = BLOCK2GO_GET_RANDOM_FAIL;
    } else if (decoded.len != length) {
      status = BLOCK2GO_GET_RANDOM_INVALID_DATA_LENGTH;
    } else {
      *random_num = (uint8_t *)malloc(length);
      memcpy(*random_num, decoded.data, length);
    }
  }
  apduresponse_destroy(&decoded);
  return status;
}

// VERIFY SIGNATURE

int block2go_verify_signature(Protocol *protocol, block2go_curve curve,
                              uint8_t *message, uint8_t message_len,
                              uint8_t *signature,
                              uint8_t public_key[BLOCK2GO_PUBLIC_KEY_LEN]) {

  uint8_t signature_len = signature[1] + 6; // + 6 from ANS.1 DER format
  uint8_t data_len = 1 + message_len + signature_len + BLOCK2GO_PUBLIC_KEY_LEN;
  uint8_t *data = (uint8_t *)malloc(data_len);
  data[0] = message_len;
  memcpy(data + 1, message, message_len);
  memcpy(data + 1 + message_len, signature, signature_len);
  memcpy(data + 1 + message_len + signature_len, public_key,
         BLOCK2GO_PUBLIC_KEY_LEN);

  APDU apdu = {.cla = 0x00,
               .ins = 0x1B,
               .p1 = curve,
               .p2 = 0x00,
               .lc = data_len,
               .data = data,
               .le = 0x00};

  APDUResponse decoded;
  int status = exchange_apdu(protocol, &apdu, &decoded);
  free(data);

  if (status == APDURESPONSE_DECODE_SUCCESS) {
    if (decoded.sw != 0x9000) {
      status = BLOCK2GO_VERIFY_SIGNATURE_FAIL;
    } else if (decoded.len != 0) {
      status = BLOCK2GO_VERIFY_SIGNATURE_INVALID_DATA_LENGTH;
    }
  }
  apduresponse_destroy(&decoded);
  return status;
}

// ENABLE PROTECTED MODE

int block2go_enable_protected_mode(Protocol *protocol) {

  APDU apdu = {.cla = 0x00,
               .ins = 0xD0,
               .p1 = 0x00,
               .p2 = 0x00,
               .lc = 0x00,
               .data = NULL,
               .le = 0x00};

  APDUResponse decoded;
  int status = exchange_apdu(protocol, &apdu, &decoded);

  if (status == APDURESPONSE_DECODE_SUCCESS) {
    if (decoded.sw != 0x9000) {
      status = BLOCK2GO_ENABLE_PROTECTED_MODE_FAIL;
    } else if (decoded.len != 0) {
      status = BLOCK2GO_ENABLE_PROTECTED_MODE_INVALID_DATA_LENGTH;
    } else {
      status = BLOCK2GO_ENABLE_PROTECTED_MODE_SUCCESS;
    }
  }
  apduresponse_destroy(&decoded);
  return status;
}

// GET STATUS
int block2go_get_status(Protocol *protocol,
                        block2go_session_type *status_info) {

  APDU apdu = {.cla = 0x00,
               .ins = 0xB0,
               .p1 = 0xDF,
               .p2 = 0x20,
               .lc = 0x00,
               .data = NULL,
               .le = 0x00};

  APDUResponse decoded;
  int status = exchange_apdu(protocol, &apdu, &decoded);

  if (status == APDURESPONSE_DECODE_SUCCESS) {
    if (decoded.sw != 0x9000) {
      status = BLOCK2GO_GET_STATUS_FAIL;
    } else if (decoded.len != 1) {
      status = BLOCK2GO_GET_STATUS_INVALID_DATA_LENGTH;
    } else {
      status = BLOCK2GO_GET_STATUS_SUCCESS;
      *status_info = *(decoded.data);
    }
  }
  apduresponse_destroy(&decoded);
  return status;
}
