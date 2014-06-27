###Reciprocally Coupled Neurons

**Requirements:** None
**Limitations:** None

![EDIT THIS LINK](http://www.rtxi.org/wp-content/uploads/2012/02/coupling.png)

This module reciprocally couples two neurons with alpha synapses. It requires the membrane potential of both cells and the inputs from two spike detector modules (eg. Spike Detect). It outputs the two synaptic currents which must then be appropriately connected. This module can be used to couple two biological neurons or one biological neuron with a model neuron. To couple the neurons, the module should be unpaused, then the “Couple Neurons” toggle button can be activated/deactivated. This module also computes the difference in time between the spiking of both cells so that coupling can be activated programmatically based on the delay. To use this feature, the “Couple Neurons” toggle button should be deactivated. The checkbox to “Automate coupling delay” should be activated before unpausing the module. The difference in spike times between the two cells is continuously computed as Cell 2 – Cell 1. This can be plotted in the Oscilloscope to see how how this relationship changes over time.

####Input Channels
1. input(0) – “Cell 1 Vm” : Membrane potential (V)
2. input(1) – “Cell 2 Vm” : Membrane potential (V)
3. input(2) – “Cell 1 Spike State” : Spike State (=1 when spike occurs)
4. input(3) – “Cell 2 Spike State” : Spike State (=1 when spike occurs)

####Output Channels
1. output(0) – “Isyn 1-2″ : Output current (A) from cell 1 to 2
2. output(1) – “Isyn 2-1″ : Output current (A) from cell 2 to 1

####Parameters
1. Coupling delay (ms) : Fixed delay at which to turn on coupling automatically
2. Tolerance (ms) : +/- tolerance for automatically turning on coupling
3. Gmax 1-2 (nS) : maximum synaptic conductance for synapse from cell 1 to 2
4. Tau 1-2 (ms) : Time constant for alpha-type synapse from cell 1 to 2
5. Esyn 1-2 (mV) : Reversal potential for synapse from cell 1 to 2
6. Gmax 2-1 (nS) : maximum synaptic conductance for synapse from cell 2 to 1
7. Tau 2-1 (ms) : Time constant for alpha-type synapse from cell 2 to 1
8. Esyn 2-1 (mV) : Reversal potential for synapse from cell 2 to 1

####States
1. Phase Diff (s) : Time difference between spikes (Cell 2 – Cell 1)
