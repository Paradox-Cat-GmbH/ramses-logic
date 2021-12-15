//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "WithTempDirectory.h"

#include "ramses-logic/LogicEngine.h"
#include "ramses-logic/LuaScript.h"
#include "ramses-logic/Property.h"
#include "ramses-logic/LuaModule.h"
#include "impl/LuaModuleImpl.h"
#include "internals/ErrorReporting.h"
#include "internals/SolState.h"
#include "internals/DeserializationMap.h"

#include "generated/LuaModuleGen.h"

#include <fstream>

namespace rlogic::internal
{
    class ALuaModule : public ::testing::Test
    {
    protected:
        const std::string_view m_moduleSourceCode = R"(
            local mymath = {}
            function mymath.add(a,b)
                print(a+b)
            end
            return mymath
        )";

        LuaConfig createDeps(const std::vector<std::pair<std::string_view, std::string_view>>& dependencies)
        {
            LuaConfig config;
            for (const auto& [alias, moduleSrc] : dependencies)
            {
                LuaModule* mod = m_logicEngine.createLuaModule(moduleSrc);
                config.addDependency(alias, *mod);
            }

            return config;
        }

        LogicEngine m_logicEngine;
    };

    TEST_F(ALuaModule, IsCreated)
    {
        const auto module = m_logicEngine.createLuaModule(m_moduleSourceCode, {}, "mymodule");
        ASSERT_NE(nullptr, module);
        EXPECT_EQ("mymodule", module->getName());
        EXPECT_EQ(module->getId(), 1u);
    }

    TEST_F(ALuaModule, ChangesName)
    {
        const auto module = m_logicEngine.createLuaModule(m_moduleSourceCode);
        ASSERT_NE(nullptr, module);

        module->setName("mm");
        EXPECT_EQ("mm", module->getName());
        EXPECT_EQ(module, this->m_logicEngine.findLuaModule("mm"));
        EXPECT_TRUE(this->m_logicEngine.getErrors().empty());
    }

    TEST_F(ALuaModule, FailsCreationIfSourceInvalid)
    {
        EXPECT_EQ(nullptr, m_logicEngine.createLuaModule("!", {}));
        ASSERT_EQ(1u, m_logicEngine.getErrors().size());
        EXPECT_THAT(m_logicEngine.getErrors().front().message, ::testing::HasSubstr("Error while loading module"));
        EXPECT_THAT(m_logicEngine.getErrors().front().message, ::testing::HasSubstr("unexpected symbol near '!'"));

        EXPECT_EQ(nullptr, m_logicEngine.createLuaModule(R"(
            local mymath = {}
            function mymath.add(a,b)
                print(a+b)
            return mymath
        )"));
        ASSERT_EQ(1u, m_logicEngine.getErrors().size());
        EXPECT_THAT(m_logicEngine.getErrors().front().message, ::testing::HasSubstr("Error while loading module"));
        EXPECT_THAT(m_logicEngine.getErrors().front().message, ::testing::HasSubstr("expected (to close 'function'"));

        EXPECT_EQ(nullptr, m_logicEngine.createLuaModule(R"(
            local mymath = {}
            function mymath.add(a,b)
                print(a+b
            end
            return mymath
        )"));
        ASSERT_EQ(1u, m_logicEngine.getErrors().size());
        EXPECT_THAT(m_logicEngine.getErrors().front().message, ::testing::HasSubstr("Error while loading module"));
        EXPECT_THAT(m_logicEngine.getErrors().front().message, ::testing::HasSubstr("expected (to close '('"));
    }

    TEST_F(ALuaModule, CanBeSerialized)
    {
        WithTempDirectory tempDir;

        {
            LogicEngine logic;
            logic.createLuaModule(m_moduleSourceCode, {}, "mymodule");
            EXPECT_TRUE(logic.saveToFile("module.tmp"));
        }

        EXPECT_TRUE(m_logicEngine.loadFromFile("module.tmp"));
        const auto module = m_logicEngine.findLuaModule("mymodule");
        ASSERT_NE(nullptr, module);
        EXPECT_EQ("mymodule", module->getName());
        EXPECT_EQ(module->getId(), 1u);
        EXPECT_EQ(m_moduleSourceCode, module->m_impl.getSourceCode());
    }

    class ALuaModule_SerializationLifecycle : public ALuaModule
    {
    protected:
        SolState                                        m_solState;
        ErrorReporting                                  m_errorReporting;
        flatbuffers::FlatBufferBuilder                  m_flatBufferBuilder;
        DeserializationMap                              m_deserializationMap;
    };

    TEST_F(ALuaModule_SerializationLifecycle, ProducesErrorWhenNameMissing)
    {
        {
            auto module = rlogic_serialization::CreateLuaModule(
                m_flatBufferBuilder,
                0 // no name
            );
            m_flatBufferBuilder.Finish(module);
        }

        const auto&                    serialized   = *flatbuffers::GetRoot<rlogic_serialization::LuaModule>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<LuaModuleImpl> deserialized = LuaModuleImpl::Deserialize(m_solState, serialized, m_errorReporting, m_deserializationMap);

        EXPECT_FALSE(deserialized);
        ASSERT_EQ(m_errorReporting.getErrors().size(), 1u);
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Fatal error during loading of LuaModule from serialized data: missing name!");
    }

    TEST_F(ALuaModule_SerializationLifecycle, ProducesErrorWhenIdMissing)
    {
        {
            auto module = rlogic_serialization::CreateLuaModule(
                m_flatBufferBuilder,
                m_flatBufferBuilder.CreateString("name"),
                0 // no id
            );
            m_flatBufferBuilder.Finish(module);
        }

        const auto&                    serialized   = *flatbuffers::GetRoot<rlogic_serialization::LuaModule>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<LuaModuleImpl> deserialized = LuaModuleImpl::Deserialize(m_solState, serialized, m_errorReporting, m_deserializationMap);

        EXPECT_FALSE(deserialized);
        ASSERT_EQ(m_errorReporting.getErrors().size(), 1u);
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Fatal error during loading of LuaModule from serialized data: missing id!");
    }

    TEST_F(ALuaModule_SerializationLifecycle, ProducesErrorWhenLuaSourceCodeMissing)
    {
        {
            auto module = rlogic_serialization::CreateLuaModule(
                m_flatBufferBuilder,
                m_flatBufferBuilder.CreateString("name"),
                1u,
                0 // no source code
            );
            m_flatBufferBuilder.Finish(module);
        }

        const auto&                    serialized   = *flatbuffers::GetRoot<rlogic_serialization::LuaModule>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<LuaModuleImpl> deserialized = LuaModuleImpl::Deserialize(m_solState, serialized, m_errorReporting, m_deserializationMap);

        EXPECT_FALSE(deserialized);
        ASSERT_EQ(m_errorReporting.getErrors().size(), 1u);
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Fatal error during loading of LuaModule from serialized data: missing source code!");
    }

    TEST_F(ALuaModule_SerializationLifecycle, ProducesErrorWhenDependenciesMissing)
    {
        {
            auto module = rlogic_serialization::CreateLuaModule(
                m_flatBufferBuilder,
                m_flatBufferBuilder.CreateString("name"),
                1u,
                m_flatBufferBuilder.CreateString(m_moduleSourceCode),
                0 // no source code
            );
            m_flatBufferBuilder.Finish(module);
        }

        const auto&                    serialized   = *flatbuffers::GetRoot<rlogic_serialization::LuaModule>(m_flatBufferBuilder.GetBufferPointer());
        std::unique_ptr<LuaModuleImpl> deserialized = LuaModuleImpl::Deserialize(m_solState, serialized, m_errorReporting, m_deserializationMap);

        EXPECT_FALSE(deserialized);
        ASSERT_EQ(m_errorReporting.getErrors().size(), 1u);
        EXPECT_EQ(m_errorReporting.getErrors()[0].message, "Fatal error during loading of LuaModule from serialized data: missing dependencies!");
    }

    class ALuaModuleWithDependency : public ALuaModule
    {
    protected:
        std::string_view m_mathSrc = R"(
            local mymath = {}
            function mymath.add(a,b)
                return a+b
            end
            return mymath
        )";

        std::string_view m_quadsSrc = R"(
            modules("mymath")
            local quads = {}
            function quads.create(a,b)
                return {math.sin(a), math.cos(b), mymath.add(a, b)}
            end
            return quads
        )";
    };

    TEST_F(ALuaModuleWithDependency, IsCreated)
    {
        const auto quadsMod = m_logicEngine.createLuaModule(m_quadsSrc, createDeps({{"mymath", m_mathSrc}}), "quadsMod");
        ASSERT_NE(nullptr, quadsMod);
        EXPECT_EQ("quadsMod", quadsMod->getName());
        EXPECT_EQ(quadsMod->getId(), 2u); // module dependency has id 1u
    }

    TEST_F(ALuaModuleWithDependency, HasTwoDependencies)
    {
        std::string_view mathSubSrc = R"(
            local mymath = {}
            function mymath.sub(a,b)
                return a-b
            end
            return mymath
        )";

        std::string_view mathCombinedSrc = R"(
            modules("_mathAdd", "_mathSub")

            local mymath = {}
            mymath.add=_mathAdd.add
            mymath.sub=_mathSub.sub
            return mymath
        )";
        const auto mathCombined = m_logicEngine.createLuaModule(mathCombinedSrc, createDeps({ {"_mathAdd", m_mathSrc}, {"_mathSub", mathSubSrc} }));

        LuaConfig config;
        config.addDependency("_math", *mathCombined);
        const auto script = m_logicEngine.createLuaScript(R"(
            modules("_math")
            function interface()
                OUT.added = INT
                OUT.subbed = INT
            end

            function run()
                OUT.added = _math.add(1,2)
                OUT.subbed = _math.sub(15,5)
            end
        )", config);
        ASSERT_NE(nullptr, script);

        m_logicEngine.update();
        EXPECT_EQ(3, *script->getOutputs()->getChild("added")->get<int32_t>());
        EXPECT_EQ(10, *script->getOutputs()->getChild("subbed")->get<int32_t>());
    }

    TEST_F(ALuaModuleWithDependency, UsesSameModuleUnderMultipleNames)
    {
        std::string_view mathCombinedSrc = R"(
            modules("math1", "math2")
            local math = {}
            math.add1=math1.add
            math.add2=math2.add
            return math
        )";
        const auto mathCombined = m_logicEngine.createLuaModule(mathCombinedSrc, createDeps({ {"math1", m_mathSrc}, {"math2", m_mathSrc} }));

        LuaConfig config;
        config.addDependency("mathAll", *mathCombined);
        const auto script = m_logicEngine.createLuaScript(R"(
            modules("mathAll")

            function interface()
                OUT.result = INT
            end

            function run()
                OUT.result = mathAll.add1(1,2) + mathAll.add2(100,10)
            end
        )", config);
        ASSERT_NE(nullptr, script);

        m_logicEngine.update();
        EXPECT_EQ(113, *script->getOutputs()->getChild("result")->get<int32_t>());
    }

    TEST_F(ALuaModuleWithDependency, TwoModulesDependOnTheSameModule)
    {
        const auto config = createDeps({ {"mymath", m_mathSrc} });

        const auto mathUser1 = m_logicEngine.createLuaModule(R"(
            modules("mymath")
            local mathUser1 = {}
            function mathUser1.add(a, b)
                return mymath.add(a + 1, b + 1)
            end
            return mathUser1
        )", config);

        const auto mathUser2 = m_logicEngine.createLuaModule(R"(
            modules("mymath")
            local mathUser2 = {}
            function mathUser2.add(a, b)
                return mymath.add(a + 10, b + 10)
            end
            return mathUser2
        )", config);

        LuaConfig scriptConfig;
        scriptConfig.addDependency("math1", *mathUser1);
        scriptConfig.addDependency("math2", *mathUser2);
        const auto script = m_logicEngine.createLuaScript(R"(
            modules("math1", "math2")
            function interface()
                OUT.result1 = INT
                OUT.result2 = INT
            end

            function run()
                OUT.result1 = math1.add(1,2)
                OUT.result2 = math2.add(1,2)
            end
        )", scriptConfig);
        ASSERT_NE(nullptr, script);

        m_logicEngine.update();
        EXPECT_EQ(5, *script->getOutputs()->getChild("result1")->get<int32_t>());
        EXPECT_EQ(23, *script->getOutputs()->getChild("result2")->get<int32_t>());
    }

    TEST_F(ALuaModuleWithDependency, CanBeSerialized)
    {
        WithTempDirectory tmpDir;
        {
            LogicEngine logic;
            LuaConfig config;
            config.addDependency("mymath", *logic.createLuaModule(m_mathSrc, {}, "mathMod"));
            logic.createLuaModule(m_quadsSrc, config, "quadsMod");
            EXPECT_TRUE(logic.saveToFile("dep_modules.tmp"));
        }

        EXPECT_TRUE(m_logicEngine.loadFromFile("dep_modules.tmp"));

        const LuaModule* mathMod = m_logicEngine.findLuaModule("mathMod");
        LuaModule* quadsMod = m_logicEngine.findLuaModule("quadsMod");
        ASSERT_NE(mathMod, nullptr);
        ASSERT_NE(quadsMod, nullptr);
        EXPECT_EQ(mathMod->getId(), 1u);
        EXPECT_EQ(quadsMod->getId(), 2u);

        EXPECT_THAT(quadsMod->m_impl.getDependencies(), ::testing::ElementsAre(std::pair<std::string, const LuaModule*>({"mymath", mathMod})));
    }

    TEST_F(ALuaModuleWithDependency, UpdatesWithoutIssues_WhenModulesWithRuntimeErrors_AreNeverCalled)
    {
        std::string_view errors = R"(
            local mymath = {}
            function mymath.add(a,b)
                error("this fails always")
                return a+b
            end
            return mymath
        )";
        const auto quadsMod = m_logicEngine.createLuaModule(m_quadsSrc, createDeps({ {"mymath", errors} }));
        ASSERT_NE(nullptr, quadsMod);

        // This works fine, because neither the the quads module nor the math modules are ever called
        EXPECT_TRUE(m_logicEngine.update());
    }

    class ALuaModuleDependencyMatch : public ALuaModuleWithDependency
    {
    };

    TEST_F(ALuaModuleDependencyMatch, FailsToBeCreatedIfDeclaredDependencyDoesNotMatchProvidedDependency_NotProvidedButDeclared)
    {
        constexpr std::string_view mathExt = R"(
            modules("dep1", "dep2")
            local mymathExt = {}
            mymathExt.pi = 3.14
            return mymathExt
        )";
        EXPECT_EQ(nullptr, m_logicEngine.createLuaModule(mathExt, createDeps({ {"dep2", m_moduleSourceCode} })));
        ASSERT_EQ(1u, m_logicEngine.getErrors().size());
        EXPECT_THAT(m_logicEngine.getErrors().front().message, ::testing::HasSubstr("Module dependencies declared in source code: dep1, dep2\n  Module dependencies provided on create API: dep2"));
    }

    TEST_F(ALuaModuleDependencyMatch, FailsToBeCreatedIfDeclaredDependencyDoesNotMatchProvidedDependency_ProvidedButNotDeclared)
    {
        constexpr std::string_view mathExt = R"(
            modules("dep1", "dep2")
            local mymathExt = {}
            mymathExt.pi = 3.14
            return mymathExt
        )";
        EXPECT_EQ(nullptr, m_logicEngine.createLuaModule(mathExt, createDeps({ {"dep1", m_moduleSourceCode}, {"dep2", m_moduleSourceCode}, {"dep3", m_moduleSourceCode} })));
        ASSERT_EQ(1u, m_logicEngine.getErrors().size());
        EXPECT_THAT(m_logicEngine.getErrors().front().message, ::testing::HasSubstr("Module dependencies declared in source code: dep1, dep2\n  Module dependencies provided on create API: dep1, dep2, dep3"));
    }

    TEST_F(ALuaModuleDependencyMatch, FailsToBeCreatedIfDeclaredDependencyDoesNotMatchProvidedDependency_ExractionError)
    {
        constexpr std::string_view mathExt = R"(
            modules("dep1", "dep1") -- duplicate dependency
            local mymathExt = {}
            mymathExt.pi = 3.14
            return mymathExt
        )";
        EXPECT_EQ(nullptr, m_logicEngine.createLuaModule(mathExt, createDeps({ {"dep1", m_moduleSourceCode} })));
        ASSERT_EQ(1u, m_logicEngine.getErrors().size());
        EXPECT_THAT(m_logicEngine.getErrors().front().message, ::testing::HasSubstr("Error while extracting module dependencies: 'dep1' appears more than once in dependency list"));
    }
}
