// VaultCrypto.cpp
#include "VaultCrypto.h"
#include <QSettings>
#include <QRandomGenerator>
#include <QDebug>

QByteArray VaultCrypto::generateVaultKey()
{
    QByteArray key(32, 0);
    QRandomGenerator::global()->fillRange(reinterpret_cast<quint32*>(key.data()), 8);
    return key;
}

QByteArray VaultCrypto::readProtectedVaultKey()
{
    QSettings settings("CalxSecure", "CalxSecureApp");
    return QByteArray::fromBase64(settings.value("session/protectedVaultKey").toByteArray());
}

QByteArray VaultCrypto::getUnprotectedVaultKey(QString& errorMessageOut)
{
    QByteArray protectedBlob = readProtectedVaultKey();
    if (protectedBlob.isEmpty()) {
        errorMessageOut = "No protected vault key found";
        return {};
    }

    QByteArray vaultKey;
    if (!unprotectWithDPAPI(protectedBlob, vaultKey)) {   // Your DPAPI function
        errorMessageOut = "Failed to unprotect vault key using DPAPI";
        return {};
    }

    if (vaultKey.size() != 32) {
        errorMessageOut = "Invalid vault key size";
        vaultKey.fill(0);
        return {};
    }

    return vaultKey;
}

bool VaultCrypto::isValidVaultKey(const QByteArray& key, QString& errorOut)
{
    if (key.size() != 32) {
        errorOut = QString("Invalid key size: %1 bytes").arg(key.size());
        return false;
    }
    return true;
}

QByteArray VaultCrypto::getValidVaultKey(QString& errorMessageOut)
{
    QByteArray key = getUnprotectedVaultKey(errorMessageOut);
    if (key.isEmpty() || !isValidVaultKey(key, errorMessageOut)) {
        return {};
    }
    return key;
}

std::optional<QByteArray> VaultCrypto::getValidVaultKey()
{
    QString error;
    QByteArray key = getValidVaultKey(error);
    if (key.isEmpty()) return std::nullopt;
    return key;
}

bool VaultCrypto::encryptData(const QString& plaintext, QByteArray& ciphertextOut,
    QByteArray& nonceOut, QByteArray& tagOut, QString& errorMessageOut)
{
    auto maybeKey = getValidVaultKey();
    if (!maybeKey.has_value()) {
        errorMessageOut = "Vault key unavailable";
        return false;
    }

    AES256 aes;
    if (!aes.encrypt(plaintext.toUtf8(), *maybeKey, ciphertextOut, nonceOut, tagOut)) {
        errorMessageOut = "AES-256-GCM encryption failed";
        return false;
    }
    return true;
}

bool VaultCrypto::decryptData(const QByteArray& ciphertext, const QByteArray& nonce,
    const QByteArray& tag, QString& plaintextOut, QString& errorMessageOut)
{
    auto maybeKey = getValidVaultKey();
    if (!maybeKey.has_value()) {
        errorMessageOut = "Vault key unavailable";
        return false;
    }

    AES256 aes;
    QByteArray plain;
    if (!aes.decrypt(ciphertext, *maybeKey, nonce, tag, plain)) {
        errorMessageOut = "AES-256-GCM decryption failed";
        return false;
    }

    plaintextOut = QString::fromUtf8(plain);
    plain.fill(0);
    return true;
}