//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "LogicEngineTest_Base.h"

namespace rlogic
{
    class ALogicEngine_Lookup : public ALogicEngine
    {
    };

    TEST_F(ALogicEngine_Lookup, FindsObjectsByTheirName)
    {
        LuaModule* luaModule = m_logicEngine.createLuaModule(m_moduleSourceCode, {}, "luaModule");
        LuaScript* script = m_logicEngine.createLuaScript(m_valid_empty_script, {}, "script");
        RamsesNodeBinding* nodeBinding = m_logicEngine.createRamsesNodeBinding(*m_node, ERotationType::Euler_XYZ, "nodebinding");
        RamsesAppearanceBinding* appearanceBinding = m_logicEngine.createRamsesAppearanceBinding(*m_appearance, "appbinding");
        RamsesCameraBinding* cameraBinding = m_logicEngine.createRamsesCameraBinding(*m_camera, "camerabinding");
        const auto dataArray = m_logicEngine.createDataArray(std::vector<float>{ 1.f, 2.f, 3.f }, "dataarray");
        const auto animNode = m_logicEngine.createAnimationNode({ { "channel", dataArray, dataArray } }, "animNode");
        const auto timerNode = m_logicEngine.createTimerNode("timerNode");

        EXPECT_EQ(luaModule, m_logicEngine.findLuaModule("luaModule"));
        EXPECT_EQ(script, m_logicEngine.findScript("script"));
        EXPECT_EQ(nodeBinding, m_logicEngine.findNodeBinding("nodebinding"));
        EXPECT_EQ(appearanceBinding, m_logicEngine.findAppearanceBinding("appbinding"));
        EXPECT_EQ(cameraBinding, m_logicEngine.findCameraBinding("camerabinding"));
        EXPECT_EQ(dataArray, m_logicEngine.findDataArray("dataarray"));
        EXPECT_EQ(animNode, m_logicEngine.findAnimationNode("animNode"));
        EXPECT_EQ(timerNode, m_logicEngine.findTimerNode("timerNode"));

        EXPECT_EQ(luaModule, m_logicEngine.findLogicObject("luaModule"));
        EXPECT_EQ(script, m_logicEngine.findLogicObject("script"));
        EXPECT_EQ(nodeBinding, m_logicEngine.findLogicObject("nodebinding"));
        EXPECT_EQ(appearanceBinding, m_logicEngine.findLogicObject("appbinding"));
        EXPECT_EQ(cameraBinding, m_logicEngine.findLogicObject("camerabinding"));
        EXPECT_EQ(dataArray, m_logicEngine.findLogicObject("dataarray"));
        EXPECT_EQ(animNode, m_logicEngine.findLogicObject("animNode"));
        EXPECT_EQ(timerNode, m_logicEngine.findLogicObject("timerNode"));

        auto it = m_logicEngine.logicObjects().cbegin();
        EXPECT_EQ(*it++, luaModule);
        EXPECT_EQ(*it++, script);
        EXPECT_EQ(*it++, nodeBinding);
        EXPECT_EQ(*it++, appearanceBinding);
        EXPECT_EQ(*it++, cameraBinding);
        EXPECT_EQ(*it++, dataArray);
        EXPECT_EQ(*it++, animNode);
        EXPECT_EQ(*it++, timerNode);
    }

