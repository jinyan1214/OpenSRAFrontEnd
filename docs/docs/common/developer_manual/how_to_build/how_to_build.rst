.. _lblHowToBuild:

============
How to Build
============

OpenSRA is comprised of a frontend UI and some backend applications. They are kept in separate GitHub repositories and are also built separately. The following3 sections outline (1) the applications needed to build and run, (2) instructions for building the backend applications, (3) instructions for building the frontend UI, and (4) operations to perform in the running UI to link the UI and the backend.


********************
Install Dependencies
********************

First, ensure the following dependencies are installed:

* **C++17 compliant compiler**: many of the workflow applications included use C++17 features; consequently, they may need a newer C++17 compliant compiler. For Windows users, MSVC in `Visual Studio (Community Edition) <https://visualstudio.microsoft.com/vs/>`_ can be used. Some extensions of Visual Studio are also needed: Open Visual Studio Installer, go to Installed / More / Modify, under the Workloads tab, check Desktop development with C++ and Visual Studio extension development; under the Individual components tab, check C++ CMake tools for windows. Then click Modify.

* **Qt**: `Qt <https://www.qt.io/download>`_ is free for open source developers. Version 5.15 or later is required. Make sure to include Qt Creator in the installer.

.. warning::

  Qt Version 6.0 is currently available. It is so new we do not use it.

******************************
Build the Backend Applications
******************************

Instructions to build the backend workflow applications on your local desktop depend on your operating system, but is a 2 step process involving some initial setup and finally the build.

Setup the development
=====================

To be added. 


Build the User Interface
========================


To build the interface, you first need to download the repo and a companion repo(SimCenterCommon) from Github using our `github repos <https://github.com/NHERI-SimCenter>`_. In a folder in which you wish to build the application, issue the following two commands.

   .. code::
      
      git clone https://github.com/NHERI-SimCenter/SimCenterCommon.git
      git clone https://github.com/NHERI-SimCenter/R2DTool.git      


.. note::

   Use the above links if you just want to download and build the applications. If you intend to make changes to any of the code in the repo's, you should fork that repo and then clone your forked repo. Forking a repo at **github** is done through your browser as shown on the following `github guides page <https://guides.github.com/activities/forking/>`_

You now have two ways to build the application: (1) using the **Qt Creator** desktop application provided by **Qt** and (2) from terminal application.


Build using Qt Creator
----------------------

1. Start Qt Creator, then open the |short tool id|.pro file located in the |short tool id| directory that was downloaded in the previous step.
2. Setup the development kit in Qt Creator. This is usually done by selecting the Qt version, compiler, and configuration and following the onscreen dialog.
3. Build the application and run it in Qt Creator IDE using the **Run** button. This can be done using the keyboard shortcut ``F5`` to build and start the tool.


Build from the Terminal
-----------------------

The operations are similar to what was done when building the backend applications. In the terminal application, starting inside the directory of the cloned application again, you will create a build directory, cd into that build directory, run **qmake**, and finally make (or on Windows nmake) to create the application.

Windows developers will type the following in a terminal or a powershell window:

    .. parsed-literal::

      mkdir build
      cd build
      conan install .. --build missing
      qmake ../|short tool id|.pro
      nmake

Linux or Mac users will type the following in a terminal window from inside the |app| directory:

    .. parsed-literal::

      mkdir build
      cd build
      conan install .. --build missing
      qmake ../|short tool id|.pro
      make

.. note::

   #. qmake is an application installed with Qt. To be able to run the command as known, the path to the Qt bin folder needs to be added to your **PATH** environment variable. Alternatively, you need to specify the full path to qmake, i.e., on my desktop (if I had not set my PATH variable). I would replace line 3 with the following:

      .. parsed-literal::

        /Users/fmckenna/Qt/5.15.2/clang_64/bin/qmake ../|short tool id|.pro


   #. On a Windows 10 with Visual Studio, the above commands need to be performed using a `Visual Studio x64 command prompt <https://docs.microsoft.com/en-us/cpp/build/how-to-enable-a-64-bit-visual-cpp-toolset-on-the-command-line?view=msvc-160>`_ . 

   #. If installed, jom can also be used to build in parallel.


Once built, you can now run the tool executable.
