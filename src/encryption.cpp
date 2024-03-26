#include <openssl/err.h>
#include <openssl/evp.h>

#include <cstring>
#include <iostream>
#include <vector>

class ImageEncryptor {
 public:
  static constexpr int AES_KEY_LENGTH = 32;     // 256 bits = 32 bytes
  static constexpr size_t AES_BLOCK_SIZE = 16;  // 128 bits = 16 bytes
  ImageEncryptor(const unsigned char* key, const unsigned char* iv) {
    memcpy(this->encryptionKey, key, AES_KEY_LENGTH);
    memcpy(this->iv, iv, AES_BLOCK_SIZE);
  }

  // ToDo: change const unsigned char* to File* for file input? or stringStream?
  std::vector<unsigned char> encrypt(const unsigned char* plaintext,
                                     int plaintext_len) {
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    std::vector<unsigned char> ciphertext(plaintext_len + AES_BLOCK_SIZE);
    int len;

    if (!ctx) {
      throw std::runtime_error("Failed to create EVP_CIPHER_CTX");
    }

    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_ctr(), NULL, encryptionKey, iv) != 1) {
      EVP_CIPHER_CTX_free(ctx);
      throw std::runtime_error("EVP_EncryptInit_ex failed");
    }

    if (EVP_EncryptUpdate(ctx, ciphertext.data(), &len, plaintext, plaintext_len) != 1) {
      EVP_CIPHER_CTX_free(ctx);
      throw std::runtime_error("EVP_EncryptUpdate failed");
    }

    int ciphertext_len = len;

    if (EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len) != 1) {
      EVP_CIPHER_CTX_free(ctx);
      throw std::runtime_error("EVP_EncryptFinal_ex failed");
    }

    ciphertext_len += len;

    EVP_CIPHER_CTX_free(ctx);
    ciphertext.resize(ciphertext_len);
    return ciphertext;
  }

  std::vector<unsigned char> generateHMAC(const unsigned char* data,
                                          int data_len,
                                          const unsigned char* hmacKey) {
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    std::vector<unsigned char> hmac(EVP_MD_size(EVP_sha256()));
    size_t len;

    if (!ctx) {
      throw std::runtime_error("Failed to create EVP_MD_CTX");
    }

    if (1 != EVP_DigestInit_ex(ctx, EVP_sha256(), NULL)) {
      EVP_MD_CTX_free(ctx);
      throw std::runtime_error("EVP_DigestInit_ex failed");
    }

    if (1 != EVP_DigestSignInit(ctx, NULL, EVP_sha256(), NULL, NULL)) {
      EVP_MD_CTX_free(ctx);
      throw std::runtime_error("EVP_DigestSignInit failed");
    }

    if (1 != EVP_DigestSignUpdate(ctx, data, data_len)) {
      EVP_MD_CTX_free(ctx);
      throw std::runtime_error("EVP_DigestSignUpdate failed");
    }

    if (1 != EVP_DigestSignFinal(ctx, hmac.data(), &len)) {
      EVP_MD_CTX_free(ctx);
      throw std::runtime_error("EVP_DigestSignFinal failed");
    }

    EVP_MD_CTX_free(ctx);
    hmac.resize(len);
    return hmac;
  }

 private:
  unsigned char encryptionKey[AES_KEY_LENGTH];
  unsigned char iv[AES_BLOCK_SIZE];
};

// Same as above
void encrypt(const unsigned char* key, const unsigned char* iv, const unsigned char* hmacKey,
            const unsigned char* plaintext, int plaintext_len) {

  ImageEncryptor encryptor(key, iv);
  try {
    auto ciphertext = encryptor.encrypt(plaintext, plaintext_len);
    auto hmac =
        encryptor.generateHMAC(ciphertext.data(), ciphertext.size(), hmacKey);

    std::cout << "Encryption and HMAC generation complete." << std::endl;
    // Process ciphertext and HMAC as needed
  } catch (const std::exception& e) {
    std::cerr << "Operation failed: " << e.what() << std::endl;
    throw e;
  }
}

void decrypt(const unsigned char* key, const unsigned char* iv, const unsigned char* hmacKey,
            const unsigned char* ciphertext, int ciphertext_len) {
  ImageEncryptor encryptor(key, iv);
  
  try {
    auto hmac = encryptor.generateHMAC(ciphertext, ciphertext_len, hmacKey);
    // Verify HMAC
    auto plaintext = encryptor.encrypt(ciphertext, ciphertext_len);
    std::cout << "Decryption complete." << std::endl;
    // Process plaintext as needed
  } catch (const std::exception& e) {
    std::cerr << "Operation failed: " << e.what() << std::endl;
    throw e;
  }
}