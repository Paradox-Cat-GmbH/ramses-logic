..
    -------------------------------------------------------------------------
    Copyright (C) 2020 BMW AG
    -------------------------------------------------------------------------
    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at https://mozilla.org/MPL/2.0/.
    -------------------------------------------------------------------------

.. _build-instructions:

The following sections describe different build scenarios with detailed
options and explanations. If you are a versed C++ developer and are familiar
with CMake, consider using the :ref:`quickstart guide <build>`.

========================================
Cloning
========================================

The ``RAMSES logic`` project uses `Git submodules <https://git-scm.com/book/en/v2/Git-Tools-Submodules>`_
to bundle its dependencies, including ``RAMSES`` itself. Git doesn't automatically
clone these, the user has to clone them explicitly like this:

.. code-block:: bash

    git submodule update --init --recursive

The "recursive" option makes sure that sub-dependencies are cloned as well. Beware that using
the ``download as ZIP`` option in ``GitHub`` also doesn't package dependencies, so if you go for
this option, make sure that you download the submodules yourself, otherwise the build will fail.

Cloning all submodules recursively will also clone ``RAMSES`` and can take quite some time. If you want
to avoid this, and you already have a ``RAMSES`` build you want to re-use, consider cloning the
submodules one-by-one and not clone ``RAMSES`` in conjunktion with the :ref:`build options` to use an
existing ``RAMSES`` target.

========================================
Build requirements
========================================

``RAMSES Logic`` requires CMake 3.13 or newer and a C++17-compatible compiler:

* Clang6 or newer
* GCC7 or newer
* MSVC2017 (_MSC_VER >= 1914)

Since ``RAMSES`` is a dependency of ``RAMSES logic`` and is built alongside it, following
additional build dependencies must be present in order to run some of the visual demos/examples:

* Windows
    * A graphics card and drivers which supports OpenGL 4.2 or newer
* Linux
    * X11 or Wayland (or both) developer libraries
    * OpenGL + EGL developer libraries

For example, for ``Ubuntu 18.04 LTS`` these can be installed like this:

.. code-block:: bash

    sudo apt-get install \
            libx11-dev \
            libwayland-dev \
            libgles2-mesa-dev

Refer to the `RAMSES build instructions <https://github.com/COVESA/ramses#building-and-testing>`_ for more details.

========================================
Build options
========================================

A standard CMake build will create the shared library version of ``Ramses Logic`` by default. A static build alongside other
things can be enabled using following CMake options:

* -Dramses-logic_SHARED_LIB_TARGET_NAME
    * options: any custom target name
    * default: ramses-logic
    * enables setting a custom name for ramses-logic shared library target.

* -Dramses-logic_RAMSES_TARGET
    * options: ramses-shared-lib-client-only, ramses-shared-lib-<platform>, or a custom target name
    * default: empty
    * enables setting a custom Ramses target instead of building and linking Ramses from source. If set,
      and if the target exists, will use it, otherwise fallback to building from source.

* -Dramses-logic_BUILD_RAMSES_RENDERER
    * options: ON/OFF
    * default: ON
    * enables building the ramses renderer as part of the build. If this option is set to ON, will build
      a default Ramses renderer for your system. If you want to build a specific platform, use either
      -Dramses-logic_PLATFORM or a custom target (-Dramses-logic_RAMSES_TARGET).

* -Dramses-logic_ALLOW_RAMSES_BUILD
    * options: ON/OFF
    * default: ON
    * Set this to OFF to explicitly disable Ramses build, even if ramses-logic_RAMSES_TARGET (see option above) is
      not a valid ramses target. This will disable the automatic fallback to building Ramses from source if the specified
      custom target is not available at generation time for some reason.

* -Dramses-logic_FIND_RAMSES_FROM_SYSTEM
    * options: ON/OFF
    * default: OFF
    * Set this to ON to find Ramses from system (using find_package) instead of building Ramses. Other build-related
      CMake options must be disabled (ramses-logic_ALLOW_RAMSES_BUILD, ramses-logic_BUILD_RAMSES_RENDERER). The
      target platform (x11, wayland etc.) is determined from the value of ${ramses-logic_PLATFORM} - if not set
      will use the client-only version of Ramses.

* -Dramses-logic_PLATFORM
    * options: any of [LINUX-X11, LINUX-WAYLAND, WINDOWS, ANDROID, or empty/not set]
    * default: not set (defaults to using client-only version of ramses)
    * Use this in conjunction with 'ramses-logic_FIND_RAMSES_FROM_SYSTEM' to search for a specific Ramses platform on the system.
      Also influences which version of the renderer is built when using -Dramses-logic_BUILD_RAMSES_RENDERER.

