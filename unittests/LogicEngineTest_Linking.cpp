//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "LogicEngineTest_Base.h"

#include "RamsesTestUtils.h"
#include "WithTempDirectory.h"

#include "ramses-client-api/EffectDescription.h"
#include "ramses-client-api/Effect.h"
#include "ramses-client-api/Node.h"
#include "ramses-client-api/Scene.h"
#include "ramses-client-api/UniformInput.h"
#include "ramses-client-api/Appearance.h"
#include "ramses-client-api/PerspectiveCamera.h"

#include "ramses-logic/LuaScript.h"
#include "ramses-logic/EStandardModule.h"
#include "ramses-logic/Property.h"
#include "ramses-logic/RamsesAppearanceBinding.h"
#include "ramses-logic/RamsesNodeBinding.h"
#include "ramses-logic/RamsesCameraBinding.h"

#include "impl/LogicEngineImpl.h"
#include "impl/RamsesNodeBindingImpl.h"
#include "impl/LogicNodeImpl.h"
#include "impl/PropertyImpl.h"
#include "internals/ApiObjects.h"

#include "fmt/format.h"
#include <array>

namespace rlogic
{
    class ALogicEngine_Linking : public ALogicEngine
    {
    protected:
        ALogicEngine_Linking()
            : m_sourceScript(*m_logicEngine.createLuaScript(m_minimalLinkScript, {}, "SourceScript"))
            , m_targetScript(*m_logicEngine.createLuaScript(m_minimalLinkScript, {}, "TargetScript"))
            , m_sourceProperty(*m_sourceScript.getOutputs()->getChild("source"))
            , m_targetProperty(*m_targetScript.getInputs()->getChild("target"))
        {
        }

        const std::string_view m_minimalLinkScript = R"(
            function interface()
                IN.target = BOOL
                OUT.source = BOOL
            end
            function run()
            end
        )";

