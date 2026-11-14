### rlab 

Examples of using NGSPICE with rlab to solve electric circuits, and obtain their solutions in form
of a rlab variable.

Files are as follows:

1. eg_spice_1_external_src.r3
   Synchronous solver with V-source function provided by rlab

2. eg_spice_2_external_src.r3
   Synchronous solver with I-source function provided by rlab

3. eg_spice_2_stop_pid_control.r3
   Asyncronous solver using stop/alter/resume to change circuit parameters at discontinuities (here assuming RC thermal
   network), where these occur as the heat source is turned on, or off.
   If one tries to solve this with switches, they will suffer curse of stiff-ode-solver trying to integrate discontinuity by    decreasing the integration time step: It will fail, and they will die of old age waiting for it to complete.
