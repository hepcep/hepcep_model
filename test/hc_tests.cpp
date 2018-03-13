#include "gtest/gtest.h"

#include "HCVState.h"
#include "AreaType.h"
#include "AgeDecade.h"
#include "AgeGroup.h"

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

AgeDecade getExpectedDecage(double age) {
    if (age <= 20) return AgeDecade::AGE_LEQ_20;
    if (age < 31) return AgeDecade::AGE_21_30;
    if (age < 41) return AgeDecade::AGE_31_40;
    if (age < 51) return AgeDecade::AGE_41_50;
    if (age < 61) return AgeDecade::AGE_51_60;

    return AgeDecade::AGE_OVER_60;
}

TEST(HCTest, testEnums) {
    AreaType type = AreaType::getAreaType("60615");
    ASSERT_EQ(AreaType::CITY, type);

    type = AreaType::getAreaType("02492");
    ASSERT_EQ(AreaType::SUBURBAN, type);
    AreaType::getAreaType("");
    ASSERT_EQ(AreaType::SUBURBAN, type);

    for (int i = 10; i < 70; ++i) {
        AgeDecade dec = AgeDecade::getAgeDecade(i);
        ASSERT_EQ(getExpectedDecage(i), dec);
    }

    for (int i = 20; i < 50; ++i) {
        AgeGroup grp = AgeGroup::getAgeGroup(i);
        AgeGroup exp = AgeGroup::LEQ_30;
        if (i > 30) exp = AgeGroup::OVER_30;
        ASSERT_EQ(exp, grp);
    }

}