    TEST_F(ALogicEngine_Lookup, FindsObjectsByTheirName_Const)
    {
        LuaModule* luaModule = m_logicEngine.createLuaModule(m_moduleSourceCode, {}, "luaModule");
        LuaScript* script = m_logicEngine.createLuaScript(m_valid_empty_script, {}, "script");
        RamsesNodeBinding* nodeBinding = m_logicEngine.createRamsesNodeBinding(*m_node, ERotationType::Euler_XYZ, "nodebinding");
        RamsesAppearanceBinding* appearanceBinding = m_logicEngine.createRamsesAppearanceBinding(*m_appearance, "appbinding");
        RamsesCameraBinding* cameraBinding = m_logicEngine.createRamsesCameraBinding(*m_camera, "camerabinding");
        const auto dataArray = m_logicEngine.createDataArray(std::vector<float>{ 1.f, 2.f, 3.f }, "dataarray");
        const auto animNode = m_logicEngine.createAnimationNode({ { "channel", dataArray, dataArray } }, "animNode");
        const auto timerNode = m_logicEngine.createTimerNode("timerNode");

        const LogicEngine& immutableLogicEngine = m_logicEngine;
        EXPECT_EQ(luaModule, immutableLogicEngine.findLuaModule("luaModule"));
        EXPECT_EQ(script, immutableLogicEngine.findScript("script"));
        EXPECT_EQ(nodeBinding, immutableLogicEngine.findNodeBinding("nodebinding"));
        EXPECT_EQ(appearanceBinding, immutableLogicEngine.findAppearanceBinding("appbinding"));
        EXPECT_EQ(cameraBinding, immutableLogicEngine.findCameraBinding("camerabinding"));
        EXPECT_EQ(dataArray, immutableLogicEngine.findDataArray("dataarray"));
        EXPECT_EQ(animNode, immutableLogicEngine.findAnimationNode("animNode"));
        EXPECT_EQ(timerNode, immutableLogicEngine.findTimerNode("timerNode"));

        EXPECT_EQ(luaModule, immutableLogicEngine.findLogicObject("luaModule"));
        EXPECT_EQ(script, immutableLogicEngine.findLogicObject("script"));
        EXPECT_EQ(nodeBinding, immutableLogicEngine.findLogicObject("nodebinding"));
        EXPECT_EQ(appearanceBinding, immutableLogicEngine.findLogicObject("appbinding"));
        EXPECT_EQ(cameraBinding, immutableLogicEngine.findLogicObject("camerabinding"));
        EXPECT_EQ(dataArray, immutableLogicEngine.findLogicObject("dataarray"));
        EXPECT_EQ(animNode, immutableLogicEngine.findLogicObject("animNode"));
        EXPECT_EQ(timerNode, immutableLogicEngine.findLogicObject("timerNode"));
    }

    TEST_F(ALogicEngine_Lookup, FindsObjectsByTheirName_CanBeUsedWithRealType)
    {
        m_logicEngine.createLuaModule(m_moduleSourceCode, {}, "luaModule");
        m_logicEngine.createLuaScript(m_valid_empty_script, {}, "script");
        m_logicEngine.createRamsesNodeBinding(*m_node, ERotationType::Euler_XYZ, "nodebinding");
        m_logicEngine.createRamsesAppearanceBinding(*m_appearance, "appbinding");
        m_logicEngine.createRamsesCameraBinding(*m_camera, "camerabinding");
        const auto dataArray = m_logicEngine.createDataArray(std::vector<float>{1.f, 2.f, 3.f}, "dataarray");
        m_logicEngine.createAnimationNode({{"channel", dataArray, dataArray}}, "animNode");
        m_logicEngine.createTimerNode("timerNode");

        const auto* luaModuleFound         = m_logicEngine.findLogicObject("luaModule")->as<LuaModule>();
        const auto* luaScriptFound         = m_logicEngine.findLogicObject("script")->as<LuaScript>();
        const auto* nodeBindingFound       = m_logicEngine.findLogicObject("nodebinding")->as<RamsesNodeBinding>();
        const auto* appearanceBindingFound = m_logicEngine.findLogicObject("appbinding")->as<RamsesAppearanceBinding>();
        const auto* cameraBindingFound     = m_logicEngine.findLogicObject("camerabinding")->as<RamsesCameraBinding>();
        const auto* dataArrayFound         = m_logicEngine.findLogicObject("dataarray")->as<DataArray>();
        const auto* animNodeFound          = m_logicEngine.findLogicObject("animNode")->as<AnimationNode>();
        const auto* timerNodeFound         = m_logicEngine.findLogicObject("timerNode")->as<TimerNode>();

        ASSERT_NE(nullptr, luaModuleFound);
        ASSERT_NE(nullptr, luaScriptFound);
        ASSERT_NE(nullptr, nodeBindingFound);
        ASSERT_NE(nullptr, appearanceBindingFound);
        ASSERT_NE(nullptr, cameraBindingFound);
        ASSERT_NE(nullptr, dataArrayFound);
        ASSERT_NE(nullptr, animNodeFound);
        ASSERT_NE(nullptr, timerNodeFound);

        EXPECT_EQ(luaModuleFound->getName(), "luaModule");
        EXPECT_EQ(luaScriptFound->getName(), "script");
        EXPECT_EQ(nodeBindingFound->getName(), "nodebinding");
        EXPECT_EQ(appearanceBindingFound->getName(), "appbinding");
        EXPECT_EQ(cameraBindingFound->getName(), "camerabinding");
        EXPECT_EQ(dataArrayFound->getName(), "dataarray");
        EXPECT_EQ(animNodeFound->getName(), "animNode");
        EXPECT_EQ(timerNodeFound->getName(), "timerNode");
    }

