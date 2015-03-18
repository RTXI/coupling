/*
Copyright (C) 2011 Georgia Institute of Technology
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

/*
* Thur 28 Apr 2011 7:53:27 PM EDT
* Reciprocally couples two neurons with alpha synapses. Requires the Vm
* input of two cells, and the input from two spike detector modules (eg.
* SpikeDetect). It outputs the two synaptic currents which must be
* appropriately connected.
*/

#include <coupling.h>
#include <math.h>
#include <default_gui_model.h>
#include <main_window.h>
#include <QtGui>

extern "C" Plugin::Object *createRTXIPlugin(void) {  
	return new Coupling();
}

static Coupling::variable_t vars[] =
{
	{ "Cell 1 Vm", "Membrane potential (V)", Coupling::INPUT, },
	{ "Cell 2 Vm", "Membrane potential (V)", Coupling::INPUT, },
	{ "Cell 1 Spike state", "Spike State (=1 when spike occurs)", Coupling::INPUT, },
	{ "Cell 2 Spike state", "Spike State (=1 when spike occurs)", Coupling::INPUT, },
	{ "Isyn 1-2", "Output current (A)", Coupling::OUTPUT, },
	{ "Isyn 2-1", "Output current (A)", Coupling::OUTPUT, },
	{ "Coupling delay (ms)", "Fixed phase diff at which to turn on Coupling automatically", Coupling::PARAMETER | Coupling::DOUBLE, },
	{ "Tolerance (ms)", "Tolerance for turning on Coupling", Coupling::PARAMETER | Coupling::DOUBLE, },
	{ "Gmax 1-2 (nS)", "Maximum synaptic conductance for stimulus", Coupling::PARAMETER | Coupling::DOUBLE, },
	{ "Tau 1-2 (ms)", "Time constant for alpha-shaped conductance", Coupling::PARAMETER	| Coupling::DOUBLE, },
	{ "Esyn 1-2 (mV)", "Reversal potential for stimulus", Coupling::PARAMETER | Coupling::DOUBLE, },
	{ "Gmax 2-1 (nS)", "Maximum synaptic conductance for stimulus", Coupling::PARAMETER | Coupling::DOUBLE, },
	{ "Tau 2-1 (ms)",	"Time constant for alpha-shaped conductance", Coupling::PARAMETER	| Coupling::DOUBLE, },
	{ "Esyn 2-1 (mV)", "Reversal potential for stimulus",	Coupling::PARAMETER | Coupling::DOUBLE, },
	{ "Phase Diff (s)", "Phase difference (Cell 2 - Cell 1) (s)", Coupling::STATE, },
	{ "Time (s)", "Time (s)", Coupling::STATE, },
};

static size_t num_vars = sizeof(vars) / sizeof(DefaultGUIModel::variable_t);

Coupling::Coupling(void) : DefaultGUIModel("Reciprocal Coupling", ::vars, ::num_vars) {
	setWhatsThis(
	"<p><b>Reciprocal Coupling</b></p><p>Takes Vm and spike state of two cells and outputs reciprocal synaptic currents to couple them together."
	"Computes a phase difference in absolute time as the difference: Spike State Cell 2 - Spike State Cell 1"
	" You can choose a specific delay at which to turn on Coupling along with a tolerance for that delay.</p>");
	initParameters();
	initStimulus();
	DefaultGUIModel::createGUI(vars, num_vars); // this is required to create the GUI
	customizeGUI();
	update(INIT);
	refresh();
	QTimer::singleShot(0, this, SLOT(resizeMe()));
}

Coupling::~Coupling(void) {}

void Coupling::execute(void) {
	systime = count * dt; // time in seconds
	
	Vmcell1 = input(0); // voltage in V
	Vmcell2 = input(1);
	
	if (mode == COUPLED) {
		// synapse from cell 1 to cell 2
		if (input(2) == 1) {// cell 1 just spiked
			Gsyncount1 = 0;
			cell1spktime = systime;
		}
		if (Gsyncount1 > nsamples1) output(0) = 0;
		else output(0) = arrGsyn1[Gsyncount1] * (Vmcell2 - esyn1);
		Gsyncount1++;
		
		// synapse from cell 2 to cell 1
		if (input(3) == 1)  // cell 2 just spiked
			{
			Gsyncount2 = 0;
			cell2spktime = systime;
			phasediff = cell2spktime - cell1spktime;
		}
		if (Gsyncount2 > nsamples2) output(1) = 0;
		else output(1) = arrGsyn2[Gsyncount2] * (Vmcell1 - esyn2);
		Gsyncount2++;
	}
	else if (mode == UNCOUPLED) {
		if (input(2) == 1)  // cell 1 just spiked
			cell1spktime = systime;
		if (input(3) == 1) { // cell 2 just spiked
			cell2spktime = systime;
			phasediff = cell2spktime - cell1spktime;
			if (automate == true && fabs(phasediff-Couplingdelay) < tolerance) { // tolerance of 1 ms
				CouplingEvent event(this, true);
				RT::System::getInstance()->postEvent(&event);
//				coupleBttn->setChecked(true);
//				mode = COUPLED;
			}
		}
	}
	
	count++; // increment time
}

