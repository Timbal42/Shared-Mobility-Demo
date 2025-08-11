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

/** \file status.h
 * Contains IFX error identifiers.
 */
#pragma once

#include "ifx/error.h"

/**
 * \brief IFX error code module identifer
 */
#define LIBBLOCK2GO 0xaf



/**
 * \brief IFX error reason for unsuccessful card behavior
 */
#define CARD_FAIL 0x00

/**
 * \brief IFX error reason for missing key label tag
 */
#define KEY_LABEL_TAG_MISSING 0x01

/**
 * \brief IFX error reason for invalid data length
 */
#define INVALID_DATA_LENGTH 0x02

/**
 * \brief IFX error code function identifier for block2go_select()
 */
#define BLOCK2GO_SELECT 0x01



/**
 * \brief IFX error code function identifier for block2go_generate_key_permanent() and block2go_generate_key_session()
 */
#define BLOCK2GO_GENERATE_KEY 0x02

/**
 * \brief IFX error code function identifier for block2go_get_key_info_permanent() and block2go_get_key_info_session()
 */
#define BLOCK2GO_GET_KEY_INFO 0x03

/**
 * \brief IFX error code function identifier for block2go_encrypted_keyimport()
 */
#define BLOCK2GO_ENCRYPTED_KEYIMPORT 0x04

/**
 * \brief IFX error code function identifier for block2go_generate_signature_permanent() and block2go_generate_signature_session()
 */
#define BLOCK2GO_GENERATE_SIGNATURE 0x05

/**
 * \brief IFX error code function identifier for block2go_create_key_label()
 */
#define BLOCK2GO_CREATE_KEY_LABEL 0x06

/**
 * \brief IFX error code function identifier for block2go_update_key_label()
 */
#define BLOCK2GO_UPDATE_KEY_LABEL 0x07

/**
 * \brief IFX error code function identifier for block2go_get_key_label()
 */
#define BLOCK2GO_GET_KEY_LABEL 0x08

/**
 * \brief IFX error code function identifier for block2go_get_random()
 */
#define BLOCK2GO_GET_RANDOM 0x09

/**
 * \brief IFX error code function identifier for block2go_verify_signature()
 */
#define BLOCK2GO_VERIFY_SIGNATURE 0x0A

/**
 * \brief IFX error code function identifier for block2go_enable_protected_mode()
 */
#define BLOCK2GO_ENABLE_PROTECTED_MODE 0x0B

/**
 * \brief IFX error code function identifier for block2go_get_status()
 */
#define BLOCK2GO_GET_STATUS 0x0C



/**
 * \brief Return code for successful calls of block2go_select()
 */
#define BLOCK2GO_SELECT_SUCCESS SUCCESS

/**
 * \brief Return code for successful calls of block2go_generate_key_permanent() and block2go_generate_key_session()
 */
#define BLOCK2GO_GENERATE_KEY_SUCCESS SUCCESS

/**
 * \brief Return code for successful calls of block2go_get_key_info_permanent() and block2go_get_key_info_session()
 */
#define BLOCK2GO_GET_KEY_INFO_SUCCESS SUCCESS

/**
 * \brief Return code for successful calls of block2go_encrypted_keyimport()
 */
#define BLOCK2GO_ENCRYPTED_KEYIMPORT_SUCCESS SUCCESS

/**
 * \brief Return code for successful calls of block2go_generate_signature_permanent() and block2go_generate_signature_session()
 */
#define BLOCK2GO_GENERATE_SIGNATURE_SUCCESS SUCCESS

/**
 * \brief Return code for successful calls of block2go_create_key_label()
 */
#define BLOCK2GO_CREATE_KEY_LABEL_SUCCESS SUCCESS

/**
 * \brief Return code for successful calls of block2go_update_key_label()
 */
#define BLOCK2GO_UPDATE_KEY_LABEL_SUCCESS SUCCESS

/**
 * \brief Return code for successful calls of block2go_get_key_label()
 */
#define BLOCK2GO_GET_KEY_LABEL_SUCCESS SUCCESS

/**
 * \brief Return code for successful calls of block2go_get_random()
 */
#define BLOCK2GO_GET_RANDOM_SUCCESS SUCCESS

/**
 * \brief Return code for successful calls of block2go_verify_signature()
 */
#define BLOCK2GO_VERIFY_SIGNATURE_SUCCESS SUCCESS

/**
 * \brief Return code for successful calls of block2go_enable_protected_mode()
 */
#define BLOCK2GO_ENABLE_PROTECTED_MODE_SUCCESS SUCCESS

/**
 * \brief Return code for successful calls of block2go_get_status()
 */
#define BLOCK2GO_GET_STATUS_SUCCESS SUCCESS