    TEST_F(ALogicEngine_Lookup, FindsObjectsByTheirName_CanBeUsedAsRealType_Const)
    {
        m_logicEngine.createLuaModule(m_moduleSourceCode, {}, "luaModule");
        m_logicEngine.createLuaScript(m_valid_empty_script, {}, "script");
        m_logicEngine.createRamsesNodeBinding(*m_node, ERotationType::Euler_XYZ, "nodebinding");
        m_logicEngine.createRamsesAppearanceBinding(*m_appearance, "appbinding");
        m_logicEngine.createRamsesCameraBinding(*m_camera, "camerabinding");
        const auto dataArray = m_logicEngine.createDataArray(std::vector<float>{1.f, 2.f, 3.f}, "dataarray");
        m_logicEngine.createAnimationNode({{"channel", dataArray, dataArray}}, "animNode");
        m_logicEngine.createTimerNode("timerNode");

        const LogicEngine& immutableLogicEngine   = m_logicEngine;
        const auto*        luaModuleFound         = immutableLogicEngine.findLogicObject("luaModule")->as<LuaModule>();
        const auto*        luaScriptFound         = immutableLogicEngine.findLogicObject("script")->as<LuaScript>();
        const auto*        nodeBindingFound       = immutableLogicEngine.findLogicObject("nodebinding")->as<RamsesNodeBinding>();
        const auto*        appearanceBindingFound = immutableLogicEngine.findLogicObject("appbinding")->as<RamsesAppearanceBinding>();
        const auto*        cameraBindingFound     = immutableLogicEngine.findLogicObject("camerabinding")->as<RamsesCameraBinding>();
        const auto*        dataArrayFound         = immutableLogicEngine.findLogicObject("dataarray")->as<DataArray>();
        const auto*        animNodeFound          = immutableLogicEngine.findLogicObject("animNode")->as<AnimationNode>();
        const auto*        timerNodeFound         = immutableLogicEngine.findLogicObject("timerNode")->as<TimerNode>();

        ASSERT_NE(nullptr, luaModuleFound);
        ASSERT_NE(nullptr, luaScriptFound);
        ASSERT_NE(nullptr, nodeBindingFound);
        ASSERT_NE(nullptr, appearanceBindingFound);
        ASSERT_NE(nullptr, cameraBindingFound);
        ASSERT_NE(nullptr, dataArrayFound);
        ASSERT_NE(nullptr, animNodeFound);
        ASSERT_NE(nullptr, timerNodeFound);

        EXPECT_EQ(luaModuleFound->getName(), "luaModule");
        EXPECT_EQ(luaScriptFound->getName(), "script");
        EXPECT_EQ(nodeBindingFound->getName(), "nodebinding");
        EXPECT_EQ(appearanceBindingFound->getName(), "appbinding");
        EXPECT_EQ(cameraBindingFound->getName(), "camerabinding");
        EXPECT_EQ(dataArrayFound->getName(), "dataarray");
        EXPECT_EQ(animNodeFound->getName(), "animNode");
        EXPECT_EQ(timerNodeFound->getName(), "timerNode");
    }

