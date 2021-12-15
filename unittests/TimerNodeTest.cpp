//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gtest/gtest.h"
#include "WithTempDirectory.h"

#include "ramses-logic/LogicEngine.h"
#include "ramses-logic/TimerNode.h"
#include "ramses-logic/Property.h"
#include "impl/TimerNodeImpl.h"
#include "impl/PropertyImpl.h"
#include "internals/ErrorReporting.h"
#include "internals/SerializationMap.h"
#include "internals/DeserializationMap.h"
#include "internals/TypeData.h"
#include "internals/EPropertySemantics.h"
#include "generated/TimerNodeGen.h"
#include "flatbuffers/flatbuffers.h"
#include <numeric>
#include <thread>

namespace rlogic::internal
{
    class ATimerNode : public ::testing::Test
    {
    protected:
        LogicEngine m_logicEngine;
    };

    TEST_F(ATimerNode, IsCreated)
    {
        const auto timerNode = m_logicEngine.createTimerNode("timerNode");
        EXPECT_TRUE(m_logicEngine.getErrors().empty());
        ASSERT_NE(nullptr, timerNode);
        EXPECT_EQ(timerNode, m_logicEngine.findTimerNode("timerNode"));

        EXPECT_EQ("timerNode", timerNode->getName());
    }

    TEST_F(ATimerNode, IsDestroyed)
    {
        const auto timerNode = m_logicEngine.createTimerNode("timerNode");
        EXPECT_TRUE(m_logicEngine.destroy(*timerNode));
        EXPECT_TRUE(m_logicEngine.getErrors().empty());
        EXPECT_EQ(nullptr, m_logicEngine.findTimerNode("timerNode"));
    }

    TEST_F(ATimerNode, FailsToBeDestroyedIfFromOtherLogicInstance)
    {
        auto timerNode = m_logicEngine.createTimerNode("timerNode");

        LogicEngine otherEngine;
        EXPECT_FALSE(otherEngine.destroy(*timerNode));
        ASSERT_FALSE(otherEngine.getErrors().empty());
        EXPECT_EQ("Can't find TimerNode in logic engine!", otherEngine.getErrors().front().message);
    }

    TEST_F(ATimerNode, ChangesName)
    {
        const auto timerNode = m_logicEngine.createTimerNode("timerNode");

        timerNode->setName("an");
        EXPECT_EQ("an", timerNode->getName());
        EXPECT_EQ(timerNode, m_logicEngine.findTimerNode("an"));
        EXPECT_TRUE(m_logicEngine.getErrors().empty());
    }

    TEST_F(ATimerNode, HasPropertiesAfterCreation)
    {
        const auto timerNode = m_logicEngine.createTimerNode("timerNode");

        const auto rootIn = timerNode->getInputs();
        EXPECT_EQ("IN", rootIn->getName());
        ASSERT_EQ(1u, rootIn->getChildCount());
        EXPECT_EQ("ticker_us", rootIn->getChild(0u)->getName());
        EXPECT_EQ(EPropertyType::Int64, rootIn->getChild(0u)->getType());

        const auto rootOut = timerNode->getOutputs();
        EXPECT_EQ("OUT", rootOut->getName());
        ASSERT_EQ(2u, rootOut->getChildCount());
        EXPECT_EQ("timeDelta", rootOut->getChild(0u)->getName());
        EXPECT_EQ("ticker_us", rootOut->getChild(1u)->getName());
        EXPECT_EQ(EPropertyType::Float, rootOut->getChild(0u)->getType());
        EXPECT_EQ(EPropertyType::Int64, rootOut->getChild(1u)->getType());
    }

