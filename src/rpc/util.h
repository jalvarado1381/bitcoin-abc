// Copyright (c) 2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_RPC_UTIL_H
#define BITCOIN_RPC_UTIL_H

#include <node/transaction.h>
#include <rpc/protocol.h>
#include <script/standard.h> // For CTxDestination
#include <univalue.h>
#include <util/check.h>

#include <string>
#include <vector>

class CChainParams;
class CKeyStore;
class CPubKey;
class CScript;
struct NodeContext;
class UniValue;

//! Pointers to interfaces that need to be accessible from RPC methods. Due to
//! limitations of the RPC framework, there's currently no direct way to pass in
//! state to RPC method implementations.
extern NodeContext *g_rpc_node;

/**
 * Wrapper for UniValue::VType, which includes typeAny: used to denote don't
 * care type.
 */
struct UniValueType {
    UniValueType(UniValue::VType _type) : typeAny(false), type(_type) {}
    UniValueType() : typeAny(true) {}
    bool typeAny;
    UniValue::VType type;
};

/**
 * Type-check arguments; throws JSONRPCError if wrong type given. Does not check
 * that the right number of arguments are passed, just that any passed are the
 * correct type.
 */
void RPCTypeCheck(const UniValue &params,
                  const std::list<UniValueType> &typesExpected,
                  bool fAllowNull = false);

/**
 * Type-check one argument; throws JSONRPCError if wrong type given.
 */
void RPCTypeCheckArgument(const UniValue &value,
                          const UniValueType &typeExpected);

/**
 * Check for expected keys/value types in an Object.
 */
void RPCTypeCheckObj(const UniValue &o,
                     const std::map<std::string, UniValueType> &typesExpected,
                     bool fAllowNull = false, bool fStrict = false);

/**
 * Utilities: convert hex-encoded values (throws error if not hex).
 */
extern uint256 ParseHashV(const UniValue &v, std::string strName);
extern uint256 ParseHashO(const UniValue &o, std::string strKey);
extern std::vector<uint8_t> ParseHexV(const UniValue &v, std::string strName);
extern std::vector<uint8_t> ParseHexO(const UniValue &o, std::string strKey);

extern Amount AmountFromValue(const UniValue &value);
extern std::string HelpExampleCli(const std::string &methodname,
                                  const std::string &args);
extern std::string HelpExampleRpc(const std::string &methodname,
                                  const std::string &args);

CPubKey HexToPubKey(const std::string &hex_in);
CPubKey AddrToPubKey(const CChainParams &chainparams, CKeyStore *const keystore,
                     const std::string &addr_in);
CScript CreateMultisigRedeemscript(const int required,
                                   const std::vector<CPubKey> &pubkeys);

UniValue DescribeAddress(const CTxDestination &dest);

RPCErrorCode RPCErrorFromTransactionError(TransactionError terr);
UniValue JSONRPCTransactionError(TransactionError terr,
                                 const std::string &err_string = "");

struct RPCArg {
    enum class Type {
        OBJ,
        ARR,
        STR,
        NUM,
        BOOL,
        //! Special type where the user must set the keys e.g. to define
        //! multiple addresses; as opposed to e.g. an options object where the
        //! keys are predefined
        OBJ_USER_KEYS,
        //! Special type representing a floating point amount (can be either NUM
        //! or STR)
        AMOUNT,
        //! Special type that is a STR with only hex chars
        STR_HEX,
    };
    //! The name of the arg (can be empty for inner args)
    const std::string m_name;
    const Type m_type;
    //! Only used for arrays or dicts
    const std::vector<RPCArg> m_inner;
    const bool m_optional;
    const std::string m_default_value; //!< Only used for optional args
    const std::string m_description;
    //! Should be empty unless it is supposed to override the auto-generated
    //! summary line
    const std::string m_oneline_description;

    //! Should be empty unless it is supposed to override the
    //! auto-generated type strings. Vector length is either 0
    //! or 2, m_type_str.at(0) will override the type of the
    //! value in a key-value pair, m_type_str.at(1) will
    //! override the type in the argument description.
    const std::vector<std::string> m_type_str;

    RPCArg(const std::string &name, const Type &type, const bool opt,
           const std::string &default_val, const std::string &description,
           const std::string &oneline_description = "",
           const std::vector<std::string> &type_str = {})
        : m_name{name}, m_type{type}, m_optional{opt},
          m_default_value{default_val}, m_description{description},
          m_oneline_description{oneline_description}, m_type_str{type_str} {
        CHECK_NONFATAL(type != Type::ARR && type != Type::OBJ);
    }

    RPCArg(const std::string &name, const Type &type, const bool opt,
           const std::string &default_val, const std::string &description,
           const std::vector<RPCArg> &inner,
           const std::string &oneline_description = "",
           const std::vector<std::string> &type_str = {})
        : m_name{name}, m_type{type}, m_inner{inner}, m_optional{opt},
          m_default_value{default_val}, m_description{description},
          m_oneline_description{oneline_description}, m_type_str{type_str} {
        CHECK_NONFATAL(type == Type::ARR || type == Type::OBJ);
    }

    /**
     * Return the type string of the argument.
     * Set oneline to allow it to be overrided by a custom oneline type string
     * (m_oneline_description).
     */
    std::string ToString(bool oneline) const;
    /**
     * Return the type string of the argument when it is in an object (dict).
     * Set oneline to get the oneline representation (less whitespace)
     */
    std::string ToStringObj(bool oneline) const;
    /**
     * Return the description string, including the argument type and whether
     * the argument is required.
     * implicitly_required is set for arguments in an array, which are neither
     * optional nor required.
     */
    std::string ToDescriptionString(bool implicitly_required = false) const;
};

struct RPCResult {
    const std::string m_cond;
    const std::string m_result;

    explicit RPCResult(std::string result)
        : m_cond{}, m_result{std::move(result)} {
        CHECK_NONFATAL(!m_result.empty());
    }

    RPCResult(std::string cond, std::string result)
        : m_cond{std::move(cond)}, m_result{std::move(result)} {
        CHECK_NONFATAL(!m_cond.empty());
        CHECK_NONFATAL(!m_result.empty());
    }
};

struct RPCResults {
    const std::vector<RPCResult> m_results;

    RPCResults() : m_results{} {}

    RPCResults(RPCResult result) : m_results{{result}} {}

    RPCResults(std::initializer_list<RPCResult> results) : m_results{results} {}

    /**
     * Return the description string.
     */
    std::string ToDescriptionString() const;
};

struct RPCExamples {
    const std::string m_examples;
    RPCExamples(std::string examples) : m_examples(std::move(examples)) {}
    RPCExamples() : m_examples(std::move("")) {}
    std::string ToDescriptionString() const;
};

class RPCHelpMan {
public:
    RPCHelpMan(std::string name, std::string description,
               std::vector<RPCArg> args, RPCResults results,
               RPCExamples examples);

    std::string ToString() const;

private:
    const std::string m_name;
    const std::string m_description;
    const std::vector<RPCArg> m_args;
    const RPCResults m_results;
    const RPCExamples m_examples;
};

#endif // BITCOIN_RPC_UTIL_H