    TEST_F(ALogicEngine_Lookup, FindsObjectsByTheirId)
    {
        LuaModule*               luaModule         = m_logicEngine.createLuaModule(m_moduleSourceCode, {}, "luaModule");
        LuaScript*               script            = m_logicEngine.createLuaScript(m_valid_empty_script, {}, "script");
        RamsesNodeBinding*       nodeBinding       = m_logicEngine.createRamsesNodeBinding(*m_node, ERotationType::Euler_XYZ, "nodebinding");
        RamsesAppearanceBinding* appearanceBinding = m_logicEngine.createRamsesAppearanceBinding(*m_appearance, "appbinding");
        RamsesCameraBinding*     cameraBinding     = m_logicEngine.createRamsesCameraBinding(*m_camera, "camerabinding");
        const auto               dataArray         = m_logicEngine.createDataArray(std::vector<float>{1.f, 2.f, 3.f}, "dataarray");
        const auto               animNode          = m_logicEngine.createAnimationNode({{"channel", dataArray, dataArray}}, "animNode");
        const auto               timerNode         = m_logicEngine.createTimerNode("timerNode");

        EXPECT_EQ(luaModule, m_logicEngine.findLogicObjectById(1u));
        EXPECT_EQ(script, m_logicEngine.findLogicObjectById(2u));
        EXPECT_EQ(nodeBinding, m_logicEngine.findLogicObjectById(3u));
        EXPECT_EQ(appearanceBinding, m_logicEngine.findLogicObjectById(4u));
        EXPECT_EQ(cameraBinding, m_logicEngine.findLogicObjectById(5u));
        EXPECT_EQ(dataArray, m_logicEngine.findLogicObjectById(6u));
        EXPECT_EQ(animNode, m_logicEngine.findLogicObjectById(7u));
        EXPECT_EQ(timerNode, m_logicEngine.findLogicObjectById(8u));
    }

    TEST_F(ALogicEngine_Lookup, FindsObjectsByTheirId_Const)
    {
        LuaModule*               luaModule         = m_logicEngine.createLuaModule(m_moduleSourceCode, {}, "luaModule");
        LuaScript*               script            = m_logicEngine.createLuaScript(m_valid_empty_script, {}, "script");
        RamsesNodeBinding*       nodeBinding       = m_logicEngine.createRamsesNodeBinding(*m_node, ERotationType::Euler_XYZ, "nodebinding");
        RamsesAppearanceBinding* appearanceBinding = m_logicEngine.createRamsesAppearanceBinding(*m_appearance, "appbinding");
        RamsesCameraBinding*     cameraBinding     = m_logicEngine.createRamsesCameraBinding(*m_camera, "camerabinding");
        const auto               dataArray         = m_logicEngine.createDataArray(std::vector<float>{1.f, 2.f, 3.f}, "dataarray");
        const auto               animNode          = m_logicEngine.createAnimationNode({{"channel", dataArray, dataArray}}, "animNode");
        const auto               timerNode         = m_logicEngine.createTimerNode("timerNode");

        const LogicEngine& immutableLogicEngine = m_logicEngine;
        EXPECT_EQ(luaModule, immutableLogicEngine.findLogicObjectById(1u));
        EXPECT_EQ(script, immutableLogicEngine.findLogicObjectById(2u));
        EXPECT_EQ(nodeBinding, immutableLogicEngine.findLogicObjectById(3u));
        EXPECT_EQ(appearanceBinding, immutableLogicEngine.findLogicObjectById(4u));
        EXPECT_EQ(cameraBinding, immutableLogicEngine.findLogicObjectById(5u));
        EXPECT_EQ(dataArray, immutableLogicEngine.findLogicObjectById(6u));
        EXPECT_EQ(animNode, immutableLogicEngine.findLogicObjectById(7u));
        EXPECT_EQ(timerNode, immutableLogicEngine.findLogicObjectById(8u));
    }