    TEST_F(ATimerNode, OutputsTimeDeltaAndTicker_userTicker)
    {
        const auto timerNode = m_logicEngine.createTimerNode("timerNode");

        EXPECT_TRUE(timerNode->getInputs()->getChild(0u)->set<int64_t>(1000000));
        EXPECT_TRUE(m_logicEngine.update());
        EXPECT_FLOAT_EQ(0.f, *timerNode->getOutputs()->getChild(0u)->get<float>()); // no time delta on 1st update
        EXPECT_EQ(1000000, *timerNode->getOutputs()->getChild(1u)->get<int64_t>());

        // +0.5 second
        EXPECT_TRUE(timerNode->getInputs()->getChild(0u)->set<int64_t>(1500000));
        EXPECT_TRUE(m_logicEngine.update());
        EXPECT_FLOAT_EQ(0.5f, *timerNode->getOutputs()->getChild(0u)->get<float>());
        EXPECT_EQ(1500000, *timerNode->getOutputs()->getChild(1u)->get<int64_t>());

        // no change
        EXPECT_TRUE(timerNode->getInputs()->getChild(0u)->set<int64_t>(1500000));
        EXPECT_TRUE(m_logicEngine.update());
        EXPECT_FLOAT_EQ(0.f, *timerNode->getOutputs()->getChild(0u)->get<float>());
        EXPECT_EQ(1500000, *timerNode->getOutputs()->getChild(1u)->get<int64_t>());

        // +10 second
        EXPECT_TRUE(timerNode->getInputs()->getChild(0u)->set<int64_t>(11500000));
        EXPECT_TRUE(m_logicEngine.update());
        EXPECT_FLOAT_EQ(10.f, *timerNode->getOutputs()->getChild(0u)->get<float>());
        EXPECT_EQ(11500000, *timerNode->getOutputs()->getChild(1u)->get<int64_t>());
    }

    TEST_F(ATimerNode, OutputsTimeDeltaAndTicker_autoTicker)
    {
        const auto timerNode = m_logicEngine.createTimerNode("timerNode");

        // auto ticker is enabled if ticker input == 0
        EXPECT_TRUE(timerNode->getInputs()->getChild(0u)->set<int64_t>(0));

        EXPECT_TRUE(m_logicEngine.update());
        EXPECT_FLOAT_EQ(0.f, *timerNode->getOutputs()->getChild(0u)->get<float>()); // no time delta on 1st update
        const auto initialTicker = *timerNode->getOutputs()->getChild(1u)->get<int64_t>();
        EXPECT_TRUE(initialTicker > 0);

        auto ticker = initialTicker;
        float deltaSum = 0.f;
        for (int i = 0; i < 10; ++i)
        {
            EXPECT_TRUE(m_logicEngine.update());
            const auto timeDelta = *timerNode->getOutputs()->getChild(0u)->get<float>();
            EXPECT_TRUE(timeDelta >= 0.f);
            deltaSum += timeDelta;
            const auto nextTicker = *timerNode->getOutputs()->getChild(1u)->get<int64_t>();
            EXPECT_TRUE(nextTicker >= ticker);
            ticker = nextTicker;
            std::this_thread::sleep_for(std::chrono::milliseconds{ 1 });
        }

        // check that auto ticker is actually progressing
        EXPECT_GT(ticker, initialTicker);
        EXPECT_GT(deltaSum, 0.f);
    }

    TEST_F(ATimerNode, FailsUpdateIfNegativeTickerProvided)
    {
        const auto timerNode = m_logicEngine.createTimerNode("timerNode");
        EXPECT_TRUE(timerNode->getInputs()->getChild(0u)->set<int64_t>(-100));
        EXPECT_FALSE(m_logicEngine.update());

        ASSERT_EQ(1u, m_logicEngine.getErrors().size());
        EXPECT_EQ("TimerNode 'timerNode' failed to update - cannot use negative ticker (-100)", m_logicEngine.getErrors().front().message);
    }

