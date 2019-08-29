// Copyright (c) 2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_PRIMITIVES_SIDECHAIN_H
#define BITCOIN_PRIMITIVES_SIDECHAIN_H

#include "amount.h"
#include "merkleblock.h"
#include "primitives/transaction.h"
#include "pubkey.h"
#include "script/script.h"
#include "serialize.h"
#include "uint256.h"

#include <limits.h>
#include <string>
#include <vector>

/* Sidechain Identifiers */

//! Sidechain address bytes
static const std::string SIDECHAIN_ADDRESS_BYTES = "6cbb79de8861f2cceb3dfc4a0e343571b9c3b7228a095a71c7aa8e83fe76527f";

//! Sidechain build commit hash
static const std::string SIDECHAIN_BUILD_COMMIT_HASH = "bb34f5688dce6006f053ed9536c9e972d515803e";

struct Sidechain {
    uint8_t nSidechain;
    CScript depositScript;

    std::string ToString() const;
    std::string GetSidechainName() const;
};

enum Sidechains {
    // This sidechain
    SIDECHAIN_TEST = 0,
};

//! WT status / zone (unspent, included in a WT^, paid out)
static const char WT_UNSPENT = 'a';
static const char WT_IN_WTPRIME = 'b';
static const char WT_SPENT = 'c';

//! KeyID for testing
static const std::string testkey = "b5437dc6a4e5da5597548cf87db009237d286636";
//mx3PT9t2kzCFgAURR9HeK6B5wN8egReUxY
//cN5CqwXiaNWhNhx3oBQtA8iLjThSKxyZjfmieTsyMpG6NnHBzR7J

//! This sidechain's fee script
static const CScript SIDECHAIN_FEESCRIPT = CScript() << OP_DUP << OP_HASH160 << ToByteVector(testkey) << OP_EQUALVERIFY << OP_CHECKSIG;

//! The default payment amount to mainchain miner for critical data commitment
static const CAmount DEFAULT_CRITICAL_DATA_AMOUNT = 1 * CENT;

//! The fee for sidechain deposits on this sidechain
static const CAmount SIDECHAIN_DEPOSIT_FEE = 0.00001 * COIN;

/**
 * Base object for sidechain related database entries
 */
struct SidechainObj {
    char sidechainop;
    uint32_t nHeight;
    uint256 txid;

    SidechainObj(void): nHeight(INT_MAX) { }
    virtual ~SidechainObj(void) { }

    uint256 GetHash(void) const;
    CScript GetScript(void) const;
    virtual std::string ToString(void) const;
};

/**
 * Sidechain individual withdrawal (WT) database object
 */
struct SidechainWT: public SidechainObj {
    uint8_t nSidechain;
    std::string strDestination;
    CAmount amount;
    char status;

    SidechainWT(void) : SidechainObj() { sidechainop = 'W'; status = WT_UNSPENT; }
    virtual ~SidechainWT(void) { }

    ADD_SERIALIZE_METHODS

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITE(sidechainop);
        READWRITE(nSidechain);
        READWRITE(strDestination);
        READWRITE(amount);
        READWRITE(status);
    }

    std::string ToString(void) const;
};

/**
 * Sidechain joined withdraw proposal (WT^) database object
 */
struct SidechainWTPrime: public SidechainObj {
    uint8_t nSidechain;
    CMutableTransaction wtPrime;

    SidechainWTPrime(void) : SidechainObj() { sidechainop = 'P'; }
    virtual ~SidechainWTPrime(void) { }

    ADD_SERIALIZE_METHODS

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITE(sidechainop);
        READWRITE(nSidechain);
        READWRITE(wtPrime);
    }

    std::string ToString(void) const;
};

/**
 * Sidechain deposit database object
 */
struct SidechainDeposit : public SidechainObj {
    uint8_t nSidechain;
    CKeyID keyID;
    CAmount amtUserPayout;
    CMutableTransaction dtx;
    CMainchainMerkleBlock proof;

    SidechainDeposit(void) : SidechainObj() { sidechainop = 'D'; }
    virtual ~SidechainDeposit(void) { }

    ADD_SERIALIZE_METHODS

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITE(sidechainop);
        READWRITE(nSidechain);
        READWRITE(keyID);
        READWRITE(amtUserPayout);
        READWRITE(dtx);
        READWRITE(proof);
    }

    std::string ToString(void) const;
    uint256 GetNonAmountHash() const
    {
        SidechainDeposit deposit(*this);
        deposit.amtUserPayout = CAmount(0);
        return deposit.GetHash();
    }
};

struct SidechainBMMProof
{
    uint256 hashBMMBlock;
    std::string txOutProof;
    std::string coinbaseHex;

    bool HasProof()
    {
        return (txOutProof.size() && coinbaseHex.size());
    }
};

/**
 * Create sidechain object
 */
SidechainObj *SidechainObjCtr(const CScript &);

#endif // BITCOIN_PRIMITIVES_SIDECHAIN_H
