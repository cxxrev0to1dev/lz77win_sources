// openssl_ecies.cpp : Defines the entry point for the console application.
//

#define _CRTDBG_MAP_ALLOC
#include<stdlib.h>
#include<crtdbg.h>
#pragma comment(lib,"libcrypto.lib")
#pragma comment(lib,"libssl.lib")
#include <openssl/err.h>
#include <openssl/ec.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/engine.h>
#include <openssl/rand.h>
#include <openssl/bio.h>
#include <openssl/bn.h>
#include <openssl/hmac.h>
#include <openssl/ssl.h>
#include "bignum_key.h"
#include "encrypt_message.h"
#include "decrypt_message.h"
#include "encrypt_file.h"
#include "decrypt_file.h"
/*
const char pkey[] = "-----BEGIN PUBLIC KEY-----\n" \
"MFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAEgv/5+wE/2gT5ANCpZ9kIy3nKgcqH\n" \
"YDyNC/d0bgf9Pdaus40CXjIZukF1uUAWAo4GTqG7uDatXWOzq1MIDgE6bA==\n" \
"-----END PUBLIC KEY-----\n";
const char privkey[] = "-----BEGIN EC PRIVATE KEY-----\n" \
"MHcCAQEEILmUAvLj7GGSykQxpHeems8Yjw03V7ACBmD8udiNm92soAoGCCqGSM49\n" \
"AwEHoUQDQgAEgv/5+wE/2gT5ANCpZ9kIy3nKgcqHYDyNC/d0bgf9Pdaus40CXjIZ\n" \
"ukF1uUAWAo4GTqG7uDatXWOzq1MIDgE6bA==\n" \
"-----END EC PRIVATE KEY-----\n";
*/
const char pkey[] = "-----BEGIN PUBLIC KEY-----\n" \
"MFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAEXnjPOhOPTU+XJtkzjNcB7i1KYlNx\n" \
"Ii0yr/dtpj2u1G9tAzck11HHGpMBxA2CyG0UjhLc6/75lwGhfzVftaiznw==\n" \
"-----END PUBLIC KEY-----\n";
const char privkey[] = "-----BEGIN EC PRIVATE KEY-----\n" \
"MHcCAQEEINQOJdOG8Gi6AM2GcfNHiTJE0EF8KvO9YmYVha72110ioAoGCCqGSM49\n" \
"AwEHoUQDQgAEXnjPOhOPTU+XJtkzjNcB7i1KYlNxIi0yr/dtpj2u1G9tAzck11HH\n" \
"GpMBxA2CyG0UjhLc6/75lwGhfzVftaiznw==\n" \
"-----END EC PRIVATE KEY-----\n";
int wmain(int argc, wchar_t* argv[])
{
#ifdef __DEBUG
  _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
#endif
  //https://github.com/cmouse/ecies/blob/master/ecies.c
  //openssl ecparam -genkey -name prime256v1 -noout -out ecprivkey.pem
  //openssl ec -in ecprivkey.pem -pubout -out ecpubkey.pem
  SSL_library_init();
  OpenSSL_add_all_algorithms();
  ERR_load_crypto_strings();

  Crypt::ECKeyGen key_gen;
  Crypt::EncryptFile encrypt_file;
  const std::string pubk = "045E78CF3A138F4D4F9726D9338CD701EE2D4A625371222D32AFF76DA63DAED46F6D033724D751C71A9301C40D82C86D148E12DCEBFEF99701A17F355FB5A8B39F";
  const std::string privk = "D40E25D386F068BA00CD8671F347893244D0417C2AF3BD62661585AEF6D75D22";
  encrypt_file.CryptFile(pubk,"D:\\workspace\\p2pupgrd\\src\\PCHunter64.exe","D:\\workspace\\p2pupgrd\\src\\PCHunter64.dst");

  Crypt::DecryptFile decrypt_file;
  decrypt_file.CryptFile(privk, "D:\\workspace\\p2pupgrd\\src\\PCHunter64.dst", "D:\\workspace\\p2pupgrd\\src\\PCHunter641.exe");

  /*
  const std::string msg = "super secret messagexxxxxxxxxxxxxxx";
  const std::string public_key = pkey;
  Crypt::EncryptMessage encrypt_message(key_gen);
  encrypt_message.EncryptString(msg);
  std::vector<std::uint8_t> salt = encrypt_message.salt();
  std::vector<std::uint8_t> checksum = encrypt_message.checksum();
  std::vector<std::uint8_t> ciphertext = encrypt_message.cipher_text();
  const std::string private_key = privkey;
  Crypt::DecryptMessage decrypt_message(key_gen);
  decrypt_message.SetSalt(salt);
  decrypt_message.SetChecksum(checksum);
  decrypt_message.SetPublicR(encrypt_message.public_r());
  decrypt_message.DecryptBytes(ciphertext);
  */
  //CONF_modules_unload(1);        //for conf  
  EVP_cleanup();                 //For EVP  
  ENGINE_cleanup();              //for engine  
  CRYPTO_cleanup_all_ex_data();  //generic   
  ERR_remove_state(0);           //for ERR  
  ERR_free_strings();            //for ERR  
	return 0;
}