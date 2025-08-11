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

/** \file blocksec2go.h
 * C-API for implementation of Blockchain Security 2 Go Starter Kit v2 command set. 
 */
#pragma once

#include "ifx/blocksec2go/status.h"
#include "ifx/protocol.h"

/**
 * \brief Length of the Blocksec2Go ID
 */
#define BLOCK2GO_ID_LEN 11

/**
 * \brief Length of the public key
 */
#define BLOCK2GO_PUBLIC_KEY_LEN 65

/**
 * \brief Length of the seed for encrypted key import
 */
#define BLOCK2GO_SEED_LEN 16

/**
 * \brief I2C address of Blocksec2Go card
 */
#define I2C_ADDRESS 0x50

/**
 * \brief Enum for the ECC curve type
 */
typedef enum {
  BLOCK2GO_CURVE_SEC_P256K1 = 0, /**< SEC-P256K1 */
  BLOCK2GO_CURVE_NIST_P256 = 1 /**< NIST-P256 */
} block2go_curve;

/**
 * \brief Enum for the key type
 */
typedef enum {
  BLOCK2GO_KEY_TYPE_PERMANENT = 0, /**< Permanent key */
  BLOCK2GO_KEY_TYPE_SESSION = 1 /**< Session key */
} block2go_key_type;

/**
 * \brief Enum for the session type
 */
typedef enum {
  BLOCK2GO_SESSION_TYPE_UNPROTECTED = 0, /**< Unproteced mode */
  BLOCK2GO_SESSION_TYPE_PROTECTED = 1 /**< Protected mode */
} block2go_session_type;

/**
 * \brief SELECT the Blockchain Security 2Go application.
 *
 * \param[in] protocol   instance of activated protocol to use
 * \param[out] id        buffer to copy SE id to
 * \param[out] version   pointer to write address of allocated zero terminated
 * version string to
 *
 * \note version has to be freed by the caller
 *
 * \retval BLOCK2GO_SELECT_SUCCESS in case of success
 * \retval BLOCK2GO_SELECT_SE_FAIL SE indicated error
 * \retval BLOCK2GO_SELECT_INVALID_DATA_LENGTH unexpectedly short response
 * \retval others indicate failures from lower layers
 */
int block2go_select(Protocol *protocol, uint8_t id[BLOCK2GO_ID_LEN],
                    char **version);

/**
 * \brief Creates new ECC public/private keypair for a session.
 *
 * \param[in] protocol  instance of activated protocol to use
 * \param[in] curve     ECC-curve used for encryption
 *
 *
 * \retval BLOCK2GO_GENERATE_SESSION_KEY_SUCCESS in case of success
 * \retval BLOCK2GO_GENERATE_SESSION_KEY_SE_FAIL SE indicated error
 * \retval BLOCK2GO_GENERATE_SESSION_KEY_INVALID_DATA_LENGTH unexpectedly
 * short/long response \retval others indicate failures from lower layers
 */
int block2go_generate_key_session(Protocol *protocol, block2go_curve curve);

/**
 * \brief Creates new permanent ECC public/private keypair in a new keyslot.
 *
 * \param[in] protocol  instance of activated protocol to use
 * \param[in] curve     ECC-curve used for encryption
 * \param[out] key_slot received keyslot index
 *
 * \retval BLOCK2GO_GENERATE_PERMANENT_KEY_SUCCESS in case of success
 * \retval BLOCK2GO_GENERATE_PERMANENT_KEY_SE_FAIL SE indicated error
 * \retval BLOCK2GO_GENERATE_PERMANENT_KEY_INVALID_DATA_LENGTH unexpectedly
 * short/long response \retval others indicate failures from lower layers
 */
int block2go_generate_key_permanent(Protocol *protocol, block2go_curve curve,
                                    uint8_t *key_slot);

/**
 * \brief Returns the public key and curve type of the session key.
 *
 * \param[in] protocol    instance of activated protocol to use
 * \param[out] curve      ECC-curve used for encryption
 * \param[out] global_counter buffer to copy remaining signatures of the card
 * into \param[out] counter buffer to copy remaining signatures for the given
 * key into
 * \param[out] public_key public key allocated by callee, to be freed by caller.
 *
 * \note public key has to be freed by the caller
 *
 * \retval BLOCK2GO_GET_KEY_INFO_SUCCESS in case of success
 * \retval BLOCK2GO_GET_KEY_INFO_SE_FAIL SE indicated error
 * \retval BLOCK2GO_GET_KEY_INFO_INVALID_DATA_LENGTH unexpectedly
 * short/long response
 * \retval others indicate failures from lower layers
 */
int block2go_get_key_info_session(Protocol *protocol, block2go_curve *curve,
                                  uint32_t *global_counter, uint32_t *counter,
                                  uint8_t **public_key);
