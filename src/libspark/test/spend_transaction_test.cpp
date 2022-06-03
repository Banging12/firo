#include "../spend_transaction.h"

#include "../../test/test_bitcoin.h"
#include <boost/test/unit_test.hpp>

namespace spark {

using namespace secp_primitives;

// Generate a random char vector from a random scalar
static std::vector<unsigned char> random_char_vector() {
    Scalar temp;
    temp.randomize();
    std::vector<unsigned char> result;
    result.resize(SCALAR_ENCODING);
    temp.serialize(result.data());

    return result;
}

BOOST_FIXTURE_TEST_SUITE(spark_spend_transaction_tests, BasicTestingSetup)

BOOST_AUTO_TEST_CASE(generate_verify)
{
    // Parameters
    const Params* params;
    params = Params::get_test();

    const std::string memo = "Spam and eggs"; // arbitrary memo

    // Generate keys
    SpendKey spend_key(params);
    FullViewKey full_view_key(spend_key);
    IncomingViewKey incoming_view_key(full_view_key);

    // Generate address
    const uint64_t i = 12345;
    Address address(incoming_view_key, i);

    // Mint some coins to the address
    std::size_t N = (std::size_t) pow(params->get_n_grootle(), params->get_m_grootle());
    std::vector<Coin> in_coins;
    for (std::size_t i = 0; i < N; i++) {
        Scalar k;
        k.randomize();

        uint64_t v = 12 + i; // arbitrary value

        in_coins.emplace_back(Coin(
            params,
            COIN_TYPE_MINT,
            k,
            address,
            v,
            memo,
            random_char_vector()
        ));
    }

    // Track values so we can set the fee to make the transaction balance
    uint64_t f = 0;

    // Choose coins to spend, recover them, and prepare them for spending
    std::vector<std::size_t> spend_indices = { 1, 3, 5 };
    std::vector<InputCoinData> spend_coin_data;
    std::vector<std::vector<unsigned char>> roots;
    const std::size_t w = spend_indices.size();
    for (std::size_t u = 0; u < w; u++) {
        IdentifiedCoinData identified_coin_data = in_coins[spend_indices[u]].identify(incoming_view_key);
        RecoveredCoinData recovered_coin_data = in_coins[spend_indices[u]].recover(full_view_key, identified_coin_data);

        roots.emplace_back(random_char_vector());

        spend_coin_data.emplace_back();
        spend_coin_data.back().index = spend_indices[u];
        spend_coin_data.back().k = identified_coin_data.k;
        spend_coin_data.back().s = recovered_coin_data.s;
        spend_coin_data.back().T = recovered_coin_data.T;
        spend_coin_data.back().v = identified_coin_data.v;

        f -= identified_coin_data.v;
    }

    // Generate new output coins and compute the fee
    const std::size_t t = 2;
    std::vector<OutputCoinData> out_coin_data;
    for (std::size_t j = 0; j < t; j++) {
        out_coin_data.emplace_back();
        out_coin_data.back().address = address;
        out_coin_data.back().v = 123 + j; // arbitrary value
        out_coin_data.back().memo = memo;

        f += out_coin_data.back().v;
    }

    // Assert the fee is correct
    uint64_t fee_test = f;
    for (std::size_t u = 0; u < w; u++) {
        fee_test += spend_coin_data[u].v;
    }
    for (std::size_t j = 0; j < t; j++) {
        fee_test -= out_coin_data[j].v;
    }
    if (fee_test != 0) {
        throw std::runtime_error("Bad fee assertion");
    }

    // Generate spend transaction
    SpendTransaction transaction(
        params,
        full_view_key,
        spend_key,
        in_coins,
        roots,
        spend_coin_data,
        f,
        out_coin_data
    );

    // Verify
    BOOST_CHECK(transaction.verify());
}

BOOST_AUTO_TEST_SUITE_END()

}
