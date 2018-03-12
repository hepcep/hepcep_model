#include "gtest/gtest.h"

#include "HCVState.h"

using namespace hepcep;

TEST(HCTest, testPlaceholder) {
    ASSERT_EQ(0, 0);
    ASSERT_NEAR(4.604, 4.60401111, 0.001);
}

TEST(HCTest, testHCVStateEnum) {
    ASSERT_EQ(HCVState::SUSCEPTIBLE, HCVState::SUSCEPTIBLE);
    ASSERT_NE(HCVState::SUSCEPTIBLE, HCVState::UNKNOWN);

    for (auto& state : HCVState::values()) {
        ASSERT_EQ(state, HCVState::valueOf(state.stringValue()));
        ASSERT_EQ(state, state);
    }

    ASSERT_EQ(8, HCVState::values().size());

    // susceptible, exposed, infectious_acute, recovered, cured, chronic, unknown, abpos
    for (auto& state : HCVState::values()) {
        switch (state.value()) {
        case HCVState::Value::susceptible:
            ASSERT_EQ(state, HCVState::SUSCEPTIBLE);
            break;
        case HCVState::Value::infectious_acute:
            ASSERT_EQ(state, HCVState::INFECTIOUS_ACUTE);
            break;
        case HCVState::Value::exposed:
            ASSERT_EQ(state, HCVState::EXPOSED);
            break;
        case HCVState::Value::recovered:
            ASSERT_EQ(state, HCVState::RECOVERED);
            break;
        case HCVState::Value::cured:
            ASSERT_EQ(state, HCVState::CURED);
            break;
        case HCVState::Value::chronic:
            ASSERT_EQ(state, HCVState::CHRONIC);
            break;
        case HCVState::Value::unknown:
            ASSERT_EQ(state, HCVState::UNKNOWN);
            break;
        case HCVState::Value::abpos:
            ASSERT_EQ(state, HCVState::ABPOS);
            break;
        default:
            ASSERT_TRUE(false);

        }
    }
}