/**
 * \brief Returns the public key and curve type of the given permanent key.
 *
 * \param[in] protocol    instance of activated protocol to use
 * \param[in] key_index   key index for which info should be given
 * \param[out] curve      ECC-curve used for encryption
 * \param[out] global_counter buffer to copy remaining signatures of the card
 * into \param[out] counter buffer to copy remaining signatures for the given
 * key into
 * \param[out] public_key public key allocated by callee, to be freed by caller.
 *
 * \note public key has to be freed by the caller
 *
 * \retval BLOCK2GO_GET_KEY_INFO_SUCCESS in case of success
 * \retval BLOCK2GO_GET_KEY_INFO_SE_FAIL SE indicated error
 * \retval BLOCK2GO_GET_KEY_INFO_INVALID_DATA_LENGTH unexpectedly
 * short/long response
 * \retval others indicate failures from lower layers
 */
int block2go_get_key_info_permanent(Protocol *protocol, uint8_t key_index,
                                    block2go_curve *curve,
                                    uint32_t *global_counter, uint32_t *counter,
                                    uint8_t **public_key);

/**
 * \brief Creates a new key pair by deriving the private key from a given seed.
 * The encrypted key is stored with the key slot index 0.
 *
 * \param[in] protocol    instance of activated protocol to use
 * \param[in] curve       ECC-curve used for encryption
 * \param[in] seed        seed which is used for key derivation
 *
 * \retval BLOCK2GO_ENCRYPTED_KEY_IMPORT_SUCCESS in case of success
 * \retval BLOCK2GO_ENCRYPTED_KEY_IMPORT_SE_FAIL SE indicated error
 * \retval BLOCK2GO_ENCRYPTED_KEY_IMPORT_TOO_LITTLE_DATA unexpectedly
 * short/long response
 * \retval others indicate failures from lower layers
 */
int block2go_encrypted_keyimport(Protocol *protocol, block2go_curve curve,
                                 uint8_t seed[BLOCK2GO_SEED_LEN]);

/**
 * \brief Signs a given block of prehashed data using the stored private key
 * that is associated with the session key.
 *
 * \param[in] protocol      instance of activated protocol to use
 * \param[in] data_to_sign  hashed data that should be signed
 * \param[out] global_counter buffer to copy remaining signatures of the card
 * into \param[out] counter buffer to copy remaining signatures for the given
 * key into \param[out] signature     buffer for storing ANS.1 DER encoded
 * signature \param[out] signature_len  buffer to copy length of the signature
 * in bytes into
 *
 * \note signature has to be freed by the caller
 *
 * \retval BLOCK2GO_ENCRYPTED_KEY_IMPORT_SUCCESS in case of success
 * \retval BLOCK2GO_ENCRYPTED_KEY_IMPORT_SE_FAIL SE indicated error
 * \retval BLOCK2GO_ENCRYPTED_KEY_IMPORT_TOO_LITTLE_DATA unexpectedly
 * short/long response
 * \retval others indicate failures from lower layers
 */
int block2go_generate_signature_session(Protocol *protocol,
                                uint8_t data_to_sign[32],
                                uint32_t *global_counter, uint32_t *counter,
                                uint8_t **signature, size_t *signature_len);
/**
 * \brief Signs a given block of prehashed data using the stored private key
 * that is associated with the given key.
 *
 * \param[in] protocol      instance of activated protocol to use
 * \param[in] key_index     key index for which signature should be generated
 * \param[in] data_to_sign  hashed data that should be signed
 * \param[out] global_counter buffer to copy remaining signatures of the card
 * into \param[out] counter buffer to copy remaining signatures for the given
 * key into \param[out] signature    buffer for storing ANS.1 DER encoded
 * signature \param[out] signature_len  buffer to copy length of the signature
 * in bytes into
 *
 * \note signature has to be freed by the caller
 *
 * \retval BLOCK2GO_ENCRYPTED_KEY_IMPORT_SUCCESS in case of success
 * \retval BLOCK2GO_ENCRYPTED_KEY_IMPORT_SE_FAIL SE indicated error
 * \retval BLOCK2GO_ENCRYPTED_KEY_IMPORT_TOO_LITTLE_DATA unexpectedly
 * short/long response
 * \retval others indicate failures from lower layers
 */
int block2go_generate_signature_permanent(Protocol *protocol, uint8_t key_index,
                                uint8_t data_to_sign[32],
                                uint32_t *global_counter, uint32_t *counter,
                                uint8_t **signature, size_t *signature_len);

/**
 * \brief Allocates storage of given size (between 01H to 400H) in persistent
 * memory to store metadata for a given key.
 *
 * \param[in] protocol          instance of activated protocol to use
 * \param[in] key_index         key to be labelled
 * \param[in] key_label_length  length of key label
 * \param[out] memory           remaining persistent memory
 *
 * \retval BLOCK2GO_CREATE_KEY_LABEL_SUCCESS in case of success
 * \retval BLOCK2GO_CREATE_KEY_LABEL_SE_FAIL SE indicated error
 * \retval BLOCK2GO_CREATE_KEY_LABEL_TOO_LITTLE_DATA unexpectedly
 * short/long response
 * \retval others indicate failures from lower layers
 */
