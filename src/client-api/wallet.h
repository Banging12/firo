// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "univalue.h"
#include <script/ismine.h>
#include <wallet/wallet.h>

UniValue FormatWalletTxForClientAPI(CWalletDB &db, const CWalletTx &wtx);

bool GetCoinControl(const UniValue& data, CCoinControl& cc);
bool doesWalletHaveMnemonics();

extern std::atomic<bool> fHasSentInitialStateWallet;
// This is used to prevent slowdowns during autoMintlelantus resulting from the balance being updated many times.
extern std::atomic<bool> fBalancePublishingEmbargo;