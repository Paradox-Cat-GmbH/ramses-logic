#  -------------------------------------------------------------------------
#  Copyright (C) 2020 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

add_subdirectory(testAssetProducer)

file(GLOB ramses-logic-test_SRC *.cpp)
file(GLOB ramses-logic-test_INC *.h)

add_executable(ramses-logic-test
    ${ramses-logic-test_INC}
    ${ramses-logic-test_SRC}
)

# TODO split tests into API tests (no sol/lua) and impl tests (with sol/lua)
target_link_libraries(ramses-logic-test
    PRIVATE
        rlogic::ramses-logic-static
        rlogic::gmock-main
        sol2::sol2
        lua::lua
        fmt::fmt
        rlogic::flatbuffers
        ${RAMSES_TARGET}

    )

target_include_directories(ramses-logic-test
    PRIVATE
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/lib/flatbuffers>
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/lib>
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/BuildConfig>
)

MakeTestFromTarget(ramses-logic-test)
folderize_target(ramses-logic-test "ramses-logic")

add_custom_command(TARGET ramses-logic-test PRE_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/res/ ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/res/unittests)
