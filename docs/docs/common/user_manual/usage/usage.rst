.. _lbl-usage:

**********
User Guide
**********


The |app| is a scientific workflow application that creates workflows and runs them in the background. These workflows can involve multiple different workflow applications (see more information the backend and workflows under :ref:`lblArchitecture`). Once the |app| is started, the user is presented with the user interface (UI) shown in |figGenericUI|. This interface allows the user to select the applications to run in a workflow, input the controlling parameters for each of these applications, start the workflow either locally or remotely, and finally view the results of the simulation.

	  
   .. _figGenericUI-R2D:

   .. figure:: figures/R2DPanel.png
	   :align: center
	   :figclass: align-center

	   The |app| user interface.

The main window of the UI is made up from the following areas:

#. **Input Panel Selection Ribbon**

   | The ribbon on the left side provides buttons that represent each step of the simulation workflow (e.g., **EVT: Event Description**, **SIM: Structural Model**, **RES: Results**). Clicking on one of these buttons shows the corresponding workflow applications in the central input panel.

#. **Input Panel**

   | The input panel is the large central area of the user-interface where the user provides input for the workflow applications. You can select a workflow application using the drop-down menu at the top of each input panel. Each panel collects input parameters and paths to files with input data for the selected workflow application. The RES (results) panel is different; it shows the results after a simulation is completed.


#. **Login Button**

   | The **Login** button is at the top right of the user interface. You can only launch jobs on DesignSafe after logging in to DesignSafe using your DesignSafe login and password. Pressing the login button will open up the login window to enter this information. You can register for an account on the `DesignSafe-CI <https://www.designsafe-ci.org/account/register>`_ website [#]_.


#. **Push Buttons**

   | This is the area near the bottom of the UI with the following four buttons:

   * **RUN**: Run the simulation locally on your computer.
   * **RUN at DesignSafe**: Process the input information and send the data to DesignSafe. The simulation will be run there on a supercomputer and the results will be stored in your DesignSafe jobs folder.
   * **GET from DesignSafe**: Obtain the list of jobs you ran on DesignSafe. You can select a job to download its results to your computer.
   * **Exit**: Close the application.


#. **Message Area**

   | The message area is located in the bottom of the UI and displays the status and error messages for a running background application. The message area is a dockable dialog which can be resized, moved, and closed as needed. The dialog visibility can be toggled by clicking on the menu item **View** -> **Program Output**. If moved from its default location, the status dialog can be restored to the bottom of the UI by dragging the dialog back to the bottom of the UI and hovering over the area.


The following sections of the user guide describe in detail each of the steps presented in the input panel selection ribbon and the corresponding workflow applications available in the tool:

   .. toctree-filt::
      :maxdepth: 1

      VIZ
      GI
      HAZ
      ASD
      HTA
      MOD
      ANA
      DL
      UQ
      RV
      RES