    TEST_F(ATimerNode, FailsUpdateIfTickerProvidedNotMonotonicallyIncreasing)
    {
        const auto timerNode = m_logicEngine.createTimerNode("timerNode");

        EXPECT_TRUE(timerNode->getInputs()->getChild(0u)->set<int64_t>(100));
        EXPECT_TRUE(m_logicEngine.update());

        EXPECT_TRUE(timerNode->getInputs()->getChild(0u)->set<int64_t>(110));
        EXPECT_TRUE(m_logicEngine.update());

        EXPECT_TRUE(timerNode->getInputs()->getChild(0u)->set<int64_t>(110));
        EXPECT_TRUE(m_logicEngine.update());

        EXPECT_TRUE(timerNode->getInputs()->getChild(0u)->set<int64_t>(109));
        EXPECT_FALSE(m_logicEngine.update());

        ASSERT_EQ(1u, m_logicEngine.getErrors().size());
        EXPECT_EQ("TimerNode 'timerNode' failed to update - ticker must be monotonically increasing (lastTick=110 newTick=109)", m_logicEngine.getErrors().front().message);
    }

    TEST_F(ATimerNode, CanBeSerializedAndDeserialized)
    {
        WithTempDirectory tempDir;
        {
            LogicEngine otherEngine;
            otherEngine.createTimerNode("timerNode");
            ASSERT_TRUE(otherEngine.saveToFile("logic_timerNode.bin"));
        }

        ASSERT_TRUE(m_logicEngine.loadFromFile("logic_timerNode.bin"));
        EXPECT_TRUE(m_logicEngine.getErrors().empty());

        EXPECT_EQ(1u, m_logicEngine.timerNodes().size());
        const auto timerNode = m_logicEngine.findTimerNode("timerNode");
        ASSERT_TRUE(timerNode);
        EXPECT_EQ("timerNode", timerNode->getName());

        const auto rootIn = timerNode->getInputs();
        EXPECT_EQ("IN", rootIn->getName());
        ASSERT_EQ(1u, rootIn->getChildCount());
        EXPECT_EQ("ticker_us", rootIn->getChild(0u)->getName());
        EXPECT_EQ(EPropertyType::Int64, rootIn->getChild(0u)->getType());

        const auto rootOut = timerNode->getOutputs();
        EXPECT_EQ("OUT", rootOut->getName());
        ASSERT_EQ(2u, rootOut->getChildCount());
        EXPECT_EQ("timeDelta", rootOut->getChild(0u)->getName());
        EXPECT_EQ("ticker_us", rootOut->getChild(1u)->getName());
        EXPECT_EQ(EPropertyType::Float, rootOut->getChild(0u)->getType());
        EXPECT_EQ(EPropertyType::Int64, rootOut->getChild(1u)->getType());
    }

    TEST_F(ATimerNode, WillNotSerializeInternalTimer)
    {
        WithTempDirectory tempDir;

        {
            LogicEngine otherEngine;
            auto timerNode = otherEngine.createTimerNode("timerNode");

            // set custom ticker to some value
            EXPECT_TRUE(timerNode->getInputs()->getChild(0u)->set<int64_t>(100));
            EXPECT_TRUE(otherEngine.update());

            ASSERT_TRUE(otherEngine.saveToFile("logic_timerNode.bin"));
        }

        ASSERT_TRUE(m_logicEngine.loadFromFile("logic_timerNode.bin"));
        const auto timerNode = m_logicEngine.findTimerNode("timerNode");
        ASSERT_TRUE(timerNode);

        EXPECT_TRUE(timerNode->getInputs()->getChild(0u)->set<int64_t>(1));
        // this would fail if internal timer would be serialized because ticker is lower
        // than value before serialization - which violates monotonic ticker input
        EXPECT_TRUE(m_logicEngine.update());
    }

    class ATimerNode_SerializationLifecycle : public ATimerNode
    {
    protected:
        enum class ESerializationIssue
        {
            AllValid,
            NameMissing,
            IdMissing,
            RootInMissing,
            RootOutMissing,
            PropertyInMissing,
            PropertyOutMissing,
            PropertyInWrongName,
            PropertyOutWrongName
        };

