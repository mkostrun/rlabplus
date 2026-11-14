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

Projects:
1. cap_measurement_using_low_pass_filter

   Project that analyzes how low pass filter can be used for capacity measurements using pulsed source for one arduino    project.

2. charging_capacitor

   Project that analyzes problem of charging a capacitor.

3. xspice-diode

   Project that develops X-spice code for diode, which introduces third electrode that sets the Delta ambient,junction
   temperature as voltage, to investigate light emitting diode loss of efficacy with heating.

   NGSPICE groupies did not want to include that in X-spice distribution with NGSPICE, so it is provided here as use-at-your-own-peril code model device.
