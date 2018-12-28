/**
 AES encryption/decryption demo program using OpenSSL EVP apis
 gcc -Wall openssl_aes.c -lcrypto
 
 this is public domain code.
 
 Saju Pillai (saju.pillai@gmail.com)
 **/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <openssl/evp.h>
#include <openssl/aes.h>

/**
 * Create an 128 bit key and IV using the supplied key_data. salt can be added for taste.
 * Fills in the encryption and decryption ctx objects and returns 0 on success
 **/
int aes_init(EVP_CIPHER_CTX *encryption_ctx,
             EVP_CIPHER_CTX *decryption_ctx)
{
	int i, nrounds = 5;
	unsigned char key[16] = "azertyuiopqsdfg";
	unsigned char iv[16] = "azertyuiopqsdfg";
	
	// Init encryption context
	EVP_CIPHER_CTX_init(encryption_ctx);
	EVP_EncryptInit_ex(encryption_ctx, EVP_aes_128_cbc(), NULL, key, iv);
	
	// Init decryption context
	EVP_CIPHER_CTX_init(decryption_ctx);
	EVP_DecryptInit_ex(decryption_ctx, EVP_aes_128_cbc(), NULL, key, iv);
	
	return 0;
}

/*
 * Encrypt input_length bytes of data
 * All data going in & out is considered binary (unsigned char[])
 */
unsigned char *aes_encrypt(EVP_CIPHER_CTX *encryption_ctx, unsigned char *plaintext, int input_length, int *output_length)
{
	/* max ciphertext len for a n bytes of plaintext is n + AES_BLOCK_SIZE -1 bytes */
	int cipher_length = input_length + AES_BLOCK_SIZE, final_length = 0;
	unsigned char *ciphertext = malloc(cipher_length);
	
	/* allows reusing of 'e' for multiple encryption cycles */
	EVP_EncryptInit_ex(encryption_ctx, NULL, NULL, NULL, NULL);
	
	/* update ciphertext, cipher_length is filled with the length of ciphertext generated,
	 *input_length is the size of plaintext in bytes */
	EVP_EncryptUpdate(encryption_ctx, ciphertext, &cipher_length, plaintext, input_length);
	
	/* update ciphertext with the final remaining bytes */
	EVP_EncryptFinal_ex(encryption_ctx, ciphertext+cipher_length, &final_length);
	
	*output_length = cipher_length + final_length;
	return ciphertext;
}

/*
 * Decrypt *len bytes of ciphertext
 */
unsigned char *aes_decrypt(EVP_CIPHER_CTX *decryption_ctx, unsigned char *ciphertext, int input_length, int *output_length)
{
	/* because we have padding ON, we must allocate an extra cipher block size of memory */
	int myoutput_length = input_length, final_length = 0;
	unsigned char *plaintext = malloc(myoutput_length + AES_BLOCK_SIZE);
	
	EVP_DecryptInit_ex(decryption_ctx, NULL, NULL, NULL, NULL);
	EVP_DecryptUpdate(decryption_ctx, plaintext, &myoutput_length, ciphertext, input_length);
	EVP_DecryptFinal_ex(decryption_ctx, plaintext+myoutput_length, &final_length);
	
	*output_length = myoutput_length + final_length;
	return plaintext;
}

void printn(const char *data, unsigned length)
{
	for (int i = 0; i < length;i++)
		printf("%02x", data[i]);
}


int main(int argc, char **argv)
{
	/* "opaque" encryption, decryption ctx structures that libcrypto uses to record
     status of enc/dec operations */
	EVP_CIPHER_CTX encryption_ctx, decryption_ctx;
	char *input = "\nWho are you ?\nI am the 'Doctor'.\n'Doctor' who ?\nPrecisely!";
	
	/* gen key and iv. init the cipher ctx object */
	if (aes_init(&encryption_ctx, &decryption_ctx)) {
		printf("Couldn't initialize AES cipher\n");
		return -1;
	}
	
	/* encrypt and decrypt each input string and compare with the original */
	char *plaintext;
	unsigned char *ciphertext;
	int output_length, input_length;
	int deciphered_length;
	int ciphered_length;
	
	/* The enc/dec functions deal with binary data and not C strings. strlen() will
	 return length of the string without counting the '\0' string marker. We always
	 pass in the marker byte to the encrypt/decrypt functions so that after decryption
	 we end up with a legal C string */
	input_length = strlen(input)+1;
	
	ciphertext = aes_encrypt(&encryption_ctx, (unsigned char *)input, input_length, &ciphered_length);
	plaintext = (char *)aes_decrypt(&decryption_ctx, ciphertext, ciphered_length, &deciphered_length);

	printf("Initial plain len = %d ; Ciphered len = %d ; Deciphered len = %d\n", input_length, ciphered_length, deciphered_length);
	
	printf("Plain text:===%s===\n", input);
	printf("Ciphered:===");
	printn(ciphertext, ciphered_length);
	printf("===\n");
	printf("Deciphered:===%s===\n", plaintext);
	
	if (strncmp(plaintext, input, input_length))
		printf("FAIL: enc/dec failed for \"%s\"\n", input);
	else
		printf("OK: enc/dec ok for \"%s\"\n", plaintext);
	
	free(ciphertext);
	free(plaintext);
	
	EVP_CIPHER_CTX_cleanup(&encryption_ctx);
	EVP_CIPHER_CTX_cleanup(&decryption_ctx);
	
	return 0;
}