    TEST_F(ALogicEngine_Lookup, FindsObjectsByTheirId_CanBeUsedWithRealType)
    {
        m_logicEngine.createLuaModule(m_moduleSourceCode, {}, "luaModule");
        m_logicEngine.createLuaScript(m_valid_empty_script, {}, "script");
        m_logicEngine.createRamsesNodeBinding(*m_node, ERotationType::Euler_XYZ, "nodebinding");
        m_logicEngine.createRamsesAppearanceBinding(*m_appearance, "appbinding");
        m_logicEngine.createRamsesCameraBinding(*m_camera, "camerabinding");
        const auto dataArray = m_logicEngine.createDataArray(std::vector<float>{1.f, 2.f, 3.f}, "dataarray");
        m_logicEngine.createAnimationNode({{"channel", dataArray, dataArray}}, "animNode");
        m_logicEngine.createTimerNode("timerNode");

        const auto* luaModuleFound         = m_logicEngine.findLogicObjectById(1u)->as<LuaModule>();
        const auto* luaScriptFound         = m_logicEngine.findLogicObjectById(2u)->as<LuaScript>();
        const auto* nodeBindingFound       = m_logicEngine.findLogicObjectById(3u)->as<RamsesNodeBinding>();
        const auto* appearanceBindingFound = m_logicEngine.findLogicObjectById(4u)->as<RamsesAppearanceBinding>();
        const auto* cameraBindingFound     = m_logicEngine.findLogicObjectById(5u)->as<RamsesCameraBinding>();
        const auto* dataArrayFound         = m_logicEngine.findLogicObjectById(6u)->as<DataArray>();
        const auto* animNodeFound          = m_logicEngine.findLogicObjectById(7u)->as<AnimationNode>();
        const auto* timerNodeFound         = m_logicEngine.findLogicObjectById(8u)->as<TimerNode>();

        ASSERT_NE(nullptr, luaModuleFound);
        ASSERT_NE(nullptr, luaScriptFound);
        ASSERT_NE(nullptr, nodeBindingFound);
        ASSERT_NE(nullptr, appearanceBindingFound);
        ASSERT_NE(nullptr, cameraBindingFound);
        ASSERT_NE(nullptr, dataArrayFound);
        ASSERT_NE(nullptr, animNodeFound);
        ASSERT_NE(nullptr, timerNodeFound);

        EXPECT_EQ(luaModuleFound->getName(), "luaModule");
        EXPECT_EQ(luaScriptFound->getName(), "script");
        EXPECT_EQ(nodeBindingFound->getName(), "nodebinding");
        EXPECT_EQ(appearanceBindingFound->getName(), "appbinding");
        EXPECT_EQ(cameraBindingFound->getName(), "camerabinding");
        EXPECT_EQ(dataArrayFound->getName(), "dataarray");
        EXPECT_EQ(animNodeFound->getName(), "animNode");
        EXPECT_EQ(timerNodeFound->getName(), "timerNode");
    }

