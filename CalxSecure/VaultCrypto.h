// VaultCrypto.h
#ifndef VAULTCRYPTO_H
#define VAULTCRYPTO_H

#include <QByteArray>
#include <QString>
#include <optional>
#include "AES256.h"
#include "dpapi_utils.h"   // If you have Windows DPAPI

class VaultCrypto
{
public:
    static QByteArray readProtectedVaultKey();
    static QByteArray getUnprotectedVaultKey(QString& errorMessageOut);
    static bool isValidVaultKey(const QByteArray& key, QString& errorOut);
    static QByteArray getValidVaultKey(QString& errorMessageOut);
    static std::optional<QByteArray> getValidVaultKey();

    // Encrypt/Decrypt sensitive data (passwords, transaction details, etc.)
    static bool encryptData(const QString& plaintext, QByteArray& ciphertextOut,
        QByteArray& nonceOut, QByteArray& tagOut, QString& errorMessageOut);

    static bool decryptData(const QByteArray& ciphertext, const QByteArray& nonce,
        const QByteArray& tag, QString& plaintextOut, QString& errorMessageOut);

private:
    static QByteArray generateVaultKey();
};

#endif // VAULTCRYPTO_H