int block2go_create_key_label(Protocol *protocol, uint8_t key_index,
                              uint16_t key_label_length, uint32_t *memory);

/**
 * \brief Sets or resets the label of a given key.
 *
 * \param[in] protocol    instance of activated protocol to use
 * \param[in] key_index   key index for which label should be set/updated
 * \param[in] key_label   key label
 * \param[in] key_label_length length of key label in byte
 *
 * \retval BLOCK2GO_UPDATE_KEY_LABEL_SUCCESS in case of success
 * \retval BLOCK2GO_UPDATE_KEY_LABEL_SE_FAIL SE indicated error
 * \retval BLOCK2GO_UPDATE_KEY_LABEL_TOO_LITTLE_DATA unexpectedly
 * short/long response
 * \retval others indicate failures from lower layers
 */
int block2go_update_key_label(Protocol *protocol, uint8_t key_index,
                              uint8_t *key_label, uint16_t key_label_length);

/**
 * \brief Returns key label of a given key index.
 *
 * \param[in] protocol    instance of activated protocol to use
 * \param[in] key_index   key index for which label should be returned
 * \param[out] key_label  buffer to copy key label into
 * \param[out] key_label_length buffer to copy length of key label into
 *
 * \note key_label has to be freed by the caller
 *
 * \retval BLOCK2GO_GET_KEY_LABEL_SUCCESS in case of success
 * \retval BLOCK2GO_GET_KEY_LABEL_SE_FAIL SE indicated error
 * \retval BLOCK2GO_GET_KEY_LABEL_TOO_LITTLE_DATA unexpectedly
 * short/long response
 * \retval others indicate failures from lower layers
 */
int block2go_get_key_label(Protocol *protocol, uint8_t key_index,
                           uint8_t **key_label, uint16_t *key_label_length);

/**
 * \brief Returns a random number having a given length.
 *
 * \param[in] protocol    instance of activated protocol to use
 * \param[in] length      length of random number
 * \param[out] random_num buffer to copy random number into
 *
 * \note random_num has to be freed by the caller
 *
 * \retval BLOCK2GO_GET_RANDOM_SUCCESS in case of success
 * \retval BLOCK2GO_GET_RANDOM_SE_FAIL SE indicated error
 * \retval BLOCK2GO_GET_RANDOM_TOO_LITTLE_DATA unexpectedly
 * short/long response
 * \retval others indicate failures from lower layers
 */
int block2go_get_random(Protocol *protocol, uint8_t length,
                        uint8_t **random_num);

/**
 * \brief Checks whether a given ECDSA signature is valid.
 *
 * \param[in] protocol    instance of activated protocol to use
 * \param[in] curve       ECC-curve used
 * \param[in] message     hashed message
 * \param[in] message_len length of message in bytes
 * \param[in] signature   ANS.1 DER encoded signature which is to be verifed
 * \param[in] public_key  Sec1 encoded umcompressed public key (65 bytes)
 *
 * \retval BLOCK2GO_VERIFY_SIGNATURE_SUCCESS in case of success
 * \retval BLOCK2GO_VERIFY_SIGNATURE_SE_FAIL SE indicated error
 * \retval BLOCK2GO_VERIFY_SIGNATURE_TOO_LITTLE_DATA unexpectedly
 * short/long response
 * \retval others indicate failures from lower layers
 */
int block2go_verify_signature(Protocol *protocol, block2go_curve curve,
                              uint8_t *message, uint8_t message_len,
                              uint8_t *signature,
                              uint8_t public_key[BLOCK2GO_PUBLIC_KEY_LEN]);

/**
 * \brief Irreversibly enables protected Mode configuration.
 *
 * \param[in] protocol  instance of activated protocol to use
 *
 * \retval BLOCK2GO_ENABLE_PROTECTED_MODE_SUCCESS in case of success
 * \retval BLOCK2GO_ENABLE_PROTECTED_MODE_SE_FAIL SE indicated error
 * \retval BLOCK2GO_ENABLE_PROTECTED_MODE_TOO_LITTLE_DATA unexpectedly
 * short/long response
 * \retval others indicate failures from lower layers
 */
int block2go_enable_protected_mode(Protocol *protocol);

/**
 * \brief Retrives current security status.
 *
 * \param[in] protocol  instance of activated protocol to use
 * \param[out] status_info buffer to copy status info into
 * 
 * \retval BLOCK2GO_GET_STATUS_SUCCESS in case of success
 * \retval BLOCK2GO_GET_STATUS_SE_FAIL SE indicated error
 * \retval BLOCK2GO_GET_STATUS_TOO_LITTLE_DATA unexpectedly
 * short/long response
 * \retval others indicate failures from lower layers
 */
int block2go_get_status(Protocol *protocol, block2go_session_type *status_info);
