//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internals/SolWrapper.h"
#include "ramses-logic/EPropertyType.h"
#include "internals/TypeData.h"

namespace rlogic::internal
{
    class PropertyImpl;
    class SolState;

    class PropertyTypeExtractor
    {
    public:
        PropertyTypeExtractor(std::string rootName, EPropertyType rootType);

        // Obtain collected type info
        HierarchicalTypeData getExtractedTypeData() const;

        // Lua overloads
        std::reference_wrapper<PropertyTypeExtractor> index(const sol::object& propertyName);
        void newIndex(const sol::object& propertyName, const sol::object& propertyValue);
        static sol::object CreateArray(sol::this_state state, const sol::object& size, std::optional<sol::object> arrayType);

        // Register symbols for type extraction to sol environment
        static void RegisterTypesToEnvironment(sol::environment& environment);

    private:
        TypeData m_typeData;

        using ChildProperties = std::vector<PropertyTypeExtractor>;

        ChildProperties m_children;

        ChildProperties::iterator findChild(const std::string_view name);
        void extractPropertiesFromTable(const sol::table& table);
    };
}
