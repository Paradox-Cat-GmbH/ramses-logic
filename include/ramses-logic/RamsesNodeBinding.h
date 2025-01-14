//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses-logic/APIExport.h"
#include "ramses-logic/RamsesBinding.h"
#include "ramses-logic/ERotationType.h"

#include <memory>

namespace ramses
{
    class Node;
}

namespace rlogic::internal
{
    class RamsesNodeBindingImpl;
}

namespace rlogic
{
    /**
     * The RamsesNodeBinding is a type of #rlogic::RamsesBinding which allows manipulation of Ramses nodes.
     * RamsesNodeBinding's can be created with #rlogic::LogicEngine::createRamsesNodeBinding.
     *
     * The RamsesNodeBinding has a fixed set of inputs which correspond to properties of ramses::Node.
     * They have a fixed type and name:
     * 'visibility' (type bool)
     * 'rotation' (type vec3f, or vec4f in case of quaternion)
     * 'translation' (type vec3f)
     * 'scaling' (type vec3f)
     *
     * The default values of the input properties are taken from the bound ramses::Node provided during construction.
     * This also applies for rotations, if the rlogic::ERotationType and ramses::ERotationConvention values of the
     * ramses node match (both are Euler, and both correspond to the same axis ordering). Otherwise a warning is
     * issued and the rotation values are set to 0.
     *
     * The RamsesNodeBinding class has no output properties (thus getOutputs() will return nullptr) because
     * the outputs are implicitly the properties of the bound Ramses node.
     *
     * \rst
    .. note::

        In case no values were set (because the user neither set a value explicitly
        nor linked the input of rlogic::RamsesNodeBinding to another LogicNode output) the Ramses values
        are not touched. It is possible to set values directly to ramses::Node which will not be overwritten
        by rlogic::RamsesNodeBinding if you never explicitly assigned a value to the rlogic::RamsesNodeBinding inputs.
        You can also mix-and-match this behavior - assign some properties and others not.

     \endrst
     *
     *
     * The LogicEngine does not restrict which Scene the bound nodes belong to - it is possible to have
     * nodes from different scenes bound to the same LogicEngine, and vice-versa. The effects
     * on the ramses::Node property values which are bound to a rlogic::RamsesNodeBinding are immediately
     * visible after rlogic::LogicEngine::update() returns, however the user has to call ramses::Scene::flush()
     * explicitly based on their scene update logic and frame lifecycle.
     *
     */
    class RamsesNodeBinding : public RamsesBinding
    {
    public:
        /**
        * Returns the bound ramses node.
        *
        * @return the bound ramses node
        */
        [[nodiscard]] RLOGIC_API ramses::Node& getRamsesNode() const;

        /**
        * Returns the statically configured rotation type for the node rotation property.
        *
        * @return the currently used rotation type
        */
        [[nodiscard]] RLOGIC_API ERotationType getRotationType() const;

        /**
        * Constructor of RamsesNodeBinding. User is not supposed to call this - RamsesNodeBindings are created by other factory classes
        *
        * @param impl implementation details of the RamsesNodeBinding
        */
        explicit RamsesNodeBinding(std::unique_ptr<internal::RamsesNodeBindingImpl> impl) noexcept;

        /**
         * Destructor of RamsesNodeBinding.
         */
        ~RamsesNodeBinding() noexcept override;

        /**
         * Copy Constructor of RamsesNodeBinding is deleted because RamsesNodeBindings are not supposed to be copied
         *
         * @param other RamsesNodeBindings to copy from
         */
        RamsesNodeBinding(const RamsesNodeBinding& other) = delete;

        /**
         * Move Constructor of RamsesNodeBinding is deleted because RamsesNodeBindings are not supposed to be moved
         *
         * @param other RamsesNodeBindings to move from
         */
        RamsesNodeBinding(RamsesNodeBinding&& other) = delete;

        /**
         * Assignment operator of RamsesNodeBinding is deleted because RamsesNodeBindings are not supposed to be copied
         *
         * @param other RamsesNodeBindings to assign from
         */
        RamsesNodeBinding& operator=(const RamsesNodeBinding& other) = delete;

        /**
         * Move assignment operator of RamsesNodeBinding is deleted because RamsesNodeBindings are not supposed to be moved
         *
         * @param other RamsesNodeBindings to assign from
         */
        RamsesNodeBinding& operator=(RamsesNodeBinding&& other) = delete;

        /**
         * Implementation detail of RamsesNodeBinding
         */
        internal::RamsesNodeBindingImpl& m_nodeBinding;
    };
}
