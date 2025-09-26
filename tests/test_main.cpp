#include <gtest/gtest.h>

// Main function for Google Test
// This is handled by gtest_main, but we can add global setup here if needed

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}