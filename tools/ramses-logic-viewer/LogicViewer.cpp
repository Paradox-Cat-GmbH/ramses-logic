//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "LogicViewer.h"
#include "LogicViewerLuaTypes.h"
#include "ramses-logic/LogicEngine.h"
#include "ramses-logic/AnimationNode.h"
#include "ramses-logic/RamsesNodeBinding.h"
#include "ramses-logic/RamsesAppearanceBinding.h"
#include "ramses-logic/RamsesCameraBinding.h"
#include "ramses-logic/LuaScript.h"
#include "ramses-logic/Property.h"
#include "fmt/format.h"
#include "internals/SolHelper.h"
#include <iostream>

namespace rlogic
{
    namespace
    {
        // NOLINTNEXTLINE(performance-unnecessary-value-param) The signature is forced by SOL. Therefore we have to disable this warning.
        int solExceptionHandler(lua_State* L, sol::optional<const std::exception&> maybe_exception, sol::string_view description)
        {
            if (maybe_exception)
            {
                return sol::stack::push(L, description);
            }
            return sol::stack::top(L);
        }

        template<class T>
        void registerNodeListType(sol::state& sol, const char* name) {
            sol.new_usertype<NodeListWrapper<T>>(name,
                                                 sol::no_constructor,
                                                 sol::meta_function::index,
                                                 &NodeListWrapper<T>::get,
                                                 sol::meta_function::call,
                                                 &NodeListWrapper<T>::iterator,
                                                 sol::meta_function::to_string,
                                                 [=]() { return name; });
            std::string iteratorName = name;
            iteratorName += "Iterator";
            sol.new_usertype<NodeListIterator<T>>(iteratorName, sol::no_constructor, sol::meta_function::call, &NodeListIterator<T>::call);
        }
    } // namespace

    struct LogicWrapper
    {
        explicit LogicWrapper(LogicEngine& logicEngine, sol::state& sol)
            : views(sol.create_table())
            , scripts(logicEngine)
            , animations(logicEngine)
            , nodeBindings(logicEngine)
            , appearanceBindings(logicEngine)
            , cameraBindings(logicEngine)
        {
        }

        sol::table views;

        NodeListWrapper<LuaScript> scripts;
        NodeListWrapper<AnimationNode> animations;
        NodeListWrapper<RamsesNodeBinding> nodeBindings;
        NodeListWrapper<RamsesAppearanceBinding> appearanceBindings;
        NodeListWrapper<RamsesCameraBinding> cameraBindings;
    };

    const char* const LogicViewer::ltnModule     = "rlogic";
    const char* const LogicViewer::ltnScript     = "scripts";
    const char* const LogicViewer::ltnAnimation  = "animationNodes";
    const char* const LogicViewer::ltnNode       = "nodeBindings";
    const char* const LogicViewer::ltnAppearance = "appearanceBindings";
    const char* const LogicViewer::ltnCamera     = "cameraBindings";
    const char* const LogicViewer::ltnScreenshot = "screenshot";
    const char* const LogicViewer::ltnViews      = "views";
    const char* const LogicViewer::ltnLink       = "link";
    const char* const LogicViewer::ltnUnlink     = "unlink";
    const char* const LogicViewer::ltnUpdate     = "update";

    const char* const LogicViewer::ltnPropertyValue   = "value";
    const char* const LogicViewer::ltnViewUpdate      = "update";
    const char* const LogicViewer::ltnViewInputs      = "inputs";
    const char* const LogicViewer::ltnViewName        = "name";
    const char* const LogicViewer::ltnViewDescription = "description";

    LogicViewer::LogicViewer(ScreenshotFunc screenshotFunc)
        : m_screenshotFunc(std::move(screenshotFunc))
    {
        m_startTime = std::chrono::steady_clock::now();
    }

    bool LogicViewer::loadRamsesLogic(const std::string& filename, ramses::Scene* scene)
    {
        m_logicFilename = filename;
        return m_logicEngine.loadFromFile(filename, scene);
    }

