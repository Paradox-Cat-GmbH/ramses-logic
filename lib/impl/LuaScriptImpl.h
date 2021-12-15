//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "impl/LogicNodeImpl.h"
#include "impl/LuaConfigImpl.h"

#include "internals/LuaCompilationUtils.h"
#include "internals/SerializationMap.h"
#include "internals/DeserializationMap.h"
#include "internals/WrappedLuaProperty.h"

#include "ramses-logic/Property.h"
#include "ramses-logic/LuaScript.h"

#include <memory>
#include <functional>
#include <string_view>

namespace flatbuffers
{
    class FlatBufferBuilder;

    class FlatBufferBuilder;
    template <typename T> struct Offset;
}

namespace rlogic_serialization
{
    struct LuaScript;
}

namespace rlogic
{
    class LuaModule;
}

namespace rlogic::internal
{
    class SolState;

    class LuaScriptImpl : public LogicNodeImpl
    {
    public:
        explicit LuaScriptImpl(LuaCompiledScript compiledScript, std::string_view name, uint64_t id);
        ~LuaScriptImpl() noexcept override = default;
        LuaScriptImpl(const LuaScriptImpl & other) = delete;
        LuaScriptImpl& operator=(const LuaScriptImpl & other) = delete;

        [[nodiscard]] static flatbuffers::Offset<rlogic_serialization::LuaScript> Serialize(
            const LuaScriptImpl& luaScript,
            flatbuffers::FlatBufferBuilder& builder,
            SerializationMap& serializationMap);

        [[nodiscard]] static std::unique_ptr<LuaScriptImpl> Deserialize(
            SolState& solState,
            const rlogic_serialization::LuaScript& luaScript,
            ErrorReporting& errorReporting,
            DeserializationMap& deserializationMap);

        std::optional<LogicNodeRuntimeError> update() override;

        [[nodiscard]] const ModuleMapping& getModules() const;

    private:
        std::string             m_source;
        WrappedLuaProperty      m_wrappedRootInput;
        WrappedLuaProperty      m_wrappedRootOutput;
        sol::protected_function m_solFunction;
        ModuleMapping           m_modules;
        StandardModules         m_stdModules;
    };
}
