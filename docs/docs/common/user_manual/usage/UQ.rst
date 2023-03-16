.. _lblUQ:

UQ: Uncertainty Quantification
==============================

The **UQ Engine** options currently available are Dakota and SimCenterUQ


Dakota UQ Engine
----------------

This UQ engine utilizes the `Dakota Software <https://dakota.sandia.gov/>`_, a state-of-the-art research application that is robust and provides many methods for optimization and UQ, a selection of which we utilize in this application. **Dakota** provides the user with a large number of methods for different kinds of analyses. For this reason we have divided the methods into categories though a pull-down menu, as shown below. Once the category has been selected, a number of different methods are made available to the user.

* By checking the ``Parallel Execution``, the UQ analysis will be performed in parallel. It will try to use all the processors available in the machine. 

* By checking the ``Save Woring dirs``, individual working directories will be saved at Local Jobs Directory. Local Jobs Directory is defined at ``File``-``Preference`` in the menubar. Otherwise, individual simulation files will be deleted after each simulation run. Users might uncheck this box when a large number of simulations is requested, to manage driver space.

.. _figDakota:

.. figure:: figures/dakotaUQ.png
   :align: center
   :figclass: align-center
   :width: 1200

   Dakota engine and category selection.

The following categories are available:

.. toctree-filt::
   :maxdepth: 1

   DakotaSampling
   DakotaSampling2