    LogicViewer::Result LogicViewer::loadLuaFile(const std::string& filename)
    {
        m_result = Result();
        m_sol = sol::state();
        m_sol.open_libraries(sol::lib::base, sol::lib::string, sol::lib::math, sol::lib::table, sol::lib::debug);
        m_sol.set_exception_handler(&solExceptionHandler);
        registerNodeListType<LuaScript>(m_sol, "LuaScripts");
        registerNodeListType<AnimationNode>(m_sol, "AnimationNodes");
        registerNodeListType<RamsesNodeBinding>(m_sol, "NodeBindings");
        registerNodeListType<RamsesAppearanceBinding>(m_sol, "AppearanceBindings");
        registerNodeListType<RamsesCameraBinding>(m_sol, "CameraBindings");
        m_sol.new_usertype<LogicNodeWrapper>("LogicNode", sol::meta_function::index, &LogicNodeWrapper::get, sol::meta_function::to_string, &LogicNodeWrapper::toString);
        m_sol.new_usertype<PropertyWrapper>("LogicProperty",
                                            ltnPropertyValue,
                                            sol::property(&PropertyWrapper::getValue, &PropertyWrapper::setValue),
                                            sol::meta_function::index,
                                            &PropertyWrapper::get,
                                            sol::meta_function::to_string,
                                            &PropertyWrapper::toString);
        m_sol.new_usertype<ConstPropertyWrapper>("ConstLogicProperty",
                                                 ltnPropertyValue,
                                                 sol::readonly_property(&ConstPropertyWrapper::getValue),
                                                 sol::meta_function::index,
                                                 &ConstPropertyWrapper::get,
                                                 sol::meta_function::to_string,
                                                 &ConstPropertyWrapper::toString);
        m_sol.new_usertype<LogicWrapper>(
            "RamsesLogic",
            sol::no_constructor,
            ltnScript,
            sol::readonly(&LogicWrapper::scripts),
            ltnAnimation,
            sol::readonly(&LogicWrapper::animations),
            ltnNode,
            sol::readonly(&LogicWrapper::nodeBindings),
            ltnAppearance,
            sol::readonly(&LogicWrapper::appearanceBindings),
            ltnCamera,
            sol::readonly(&LogicWrapper::cameraBindings),
            ltnViews,
            &LogicWrapper::views,
            ltnScreenshot,
            [&](const std::string& screenshotFile) {
                updateEngine();
                return m_screenshotFunc(screenshotFile);
            },
            ltnUpdate,
            [&]() { updateEngine(); },
            ltnLink,
            [&](const ConstPropertyWrapper& src, const PropertyWrapper& target) { return m_logicEngine.link(src.m_property, target.m_property); },
            ltnUnlink,
            [&](const ConstPropertyWrapper& src, const PropertyWrapper& target) { return m_logicEngine.unlink(src.m_property, target.m_property); });

        m_sol[ltnModule] = LogicWrapper(m_logicEngine, m_sol);

        m_luaFilename  = filename;
        auto result = m_sol.script_file(filename);
        if (!result.valid())
        {
            sol::error err = result;
            std::cerr << err.what() << std::endl;
            m_result = Result(err.what());
        }
        return m_result;
    }

    LogicViewer::Result LogicViewer::call(const std::string& functionName)
    {
        auto result = m_sol[functionName]();
        if (!result.valid())
        {
            sol::error err = result;
            m_result = Result(err.what());
        }
        return m_result;
    }

    LogicViewer::Result LogicViewer::update()
    {
        updateEngine();
        // don't update if there's already an error
        if (m_result.ok())
        {
            sol::optional<sol::table> view = m_sol[ltnModule][ltnViews][m_view];
            if (view)
            {
                const auto elapsed   = std::chrono::steady_clock::now() - m_startTime;
                const auto millisecs = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
                sol::optional<sol::function> func = (*view)[ltnViewUpdate];
                if (func)
                {
                    auto result = (*func)(millisecs);
                    if (!result.valid())
                    {
                        sol::error err = result;
                        std::cerr << err.what() << std::endl;
                        m_result = Result(err.what());
                        return m_result;
                    }
                }
                else
                {
                    m_result = Result("update() function is missing for current view");
                    return m_result;
                }
            }
        }
        return Result();
    }

    size_t LogicViewer::getViewCount() const
    {
        sol::optional<sol::table> tbl = m_sol[ltnModule][ltnViews];
        return tbl ? tbl->size() : 0U;
    }

    void LogicViewer::setCurrentView(size_t viewId)
    {
        if (viewId >= 1U && viewId <= getViewCount())
            m_view = viewId;
    }

    LogicViewer::View LogicViewer::getView(size_t viewId) const
    {
        sol::optional<sol::table> tbl = m_sol[ltnModule][ltnViews][viewId];
        return View(std::move(tbl));
    }

    void LogicViewer::updateEngine()
    {
        m_logicEngine.update();
        if (m_updateReportEnabled)
        {
            m_updateReportSummary.add(m_logicEngine.getLastUpdateReport());
        }
    }
}