        std::unique_ptr<TimerNodeImpl> deserializeSerializedDataWithIssue(ESerializationIssue issue)
        {
            flatbuffers::FlatBufferBuilder flatBufferBuilder;
            SerializationMap serializationMap;
            DeserializationMap deserializationMap;

            {
                HierarchicalTypeData inputs = MakeStruct("IN", {});
                if (issue == ESerializationIssue::PropertyInWrongName)
                {
                    inputs.children.push_back(MakeType("wrongInput", EPropertyType::Int64));
                }
                else if (issue != ESerializationIssue::PropertyInMissing)
                {
                    inputs.children.push_back(MakeType("ticker_us", EPropertyType::Int64));
                }
                auto inputsImpl = std::make_unique<PropertyImpl>(std::move(inputs), EPropertySemantics::ScriptInput);

                HierarchicalTypeData outputs = MakeStruct("OUT", {});
                if (issue == ESerializationIssue::PropertyOutWrongName)
                {
                    outputs.children.push_back(MakeType("wrongOutput", EPropertyType::Float));
                }
                else
                {
                    outputs.children.push_back(MakeType("timeDelta", EPropertyType::Float));
                }
                if (issue != ESerializationIssue::PropertyOutMissing)
                    outputs.children.push_back(MakeType("ticker_us", EPropertyType::Int64));
                auto outputsImpl = std::make_unique<PropertyImpl>(std::move(outputs), EPropertySemantics::ScriptOutput);

                const auto timerNodeFB = rlogic_serialization::CreateTimerNode(
                    flatBufferBuilder,
                    issue == ESerializationIssue::NameMissing ? 0 : flatBufferBuilder.CreateString("timerNode"),
                    issue == ESerializationIssue::IdMissing ? 0 : 1u,
                    issue == ESerializationIssue::RootInMissing ? 0 : PropertyImpl::Serialize(*inputsImpl, flatBufferBuilder, serializationMap),
                    issue == ESerializationIssue::RootOutMissing ? 0 : PropertyImpl::Serialize(*outputsImpl, flatBufferBuilder, serializationMap)
                );

                flatBufferBuilder.Finish(timerNodeFB);
            }

            const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::TimerNode>(flatBufferBuilder.GetBufferPointer());
            return TimerNodeImpl::Deserialize(serialized, m_errorReporting, deserializationMap);
        }

        ErrorReporting m_errorReporting;
    };

    TEST_F(ATimerNode_SerializationLifecycle, FailsDeserializationIfEssentialDataMissing)
    {
        EXPECT_TRUE(deserializeSerializedDataWithIssue(ATimerNode_SerializationLifecycle::ESerializationIssue::AllValid));
        EXPECT_TRUE(m_errorReporting.getErrors().empty());

        for (const auto issue : { ESerializationIssue::NameMissing, ESerializationIssue::IdMissing, ESerializationIssue::RootInMissing, ESerializationIssue::RootOutMissing })
        {
            EXPECT_FALSE(deserializeSerializedDataWithIssue(issue));
            ASSERT_FALSE(m_errorReporting.getErrors().empty());
            EXPECT_EQ("Fatal error during loading of TimerNode from serialized data: missing name, id or in/out property data!", m_errorReporting.getErrors().front().message);
            m_errorReporting.clear();
        }
    }

    TEST_F(ATimerNode_SerializationLifecycle, FailsDeserializationIfInvalidInterpolationType)
    {
        for (const auto issue : { ESerializationIssue::PropertyInMissing, ESerializationIssue::PropertyOutMissing, ESerializationIssue::PropertyInWrongName, ESerializationIssue::PropertyOutWrongName })
        {
            EXPECT_FALSE(deserializeSerializedDataWithIssue(issue));
            ASSERT_FALSE(m_errorReporting.getErrors().empty());
            EXPECT_EQ("Fatal error during loading of TimerNode 'timerNode': missing or invalid properties!", m_errorReporting.getErrors().front().message);
            m_errorReporting.clear();
        }
    }
}