/**
 * \brief IFX error code for unsuccessful call of block2go_select() due to card failure
 */
#define BLOCK2GO_SELECT_SE_FAIL                                                \
  IFX_ERROR(LIBBLOCK2GO, BLOCK2GO_SELECT, CARD_FAIL)

/**
 * \brief IFX error code for unsuccessful call of block2go_select() due to invalid data length
 */
#define BLOCK2GO_SELECT_INVALID_DATA_LENGTH                                    \
  IFX_ERROR(LIBBLOCK2GO, BLOCK2GO_SELECT, INVALID_DATA_LENGTH)

/**
 * \brief IFX error code for unsuccessful call of block2go_generate_key_permanent() or 
 * block2go_generate_key_session() due to card failure
 */
#define BLOCK2GO_GENERATE_KEY_SE_FAIL                                          \
  IFX_ERROR(LIBBLOCK2GO, BLOCK2GO_GENERATE_KEY, CARD_FAIL)

/**
 * \brief IFX error code for unsuccessful call of block2go_generate_key_permanent() or 
 * block2go_generate_key_session() due to invalid data length
 */
#define BLOCK2GO_GENERATE_KEY_INVALID_DATA_LENGTH                              \
  IFX_ERROR(LIBBLOCK2GO, BLOCK2GO_GENERATE_KEY, INVALID_DATA_LENGTH)

/**
 * \brief IFX error code for unsuccessful call of block2go_get_key_info_permanent() or 
 * block2go_get_key_info_session() due to card failure
 */
#define BLOCK2GO_GET_KEY_INFO_SE_FAIL                                          \
  IFX_ERROR(LIBBLOCK2GO, BLOCK2GO_GET_KEY_INFO, CARD_FAIL)

/**
 * \brief IFX error code for unsuccessful call of block2go_get_key_info_permanent() or 
 * block2go_get_key_info_session() due to invalid data length
 */
#define BLOCK2GO_GET_KEY_INFO_INVALID_DATA_LENGTH                              \
  IFX_ERROR(LIBBLOCK2GO, BLOCK2GO_GET_KEY_INFO, INVALID_DATA_LENGTH)

/**
 * \brief IFX error code for unsuccessful call of block2go_encrypted_keyimport() 
 * due to card failure
 */
#define BLOCK2GO_ENCRYPTED_KEYIMPORT_FAIL                                      \
  IFX_ERROR(LIBBLOCK2GO, BLOCK2GO_ENCRYPTED_KEYIMPORT, CARD_FAIL)

/**
 * \brief IFX error code for unsuccessful call of block2go_encrypted_keyimport() 
 * due to invalid data length
 */
#define BLOCK2GO_ENCRYPTED_KEYIMPORT_INVALID_DATA_LENGTH                       \
  IFX_ERROR(LIBBLOCK2GO, BLOCK2GO_ENCRYPTED_KEYIMPORT, INVALID_DATA_LENGTH)

/**
 * \brief IFX error code for unsuccessful call of block2go_generate_signature_permanent() 
 * or block2go_generate_signature_session() due to card failure
 */
#define BLOCK2GO_GENERATE_SIGNATURE_FAIL                                       \
  IFX_ERROR(LIBBLOCK2GO, BLOCK2GO_GENERATE_SIGNATURE, CARD_FAIL)

/**
 * \brief IFX error code for unsuccessful call of block2go_generate_signature_permanent() 
 * or block2go_generate_signature_session() due to invalid data length
 */
#define BLOCK2GO_GENERATE_SIGNATURE_INVALID_DATA_LENGTH                        \
  IFX_ERROR(LIBBLOCK2GO, BLOCK2GO_GENERATE_SIGNATURE, INVALID_DATA_LENGTH)

/**
 * \brief IFX error code for unsuccessful call of block2go_create_key_label() 
 * due to card failure
 */
#define BLOCK2GO_CREATE_KEY_LABEL_FAIL                                         \
  IFX_ERROR(LIBBLOCK2GO, BLOCK2GO_CREATE_KEY_LABEL, CARD_FAIL)

/**
 * \brief IFX error code for unsuccessful call of block2go_create_key_label() 
 * due to invalid data length
 */
#define BLOCK2GO_CREATE_KEY_LABEL_INVALID_DATA_LENGTH                          \
  IFX_ERROR(LIBBLOCK2GO, BLOCK2GO_CREATE_KEY_LABEL, INVALID_DATA_LENGTH)

/**
 * \brief IFX error code for unsuccessful call of block2go_update_key_label() 
 * due to card failure
 */
#define BLOCK2GO_UPDATE_KEY_LABEL_FAIL                                         \
  IFX_ERROR(LIBBLOCK2GO, BLOCK2GO_UPDATE_KEY_LABEL, CARD_FAIL)

