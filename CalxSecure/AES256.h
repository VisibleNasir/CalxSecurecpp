// AES256.h
#ifndef AES256_H
#define AES256_H

#include <QByteArray>
#include <QString>

class AES256
{
public:
    explicit AES256(QObject* parent = nullptr);

    // Encrypt using AES-256-GCM
    bool encrypt(const QByteArray& plaintext, const QByteArray& key,
        QByteArray& ciphertextOut, QByteArray& nonceOut, QByteArray& tagOut);

    // Decrypt using AES-256-GCM
    bool decrypt(const QByteArray& ciphertext, const QByteArray& key,
        const QByteArray& nonce, const QByteArray& tag,
        QByteArray& plaintextOut);

private:
    // You can use OpenSSL or Qt's cryptographic functions.
    // For simplicity and Qt-only, we'll use a placeholder.
    // In production, use OpenSSL (libcrypto) for real AES-GCM.
    bool aesGcmEncrypt(const QByteArray& key, const QByteArray& iv,
        const QByteArray& plaintext, QByteArray& ciphertext, QByteArray& tag);
    bool aesGcmDecrypt(const QByteArray& key, const QByteArray& iv,
        const QByteArray& ciphertext, const QByteArray& tag, QByteArray& plaintext);
};

#endif // AES256_H