#include <gtest/gtest.h>
#include "optimum_p2p/utils.hpp"
#include <vector>
#include <string>

namespace optimum_p2p {

class HexUtilsTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

// Test HeadHex with data smaller than n
TEST_F(HexUtilsTest, HeadHex_SmallerThanN) {
    std::vector<uint8_t> data = {0x01, 0x02, 0x03};
    std::string hex = HeadHex(data, 10);
    
    // Should return hex of all data
    EXPECT_EQ(hex, "010203");
}

// Test HeadHex with data larger than n
TEST_F(HexUtilsTest, HeadHex_LargerThanN) {
    std::vector<uint8_t> data = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    std::string hex = HeadHex(data, 4);
    
    // Should return hex of first 4 bytes
    EXPECT_EQ(hex, "01020304");
}

// Test HeadHex with empty data
TEST_F(HexUtilsTest, HeadHex_EmptyData) {
    std::vector<uint8_t> data;
    std::string hex = HeadHex(data, 10);
    
    EXPECT_TRUE(hex.empty());
}

// Test HeadHex with exact size
TEST_F(HexUtilsTest, HeadHex_ExactSize) {
    std::vector<uint8_t> data = {0xFF, 0xEE, 0xDD};
    std::string hex = HeadHex(data, 3);
    
    EXPECT_EQ(hex, "ffeedd");
}

// Test HeadHex with n=0
TEST_F(HexUtilsTest, HeadHex_ZeroN) {
    std::vector<uint8_t> data = {0x01, 0x02, 0x03};
    std::string hex = HeadHex(data, 0);
    
    EXPECT_TRUE(hex.empty());
}

// Test HeadHex with binary data
TEST_F(HexUtilsTest, HeadHex_BinaryData) {
    std::vector<uint8_t> data = {0x00, 0xFF, 0x80, 0x7F};
    std::string hex = HeadHex(data, 4);
    
    EXPECT_EQ(hex, "00ff807f");
}

} // namespace optimum_p2p

