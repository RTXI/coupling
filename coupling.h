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

#include <default_gui_model.h>
#include <math.h>
#include <string>

class coupling : public DefaultGUIModel
{

Q_OBJECT

public:

  coupling(void);
  virtual
  ~coupling(void);

  void
  execute(void);
  void
  createGUI(DefaultGUIModel::variable_t *, int);

  enum mode_t
  {
    COUPLED, UNCOUPLED,
  };

public slots:

  signals: // custom signals

  void setFIRange(double newminamp, double newmaxamp, double minHz, double maxHz);
  void
  newDataPoint(double newx, double newy);
  void
  setStepMode(bool);
  void
  setPlotMode(bool);
  void
  saveImage(QString fileName);
  void
  drawFit(double* x, double* y, int size);
  void
  setEqnMsg(const QString &);

protected:

  virtual void
  update(DefaultGUIModel::update_flags_t);

private:

  void
  initParameters();
  void
  initStimulus(); // creates template conductance array
  double phasediff;
  double couplingdelay;
  double tolerance;
  bool automate;
  double dt; // system period
  double systime;
  long long count;
  mode_t mode;

  // parameters for synapse from cell 1 to cell 2
  double Vmcell1;
  double* arrGsyn1; // holds the stimulus conductance values
  int nsamples1; // holds the size of arrGsyn
  int Gsyncount1; // holds current arrGsyn position
  double gmax1;
  double tau1;
  double esyn1;
  double cell1spktime;

  // parameters for synapse from cell 2 to cell 1
  double Vmcell2;
  double* arrGsyn2; // holds the stimulus conductance values
  int nsamples2; // holds the size of arrGsyn
  int Gsyncount2; // holds current arrGsyn position
  double gmax2;
  double tau2;
  double esyn2;
  double cell2spktime;

  // QT components
  QPushButton *coupleBttn;

private slots:

  void startCoupling(bool);
  void
  toggleAutomation(bool);

};