        const std::string_view m_linkScriptMultipleTypes = R"(
            function interface()
                IN.target_INT = INT
                OUT.source_INT = INT
                IN.target_VEC3F = VEC3F
                OUT.source_VEC3F = VEC3F
            end
            function run()
                OUT.source_INT = IN.target_INT
                OUT.source_VEC3F = IN.target_VEC3F
            end
        )";

        LuaScript& m_sourceScript;
        LuaScript& m_targetScript;

        const Property& m_sourceProperty;
        Property& m_targetProperty;
    };

    TEST_F(ALogicEngine_Linking, ProducesErrorIfPropertiesWithMismatchedTypesAreLinked)
    {
        const char* errorString = "Types of source property 'outParam:{}' does not match target property 'inParam:{}'";

        std::array<std::tuple<std::string, std::string, std::string>, 7> errorCases = {
            std::make_tuple("FLOAT", "INT",   fmt::format(errorString, "FLOAT", "INT32")),
            std::make_tuple("INT32", "INT64", fmt::format(errorString, "INT32", "INT64")),
            std::make_tuple("INT64", "INT32", fmt::format(errorString, "INT64", "INT32")),
            std::make_tuple("VEC3F", "VEC3I", fmt::format(errorString, "VEC3F", "VEC3I")),
            std::make_tuple("VEC2F", "VEC4I", fmt::format(errorString, "VEC2F", "VEC4I")),
            std::make_tuple("VEC2I", "FLOAT", fmt::format(errorString, "VEC2I", "FLOAT")),
            std::make_tuple("INT",
            R"({
                param1 = INT,
                param2 = FLOAT
            })", fmt::format(errorString, "INT32", "STRUCT"))
        };

        for (const auto& errorCase : errorCases)
        {
            const auto  luaScriptSource = fmt::format(R"(
                function interface()
                    IN.inParam = {}
                    OUT.outParam = {}
                end
                function run()
                end
            )", std::get<1>(errorCase), std::get<0>(errorCase));

            auto sourceScript = m_logicEngine.createLuaScript(luaScriptSource);
            auto targetScript = m_logicEngine.createLuaScript(luaScriptSource);

            const auto sourceProperty = sourceScript->getOutputs()->getChild("outParam");
            const auto targetProperty = targetScript->getInputs()->getChild("inParam");

            EXPECT_FALSE(m_logicEngine.link(
                *sourceProperty,
                *targetProperty
            ));

            const auto& errors = m_logicEngine.getErrors();
            ASSERT_EQ(1u, errors.size());
            EXPECT_EQ(errors[0].message, std::get<2>(errorCase));
        }
    }

    TEST_F(ALogicEngine_Linking, ProducesErrorIfLogicNodeIsLinkedToItself)
    {
        const auto targetPropertyFromTheSameScript = m_sourceScript.getInputs()->getChild("target");

        EXPECT_FALSE(m_logicEngine.link(m_sourceProperty, *targetPropertyFromTheSameScript));

        const auto& errors = m_logicEngine.getErrors();
        ASSERT_EQ(1u, errors.size());
        // TODO Violin error message is not giving enough info where the error came from - improve
        EXPECT_EQ(errors[0].message, "SourceNode and TargetNode are equal");
    }

    TEST_F(ALogicEngine_Linking, ProducesErrorIfInputIsLinkedToOutput)
    {
        const auto sourceProperty = m_sourceScript.getOutputs()->getChild("source");
        const auto targetProperty = m_targetScript.getInputs()->getChild("target");

        EXPECT_FALSE(m_logicEngine.link(*targetProperty, *sourceProperty));
        const auto errors = m_logicEngine.getErrors();
        ASSERT_EQ(1u, errors.size());
        EXPECT_EQ("Failed to link input property 'target' to output property 'source'. Only outputs can be linked to inputs", errors[0].message);
    }

    TEST_F(ALogicEngine_Linking, ProducesErrorIfInputIsLinkedToInput)
    {
        const auto sourceInput = m_sourceScript.getInputs()->getChild("target");
        const auto targetInput = m_targetScript.getInputs()->getChild("target");

        EXPECT_FALSE(m_logicEngine.link(*sourceInput, *targetInput));
        const auto errors = m_logicEngine.getErrors();
        ASSERT_EQ(1u, errors.size());
        EXPECT_EQ("Failed to link input property 'target' to input property 'target'. Only outputs can be linked to inputs", errors[0].message);
    }

    TEST_F(ALogicEngine_Linking, ProducesErrorIfOutputIsLinkedToOutput)
    {
        auto       sourceScript = m_logicEngine.createLuaScript(m_linkScriptMultipleTypes);
        auto       targetScript = m_logicEngine.createLuaScript(m_linkScriptMultipleTypes);

        const auto sourceOuput  = sourceScript->getOutputs()->getChild("source_INT");
        const auto targetOutput = targetScript->getOutputs()->getChild("source_INT");

        EXPECT_FALSE(m_logicEngine.link(*sourceOuput, *targetOutput));

        const auto errors = m_logicEngine.getErrors();
        ASSERT_EQ(1u, errors.size());
        EXPECT_EQ("Failed to link output property 'source_INT' to output property 'source_INT'. Only outputs can be linked to inputs", errors[0].message);
    }

    TEST_F(ALogicEngine_Linking, ProducesNoErrorIfMatchingPropertiesAreLinked)
    {
        EXPECT_TRUE(m_logicEngine.link(m_sourceProperty, m_targetProperty));
    }

    TEST_F(ALogicEngine_Linking, ProducesErrorIfPropertyIsLinkedTwiceToSameProperty_LuaScript)
    {
        EXPECT_TRUE(m_logicEngine.link(m_sourceProperty, m_targetProperty));
        EXPECT_FALSE(m_logicEngine.link(m_sourceProperty, m_targetProperty));

        const auto& errors = m_logicEngine.getErrors();
        ASSERT_EQ(1u, errors.size());
        EXPECT_EQ(errors[0].message, "The property 'target' of LogicNode 'TargetScript' is already linked (to property 'source' of LogicNode 'SourceScript')");
    }

    TEST_F(ALogicEngine_Linking, ProducesErrorIfPropertyIsLinkedTwice_RamsesBinding)
    {
        auto ramsesBinding = m_logicEngine.createRamsesNodeBinding(*m_node, ERotationType::Euler_XYZ, "RamsesBinding");

        const auto visibilityProperty = ramsesBinding->getInputs()->getChild("visibility");

        EXPECT_TRUE(m_logicEngine.link(m_sourceProperty, *visibilityProperty));
        EXPECT_FALSE(m_logicEngine.link(m_sourceProperty, *visibilityProperty));

        const auto& errors = m_logicEngine.getErrors();
        ASSERT_EQ(1u, errors.size());
        EXPECT_EQ(errors[0].message, "The property 'visibility' of LogicNode 'RamsesBinding' is already linked (to property 'source' of LogicNode 'SourceScript')");
    }

    TEST_F(ALogicEngine_Linking, ProducesErrorIfNotLinkedPropertyIsUnlinked_LuaScript)
    {
        EXPECT_FALSE(m_logicEngine.unlink(m_sourceProperty, m_targetProperty));

        const auto& errors = m_logicEngine.getErrors();
        ASSERT_EQ(1u, errors.size());
        EXPECT_EQ(errors[0].message, "Input property 'target' is not currently linked!");
    }

    TEST_F(ALogicEngine_Linking, ProducesErrorIfNotLinkedPropertyIsUnlinked_RamsesNodeBinding)
    {
        auto ramsesBinding = m_logicEngine.createRamsesNodeBinding(*m_node, ERotationType::Euler_XYZ, "RamsesBinding");

        const auto visibilityProperty = ramsesBinding->getInputs()->getChild("visibility");

        EXPECT_FALSE(m_logicEngine.unlink(m_sourceProperty, *visibilityProperty));

        const auto& errors = m_logicEngine.getErrors();
        ASSERT_EQ(1u, errors.size());
        EXPECT_EQ(errors[0].message, "Input property 'visibility' is not currently linked!");
    }

    TEST_F(ALogicEngine_Linking, ProducesNoErrorIfLinkedToMatchingType)
    {
        const auto  luaScriptSource = R"(
            function interface()
                IN.boolTarget  = BOOL
                IN.intTarget   = INT
                IN.int64Target = INT64
                IN.floatTarget = FLOAT
                IN.vec2Target  = VEC2F
                IN.vec3Target  = VEC3F
                OUT.boolSource  = BOOL
                OUT.intSource   = INT
                OUT.int64Source = INT64
                OUT.floatSource = FLOAT
                OUT.vec2Source  = VEC2F
                OUT.vec3Source  = VEC3F
            end
            function run()
            end
        )";

        auto sourceScript = m_logicEngine.createLuaScript(luaScriptSource);
        auto targetScript = m_logicEngine.createLuaScript(luaScriptSource);

        const auto outputs  = sourceScript->getOutputs();
        const auto inputs   = targetScript->getInputs();

        auto boolTarget = inputs->getChild("boolTarget");
        auto intTarget = inputs->getChild("intTarget");
        auto int64Target = inputs->getChild("int64Target");
        auto floatTarget = inputs->getChild("floatTarget");
        auto vec2Target = inputs->getChild("vec2Target");
        auto vec3Target = inputs->getChild("vec3Target");

        auto boolSource = outputs->getChild("boolSource");
        auto intSource = outputs->getChild("intSource");
        auto int64Source = outputs->getChild("int64Source");
        auto floatSource = outputs->getChild("floatSource");
        auto vec2Source = outputs->getChild("vec2Source");
        auto vec3Source = outputs->getChild("vec3Source");

        EXPECT_TRUE(m_logicEngine.link(*boolSource, *boolTarget));
        EXPECT_TRUE(m_logicEngine.link(*intSource, *intTarget));
        EXPECT_TRUE(m_logicEngine.link(*int64Source, *int64Target));
        EXPECT_TRUE(m_logicEngine.link(*floatSource, *floatTarget));
        EXPECT_TRUE(m_logicEngine.link(*vec2Source, *vec2Target));
        EXPECT_TRUE(m_logicEngine.link(*vec3Source, *vec3Target));
    }

    TEST_F(ALogicEngine_Linking, ProducesErrorOnNextUpdateIfLinkCycleWasCreated)
    {
        LuaScript& loopScript = *m_logicEngine.createLuaScript(m_minimalLinkScript);
        const Property* sourceInput = m_sourceScript.getInputs()->getChild("target");
        const Property* sourceOutput = m_sourceScript.getOutputs()->getChild("source");
        const Property* targetInput = m_targetScript.getInputs()->getChild("target");
        const Property* targetOutput = m_targetScript.getOutputs()->getChild("source");
        const Property* loopInput = loopScript.getInputs()->getChild("target");
        const Property* loopOutput = loopScript.getOutputs()->getChild("source");

        EXPECT_TRUE(m_logicEngine.link(*sourceOutput, *targetInput));
        EXPECT_TRUE(m_logicEngine.link(*targetOutput, *loopInput));
        EXPECT_TRUE(m_logicEngine.link(*loopOutput, *sourceInput));
        EXPECT_FALSE(m_logicEngine.update());
        auto errors = m_logicEngine.getErrors();
        ASSERT_EQ(1u, errors.size());
        EXPECT_EQ("Failed to sort logic nodes based on links between their properties. Create a loop-free link graph before calling update()!", errors[0].message);

        // Also refuse to save to file
        EXPECT_FALSE(m_logicEngine.saveToFile("will_not_write"));
        errors = m_logicEngine.getErrors();
        ASSERT_EQ(1u, errors.size());
        EXPECT_EQ("Failed to sort logic nodes based on links between their properties. Create a loop-free link graph before calling saveToFile()!", errors[0].message);
    }

    TEST_F(ALogicEngine_Linking, PropagatesValuesAcrossMultipleLinksInAChain)
    {
        auto scriptSource = R"(
            function interface()
                IN.inString1 = STRING
                IN.inString2 = STRING
                OUT.outString = STRING
            end
            function run()
                OUT.outString = IN.inString1 .. IN.inString2
            end
        )";

        auto script1 = m_logicEngine.createLuaScript(scriptSource);
        auto script2 = m_logicEngine.createLuaScript(scriptSource);
        auto script3 = m_logicEngine.createLuaScript(scriptSource);

        auto script1Input2 = script1->getInputs()->getChild("inString2");
        auto script2Input1 = script2->getInputs()->getChild("inString1");
        auto script2Input2 = script2->getInputs()->getChild("inString2");
        auto script3Input1 = script3->getInputs()->getChild("inString1");
        auto script3Input2 = script3->getInputs()->getChild("inString2");
        auto script1Output = script1->getOutputs()->getChild("outString");
        auto script2Output = script2->getOutputs()->getChild("outString");
        auto script3Output = script3->getOutputs()->getChild("outString");

        m_logicEngine.link(*script1Output, *script2Input1);
        m_logicEngine.link(*script2Output, *script3Input1);

        script1Input2->set(std::string("Script1"));
        script2Input2->set(std::string("Script2"));
        script3Input2->set(std::string("Script3"));

        m_logicEngine.update();

        EXPECT_EQ("Script1Script2Script3", script3Output->get<std::string>());
    }

    TEST_F(ALogicEngine_Linking, ProducesErrorOnLinkingStructs)
    {
        const auto  luaScriptSource = R"(
            function interface()
                IN.intTarget = INT
                IN.structTarget = {
                    intTarget = INT,
                    floatTarget = FLOAT
                }
                OUT.intSource = INT
                OUT.structSource  = {
                    intTarget = INT,
                    floatTarget = FLOAT
                }
            end
            function run()
            end
        )";

        auto sourceScript = m_logicEngine.createLuaScript(luaScriptSource);
        auto targetScript = m_logicEngine.createLuaScript(luaScriptSource);

        const auto outputs = sourceScript->getOutputs();
        const auto inputs  = targetScript->getInputs();

        auto structTarget = inputs->getChild("structTarget");
        auto structSource = outputs->getChild("structSource");

        EXPECT_FALSE(m_logicEngine.link(*structSource, *structTarget));
        auto errors = m_logicEngine.getErrors();
        ASSERT_EQ(1u, errors.size());
        EXPECT_EQ("Can't link properties of complex types directly, currently only primitive properties can be linked", errors[0].message);

        EXPECT_FALSE(m_logicEngine.link(*outputs, *inputs));
        errors = m_logicEngine.getErrors();
        ASSERT_EQ(1u, errors.size());
        EXPECT_EQ("Can't link properties of complex types directly, currently only primitive properties can be linked", errors[0].message);
    }

    TEST_F(ALogicEngine_Linking, ProducesErrorOnLinkingArrays)
    {
        const auto  luaScriptSource = R"(
            function interface()
                IN.array = ARRAY(2, INT)
                OUT.array = ARRAY(2, INT)
            end
            function run()
            end
        )";

        auto sourceScript = m_logicEngine.createLuaScript(luaScriptSource);
        auto targetScript = m_logicEngine.createLuaScript(luaScriptSource);

        auto arrayTarget = targetScript->getInputs()->getChild("array");
        auto arraySource = sourceScript->getOutputs()->getChild("array");

        EXPECT_FALSE(m_logicEngine.link(*arraySource, *arrayTarget));
        auto errors = m_logicEngine.getErrors();
        ASSERT_EQ(1u, errors.size());
        EXPECT_EQ("Can't link properties of complex types directly, currently only primitive properties can be linked", errors[0].message);
    }

    TEST_F(ALogicEngine_Linking, ProducesErrorIfNotLinkedPropertyIsUnlinked_WhenAnotherLinkFromTheSameScriptExists)
    {
        const auto  luaScriptSource = R"(
            function interface()
                IN.intTarget1 = INT
                IN.intTarget2 = INT
                OUT.intSource = INT
            end
            function run()
            end
        )";

        auto sourceScript = m_logicEngine.createLuaScript(luaScriptSource);
        auto targetScript = m_logicEngine.createLuaScript(luaScriptSource);

        const auto sourceProperty = sourceScript->getOutputs()->getChild("intSource");
        const auto targetProperty1 = targetScript->getInputs()->getChild("intTarget1");
        const auto targetProperty2 = targetScript->getInputs()->getChild("intTarget2");

        EXPECT_TRUE(m_logicEngine.link(*sourceProperty, *targetProperty1));

        EXPECT_FALSE(m_logicEngine.unlink(*sourceProperty, *targetProperty2));

        const auto& errors = m_logicEngine.getErrors();
        ASSERT_EQ(1u, errors.size());
        EXPECT_EQ(errors[0].message, "Input property 'intTarget2' is not currently linked!");
    }

    TEST_F(ALogicEngine_Linking, ProducesErrorIfNotLinkedPropertyIsUnlinked_RamsesBinding)
    {
        auto targetBinding = m_logicEngine.createRamsesNodeBinding(*m_node, ERotationType::Euler_XYZ, "NodeBinding");
        const auto visibilityProperty = targetBinding->getInputs()->getChild("visibility");
        const auto unlinkedTargetProperty = targetBinding->getInputs()->getChild("translation");

        EXPECT_TRUE(m_logicEngine.link(m_sourceProperty, *visibilityProperty));

        EXPECT_FALSE(m_logicEngine.unlink(m_sourceProperty, *unlinkedTargetProperty));

        const auto& errors = m_logicEngine.getErrors();
        ASSERT_EQ(1u, errors.size());
        EXPECT_EQ(errors[0].message, "Input property 'translation' is not currently linked!");
    }

    TEST_F(ALogicEngine_Linking, UnlinksPropertiesWhichAreLinked)
    {
        ASSERT_TRUE(m_logicEngine.link(
            m_sourceProperty,
            m_targetProperty
        ));

        EXPECT_TRUE(m_logicEngine.unlink(
            m_sourceProperty,
            m_targetProperty
        ));
    }

    TEST_F(ALogicEngine_Linking, ProducesNoErrorsIfMultipleLinksFromSameSourceAreUnlinked)
    {
        auto targetScript2 = m_logicEngine.createLuaScript(m_minimalLinkScript);

        const auto targetProperty2 = targetScript2->getInputs()->getChild("target");

        m_logicEngine.link(
            m_sourceProperty,
            m_targetProperty
        );

        m_logicEngine.link(
            m_sourceProperty,
            *targetProperty2
        );

        EXPECT_TRUE(m_logicEngine.unlink(
            m_sourceProperty,
            m_targetProperty
        ));

        EXPECT_TRUE(m_logicEngine.unlink(
            m_sourceProperty,
            *targetProperty2
        ));
    }

    TEST_F(ALogicEngine_Linking, PropagatesOutputsToInputsIfLinked)
    {
        auto sourceScript = m_logicEngine.createLuaScript(m_linkScriptMultipleTypes);
        auto targetScript = m_logicEngine.createLuaScript(m_linkScriptMultipleTypes);

        auto output = sourceScript->getOutputs()->getChild("source_INT");
        auto input  = targetScript->getInputs()->getChild("target_INT");

        EXPECT_TRUE(m_logicEngine.link(*output, *input));

        sourceScript->getInputs()->getChild("target_INT")->set<int32_t>(42);

        m_logicEngine.update();

        EXPECT_EQ(42, *targetScript->getOutputs()->getChild("source_INT")->get<int32_t>());
    }

    TEST_F(ALogicEngine_Linking, PropagatesOutputsToInputsIfLinked_Int64)
    {
        const std::string_view scriptSrc = R"(
            function interface()
                IN.data = INT64
                OUT.data = INT64
            end
            function run()
                OUT.data = IN.data
            end
        )";

        auto sourceScript = m_logicEngine.createLuaScript(scriptSrc);
        auto targetScript = m_logicEngine.createLuaScript(scriptSrc);

        auto output = sourceScript->getOutputs()->getChild("data");
        auto input = targetScript->getInputs()->getChild("data");

        EXPECT_TRUE(m_logicEngine.link(*output, *input));

        const int64_t value = static_cast<int64_t>(std::numeric_limits<int32_t>::max()) + 1;
        sourceScript->getInputs()->getChild("data")->set<int64_t>(value);

        EXPECT_TRUE(m_logicEngine.update());

        EXPECT_EQ(value, *targetScript->getOutputs()->getChild("data")->get<int64_t>());
    }

    TEST_F(ALogicEngine_Linking, PropagatesOutputsToInputsIfLinked_ArraysOfStructs)
    {
        const std::string_view scriptArrayOfStructs = R"(
            function interface()
                IN.data = ARRAY(3,
                    {
                        one = INT,
                        two = INT
                    }
                )
                OUT.data = ARRAY(3,
                    {
                        one = INT,
                        two = INT
                    }
                )
            end
            function run()
                OUT.data = IN.data
            end
        )";

        auto sourceScript = m_logicEngine.createLuaScript(scriptArrayOfStructs);
        auto targetScript = m_logicEngine.createLuaScript(scriptArrayOfStructs);

        auto output = sourceScript->getOutputs()->getChild("data")->getChild(1)->getChild("one");
        auto input = targetScript->getInputs()->getChild("data")->getChild(1)->getChild("two");

        EXPECT_TRUE(m_logicEngine.link(*output, *input));

        sourceScript->getInputs()->getChild("data")->getChild(1)->getChild("one")->set<int32_t>(42);

        m_logicEngine.update();

        EXPECT_EQ(42, *targetScript->getOutputs()->getChild("data")->getChild(1)->getChild("two")->get<int32_t>());
    }

    TEST_F(ALogicEngine_Linking, PropagatesOutputsToInputsIfLinked_StructOfArrays)
    {
        const std::string_view scriptArrayOfStructs = R"(
            function interface()
                IN.data =
                {
                    one = ARRAY(3, INT),
                    two = ARRAY(3, INT)
                }
                OUT.data =
                {
                    one = ARRAY(3, INT),
                    two = ARRAY(3, INT)
                }
            end
            function run()
                OUT.data = IN.data
            end
        )";

        auto sourceScript = m_logicEngine.createLuaScript(scriptArrayOfStructs);
        auto targetScript = m_logicEngine.createLuaScript(scriptArrayOfStructs);

        auto output = sourceScript->getOutputs()->getChild("data")->getChild("one")->getChild(1);
        auto input = targetScript->getInputs()->getChild("data")->getChild("two")->getChild(1);

        EXPECT_TRUE(m_logicEngine.link(*output, *input));

        sourceScript->getInputs()->getChild("data")->getChild("one")->getChild(1)->set<int32_t>(42);

        m_logicEngine.update();

        EXPECT_EQ(42, *targetScript->getOutputs()->getChild("data")->getChild("two")->getChild(1)->get<int32_t>());
    }

    TEST_F(ALogicEngine_Linking, DoesNotPropagateOutputsToInputsAfterUnlink)
    {
        const auto  luaScriptSource = R"(
            function interface()
                IN.intTarget = INT
                OUT.intSource = INT
            end
            function run()
                OUT.intSource = IN.intTarget
            end
        )";

        auto sourceScript = m_logicEngine.createLuaScript(luaScriptSource);
        auto targetScript = m_logicEngine.createLuaScript(luaScriptSource);

        auto output = sourceScript->getOutputs()->getChild("intSource");
        auto input  = targetScript->getInputs()->getChild("intTarget");

        EXPECT_TRUE(m_logicEngine.link(*output, *input));
        sourceScript->getInputs()->getChild("intTarget")->set(42);

        EXPECT_TRUE(m_logicEngine.unlink(
            *output,
            *input
        ));

        m_logicEngine.update();

        EXPECT_EQ(0, *targetScript->getOutputs()->getChild("intSource")->get<int32_t>());
    }

    // TODO Violin add test with 2 scripts , one input in each
    TEST_F(ALogicEngine_Linking, PropagatesOneOutputToMultipleInputs)
    {
        const auto  luaScriptSource1 = R"(
            function interface()
                OUT.intSource = INT
            end
            function run()
                OUT.intSource = 5
            end
        )";

        const auto  luaScriptSource2 = R"(
            function interface()
                IN.intTarget1 = INT
                IN.intTarget2 = INT
            end
            function run()
            end
        )";

        auto sourceScript = m_logicEngine.createLuaScript(luaScriptSource1);
        auto targetScript = m_logicEngine.createLuaScript(luaScriptSource2);

        auto output = sourceScript->getOutputs()->getChild("intSource");
        auto input1 = targetScript->getInputs()->getChild("intTarget1");
        auto input2 = targetScript->getInputs()->getChild("intTarget2");

        EXPECT_TRUE(m_logicEngine.link(*output, *input1));
        EXPECT_TRUE(m_logicEngine.link(*output, *input2));

        m_logicEngine.update();

        EXPECT_EQ(5, *targetScript->getInputs()->getChild("intTarget1")->get<int32_t>());
        EXPECT_EQ(5, *targetScript->getInputs()->getChild("intTarget2")->get<int32_t>());

        EXPECT_TRUE(m_logicEngine.unlink(*output, *input1));
        input1->set(6);

        m_logicEngine.update();

        EXPECT_EQ(6, *input1->get<int32_t>());
        EXPECT_EQ(5, *input2->get<int32_t>());
    }

    // TODO Violin need more tests - what is with default values after unlinking?
    TEST_F(ALogicEngine_Linking, PropagatesOutputsToInputsIfLinkedForRamsesAppearanceBindings)
    {
        const auto  luaScriptSource = R"(
            function interface()
                IN.floatInput = FLOAT
                OUT.floatOutput = FLOAT
            end
            function run()
                OUT.floatOutput = IN.floatInput
            end
        )";

        auto sourceScript  = m_logicEngine.createLuaScript(luaScriptSource);
        auto targetBinding = m_logicEngine.createRamsesAppearanceBinding(*m_appearance, "TargetBinding");

        auto sourceInput  = sourceScript->getInputs()->getChild("floatInput");
        auto sourceOutput = sourceScript->getOutputs()->getChild("floatOutput");
        auto targetInput  = targetBinding->getInputs()->getChild("floatUniform");

        m_logicEngine.link(*sourceOutput, *targetInput);

        sourceInput->set(47.11f);
        m_logicEngine.update();

        ramses::UniformInput floatUniform;
        m_appearance->getEffect().findUniformInput("floatUniform", floatUniform);
        float result = 0.0f;
        m_appearance->getInputValueFloat(floatUniform, result);
        EXPECT_FLOAT_EQ(47.11f, result);
    }

    TEST_F(ALogicEngine_Linking, PropagatesOutputsToInputsIfLinkedForRamsesCameraBindings)
    {
        const auto  luaScriptSource = R"(
            function interface()
                IN.floatInput = FLOAT
                OUT.floatOutput = FLOAT
            end
            function run()
                OUT.floatOutput = IN.floatInput
            end
        )";

        auto sourceScript = m_logicEngine.createLuaScript(luaScriptSource);
        auto targetBinding = m_logicEngine.createRamsesCameraBinding(*m_camera, "TargetBinding");

        auto sourceInput = sourceScript->getInputs()->getChild("floatInput");
        auto sourceOutput = sourceScript->getOutputs()->getChild("floatOutput");
        auto targetInput = targetBinding->getInputs()->getChild("frustum")->getChild("farPlane");

        m_logicEngine.link(*sourceOutput, *targetInput);

        sourceInput->set(47.11f);
        m_logicEngine.update();

        EXPECT_FLOAT_EQ(47.11f, m_camera->getFarPlane());
    }

    // TODO Violin test should actually test that the links propagates the value *even if the output is NOT set any more in the source script!*
    TEST_F(ALogicEngine_Linking, PropagatesValueIfLinkIsCreatedAndOutputValueIsSetBeforehand)
    {
        const auto  luaScriptSource1 = R"(
            function interface()
                OUT.output = INT
            end
            function run()
                OUT.output = 5
            end
        )";

        const auto  luaScriptSource2 = R"(
            function interface()
                IN.input = INT
            end
            function run()
            end
        )";

        auto sourceScript = m_logicEngine.createLuaScript(luaScriptSource1);
        auto targetScript = m_logicEngine.createLuaScript(luaScriptSource2);

        auto sourceOutput = sourceScript->getOutputs()->getChild("output");
        auto targetInput  = targetScript->getInputs()->getChild("input");

        //propagates source input to source output
        ASSERT_TRUE(m_logicEngine.update());
        EXPECT_EQ(5, *sourceOutput->get<int32_t>());
        EXPECT_EQ(0, *targetInput->get<int32_t>());

        ASSERT_TRUE(m_logicEngine.link(*sourceOutput, *targetInput));
        m_logicEngine.update();

        EXPECT_EQ(5, *targetInput->get<int32_t>());
    }

    TEST_F(ALogicEngine_Linking, PropagatesValueIfLinkIsCreatedAndInputValueIsSetBeforehand)
    {
        const auto  luaScriptSource1 = R"(
            function interface()
                OUT.output = INT
            end
            function run()
                OUT.output = 5
            end
        )";

        const auto  luaScriptSource2 = R"(
            function interface()
                IN.input = INT
            end
            function run()
            end
        )";

        auto        sourceScript = m_logicEngine.createLuaScript(luaScriptSource1);
        auto        targetScript = m_logicEngine.createLuaScript(luaScriptSource2);

        auto sourceOutput = sourceScript->getOutputs()->getChild("output");
        auto targetInput = targetScript->getInputs()->getChild("input");

        targetInput->set<int32_t>(100);
        ASSERT_TRUE(m_logicEngine.update());

        ASSERT_EQ(5, *sourceOutput->get<int32_t>());
        ASSERT_EQ(100, *targetInput->get<int32_t>());

        m_logicEngine.link(*sourceOutput, *targetInput);
        m_logicEngine.update();

        EXPECT_EQ(5, *targetInput->get<int32_t>());

        m_logicEngine.unlink(*sourceOutput, *targetInput);
        m_logicEngine.update();

        // Value was overwritten after link + update
        EXPECT_EQ(5, *targetInput->get<int32_t>());
    }

    TEST_F(ALogicEngine_Linking, ProducesErrorIfLinkIsCreatedBetweenDifferentLogicEngines)
    {
        LogicEngine otherLogicEngine;
        const auto  luaScriptSource = R"(
            function interface()
                IN.floatInput = FLOAT
                OUT.floatOutput = FLOAT
            end
            function run()
                OUT.floatOutput = IN.floatInput
            end
        )";

        auto sourceScript = m_logicEngine.createLuaScript(luaScriptSource, {}, "SourceScript");
        auto targetScript = otherLogicEngine.createLuaScript(luaScriptSource, {}, "TargetScript");

        const auto sourceOutput = sourceScript->getOutputs()->getChild("floatOutput");
        const auto targetInput  = targetScript->getInputs()->getChild("floatInput");

        EXPECT_FALSE(m_logicEngine.link(*sourceOutput, *targetInput));
        {
            auto errors = m_logicEngine.getErrors();
            ASSERT_EQ(1u, errors.size());
            EXPECT_EQ("LogicNode 'TargetScript' is not an instance of this LogicEngine", errors[0].message);
        }

        EXPECT_FALSE(otherLogicEngine.link(*sourceOutput, *targetInput));
        {
            auto errors = otherLogicEngine.getErrors();
            ASSERT_EQ(1u, errors.size());
            EXPECT_EQ("LogicNode 'SourceScript' is not an instance of this LogicEngine", errors[0].message);
        }
    }

    TEST_F(ALogicEngine_Linking, PropagatesValuesFromMultipleOutputScriptsToOneInputScript)
    {
        const auto  sourceScript = R"(
            function interface()
                IN.floatInput = FLOAT
                OUT.floatOutput = FLOAT
            end
            function run()
                OUT.floatOutput = IN.floatInput
            end
        )";
        const auto  targetScript = R"(
            function interface()
                IN.floatInput1 = FLOAT
                IN.floatInput2 = FLOAT
                OUT.floatOutput1 = FLOAT
                OUT.floatOutput2 = FLOAT
            end
            function run()
                OUT.floatOutput1 = IN.floatInput1
                OUT.floatOutput2 = IN.floatInput2
            end
        )";

        auto scriptA = m_logicEngine.createLuaScript(sourceScript);
        auto scriptB = m_logicEngine.createLuaScript(sourceScript);
        auto scriptC = m_logicEngine.createLuaScript(targetScript);

        auto inputA  = scriptA->getInputs()->getChild("floatInput");
        auto outputA = scriptA->getOutputs()->getChild("floatOutput");
        auto inputB  = scriptB->getInputs()->getChild("floatInput");
        auto outputB = scriptB->getOutputs()->getChild("floatOutput");

        auto inputC1  = scriptC->getInputs()->getChild("floatInput1");
        auto inputC2  = scriptC->getInputs()->getChild("floatInput2");
        auto outputC1 = scriptC->getOutputs()->getChild("floatOutput1");
        auto outputC2 = scriptC->getOutputs()->getChild("floatOutput2");

        m_logicEngine.link(*outputA, *inputC1);
        m_logicEngine.link(*outputB, *inputC2);

        inputA->set(42.f);
        inputB->set(24.f);

        m_logicEngine.update();

        EXPECT_FLOAT_EQ(42.f, *outputC1->get<float>());
        EXPECT_FLOAT_EQ(24.f, *outputC2->get<float>());
    }

    TEST_F(ALogicEngine_Linking, PropagatesValuesFromOutputScriptToMultipleInputScripts)
    {
        const auto  scriptSource = R"(
            function interface()
                IN.floatInput = FLOAT
                OUT.floatOutput = FLOAT
            end
            function run()
                OUT.floatOutput = IN.floatInput
            end
        )";

        auto scriptA = m_logicEngine.createLuaScript(scriptSource);
        auto scriptB = m_logicEngine.createLuaScript(scriptSource);
        auto scriptC = m_logicEngine.createLuaScript(scriptSource);

        auto inputA  = scriptA->getInputs()->getChild("floatInput");
        auto outputA = scriptA->getOutputs()->getChild("floatOutput");
        auto inputB  = scriptB->getInputs()->getChild("floatInput");
        auto outputB = scriptB->getOutputs()->getChild("floatOutput");
        auto inputC  = scriptC->getInputs()->getChild("floatInput");
        auto outputC = scriptC->getOutputs()->getChild("floatOutput");

        m_logicEngine.link(*outputA, *inputB);
        m_logicEngine.link(*outputA, *inputC);

        inputA->set<float>(42.f);

        m_logicEngine.update();

        EXPECT_FLOAT_EQ(42.f, *outputB->get<float>());
        EXPECT_FLOAT_EQ(42.f, *outputC->get<float>());
    }

    TEST_F(ALogicEngine_Linking, PropagatesOutputToMultipleScriptsWithMultipleInputs)
    {
        const auto  sourceScript = R"(
            function interface()
                IN.floatInput = FLOAT
                OUT.floatOutput = FLOAT
            end
            function run()
                OUT.floatOutput = IN.floatInput
            end
        )";
        const auto  targetScript = R"(
            function interface()
                IN.floatInput1 = FLOAT
                IN.floatInput2 = FLOAT
                OUT.floatOutput1 = FLOAT
                OUT.floatOutput2 = FLOAT
            end
            function run()
                OUT.floatOutput1 = IN.floatInput1
                OUT.floatOutput2 = IN.floatInput2
            end
        )";

        auto scriptA = m_logicEngine.createLuaScript(sourceScript);
        auto scriptB = m_logicEngine.createLuaScript(targetScript);
        auto scriptC = m_logicEngine.createLuaScript(targetScript);

        auto inputA  = scriptA->getInputs()->getChild("floatInput");
        auto outputA = scriptA->getOutputs()->getChild("floatOutput");

        auto inputB1  = scriptB->getInputs()->getChild("floatInput1");
        auto inputB2  = scriptB->getInputs()->getChild("floatInput2");
        auto outputB1 = scriptB->getOutputs()->getChild("floatOutput1");
        auto outputB2 = scriptB->getOutputs()->getChild("floatOutput2");
        auto inputC1  = scriptC->getInputs()->getChild("floatInput1");
        auto inputC2  = scriptC->getInputs()->getChild("floatInput2");
        auto outputC1 = scriptC->getOutputs()->getChild("floatOutput1");
        auto outputC2 = scriptC->getOutputs()->getChild("floatOutput2");

        m_logicEngine.link(*outputA, *inputB1);
        m_logicEngine.link(*outputA, *inputB2);
        m_logicEngine.link(*outputA, *inputC1);
        m_logicEngine.link(*outputA, *inputC2);

        inputA->set(42.f);

        m_logicEngine.update();

        EXPECT_FLOAT_EQ(42.f, *outputB1->get<float>());
        EXPECT_FLOAT_EQ(42.f, *outputB2->get<float>());
        EXPECT_FLOAT_EQ(42.f, *outputC1->get<float>());
        EXPECT_FLOAT_EQ(42.f, *outputC2->get<float>());
    }

    TEST_F(ALogicEngine_Linking, DoesNotPropagateValuesIfScriptIsDestroyed)
    {
        const auto  scriptSource = R"(
            function interface()
                IN.floatInput = FLOAT
                OUT.floatOutput = FLOAT
            end
            function run()
                OUT.floatOutput = IN.floatInput
            end
        )";

        auto scriptA = m_logicEngine.createLuaScript(scriptSource);
        auto scriptB = m_logicEngine.createLuaScript(scriptSource);
        auto scriptC = m_logicEngine.createLuaScript(scriptSource);

        auto inputA  = scriptA->getInputs()->getChild("floatInput");
        auto outputA = scriptA->getOutputs()->getChild("floatOutput");
        auto inputB  = scriptB->getInputs()->getChild("floatInput");
        auto outputB = scriptB->getOutputs()->getChild("floatOutput");
        auto inputC  = scriptC->getInputs()->getChild("floatInput");
        auto outputC = scriptC->getOutputs()->getChild("floatOutput");

        m_logicEngine.link(*outputA, *inputB);
        m_logicEngine.link(*outputB, *inputC);

        m_logicEngine.destroy(*scriptB);

        inputA->set(42.f);

        m_logicEngine.update();

        EXPECT_FLOAT_EQ(42.f, *outputA->get<float>());
        EXPECT_FLOAT_EQ(0.f,  *inputC->get<float>());
        EXPECT_FLOAT_EQ(0.f,  *outputC->get<float>());
    }

    TEST_F(ALogicEngine_Linking, LinksNestedPropertiesBetweenScripts)
    {
        const auto  srcScriptA = R"(
            function interface()
                OUT.output = STRING
                OUT.nested = {
                    str1 = STRING,
                    str2 = STRING
                }
            end
            function run()
                OUT.output = "foo"
                OUT.nested = {str1 = "str1", str2 = "str2"}
            end
        )";
        const auto  srcScriptB = R"(
            function interface()
                IN.input = STRING
                IN.nested = {
                    str1 = STRING,
                    str2 = STRING
                }
                OUT.concat_all = STRING
            end
            function run()
                OUT.concat_all = IN.input .. " {" .. IN.nested.str1 .. ", " .. IN.nested.str2 .. "}"
            end
        )";

        // Create scripts in reversed order to make it more likely that order will be wrong unless ordered by dependencies
        auto scriptB = m_logicEngine.createLuaScript(srcScriptB);
        auto scriptA = m_logicEngine.createLuaScript(srcScriptA);

        auto scriptAOutput = scriptA->getOutputs()->getChild("output");
        auto scriptAnested_str1 = scriptA->getOutputs()->getChild("nested")->getChild("str1");
        auto scriptAnested_str2 = scriptA->getOutputs()->getChild("nested")->getChild("str2");

        auto scriptBInput = scriptB->getInputs()->getChild("input");
        auto scriptBnested_str1 = scriptB->getInputs()->getChild("nested")->getChild("str1");
        auto scriptBnested_str2 = scriptB->getInputs()->getChild("nested")->getChild("str2");

        // Do a crossover link between nested property and non-nested property
        EXPECT_TRUE(m_logicEngine.link(*scriptAOutput, *scriptBnested_str1));
        EXPECT_TRUE(m_logicEngine.link(*scriptAnested_str1, *scriptBInput));
        EXPECT_TRUE(m_logicEngine.link(*scriptAnested_str2, *scriptBnested_str2));

        EXPECT_TRUE(m_logicEngine.update());

        auto scriptB_concatenated = scriptB->getOutputs()->getChild("concat_all");
        EXPECT_EQ(std::string("str1 {foo, str2}"), *scriptB_concatenated->get<std::string>());
    }

    TEST_F(ALogicEngine_Linking, LinksNestedScriptPropertiesToBindingInputs)
    {
        const auto  scriptSrc = R"(
            function interface()
                OUT.nested = {
                    bool = BOOL,
                    vec3f = VEC3F
                }
            end
            function run()
                OUT.nested = {bool = false, vec3f = {0.1, 0.2, 0.3}}
            end
        )";

        auto script = m_logicEngine.createLuaScript(scriptSrc);
        // TODO Violin add appearance binding here too, once test PR #305 is merged
        auto nodeBinding = m_logicEngine.createRamsesNodeBinding(*m_node, ERotationType::Euler_XYZ, "NodeBinding");

        auto nestedOutput_bool = script->getOutputs()->getChild("nested")->getChild("bool");
        auto nestedOutput_vec3f = script->getOutputs()->getChild("nested")->getChild("vec3f");

        auto nodeBindingInput_bool = nodeBinding->getInputs()->getChild("visibility");
        auto nodeBindingInput_vec3f = nodeBinding->getInputs()->getChild("translation");

        ASSERT_TRUE(m_logicEngine.link(*nestedOutput_bool, *nodeBindingInput_bool));
        ASSERT_TRUE(m_logicEngine.link(*nestedOutput_vec3f, *nodeBindingInput_vec3f));

        ASSERT_TRUE(m_logicEngine.update());

        EXPECT_EQ(false, *nodeBindingInput_bool->get<bool>());
        EXPECT_THAT(*nodeBindingInput_vec3f->get<vec3f>(), ::testing::ElementsAre(0.1f, 0.2f, 0.3f));
    }

    TEST_F(ALogicEngine_Linking, PropagatesValuesCorrectlyAfterUnlink)
    {
        /*
         *            --> ScriptB
         *          /            \
         *  ScriptA ---------------->ScriptC
         */

        const auto  sourceScript = R"(
            function interface()
                IN.floatInput = FLOAT
                OUT.floatOutput = FLOAT
            end
            function run()
                OUT.floatOutput = IN.floatInput
            end
        )";
        const auto  targetScript = R"(
            function interface()
                IN.floatInput1 = FLOAT
                IN.floatInput2 = FLOAT
                OUT.floatOutput1 = FLOAT
                OUT.floatOutput2 = FLOAT
            end
            function run()
                OUT.floatOutput1 = IN.floatInput1
                OUT.floatOutput2 = IN.floatInput2
            end
        )";

        auto scriptA = m_logicEngine.createLuaScript(sourceScript);
        auto scriptB = m_logicEngine.createLuaScript(sourceScript);
        auto scriptC = m_logicEngine.createLuaScript(targetScript);

        auto scriptAInput  = scriptA->getInputs()->getChild("floatInput");
        auto scriptAOutput = scriptA->getOutputs()->getChild("floatOutput");

        auto scriptBInput = scriptB->getInputs()->getChild("floatInput");
        auto scriptBOutput = scriptB->getOutputs()->getChild("floatOutput");

        auto scriptCInput1 = scriptC->getInputs()->getChild("floatInput1");
        auto scriptCInput2 = scriptC->getInputs()->getChild("floatInput2");
        auto scriptCOutput1 = scriptC->getOutputs()->getChild("floatOutput1");
        auto scriptCOutput2 = scriptC->getOutputs()->getChild("floatOutput2");

        m_logicEngine.link(*scriptAOutput, *scriptBInput);
        m_logicEngine.link(*scriptAOutput, *scriptCInput1);
        m_logicEngine.link(*scriptBOutput, *scriptCInput2);

        scriptAInput->set(42.f);

        m_logicEngine.update();

        EXPECT_FLOAT_EQ(42.f, *scriptCOutput1->get<float>());
        EXPECT_FLOAT_EQ(42.f, *scriptCOutput2->get<float>());

        /*
         *           ScriptB
         *                  \
         *  ScriptA ----------->ScriptC
         */
        m_logicEngine.unlink(*scriptAOutput, *scriptBInput);

        scriptBInput->set(23.f);

        m_logicEngine.update();

        EXPECT_FLOAT_EQ(42.f, *scriptCOutput1->get<float>());
        EXPECT_FLOAT_EQ(23.f, *scriptCOutput2->get<float>());
    }

    TEST_F(ALogicEngine_Linking, canDestroyBindingAfterUnlinkingFromScript)
    {
        RamsesNodeBinding& nodeBinding = *m_logicEngine.createRamsesNodeBinding(*m_node, ERotationType::Euler_XYZ, "");

        auto* outScript = m_logicEngine.createLuaScript(R"(
            function interface()
                OUT.out_vec3f = VEC3F
            end

            function run()
                OUT.out_vec3f = { 0.0, 0.0, 0.0 }
            end
            )");

        const Property* nodeTranslation = nodeBinding.getInputs()->getChild("translation");
        const Property* scriptOutVec3f = outScript->getOutputs()->getChild("out_vec3f");

        EXPECT_TRUE(m_logicEngine.link(*scriptOutVec3f, *nodeTranslation));
        EXPECT_TRUE(m_logicEngine.unlink(*scriptOutVec3f, *nodeTranslation));

        EXPECT_FALSE(m_logicEngine.m_impl->getApiObjects().getLogicNodeDependencies().isLinked(nodeBinding.m_impl));
        EXPECT_FALSE(m_logicEngine.m_impl->getApiObjects().getLogicNodeDependencies().isLinked(outScript->m_impl));
        EXPECT_FALSE(m_logicEngine.m_impl->getApiObjects().getLogicNodeDependencies().isLinked(nodeBinding.m_impl));
        EXPECT_FALSE(m_logicEngine.m_impl->getApiObjects().getLogicNodeDependencies().isLinked(outScript->m_impl));
        EXPECT_FALSE(m_logicEngine.m_impl->isLinked(*outScript));
        EXPECT_FALSE(m_logicEngine.m_impl->isLinked(nodeBinding));

        EXPECT_TRUE(m_logicEngine.destroy(nodeBinding));

        EXPECT_TRUE(m_logicEngine.update());
    }

    TEST_F(ALogicEngine_Linking, WHEN_ScriptWasUnlinkedFromBindingAndMultipleLinksDestroyed_THEN_UpdateDoesNotOverwriteBindingInputsByDanglingLinks)
    {
        RamsesNodeBinding& nodeBinding = *m_logicEngine.createRamsesNodeBinding(*m_node, ERotationType::Euler_XYZ, "");

        auto* outScript = m_logicEngine.createLuaScript(R"(
            function interface()
                OUT.translation = VEC3F
                OUT.visibility = BOOL
            end

            function run()
            end
            )");

        rlogic::Property* nodeTranslation = nodeBinding.getInputs()->getChild("translation");
        rlogic::Property* nodeVisibility = nodeBinding.getInputs()->getChild("visibility");

        const rlogic::Property* scriptTranslation = outScript->getOutputs()->getChild("translation");
        const rlogic::Property* scriptVisiblity = outScript->getOutputs()->getChild("visibility");

        EXPECT_TRUE(m_logicEngine.link(*scriptVisiblity, *nodeVisibility));
        EXPECT_TRUE(m_logicEngine.link(*scriptTranslation, *nodeTranslation));

        EXPECT_TRUE(m_logicEngine.update());

        EXPECT_TRUE(m_logicEngine.unlink(*scriptVisiblity, *nodeVisibility));
        EXPECT_TRUE(m_logicEngine.unlink(*scriptTranslation, *nodeTranslation));

        EXPECT_FALSE(m_logicEngine.m_impl->getApiObjects().getLogicNodeDependencies().isLinked(nodeBinding.m_impl));
        EXPECT_FALSE(m_logicEngine.m_impl->getApiObjects().getLogicNodeDependencies().isLinked(outScript->m_impl));
        EXPECT_FALSE(m_logicEngine.m_impl->getApiObjects().getLogicNodeDependencies().isLinked(nodeBinding.m_impl));
        EXPECT_FALSE(m_logicEngine.m_impl->getApiObjects().getLogicNodeDependencies().isLinked(outScript->m_impl));
        EXPECT_FALSE(m_logicEngine.m_impl->isLinked(*outScript));
        EXPECT_FALSE(m_logicEngine.m_impl->isLinked(nodeBinding));

        EXPECT_TRUE(m_logicEngine.destroy(*outScript));

        // Set some custom values after unlink
        const vec3f translationValues{ 11.f, 12.0f, 13.0f };
        nodeTranslation->set<vec3f>(translationValues);
        nodeVisibility->set<bool>(false);

        EXPECT_TRUE(m_logicEngine.update());

        // Check that custom values are kept
        EXPECT_EQ(*nodeTranslation->get<vec3f>(), translationValues);
        EXPECT_FALSE(*nodeVisibility->get<bool>());
    }

    class ALogicEngine_Linking_WithFiles : public ALogicEngine_Linking
    {
    protected:
        WithTempDirectory tempFolder;
    };

    TEST_F(ALogicEngine_Linking_WithFiles, PreservesLinksBetweenScriptsAfterSavingAndLoading)
    {
        {
            /*
             *            ->  ScriptB --
             *          /               \
             *  ScriptA ------------------> ScriptC
             */

            LogicEngine tmpLogicEngine;
            LuaConfig config;
            config.addStandardModuleDependency(EStandardModule::Base);
            const auto  srcScriptAB = R"(
                function interface()
                    IN.input = STRING
                    OUT.output = STRING
                end
                function run()
                    OUT.output = "forward " .. tostring(IN.input)
                end
            )";
            const auto  srcScriptCsrc = R"(
                function interface()
                    IN.fromA = STRING
                    IN.fromB = STRING
                    OUT.concatenate_AB = STRING
                end
                function run()
                    OUT.concatenate_AB = "A: " .. IN.fromA .. " & B: " .. IN.fromB
                end
            )";

            // Create them in reversed order to make sure they are ordered wrongly if not ordered explicitly
            auto scriptC = tmpLogicEngine.createLuaScript(srcScriptCsrc, config, "ScriptC");
            auto scriptB = tmpLogicEngine.createLuaScript(srcScriptAB, config, "ScriptB");
            auto scriptA = tmpLogicEngine.createLuaScript(srcScriptAB, config, "ScriptA");

            auto scriptAInput  = scriptA->getInputs()->getChild("input");
            auto scriptAOutput = scriptA->getOutputs()->getChild("output");

            auto scriptBInput = scriptB->getInputs()->getChild("input");
            auto scriptBOutput = scriptB->getOutputs()->getChild("output");

            auto scriptC_fromA = scriptC->getInputs()->getChild("fromA");
            auto scriptC_fromB = scriptC->getInputs()->getChild("fromB");
            auto scriptC_concatenate_AB = scriptC->getOutputs()->getChild("concatenate_AB");

            tmpLogicEngine.link(*scriptAOutput, *scriptBInput);
            tmpLogicEngine.link(*scriptAOutput, *scriptC_fromA);
            tmpLogicEngine.link(*scriptBOutput, *scriptC_fromB);

            scriptAInput->set<std::string>("'From A'");

            tmpLogicEngine.update();

            ASSERT_EQ(std::string("A: forward 'From A' & B: forward forward 'From A'"), *scriptC_concatenate_AB->get<std::string>());

            tmpLogicEngine.saveToFile("links.bin");
        }

        {
            ASSERT_TRUE(m_logicEngine.loadFromFile("links.bin"));

            // Load all scripts and their properties
            auto scriptC = m_logicEngine.findScript("ScriptC");
            auto scriptB = m_logicEngine.findScript("ScriptB");
            auto scriptA = m_logicEngine.findScript("ScriptA");

            auto scriptAInput = scriptA->getInputs()->getChild("input");
            auto scriptAOutput = scriptA->getOutputs()->getChild("output");

            auto scriptBInput = scriptB->getInputs()->getChild("input");
            auto scriptBOutput = scriptB->getOutputs()->getChild("output");

            auto scriptC_fromA = scriptC->getInputs()->getChild("fromA");
            auto scriptC_fromB = scriptC->getInputs()->getChild("fromB");
            auto scriptC_concatenate_AB = scriptC->getOutputs()->getChild("concatenate_AB");

            // Internal check that deserialization did not result in more link copies
            EXPECT_EQ(scriptBInput->m_impl->getLinkedIncomingProperty(), scriptAOutput->m_impl.get());
            EXPECT_EQ(scriptC_fromA->m_impl->getLinkedIncomingProperty(), scriptAOutput->m_impl.get());
            EXPECT_EQ(scriptC_fromB->m_impl->getLinkedIncomingProperty(), scriptBOutput->m_impl.get());
            EXPECT_THAT(scriptAOutput->m_impl->getLinkedOutgoingProperties(), ::testing::UnorderedElementsAreArray({ scriptBInput->m_impl.get(), scriptC_fromA->m_impl.get() }));
            EXPECT_EQ(scriptBOutput->m_impl->getLinkedOutgoingProperties().size(), 1u);
            EXPECT_EQ(scriptBOutput->m_impl->getLinkedOutgoingProperties()[0], scriptC_fromB->m_impl.get());
            EXPECT_TRUE(scriptC_concatenate_AB->m_impl->getLinkedOutgoingProperties().empty());

            // Before update, values should be still as before saving
            EXPECT_EQ(std::string("forward 'From A'"), *scriptAOutput->get<std::string>());
            EXPECT_EQ(std::string("forward forward 'From A'"), *scriptBOutput->get<std::string>());
            EXPECT_EQ(std::string("A: forward 'From A' & B: forward forward 'From A'"), *scriptC_concatenate_AB->get<std::string>());

            EXPECT_TRUE(m_logicEngine.update());

            // Values should be still the same - because the data didn't change
            EXPECT_EQ(std::string("forward 'From A'"), *scriptAOutput->get<std::string>());
            EXPECT_EQ(std::string("forward forward 'From A'"), *scriptBOutput->get<std::string>());
            EXPECT_EQ(std::string("A: forward 'From A' & B: forward forward 'From A'"), *scriptC_concatenate_AB->get<std::string>());

            // Set different data manually
            EXPECT_TRUE(scriptAInput->set<std::string>("'A++'"));
            // linked inputs cannot be set manually, so this will fail
            EXPECT_FALSE(scriptBInput->set<std::string>("xxx"));
            EXPECT_FALSE(scriptC_fromA->set<std::string>("yyy"));
            EXPECT_FALSE(scriptC_fromB->set<std::string>("zzz"));

            EXPECT_TRUE(m_logicEngine.update());

            EXPECT_EQ(std::string("forward 'A++'"), *scriptAOutput->get<std::string>());
            EXPECT_EQ(std::string("forward forward 'A++'"), *scriptBOutput->get<std::string>());
            EXPECT_EQ(std::string("A: forward 'A++' & B: forward forward 'A++'"), *scriptC_concatenate_AB->get<std::string>());
        }
    }

    TEST_F(ALogicEngine_Linking_WithFiles, PreservesNestedLinksBetweenScriptsAfterSavingAndLoading)
    {
        {
            LogicEngine tmpLogicEngine;
            const auto  srcScriptA = R"(
                function interface()
                    IN.appendixNestedStr2 = STRING
                    OUT.output = STRING
                    OUT.nested = {
                        str1 = STRING,
                        str2 = STRING
                    }
                end
                function run()
                    OUT.output = "foo"
                    OUT.nested = {str1 = "str1", str2 = "str2" .. IN.appendixNestedStr2}
                end
            )";
            const auto  srcScriptB = R"(
                function interface()
                    IN.input = STRING
                    IN.nested = {
                        str1 = STRING,
                        str2 = STRING
                    }
                    OUT.concat_all = STRING
                end
                function run()
                    OUT.concat_all = IN.input .. " {" .. IN.nested.str1 .. ", " .. IN.nested.str2 .. "}"
                end
            )";

            // Create scripts in reversed order to make it more likely that order will be wrong unless ordered by dependencies
            auto scriptB = tmpLogicEngine.createLuaScript(srcScriptB, {}, "ScriptB");
            auto scriptA = tmpLogicEngine.createLuaScript(srcScriptA, {}, "ScriptA");

            auto scriptAOutput = scriptA->getOutputs()->getChild("output");
            auto scriptAnested_str1 = scriptA->getOutputs()->getChild("nested")->getChild("str1");
            auto scriptAnested_str2 = scriptA->getOutputs()->getChild("nested")->getChild("str2");

            auto scriptBInput = scriptB->getInputs()->getChild("input");
            auto scriptBnested_str1 = scriptB->getInputs()->getChild("nested")->getChild("str1");
            auto scriptBnested_str2 = scriptB->getInputs()->getChild("nested")->getChild("str2");

            // Do a crossover link between nested property and non-nested property
            ASSERT_TRUE(tmpLogicEngine.link(*scriptAOutput, *scriptBnested_str1));
            ASSERT_TRUE(tmpLogicEngine.link(*scriptAnested_str1, *scriptBInput));
            ASSERT_TRUE(tmpLogicEngine.link(*scriptAnested_str2, *scriptBnested_str2));

            ASSERT_TRUE(tmpLogicEngine.update());

            auto scriptB_concatenated = scriptB->getOutputs()->getChild("concat_all");
            ASSERT_EQ(std::string("str1 {foo, str2}"), *scriptB_concatenated->get<std::string>());

            tmpLogicEngine.saveToFile("nested_links.bin");
        }

        {
            ASSERT_TRUE(m_logicEngine.loadFromFile("nested_links.bin"));

            // Load all scripts and their properties
            auto scriptA = m_logicEngine.findScript("ScriptA");
            auto scriptB = m_logicEngine.findScript("ScriptB");

            auto scriptAOutput = scriptA->getOutputs()->getChild("output");
            auto scriptAnested_str1 = scriptA->getOutputs()->getChild("nested")->getChild("str1");
            auto scriptAnested_str2 = scriptA->getOutputs()->getChild("nested")->getChild("str2");

            auto scriptBInput = scriptB->getInputs()->getChild("input");
            auto scriptBnested_str1 = scriptB->getInputs()->getChild("nested")->getChild("str1");
            auto scriptBnested_str2 = scriptB->getInputs()->getChild("nested")->getChild("str2");
            auto scriptB_concatenated = scriptB->getOutputs()->getChild("concat_all");

            // Internal check that deserialization did not result in more link copies
            EXPECT_EQ(scriptBnested_str1->m_impl->getLinkedIncomingProperty(), scriptAOutput->m_impl.get());
            EXPECT_EQ(scriptBInput->m_impl->getLinkedIncomingProperty(), scriptAnested_str1->m_impl.get());
            EXPECT_EQ(scriptBnested_str2->m_impl->getLinkedIncomingProperty(), scriptAnested_str2->m_impl.get());
            EXPECT_EQ(scriptAOutput->m_impl->getLinkedOutgoingProperties().size(), 1u);
            EXPECT_EQ(scriptAnested_str1->m_impl->getLinkedOutgoingProperties().size(), 1u);
            EXPECT_EQ(scriptAnested_str2->m_impl->getLinkedOutgoingProperties().size(), 1u);
            EXPECT_EQ(scriptAOutput->m_impl->getLinkedOutgoingProperties()[0], scriptBnested_str1->m_impl.get());
            EXPECT_EQ(scriptAnested_str1->m_impl->getLinkedOutgoingProperties()[0], scriptBInput->m_impl.get());
            EXPECT_EQ(scriptAnested_str2->m_impl->getLinkedOutgoingProperties()[0], scriptBnested_str2->m_impl.get());

            // Before update, values should be still as before saving
            EXPECT_EQ(std::string("foo"), *scriptAOutput->get<std::string>());
            EXPECT_EQ(std::string("str1"), *scriptAnested_str1->get<std::string>());
            EXPECT_EQ(std::string("str2"), *scriptAnested_str2->get<std::string>());
            EXPECT_EQ(std::string("str1"), *scriptBInput->get<std::string>());
            EXPECT_EQ(std::string("foo"), *scriptBnested_str1->get<std::string>());
            EXPECT_EQ(std::string("str2"), *scriptBnested_str2->get<std::string>());
            EXPECT_EQ(std::string("str1 {foo, str2}"), *scriptB_concatenated->get<std::string>());

            EXPECT_TRUE(m_logicEngine.update());

            // Values should be still the same - because the data didn't change
            EXPECT_EQ(std::string("str1 {foo, str2}"), *scriptB_concatenated->get<std::string>());

            // Set different data manually
            auto scriptAappendix = scriptA->getInputs()->getChild("appendixNestedStr2");
            EXPECT_TRUE(scriptAappendix->set<std::string>("!bar"));
            // linked inputs cannot be set manually, so this will fail
            EXPECT_FALSE(scriptBInput->set<std::string>("xxx"));
            EXPECT_FALSE(scriptBnested_str1->set<std::string>("yyy"));
            EXPECT_FALSE(scriptBnested_str2->set<std::string>("zzz"));

            EXPECT_TRUE(m_logicEngine.update());

            EXPECT_EQ(std::string("str1 {foo, str2!bar}"), *scriptB_concatenated->get<std::string>());
        }
    }

    class ALogicEngine_Linking_WithBindings : public ALogicEngine_Linking_WithFiles
    {
    protected:
        ALogicEngine_Linking_WithBindings()
        {
            // TODO Violin clean this up once PR #305 went in
            std::array<const char*, 3> commandLineConfigForTest = { "test", "-l", "off" };
            ramses::RamsesFrameworkConfig frameworkConfig(static_cast<uint32_t>(commandLineConfigForTest.size()), commandLineConfigForTest.data());
            m_ramsesFramework = std::make_unique<ramses::RamsesFramework>(frameworkConfig);
            m_ramsesClient = m_ramsesFramework->createClient("TheClient");
            m_scene = m_ramsesClient->createScene(ramses::sceneId_t(1));
        }

        using ENodePropertyStaticIndex = rlogic::internal::ENodePropertyStaticIndex;

        static void ExpectValues(ramses::Node& node, ENodePropertyStaticIndex prop, vec3f expectedValues)
        {
            std::array<float, 3> values = { 0.0f, 0.0f, 0.0f };
            if (prop == ENodePropertyStaticIndex::Rotation)
            {
                ramses::ERotationConvention unused;
                (void)unused;
                node.getRotation(values[0], values[1], values[2], unused);
            }
            if (prop == ENodePropertyStaticIndex::Translation)
            {
                node.getTranslation(values[0], values[1], values[2]);
            }
            if (prop == ENodePropertyStaticIndex::Scaling)
            {
                node.getScaling(values[0], values[1], values[2]);
            }
            EXPECT_THAT(values, ::testing::ElementsAre(expectedValues[0], expectedValues[1], expectedValues[2]));
        }

        static void ExpectVec3f(ramses::Appearance& appearance, const char* uniformName, vec3f expectedValues)
        {
            std::array<float, 3> values = { 0.0f, 0.0f, 0.0f };
            ramses::UniformInput uniform;
            appearance.getEffect().findUniformInput(uniformName, uniform);
            appearance.getInputValueVector3f(uniform, values[0], values[1], values[2]);
            EXPECT_THAT(values, ::testing::ElementsAre(expectedValues[0], expectedValues[1], expectedValues[2]));
        }

        ramses::Effect& createTestEffect(std::string_view vertShader, std::string_view fragShader)
        {
            ramses::EffectDescription effectDesc;
            effectDesc.setVertexShader(vertShader.data());
            effectDesc.setFragmentShader(fragShader.data());
            return *m_scene->createEffect(effectDesc);
        }

        ramses::Appearance& createTestAppearance(ramses::Effect& effect)
        {
            return *m_scene->createAppearance(effect);
        }

        const std::string_view m_vertShader = R"(
            #version 300 es

            uniform highp vec3 uniform1;
            uniform highp vec3 uniform2;

            void main()
            {
                gl_Position = vec4(uniform1 + uniform2, 1.0);
            })";

        const std::string_view m_fragShader = R"(
            #version 300 es

            out lowp vec4 color;
            void main(void)
            {
                color = vec4(1.0, 0.0, 0.0, 1.0);
            })";

        std::unique_ptr<ramses::RamsesFramework> m_ramsesFramework;
        ramses::RamsesClient* m_ramsesClient = nullptr;
        ramses::Scene* m_scene = nullptr;
    };

    TEST_F(ALogicEngine_Linking_WithBindings, PreservesLinksToNodeBindingsAfterSavingAndLoadingFromFile)
    {
        auto ramsesNode1 = m_scene->createNode();
        auto ramsesNode2 = m_scene->createNode();

        ramsesNode1->setTranslation(1.1f, 1.2f, 1.3f);
        ramsesNode1->setRotation(2.1f, 2.2f, 2.3f, ramses::ERotationConvention::XYZ);
        ramsesNode1->setScaling(3.1f, 3.2f, 3.3f);

        ramsesNode2->setTranslation(11.1f, 11.2f, 11.3f);

        {
            LogicEngine tmpLogicEngine;
            const auto  scriptSrc = R"(
                function interface()
                    OUT.vec3f = VEC3F
                    OUT.visibility = BOOL
                end
                function run()
                    OUT.vec3f = {100.0, 200.0, 300.0}
                    OUT.visibility = false
                end
            )";

            auto script = tmpLogicEngine.createLuaScript(scriptSrc);
            auto nodeBinding1 = tmpLogicEngine.createRamsesNodeBinding(*ramsesNode1, ERotationType::Euler_XYZ, "NodeBinding1");
            auto nodeBinding2 = tmpLogicEngine.createRamsesNodeBinding(*ramsesNode2, ERotationType::Euler_XYZ, "NodeBinding2");

            auto scriptOutputVec3f = script->getOutputs()->getChild("vec3f");
            auto scriptOutputBool = script->getOutputs()->getChild("visibility");
            auto binding1TranslationInput = nodeBinding1->getInputs()->getChild("translation");
            auto binding2RotationInput = nodeBinding2->getInputs()->getChild("rotation");
            auto binding1VisibilityInput = nodeBinding1->getInputs()->getChild("visibility");

            ASSERT_TRUE(tmpLogicEngine.link(*scriptOutputBool, *binding1VisibilityInput));
            ASSERT_TRUE(tmpLogicEngine.link(*scriptOutputVec3f, *binding1TranslationInput));
            ASSERT_TRUE(tmpLogicEngine.link(*scriptOutputVec3f, *binding2RotationInput));

            ASSERT_TRUE(tmpLogicEngine.update());

            ASSERT_THAT(*binding1TranslationInput->get<vec3f>(), ::testing::ElementsAre(100.0f, 200.0f, 300.0f));
            ASSERT_THAT(*binding2RotationInput->get<vec3f>(), ::testing::ElementsAre(100.0f, 200.0f, 300.0f));
            ASSERT_EQ(false, *binding1VisibilityInput->get<bool>());

            ExpectValues(*ramsesNode1, ENodePropertyStaticIndex::Rotation, { 2.1f, 2.2f, 2.3f });
            ExpectValues(*ramsesNode1, ENodePropertyStaticIndex::Scaling, { 3.1f, 3.2f, 3.3f });
            ExpectValues(*ramsesNode1, ENodePropertyStaticIndex::Translation, { 100.0f, 200.0f, 300.0f });
            EXPECT_EQ(ramsesNode1->getVisibility(), ramses::EVisibilityMode::Invisible);

            ExpectValues(*ramsesNode2, ENodePropertyStaticIndex::Rotation, { 100.0f, 200.0f, 300.0f });
            ExpectValues(*ramsesNode2, ENodePropertyStaticIndex::Scaling, { 1.0f, 1.0f, 1.0f });
            ExpectValues(*ramsesNode2, ENodePropertyStaticIndex::Translation, { 11.1f, 11.2f, 11.3f });
            EXPECT_EQ(ramsesNode2->getVisibility(), ramses::EVisibilityMode::Visible);

            tmpLogicEngine.saveToFile("binding_links.bin");
        }

        // Make sure loading of bindings doesn't do anything to the node until update() is called
        // To test that, we reset one node's properties to default
        ramsesNode1->setTranslation(0.0f, 0.0f, 0.0f);
        ramsesNode1->setRotation(0.0f, 0.0f, 0.0f);
        ramsesNode1->setScaling(1.0f, 1.0f, 1.0f);
        ramsesNode1->setVisibility(ramses::EVisibilityMode::Visible);

        {
            ASSERT_TRUE(m_logicEngine.loadFromFile("binding_links.bin", m_scene));

            ExpectValues(*ramsesNode1, ENodePropertyStaticIndex::Rotation, { 0.0f, 0.0f, 0.0f });
            ExpectValues(*ramsesNode1, ENodePropertyStaticIndex::Scaling, { 1.0f, 1.0f, 1.0f });
            ExpectValues(*ramsesNode1, ENodePropertyStaticIndex::Translation, { 0.0f, 0.0f, 0.0f });
            EXPECT_EQ(ramsesNode1->getVisibility(), ramses::EVisibilityMode::Visible);

            auto nodeBinding1 = m_logicEngine.findNodeBinding("NodeBinding1");
            auto nodeBinding2 = m_logicEngine.findNodeBinding("NodeBinding2");

            auto binding1TranslationInput = nodeBinding1->getInputs()->getChild("translation");
            auto binding2RotationInput = nodeBinding2->getInputs()->getChild("rotation");
            auto notLinkedManualInputProperty = nodeBinding2->getInputs()->getChild("translation");
            auto bindingVisibilityInput = nodeBinding1->getInputs()->getChild("visibility");

            // These values should be overwritten by the link - set them to a different value to make sure that happens
            binding1TranslationInput->m_impl->setValue(vec3f{ 99.0f, 99.0f, 99.0f });
            binding2RotationInput->m_impl->setValue(vec3f{ 99.0f, 99.0f, 99.0f });
            bindingVisibilityInput->m_impl->setValue(true);
            // This should not be overwritten, but should keep the manual value instead
            ASSERT_TRUE(notLinkedManualInputProperty->set<vec3f>({100.0f, 101.0f, 102.0f}));
            EXPECT_TRUE(m_logicEngine.update());

            // These have default values
            ExpectValues(*ramsesNode1, ENodePropertyStaticIndex::Rotation, { 0.0f, 0.0f, 0.0f });
            ExpectValues(*ramsesNode1, ENodePropertyStaticIndex::Scaling, { 1.0f, 1.0f, 1.0f });
            // These came over the link
            ExpectValues(*ramsesNode1, ENodePropertyStaticIndex::Translation, { 100.0f, 200.0f, 300.0f });
            EXPECT_EQ(ramsesNode1->getVisibility(), ramses::EVisibilityMode::Invisible);

            // These came over the link
            ExpectValues(*ramsesNode2, ENodePropertyStaticIndex::Rotation, { 100.0f, 200.0f, 300.0f });
            // These came over manual set after loading
            ExpectValues(*ramsesNode2, ENodePropertyStaticIndex::Translation, { 100.0f, 101.0f, 102.0f });
            // These have default values
            ExpectValues(*ramsesNode2, ENodePropertyStaticIndex::Scaling, { 1.0f, 1.0f, 1.0f });
            EXPECT_EQ(ramsesNode2->getVisibility(), ramses::EVisibilityMode::Visible);
        }
    }

    TEST_F(ALogicEngine_Linking_WithBindings, PreservesLinksToAppearanceBindingsAfterSavingAndLoadingFromFile)
    {
        ramses::Effect& effect = createTestEffect(m_vertShader, m_fragShader);
        auto& appearance1 = createTestAppearance(effect);
        auto& appearance2 = createTestAppearance(effect);
        ramses::UniformInput uniform1;
        ramses::UniformInput uniform2;
        appearance1.getEffect().findUniformInput("uniform1", uniform1);
        appearance1.getEffect().findUniformInput("uniform2", uniform2);

        appearance1.setInputValueVector3f(uniform1, 1.1f, 1.2f, 1.3f);
        appearance1.setInputValueVector3f(uniform2, 2.1f, 2.2f, 2.3f);
        appearance2.setInputValueVector3f(uniform1, 3.1f, 3.2f, 3.3f);
        appearance2.setInputValueVector3f(uniform2, 4.1f, 4.2f, 4.3f);

        {
            LogicEngine tmpLogicEngine;
            const auto  scriptSrc = R"(
                function interface()
                    OUT.uniform = VEC3F
                end
                function run()
                    OUT.uniform = {100.0, 200.0, 300.0}
                end
            )";

            auto script = tmpLogicEngine.createLuaScript(scriptSrc);
            auto appBinding1 = tmpLogicEngine.createRamsesAppearanceBinding(appearance1, "AppBinding1");
            auto appBinding2 = tmpLogicEngine.createRamsesAppearanceBinding(appearance2, "AppBinding2");

            auto scriptOutput = script->getOutputs()->getChild("uniform");
            auto binding1uniform1 = appBinding1->getInputs()->getChild("uniform1");
            auto binding2uniform1 = appBinding2->getInputs()->getChild("uniform1");
            auto binding2uniform2 = appBinding2->getInputs()->getChild("uniform2");

            ASSERT_TRUE(tmpLogicEngine.link(*scriptOutput, *binding1uniform1));
            ASSERT_TRUE(tmpLogicEngine.link(*scriptOutput, *binding2uniform1));
            ASSERT_TRUE(tmpLogicEngine.link(*scriptOutput, *binding2uniform2));

            ASSERT_TRUE(tmpLogicEngine.update());

            ExpectVec3f(appearance1, "uniform1", { 100.0f, 200.0f, 300.0f });
            ExpectVec3f(appearance1, "uniform2", { 2.1f, 2.2f, 2.3f });
            ExpectVec3f(appearance2, "uniform1", { 100.0f, 200.0f, 300.0f });
            ExpectVec3f(appearance2, "uniform2", { 100.0f, 200.0f, 300.0f });

            tmpLogicEngine.saveToFile("binding_links.bin");
        }

        // Make sure loading of bindings doesn't do anything to the appearance until update() is called
        // To test that, we reset both appearances' properties to zeroes
        appearance1.setInputValueVector3f(uniform1, 0.0f, 0.0f, 0.0f);
        appearance1.setInputValueVector3f(uniform2, 0.0f, 0.0f, 0.0f);
        appearance2.setInputValueVector3f(uniform1, 0.0f, 0.0f, 0.0f);
        appearance2.setInputValueVector3f(uniform2, 0.0f, 0.0f, 0.0f);

        {
            ASSERT_TRUE(m_logicEngine.loadFromFile("binding_links.bin", m_scene));

            ExpectVec3f(appearance1, "uniform1", { 0.0f, 0.0f, 0.0f });
            ExpectVec3f(appearance1, "uniform2", { 0.0f, 0.0f, 0.0f });
            ExpectVec3f(appearance2, "uniform1", { 0.0f, 0.0f, 0.0f });
            ExpectVec3f(appearance2, "uniform2", { 0.0f, 0.0f, 0.0f });

            auto appBinding1 = m_logicEngine.findAppearanceBinding("AppBinding1");
            auto appBinding2 = m_logicEngine.findAppearanceBinding("AppBinding2");

            auto binding1uniform1 = appBinding1->getInputs()->getChild("uniform1");
            auto binding1uniform2 = appBinding1->getInputs()->getChild("uniform2");
            auto binding2uniform1 = appBinding2->getInputs()->getChild("uniform1");
            auto binding2uniform2 = appBinding2->getInputs()->getChild("uniform2");

            // These values should be overwritten by the link - set them to a different value to make sure that happens
            binding1uniform1->m_impl->setValue(vec3f{ 99.0f, 99.0f, 99.0f });
            // This should not be overwritten, but should keep the manual value instead, because no link points to it
            ASSERT_TRUE(binding1uniform2->set<vec3f>({ 100.0f, 101.0f, 102.0f }));
            // These values should be overwritten by the link - set them to a different value to make sure that happens
            binding2uniform1->m_impl->setValue(vec3f{ 99.0f, 99.0f, 99.0f });
            binding2uniform2->m_impl->setValue(vec3f{ 99.0f, 99.0f, 99.0f });
            EXPECT_TRUE(m_logicEngine.update());

            ExpectVec3f(appearance1, "uniform1", { 100.0f, 200.0f, 300.0f });
            ExpectVec3f(appearance1, "uniform2", { 100.0f, 101.0f, 102.0f });
            ExpectVec3f(appearance2, "uniform1", { 100.0f, 200.0f, 300.0f });
            ExpectVec3f(appearance2, "uniform2", { 100.0f, 200.0f, 300.0f });
        }
    }

    TEST_F(ALogicEngine_Linking_WithBindings, PreservesLinksToCameraBindingsAfterSavingAndLoadingFromFile)
    {
        auto camera1 = m_scene->createPerspectiveCamera();
        auto camera2 = m_scene->createPerspectiveCamera();

        camera1->setViewport(1, 2, 3u, 4u);
        camera1->setFrustum(30.f, 640.f / 480.f, 1.f, 2.f);
        camera2->setViewport(5, 6, 7u, 8u);
        camera2->setFrustum(15.f, 320.f / 240.f, 3.f, 4.f);

        {
            LogicEngine tmpLogicEngine;
                    const std::string_view scriptSrc = R"(
            function interface()
                OUT.vpOffsetX = INT
                OUT.farPlane = FLOAT
                end
                function run()
                    OUT.vpOffsetX = 19
                    OUT.farPlane = 7.8
                end
            )";

            auto script = tmpLogicEngine.createLuaScript(scriptSrc);
            auto cameraBinding1 = tmpLogicEngine.createRamsesCameraBinding(*camera1, "CameraBinding1");
            auto cameraBinding2 = tmpLogicEngine.createRamsesCameraBinding(*camera2, "CameraBinding2");

            auto scriptOutputOffset = script->getOutputs()->getChild("vpOffsetX");
            auto scriptOutputFarPlane = script->getOutputs()->getChild("farPlane");
            auto binding1cameraProperty1 = cameraBinding1->getInputs()->getChild("viewport")->getChild("offsetX");
            auto binding2cameraProperty1 = cameraBinding2->getInputs()->getChild("viewport")->getChild("offsetX");
            auto binding2cameraProperty2 = cameraBinding2->getInputs()->getChild("frustum")->getChild("farPlane");

            ASSERT_TRUE(tmpLogicEngine.link(*scriptOutputOffset, *binding1cameraProperty1));
            ASSERT_TRUE(tmpLogicEngine.link(*scriptOutputOffset, *binding2cameraProperty1));
            ASSERT_TRUE(tmpLogicEngine.link(*scriptOutputFarPlane, *binding2cameraProperty2));

            ASSERT_TRUE(tmpLogicEngine.update());

            EXPECT_EQ(camera1->getViewportX(), 19);
            EXPECT_EQ(camera1->getFarPlane(), 2.f);
            EXPECT_EQ(camera2->getViewportX(), 19);
            EXPECT_EQ(camera2->getFarPlane(), 7.8f);

            tmpLogicEngine.saveToFile("binding_links.bin");
        }

        // Make sure loading of bindings doesn't do anything to the camera until update() is called
        // To test that, we reset both cameras' properties
        camera1->setViewport(0, 0, 1u, 1u);
        camera1->setFrustum(0.1f, 0.1f, .1f, .2f);
        camera2->setViewport(0, 0, 1u, 1u);
        camera2->setFrustum(0.1f, 0.1f, .1f, .2f);
        {
            ASSERT_TRUE(m_logicEngine.loadFromFile("binding_links.bin", m_scene));

            EXPECT_EQ(camera1->getViewportX(), 0);
            EXPECT_EQ(camera1->getFarPlane(), .2f);
            EXPECT_EQ(camera2->getViewportX(), 0);
            EXPECT_EQ(camera2->getFarPlane(), .2f);

            auto cameraBinding1 = m_logicEngine.findCameraBinding("CameraBinding1");
            auto cameraBinding2 = m_logicEngine.findCameraBinding("CameraBinding2");

            auto binding1cameraProperty1 = cameraBinding1->getInputs()->getChild("viewport")->getChild("offsetX");
            auto binding1cameraProperty2 = cameraBinding1->getInputs()->getChild("frustum")->getChild("farPlane");
            auto binding2cameraProperty1 = cameraBinding2->getInputs()->getChild("viewport")->getChild("offsetX");
            auto binding2cameraProperty2 = cameraBinding2->getInputs()->getChild("frustum")->getChild("farPlane");

            // These values should be overwritten by the link - set them to a different value to make sure that happens
            binding1cameraProperty1->m_impl->setValue(100);
            // This should not be overwritten, but should keep the manual value instead, because no link points to it
            ASSERT_TRUE(binding1cameraProperty2->set<float>(100.f));
            // These values should be overwritten by the link - set them to a different value to make sure that happens
            binding2cameraProperty1->m_impl->setValue(100);
            binding2cameraProperty2->m_impl->setValue(100.f);
            EXPECT_TRUE(m_logicEngine.update());

            EXPECT_EQ(camera1->getViewportX(), 19);
            EXPECT_EQ(camera1->getFarPlane(), 100.f);
            EXPECT_EQ(camera2->getViewportX(), 19);
            EXPECT_EQ(camera2->getFarPlane(), 7.8f);
        }
    }

    TEST_F(ALogicEngine_Linking, ReportsNodeAsLinked_IFF_ItHasIncomingOrOutgoingLinks)
    {
        auto        scriptSource = R"(
            function interface()
                IN.input = {
                    inBool = BOOL
                }
                OUT.output = {
                    outBool = BOOL
                }
            end
            function run()
            end
        )";

        auto sourceScript = m_logicEngine.createLuaScript(scriptSource);
        auto middleScript = m_logicEngine.createLuaScript(scriptSource);
        auto targetBinding = m_logicEngine.createRamsesNodeBinding(*m_node, ERotationType::Euler_XYZ, "NodeBinding");

        auto sourceOutputBool = sourceScript->getOutputs()->getChild("output")->getChild("outBool");
        auto middleInputBool  = middleScript->getInputs()->getChild("input")->getChild("inBool");
        auto middleOutputBool = middleScript->getOutputs()->getChild("output")->getChild("outBool");
        auto targetInputBool  = targetBinding->getInputs()->getChild("visibility");

        m_logicEngine.link(*sourceOutputBool, *middleInputBool);
        m_logicEngine.link(*middleOutputBool, *targetInputBool);

        EXPECT_TRUE(m_logicEngine.isLinked(*sourceScript));
        EXPECT_TRUE(m_logicEngine.isLinked(*middleScript));
        EXPECT_TRUE(m_logicEngine.isLinked(*targetBinding));

        m_logicEngine.unlink(*middleOutputBool, *targetInputBool);

        EXPECT_TRUE(m_logicEngine.isLinked(*sourceScript));
        EXPECT_TRUE(m_logicEngine.isLinked(*middleScript));
        EXPECT_FALSE(m_logicEngine.isLinked(*targetBinding));

        m_logicEngine.unlink(*sourceOutputBool, *middleInputBool);

        EXPECT_FALSE(m_logicEngine.isLinked(*sourceScript));
        EXPECT_FALSE(m_logicEngine.isLinked(*middleScript));
        EXPECT_FALSE(m_logicEngine.isLinked(*targetBinding));
    }

    TEST_F(ALogicEngine_Linking, SetsAffectedNodesToDirtyAfterLinking)
    {
        auto        scriptSource = R"(
            function interface()
                IN.input = BOOL
                OUT.output = BOOL
            end
            function run()
            end
        )";

        auto sourceScript  = m_logicEngine.createLuaScript(scriptSource);
        auto targetBinding = m_logicEngine.createRamsesNodeBinding(*m_node, ERotationType::Euler_XYZ, "RamsesBinding");

        m_logicEngine.update();

        EXPECT_FALSE(sourceScript->m_impl.isDirty());
        EXPECT_FALSE(targetBinding->m_impl.isDirty());

        auto output = sourceScript->getOutputs()->getChild("output");
        auto input  = targetBinding->getInputs()->getChild("visibility");

        m_logicEngine.link(*output, *input);

        EXPECT_TRUE(sourceScript->m_impl.isDirty());
        EXPECT_TRUE(targetBinding->m_impl.isDirty());
    }

    TEST_F(ALogicEngine_Linking, SetsAffectedNodesToDirtyAfterLinkingWithStructs)
    {
        auto scriptSource = R"(
            function interface()
                IN.struct = {
                    inBool = BOOL
                }
                OUT.struct = {
                    outBool = BOOL
                }
            end
            function run()
            end
        )";

        auto sourceScript  = m_logicEngine.createLuaScript(scriptSource);
        auto targetScript = m_logicEngine.createLuaScript(scriptSource);

        m_logicEngine.update();

        EXPECT_FALSE(sourceScript->m_impl.isDirty());
        EXPECT_FALSE(targetScript->m_impl.isDirty());

        auto output = sourceScript->getOutputs()->getChild("struct")->getChild("outBool");
        auto input = targetScript->getInputs()->getChild("struct")->getChild("inBool");

        m_logicEngine.link(*output, *input);

        EXPECT_TRUE(sourceScript->m_impl.isDirty());
        EXPECT_TRUE(targetScript->m_impl.isDirty());
    }

    TEST_F(ALogicEngine_Linking, SetsNeitherTargetNodeNorSourceNodeToDirtyAfterUnlink)
    {
        auto        scriptSource = R"(
            function interface()
                IN.input = BOOL
                OUT.output = BOOL
            end
            function run()
            end
        )";

        auto sourceScript = m_logicEngine.createLuaScript(scriptSource);
        auto targetBinding = m_logicEngine.createRamsesNodeBinding(*m_node, ERotationType::Euler_XYZ, "RamsesBinding");

        auto output = sourceScript->getOutputs()->getChild("output");
        auto input  = targetBinding->getInputs()->getChild("visibility");

        m_logicEngine.link(*output, *input);

        m_logicEngine.update();

        EXPECT_FALSE(sourceScript->m_impl.isDirty());
        EXPECT_FALSE(targetBinding->m_impl.isDirty());

        m_logicEngine.unlink(*output, *input);

        EXPECT_FALSE(sourceScript->m_impl.isDirty());
        EXPECT_FALSE(targetBinding->m_impl.isDirty());
    }

    class ALogicEngine_Linking_Confidence : public ALogicEngine
    {
    protected:
        const std::string_view m_scriptNestedStructs = R"(
            function interface()
                IN.struct = {
                    nested = {
                        vec3f = VEC3F
                    }
                }

                OUT.struct = {
                    nested = {
                        vec3f = VEC3F
                    }
                }
            end

            function run()
            end
        )";

        const std::string_view m_scriptArrayOfStructs = R"(
            function interface()
                IN.array = ARRAY(2, {
                        vec3f = VEC3F
                    })

                OUT.array = ARRAY(2, {
                        vec3f = VEC3F
                    })
            end

            function run()
            end
        )";
    };

    TEST_F(ALogicEngine_Linking_Confidence, CanDestroyLinkedScriptsWithComplexTypes_WithLinkStillActive_DestroySourceFirst)
    {
        LuaScript& sourceScript(*m_logicEngine.createLuaScript(m_scriptNestedStructs));
        LuaScript& targetScript(*m_logicEngine.createLuaScript(m_scriptNestedStructs));

        const Property& sourceVec(*sourceScript.getOutputs()->getChild("struct")->getChild("nested")->getChild("vec3f"));
        Property& targetVec(*targetScript.getInputs()->getChild("struct")->getChild("nested")->getChild("vec3f"));

        EXPECT_TRUE(m_logicEngine.update());
        EXPECT_TRUE(m_logicEngine.link(sourceVec, targetVec));
        EXPECT_TRUE(m_logicEngine.update());
        EXPECT_TRUE(m_logicEngine.destroy(sourceScript));
        EXPECT_TRUE(m_logicEngine.update());
        EXPECT_TRUE(m_logicEngine.destroy(targetScript));
        EXPECT_TRUE(m_logicEngine.update());
    }

    TEST_F(ALogicEngine_Linking_Confidence, CanDestroyLinkedScriptsWithComplexTypes_WithLinkStillActive_DestroyTargetFirst)
    {
        LuaScript& sourceScript(*m_logicEngine.createLuaScript(m_scriptNestedStructs));
        LuaScript& targetScript(*m_logicEngine.createLuaScript(m_scriptNestedStructs));

        const Property& sourceVec(*sourceScript.getOutputs()->getChild("struct")->getChild("nested")->getChild("vec3f"));
        Property& targetVec(*targetScript.getInputs()->getChild("struct")->getChild("nested")->getChild("vec3f"));

        // Update in-between each step, to make sure no crashes
        EXPECT_TRUE(m_logicEngine.update());
        EXPECT_TRUE(m_logicEngine.link(sourceVec, targetVec));
        EXPECT_TRUE(m_logicEngine.update());
        EXPECT_TRUE(m_logicEngine.destroy(targetScript));
        EXPECT_TRUE(m_logicEngine.update());
        EXPECT_TRUE(m_logicEngine.destroy(sourceScript));
        EXPECT_TRUE(m_logicEngine.update());
    }

    TEST_F(ALogicEngine_Linking_Confidence, CanDestroyLinkedScriptsWithComplexTypes_WithLinkStillActive_ArrayOfStructs)
    {
        LuaScript& sourceScript(*m_logicEngine.createLuaScript(m_scriptArrayOfStructs));
        LuaScript& targetScript(*m_logicEngine.createLuaScript(m_scriptNestedStructs));

        const Property& sourceVec(*sourceScript.getOutputs()->getChild("array")->getChild(0)->getChild("vec3f"));
        Property& targetVec(*targetScript.getInputs()->getChild("struct")->getChild("nested")->getChild("vec3f"));

        // Update in-between each step, to make sure no crashes
        EXPECT_TRUE(m_logicEngine.update());
        EXPECT_TRUE(m_logicEngine.link(sourceVec, targetVec));
        EXPECT_TRUE(m_logicEngine.update());
        EXPECT_TRUE(m_logicEngine.destroy(sourceScript));
        EXPECT_TRUE(m_logicEngine.update());
        EXPECT_TRUE(m_logicEngine.destroy(targetScript));
        EXPECT_TRUE(m_logicEngine.update());
    }

    TEST_F(ALogicEngine_Linking_Confidence, CanDestroyLinkedBinding_WithNestedLinkStillActive)
    {
        LuaScript& sourceScript(*m_logicEngine.createLuaScript(m_scriptNestedStructs));

        auto targetBinding = m_logicEngine.createRamsesNodeBinding(*m_node, ERotationType::Euler_XYZ, "NodeBinding");
        auto translationProperty = targetBinding->getInputs()->getChild("translation");

        const Property& sourceVec(*sourceScript.getOutputs()->getChild("struct")->getChild(0)->getChild("vec3f"));

        // Update in-between each step, to make sure no crashes
        EXPECT_TRUE(m_logicEngine.update());
        EXPECT_TRUE(m_logicEngine.link(sourceVec, *translationProperty));
        EXPECT_TRUE(m_logicEngine.update());
        EXPECT_TRUE(m_logicEngine.destroy(sourceScript));
        EXPECT_TRUE(m_logicEngine.update());
        EXPECT_TRUE(m_logicEngine.destroy(*targetBinding));
        EXPECT_TRUE(m_logicEngine.update());
    }
}
