/*
 * aes.h
 *
 *  Created on: Oct 30, 2012
 *      Author: mayur
 */

#include <openssl/aes.h>

#ifndef AES_H_
#define AES_H_
#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))

extern const  int AES_KEY_LENGTH_IN_BITS;
extern const  int AES_KEY_LENGTH_IN_CHARS;

void class_AES_set_encrypt_key(unsigned char *key_text, AES_KEY *enc_key);
void class_AES_set_decrypt_key(unsigned char *key_text, AES_KEY *dec_key);

void class_AES_encrypt_with_padding(unsigned char *in, int len, unsigned char **out, int *out_len, AES_KEY *enc_key);
void class_AES_decrypt_with_padding(unsigned char *in, int len, unsigned char **out, int *out_len, AES_KEY *dec_key);


#endif /* AES_H_ */
