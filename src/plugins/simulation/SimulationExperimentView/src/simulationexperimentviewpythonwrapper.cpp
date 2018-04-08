/*******************************************************************************

Copyright (C) The University of Auckland

OpenCOR is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

OpenCOR is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*******************************************************************************/

//==============================================================================
// Python wrapper for SimulationExperimentView classes
//==============================================================================

#include "centralwidget.h"
#include "coreguiutils.h"
#include "file.h"
#include "pythonsupport.h"
#include "simulation.h"
#include "simulationexperimentviewplugin.h"
#include "simulationexperimentviewpythonwrapper.h"
#include "simulationexperimentviewsimulationwidget.h"
#include "simulationexperimentviewwidget.h"

//==============================================================================

#include <stdexcept>

//==============================================================================

namespace OpenCOR {

//==============================================================================

namespace SimulationSupport {
    class Simulation;
}   // namespace Simulationsupport

//==============================================================================

namespace SimulationExperimentView {

//==============================================================================

static SimulationSupport::Simulation *getSimulation(const QString &pFileName,
                                                    SimulationExperimentViewWidget *pSimulationExperimentViewWidget)
{
    SimulationSupport::Simulation *simulation = pSimulationExperimentViewWidget->simulation(pFileName);

    if (simulation) {
        // Let the simulation's widget know when we start running
        // Note: connect is also in the PythonQt namespace

        QObject::connect(simulation, &SimulationSupport::Simulation::runStarting,
                         pSimulationExperimentViewWidget, &SimulationExperimentViewWidget::startingRun);

        // Use the simulation's widget to clear our data

        QObject::connect(simulation, &SimulationSupport::Simulation::clearData,
                         pSimulationExperimentViewWidget, &SimulationExperimentViewWidget::clearData);
    }

    return simulation;
}

//==============================================================================

static PyObject *initializeSimulation(const QString &pFileName)
{
    SimulationExperimentViewWidget *simulationExperimentViewWidget = SimulationExperimentViewPlugin::instance()->viewWidget();

    if (simulationExperimentViewWidget) {
        simulationExperimentViewWidget->initialize(pFileName);

        auto simulation = getSimulation(pFileName, simulationExperimentViewWidget);

        if (simulation)
            return PythonQt::priv()->wrapQObject(simulation);
    }

    Py_RETURN_NONE;
}

//==============================================================================

static PyObject *openSimulation(PyObject *self, PyObject *args)
{
    Q_UNUSED(self);

    PyObject *bytes;
    char *name;
    Py_ssize_t len;
    if (!PyArg_ParseTuple(args, "O&", PyUnicode_FSConverter, &bytes)) {
        Py_RETURN_NONE;
    }
    PyBytes_AsStringAndSize(bytes, &name, &len);
    QString fileName = QString::fromUtf8(name, len);
    Py_DECREF(bytes);

    QString ioError = Core::centralWidget()->openFile(fileName, Core::File::Local,
                                                      QString(), false);
    if (!ioError.isEmpty()) {
        PyErr_SetString(PyExc_IOError, ioError.toStdString().c_str());
        return NULL;
    } else {
        return initializeSimulation(QFileInfo(fileName).canonicalFilePath());
    }
}

//==============================================================================

static PyObject *openRemoteSimulation(PyObject *self, PyObject *args)
{
    Q_UNUSED(self);

    PyObject *bytes;
    char *name;
    Py_ssize_t len;
    if (!PyArg_ParseTuple(args, "O&", PyUnicode_FSConverter, &bytes)) {
        Py_RETURN_NONE;
    }
    PyBytes_AsStringAndSize(bytes, &name, &len);
    QString url = QString::fromUtf8(name, len);
    Py_DECREF(bytes);

    QString ioError = Core::centralWidget()->openRemoteFile(url, false); // No warning...
    if (!ioError.isEmpty()) {
        PyErr_SetString(PyExc_IOError, ioError.toStdString().c_str());
        return NULL;
    }

    return initializeSimulation(Core::centralWidget()->localFileName(url));
}

//==============================================================================

static PyObject *OpenCOR_simulations(PyObject *self,  PyObject *args)
{
    Q_UNUSED(self);
    Q_UNUSED(args);

    PyObject *simulationDict = PyDict_New();
    SimulationExperimentViewWidget *simulationExperimentViewWidget = SimulationExperimentViewPlugin::instance()->viewWidget();

    if (simulationExperimentViewWidget) {
        foreach (const QString &fileName, simulationExperimentViewWidget->fileNames()) {
            auto simulation = getSimulation(fileName, simulationExperimentViewWidget);

            // Add the simulation to the dictionary

            if (simulation)
                PythonSupport::addObject(simulationDict, fileName, simulation);
        }
    }

    return simulationDict;
}

//==============================================================================

static PyObject *OpenCOR_simulation(PyObject *self,  PyObject *args)
{
    Q_UNUSED(self);
    Q_UNUSED(args);

    SimulationExperimentViewWidget *simulationExperimentViewWidget = SimulationExperimentViewPlugin::instance()->viewWidget();
    if (simulationExperimentViewWidget) {
        auto simulation = getSimulation(Core::centralWidget()->currentFileName(), simulationExperimentViewWidget);

        // Return the simulation as a Python object

        if (simulation)
            return PythonQt::priv()->wrapQObject(simulation);

    }

    Py_RETURN_NONE;
}

//==============================================================================

static PyMethodDef pythonSimulationExperimentViewMethods[] = {
    {"openSimulation",  openSimulation, METH_VARARGS, "Open a simulation."},
    {"openRemoteSimulation",  openRemoteSimulation, METH_VARARGS, "Open a remote simulation."},
    {"simulation",  OpenCOR_simulation, METH_VARARGS, "Current simulation."},
    {"simulations",  OpenCOR_simulations, METH_VARARGS, "Dictionary of simulations."},
    {NULL, NULL, 0, NULL}
};

//==============================================================================

SimulationExperimentViewPythonWrapper::SimulationExperimentViewPythonWrapper(PyObject *pModule, QObject *pParent) : QObject(pParent)
{
    PyModule_AddFunctions(pModule, pythonSimulationExperimentViewMethods);
}

//==============================================================================
}   // namespace SimulationExperimentView
}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