void Coupling::update(Coupling::update_flags_t flag) {
	switch (flag) {
		case INIT:
			setParameter("Coupling delay (ms)", QString::number(Couplingdelay * 1e3)); // initialized in s, display in ms
			setParameter("Tolerance (ms)", QString::number(tolerance * 1e3)); // initialized in s, display in ms
			setParameter("Gmax 1-2 (nS)", QString::number(gmax1 * 1e9)); // initialized in S, display in nS
			setParameter("Tau 1-2 (ms)", QString::number(tau1 * 1e3)); // initialized in s, display in ms
			setParameter("Esyn 1-2 (mV)", QString::number(esyn1 * 1e3)); // initialized in V, display in mV
			setParameter("Gmax 2-1 (nS)", QString::number(gmax2 * 1e9)); // initialized in S, display in nS
			setParameter("Tau 2-1 (ms)", QString::number(tau2 * 1e3)); // initialized in s, display in ms
			setParameter("Esyn 2-1 (mV)", QString::number(esyn2 * 1e3)); // initialized in V, display in mV
			setState("Phase Diff (s)", phasediff);
			setState("Time (s)", systime);
			break;
		
		case MODIFY:
			Couplingdelay = getParameter("Coupling delay (ms)").toDouble() * 1e-3; // set by user in ms, change to s
			tolerance = getParameter("Tolerance (ms)").toDouble() * 1e-3; // set by user in ms, change to s
			gmax1 = getParameter("Gmax 1-2 (nS)").toDouble() * 1e-9; // set by user in nS, change to S
			tau1 = getParameter("Tau 1-2 (ms)").toDouble() * 1e-3; // set by user in ms, change to s
			esyn1 = getParameter("Esyn 1-2 (mV)").toDouble() * 1e-3; // set by user in mV, change to V
			gmax2 = getParameter("Gmax 2-1 (nS)").toDouble() * 1e-9; // set by user in nS, change to S
			tau2 = getParameter("Tau 2-1 (ms)").toDouble() * 1e-3; // set by user in ms, change to s
			esyn2 = getParameter("Esyn 2-1 (mV)").toDouble() * 1e-3; // set by user in mV, change to V
			initStimulus();
			break;
		
		case PERIOD:
			dt = RT::System::getInstance()->getPeriod() * 1e-9; // time in seconds
			initStimulus();
			break;
		
		case PAUSE:
			output(0) = 0.0;
			output(1) = 0;
			break;
		
		case UNPAUSE:
			systime = 0;
			count = 0;
			Gsyncount1 = 0;
			Gsyncount2 = 0;
			break;
	
		default:
			break;
	}
}

void Coupling::initParameters() {
	Couplingdelay = .1875; // s
	tolerance = .010; // s
	automate = false;
	mode = UNCOUPLED;
	
	// parameters for synapse from cell 1 to cell 2
	gmax1 = .04e-9;  // S
	tau1  = 10e-3;   // s
	esyn1 = -70e-3;  // V inhibitory
	Gsyncount1 = 0;
	
	// parameters for synapse from cell 2 to cell 1
	gmax2 = .04e-9;  // S
	tau2  = 10e-3;   // s
	esyn2 = -70e-3;  // V inhibitory
	Gsyncount2 = 0;
	
	dt = RT::System::getInstance()->getPeriod() * 1e-9; // s
	systime = 0;
	count = 0;
	output(0) = 0;
	output(1) = 0;
}

void Coupling::initStimulus() {
	// synapse from cell 1 to cell 2
	arrGsyn1 = new double[int(tau1 * 10 / dt + 1)]; // compute for 10 times tau to get long enough perturbation
	for (int i = 0; i < int(tau1 * 10 / dt + 1); i++) {
		arrGsyn1[i] = -1 * gmax1 * (i * dt) / tau1 * exp(-(i * dt - tau1) / tau1);
	}
	nsamples1 = int(tau1 * 10 / dt + 1);
	
	// synapse from cell 2 to cell 1
	arrGsyn2 = new double[int(tau2 * 10 / dt + 1)]; // compute for 10 times tau to get long enough perturbation
	for (int i = 0; i < int(tau2 * 10 / dt + 1); i++) {
		arrGsyn2[i] = -1 * gmax2 * (i * dt) / tau2 * exp(-(i * dt - tau2) / tau2);
	}
	nsamples2 = int(tau2 * 10 / dt + 1);
}

void Coupling::startCoupling(bool coupled) {
	if (coupled) mode = COUPLED;
	else mode = UNCOUPLED;
	output(0) = 0;
	output(1) = 0;
}

void Coupling::toggleAutomation(bool on) {
	automate = on;
}

void Coupling::customizeGUI(void) {
	QGridLayout *customlayout = DefaultGUIModel::getLayout();
	
	QGroupBox *bttnBox = new QGroupBox("Coupling Functions", this);
	QHBoxLayout *bttnBoxLayout = new QHBoxLayout;
	bttnBox->setLayout(bttnBoxLayout);
	QPushButton *coupleBttn = new QPushButton("Couple Neurons");
	bttnBoxLayout->addWidget(coupleBttn);
	coupleBttn->setCheckable(true);
	coupleBttn->setToolTip("Manually turn on synapses to couple neurons.");
	QObject::connect(coupleBttn, SIGNAL(toggled(bool)), this, SLOT(startCoupling(bool)));
	
	QHBoxLayout *optionBoxLayout = new QHBoxLayout;
	QCheckBox *automateCheckBox = new QCheckBox("Automate coupling delay.");
	optionBoxLayout->addWidget(automateCheckBox);
	QObject::connect(automateCheckBox, SIGNAL(toggled(bool)), this, SLOT(toggleAutomation(bool)));
	
	customlayout->addWidget(bttnBox, 0, 0);
	customlayout->addLayout(optionBoxLayout, 2, 0);
	setLayout(customlayout);
}