* -Dramses-logic_WARNINGS_AS_ERRORS
    * options: ON/OFF
    * default: ON
    * treats compiler warnings as errors and aborts the build. Use this option if your compiler generates warnings which are not fixed yet.

* -Dramses-logic_BUILD_SHARED_LIB
    * options: ON/OFF
    * default: ON
    * Enables building of the shared (dynamic) library version of logic engine (we recommend this over the static build)

* -Dramses-logic_BUILD_STATIC_LIB
    * options: ON/OFF
    * default: OFF
    * Enables building of the static library version of logic engine

* -Dramses-logic_ENABLE_INSTALL
    * options: ON/OFF
    * default: ON
    * Set this to OFF to prevent Ramses Logic installing its targets, headers, and documentation. Use this if you link statically and don't
      have to expose any Ramses Logic headers or ship the shared library with your installation. It has no effect on the installation of Ramses though!

* -Dramses-logic_FORCE_BUILD_TESTS
    * options: ON/OFF
    * default: OFF
    * by default, tests are built only when RAMSES Logic is built as a standalone projects. Use this to force building them.

* -Dramses-logic_FORCE_OFF_TESTS
    * options: ON/OFF
    * default: OFF
    * Fully disable building tests

* -Dramses-logic_BUILD_EXAMPLES
    * options: ON/OFF
    * default: ON
    * set to OFF if you don't need the examples and want to reduce building time

* -Dramses-logic_BUILD_TOOLS
    * options: ON/OFF
    * default: ON
    * set to OFF if you don't need the tools (e.g. imgui-based viewer) and want to reduce building time

* -Dramses-logic_BUILD_DOCUMENTATION
    * options: ON/OFF
    * default: ON
    * enable building documentation when dependencies available

* -Dramses-logic_ENABLE_TEST_COVERAGE
    * options: ON/OFF
    * default: OFF
    * enables clang's options to generate code coverage from test executables

* -Dramses-logic_BUILD_WITH_LTO
    * options: ON/OFF
    * default: OFF
    * turns clang's link-time optimizations on (details `here <https://llvm.org/docs/LinkTimeOptimization.html>`_)

* -DCMAKE_TOOLCHAIN_FILE=<file>
    * options: any of the files in `cmake/toolchain <https://github.com/COVESA/ramses-logic/tree/master/cmake/toolchain>`_ or your custom cross-compilation toolchain file
    * default: not set
    * This is a standard CMake feature. We provide several toolchain files for popular compilers, use them or create your own


=======================================
Project version
=======================================

You can check the project version at build time and at runtime. At build time it is available
as a CMake cache variable named ``ramses-logic_VERSION``. At runtime, you can use the
:func:`rlogic::GetRamsesLogicVersion` function to get the version as integer or as string.

========================================
Building on Windows
========================================

- Start CMake GUI
- Select <ramses-logic> as source path, choose arbitrary <build> folder.
- Click 'Configure'
- Optionally, set some of the options above
- Click 'Generate'
- Open solution in Visual Studio


============================================================
Building in Docker (Windows or Linux)
============================================================

We prefer to build ``RAMSES logic`` in Docker because it abstracts the dependency installation
and the ``CMake`` invocations away from the user. Docker itself is installed slightly differently
on different distributions, check the docker manual for your distro. The instructions below
are for Ubuntu 18.04 LTS:

.. code-block:: bash

    apt install docker.io
    groupadd docker             #can fail if already exists
    usermod –aG docker $USER
    newgrp docker               #logs you into the new group in the current terminal session

    docker run hello-world      #check that docker works

    cd <ramses-sdk>/ci/docker
    ./build-image.sh            # builds the docker image locally (will take a while)

    ./start-container.sh        # Starts a docker container

    # We are inside the docker container now

    ./build.sh L64_LLVM Release # Builds with Clang/llvm x64 in Release mode

    ./build.sh L64_GCC Debug    # Builds with GCC x64 in Debug mode

    cd build/L64_GCC-Debug
    ctest -C Debug -V           # Runs unit tests with CTest

    # Build artifacts are stored in build/<compiler>-<buildtype>
    # This folder is persistent across docker executions! You can access it from your host after exiting the container

    ./build.sh DOC              # Builds the documentation and puts it into $PACKAGE_DIR as archive

========================================
Building on Linux natively
========================================

Assuming you have met the :ref:`build requirements`, you can build ``RAMSES logic`` like this:

.. code-block:: bash

    cd <ramses-logic-src>
    mkdir build && cd build
    export OPTIONS="-Dramses-logic_OPTION_1=ON ..." # see above for a list of options
    cmake $OPTIONS ../
    make

