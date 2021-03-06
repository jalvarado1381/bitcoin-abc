// Copyright (c) 2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_WALLET_RPCWALLET_H
#define BITCOIN_WALLET_RPCWALLET_H

#include <script/sighashtype.h>

#include <memory>
#include <string>
#include <vector>

class Config;
class CRPCTable;
class CTransaction;
class CWallet;
class JSONRPCRequest;
struct PartiallySignedTransaction;
class UniValue;

namespace interfaces {
class Chain;
class Handler;
} // namespace interfaces

void RegisterWalletRPCCommands(
    interfaces::Chain &chain,
    std::vector<std::unique_ptr<interfaces::Handler>> &handlers);

/**
 * Figures out what wallet, if any, to use for a JSONRPCRequest.
 *
 * @param[in] request JSONRPCRequest that wishes to access a wallet
 * @return NULL if no wallet should be used, or a pointer to the CWallet
 */
std::shared_ptr<CWallet>
GetWalletForJSONRPCRequest(const JSONRPCRequest &request);

std::string HelpRequiringPassphrase(CWallet *);
void EnsureWalletIsUnlocked(CWallet *);
bool EnsureWalletIsAvailable(CWallet *, bool avoidException);

UniValue signrawtransactionwithwallet(const Config &config,
                                      const JSONRPCRequest &request);
UniValue getaddressinfo(const Config &config, const JSONRPCRequest &request);

#endif // BITCOIN_WALLET_RPCWALLET_H