    TEST_F(ALogicEngine_Lookup, FindsObjectsByTheirId_CanBeUsedAsRealType_Const)
    {
        m_logicEngine.createLuaModule(m_moduleSourceCode, {}, "luaModule");
        m_logicEngine.createLuaScript(m_valid_empty_script, {}, "script");
        m_logicEngine.createRamsesNodeBinding(*m_node, ERotationType::Euler_XYZ, "nodebinding");
        m_logicEngine.createRamsesAppearanceBinding(*m_appearance, "appbinding");
        m_logicEngine.createRamsesCameraBinding(*m_camera, "camerabinding");
        const auto dataArray = m_logicEngine.createDataArray(std::vector<float>{1.f, 2.f, 3.f}, "dataarray");
        m_logicEngine.createAnimationNode({{"channel", dataArray, dataArray}}, "animNode");
        m_logicEngine.createTimerNode("timerNode");

        const LogicEngine& immutableLogicEngine   = m_logicEngine;
        const auto*        luaModuleFound         = immutableLogicEngine.findLogicObjectById(1u)->as<LuaModule>();
        const auto*        luaScriptFound         = immutableLogicEngine.findLogicObjectById(2u)->as<LuaScript>();
        const auto*        nodeBindingFound       = immutableLogicEngine.findLogicObjectById(3u)->as<RamsesNodeBinding>();
        const auto*        appearanceBindingFound = immutableLogicEngine.findLogicObjectById(4u)->as<RamsesAppearanceBinding>();
        const auto*        cameraBindingFound     = immutableLogicEngine.findLogicObjectById(5u)->as<RamsesCameraBinding>();
        const auto*        dataArrayFound         = immutableLogicEngine.findLogicObjectById(6u)->as<DataArray>();
        const auto*        animNodeFound          = immutableLogicEngine.findLogicObjectById(7u)->as<AnimationNode>();
        const auto*        timerNodeFound         = immutableLogicEngine.findLogicObjectById(8u)->as<TimerNode>();

        ASSERT_NE(nullptr, luaModuleFound);
        ASSERT_NE(nullptr, luaScriptFound);
        ASSERT_NE(nullptr, nodeBindingFound);
        ASSERT_NE(nullptr, appearanceBindingFound);
        ASSERT_NE(nullptr, cameraBindingFound);
        ASSERT_NE(nullptr, dataArrayFound);
        ASSERT_NE(nullptr, animNodeFound);
        ASSERT_NE(nullptr, timerNodeFound);

        EXPECT_EQ(luaModuleFound->getName(), "luaModule");
        EXPECT_EQ(luaScriptFound->getName(), "script");
        EXPECT_EQ(nodeBindingFound->getName(), "nodebinding");
        EXPECT_EQ(appearanceBindingFound->getName(), "appbinding");
        EXPECT_EQ(cameraBindingFound->getName(), "camerabinding");
        EXPECT_EQ(dataArrayFound->getName(), "dataarray");
        EXPECT_EQ(animNodeFound->getName(), "animNode");
        EXPECT_EQ(timerNodeFound->getName(), "timerNode");
    }

    TEST_F(ALogicEngine_Lookup, FindsObjectsByTheirName_CutsNameAtNullTermination)
    {
        RamsesAppearanceBinding* appearanceBinding = m_logicEngine.createRamsesAppearanceBinding(*m_appearance, "appbinding");
        EXPECT_EQ(appearanceBinding, m_logicEngine.findAppearanceBinding("appbinding\0withsurprise"));
    }

