.. _troubleshootingRunning:

Issues While Running
--------------------

Common causes of failure include incorrect setup, non-functioning or poorly functioning websites, user error, and possible bugs in the software. To discover the errors, it is useful to understand how the user interface (UI) and the backend work when the user submits to run a job. The following is a list of issues we have observed when the user interface informs the user of a failure and steps you can take to fix them:

#.  **Could not create working dir**: The user does not have permission to create the ``tmp.SimCenter`` folder in the working directory. Change the **Local Jobs Directory** and the **Remote Jobs Directory** in the application's **Preferences** menu option.

#. **No Script File**: The application cannot find the main Python scripts (rWHALE.py, sWHALE.py, or qWHALE.py) that run the workflow using our backend engine. You might have changed the Local Applications directory location in Preferences, or modified the applications folder that accompanies the installation. Make sure the Local Applications points to the correct directory location. If that does not help, re-install the tool to fix a corrupted applications folder.



If the problems persist please contact |contact person|. Be as detailed as possible when reporting an error. 