/**
 * \brief IFX error code for unsuccessful call of block2go_update_key_label() 
 * due to invalid data length
 */
#define BLOCK2GO_UPDATE_KEY_LABEL_INVALID_DATA_LENGTH                              \
  IFX_ERROR(LIBBLOCK2GO, BLOCK2GO_UPDATE_KEY_LABEL, INVALID_DATA_LENGTH)

/**
 * \brief IFX error code for unsuccessful call of block2go_update_key_label() 
 * due to too large label length
 */
#define BLOCK2GO_UPDATE_KEY_LABEL_OUT_OF_MEMORY                                \
  IFX_ERROR(LIBBLOCK2GO, BLOCK2GO_UPDATE_KEY_LABEL, OUT_OF_MEMORY)

/**
 * \brief IFX error code for unsuccessful call of block2go_get_key_label() 
 * due to card failure
 */
#define BLOCK2GO_GET_KEY_LABEL_FAIL                                            \
  IFX_ERROR(LIBBLOCK2GO, BLOCK2GO_GET_KEY_LABEL, CARD_FAIL)

/**
 * \brief IFX error code for unsuccessful call of block2go_get_key_label() 
 * due to invalid data length
 */
#define BLOCK2GO_GET_KEY_LABEL_INVALID_DATA_LENGTH                                 \
  IFX_ERROR(LIBBLOCK2GO, BLOCK2GO_GET_KEY_LABEL, INVALID_DATA_LENGTH)

/**
 * \brief IFX error code for unsuccessful call of block2go_get_key_label() 
 * due to missing key label tag
 */
#define BLOCK2GO_GET_KEY_LABEL_KEY_LABEL_TAG_MISSING                                 \
  IFX_ERROR(LIBBLOCK2GO, BLOCK2GO_GET_KEY_LABEL, KEY_LABEL_TAG_MISSING)

/**
 * \brief IFX error code for unsuccessful call of block2go_get_random() 
 * due to card failure
 */
#define BLOCK2GO_GET_RANDOM_FAIL                                               \
  IFX_ERROR(LIBBLOCK2GO, BLOCK2GO_GET_RANDOM, CARD_FAIL)

/**
 * \brief IFX error code for unsuccessful call of block2go_get_random() 
 * due to invalid data length
 */
#define BLOCK2GO_GET_RANDOM_INVALID_DATA_LENGTH                                    \
  IFX_ERROR(LIBBLOCK2GO, BLOCK2GO_GET_RANDOM, INVALID_DATA_LENGTH)

/**
 * \brief IFX error code for unsuccessful call of block2go_verify_signaure()
 * due to card failure
 */
#define BLOCK2GO_VERIFY_SIGNATURE_FAIL                                         \
  IFX_ERROR(LIBBLOCK2GO, BLOCK2GO_VERIFY_SIGNATURE, CARD_FAIL)

/**
 * \brief IFX error code for unsuccessful call of block2go_verify_signaure()
 * due to invalid data length
 */
#define BLOCK2GO_VERIFY_SIGNATURE_INVALID_DATA_LENGTH                              \
  IFX_ERROR(LIBBLOCK2GO, BLOCK2GO_VERIFY_SIGNATURE, INVALID_DATA_LENGTH)

/**
 * \brief IFX error code for unsuccessful call of block2go_enable_protected_mode()
 * due to card failure
 */
#define BLOCK2GO_ENABLE_PROTECTED_MODE_FAIL                                    \
  IFX_ERROR(LIBBLOCK2GO, BLOCK2GO_ENABLE_PROTECTED_MODE, CARD_FAIL)

/**
 * \brief IFX error code for unsuccessful call of block2go_enable_protected_mode()
 * due to invalid data length
 */
#define BLOCK2GO_ENABLE_PROTECTED_MODE_INVALID_DATA_LENGTH                         \
  IFX_ERROR(LIBBLOCK2GO, BLOCK2GO_ENABLE_PROTECTED_MODE, INVALID_DATA_LENGTH)

/**
 * \brief IFX error code for unsuccessful call of block2go_get_status()
 * due to card fail
 */
#define BLOCK2GO_GET_STATUS_FAIL                                               \
  IFX_ERROR(LIBBLOCK2GO, BLOCK2GO_GET_STATUS, CARD_FAIL)

/**
 * \brief IFX error code for unsuccessful call of block2go_get_status()
 * due to invalid data length
 */
#define BLOCK2GO_GET_STATUS_INVALID_DATA_LENGTH                                    \
  IFX_ERROR(LIBBLOCK2GO, BLOCK2GO_GET_STATUS, INVALID_DATA_LENGTH)

  