    TEST_F(ALogicEngine_Lookup, FindsObjectsAfterRenaming_ByNewNameOnly)
    {
        LuaModule* luaModule = m_logicEngine.createLuaModule(m_moduleSourceCode, {}, "luaModule");
        LuaScript* script = m_logicEngine.createLuaScript(m_valid_empty_script, {}, "script");
        RamsesNodeBinding* nodeBinding = m_logicEngine.createRamsesNodeBinding(*m_node, ERotationType::Euler_XYZ, "nodebinding");
        RamsesAppearanceBinding* appearanceBinding = m_logicEngine.createRamsesAppearanceBinding(*m_appearance, "appbinding");
        RamsesCameraBinding* cameraBinding = m_logicEngine.createRamsesCameraBinding(*m_camera, "camerabinding");
        auto dataArray = m_logicEngine.createDataArray(std::vector<float>{ 1.f, 2.f, 3.f }, "dataarray");
        auto animNode = m_logicEngine.createAnimationNode({ { "channel", dataArray, dataArray } }, "animNode");
        auto timerNode = m_logicEngine.createTimerNode("timerNode");

        // Rename
        luaModule->setName("L");
        script->setName("S");
        nodeBinding->setName("NB");
        appearanceBinding->setName("AB");
        cameraBinding->setName("CB");
        dataArray->setName("DA");
        animNode->setName("AN");
        timerNode->setName("TN");

        // Can't find by old name
        EXPECT_EQ(nullptr, m_logicEngine.findLuaModule("luaModule"));
        EXPECT_EQ(nullptr, m_logicEngine.findScript("script"));
        EXPECT_EQ(nullptr, m_logicEngine.findNodeBinding("nodebinding"));
        EXPECT_EQ(nullptr, m_logicEngine.findAppearanceBinding("appbinding"));
        EXPECT_EQ(nullptr, m_logicEngine.findCameraBinding("camerabinding"));
        EXPECT_EQ(nullptr, m_logicEngine.findDataArray("dataarray"));
        EXPECT_EQ(nullptr, m_logicEngine.findAnimationNode("animNode"));
        EXPECT_EQ(nullptr, m_logicEngine.findTimerNode("timerNode"));

        // Found by new name
        EXPECT_EQ(luaModule, m_logicEngine.findLuaModule("L"));
        EXPECT_EQ(script, m_logicEngine.findScript("S"));
        EXPECT_EQ(nodeBinding, m_logicEngine.findNodeBinding("NB"));
        EXPECT_EQ(appearanceBinding, m_logicEngine.findAppearanceBinding("AB"));
        EXPECT_EQ(cameraBinding, m_logicEngine.findCameraBinding("CB"));
        EXPECT_EQ(dataArray, m_logicEngine.findDataArray("DA"));
        EXPECT_EQ(animNode, m_logicEngine.findAnimationNode("AN"));
        EXPECT_EQ(timerNode, m_logicEngine.findTimerNode("TN"));
    }

    TEST_F(ALogicEngine_Lookup, FindsObjectByNameOnlyIfTypeMatches)
    {
        m_logicEngine.createLuaModule(m_moduleSourceCode, {}, "luaModule");
        m_logicEngine.createLuaScript(m_valid_empty_script, {}, "script");
        m_logicEngine.createRamsesNodeBinding(*m_node, ERotationType::Euler_XYZ, "nodebinding");
        m_logicEngine.createRamsesAppearanceBinding(*m_appearance, "appbinding");
        m_logicEngine.createRamsesCameraBinding(*m_camera, "camerabinding");
        const auto dataArray = m_logicEngine.createDataArray(std::vector<float>{ 1.f, 2.f, 3.f }, "dataarray");
        m_logicEngine.createAnimationNode({ { "channel", dataArray, dataArray } }, "animNode");
        m_logicEngine.createTimerNode("timerNode");

        EXPECT_EQ(nullptr, m_logicEngine.findLuaModule("dataarray"));
        EXPECT_EQ(nullptr, m_logicEngine.findScript("nodebinding"));
        EXPECT_EQ(nullptr, m_logicEngine.findNodeBinding("appbinding"));
        EXPECT_EQ(nullptr, m_logicEngine.findScript("camerabinding"));
        EXPECT_EQ(nullptr, m_logicEngine.findAppearanceBinding("animNode"));
        EXPECT_EQ(nullptr, m_logicEngine.findCameraBinding("script"));
        EXPECT_EQ(nullptr, m_logicEngine.findDataArray("appbinding"));
        EXPECT_EQ(nullptr, m_logicEngine.findAnimationNode("dataarray"));
        EXPECT_EQ(nullptr, m_logicEngine.findLuaModule("timerNode"));
    }

    TEST_F(ALogicEngine_Lookup, FindsObjectByNameOnlyStringMatchesExactly)
    {
        m_logicEngine.createRamsesNodeBinding(*m_node, ERotationType::Euler_XYZ, "nodebinding");

        EXPECT_EQ(nullptr, m_logicEngine.findNodeBinding("Nodebinding"));
        EXPECT_EQ(nullptr, m_logicEngine.findNodeBinding("node"));
        EXPECT_EQ(nullptr, m_logicEngine.findNodeBinding("binding"));
        EXPECT_EQ(nullptr, m_logicEngine.findNodeBinding("Xnodebinding"));
        EXPECT_EQ(nullptr, m_logicEngine.findNodeBinding("nodebindinY"));
    }
}

