.. _lblTroubleshootingStartup:

Startup Issues
--------------

On Windows operating systems, you might receive the error shown in :numref:`figGenericCRT` when starting the application. This error is caused by a missing Visual C/C++ runtime library. You can fix this by running the installer for the Visual C/C++ redistributable package **vc_redist.x64.exe** which is included in the folder of every SimCenter desktop application.

   .. _figGenericCRT:

   .. figure:: figures/MissingCRT.png
      :align: center
      :figclass: align-center

      Error message for missing Visual C/C++ runtime library.


