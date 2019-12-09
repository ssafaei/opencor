/*******************************************************************************

Copyright (C) The University of Auckland

OpenCOR is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

OpenCOR is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <https://gnu.org/licenses>.

*******************************************************************************/

//==============================================================================
// An dockable iPython console window for Qt
//==============================================================================

#pragma once

//==============================================================================

#include "i18ninterface.h"
#include "plugininfo.h"
#include "plugininterface.h"
#include "windowinterface.h"

//==============================================================================

namespace OpenCOR {
namespace PythonConsoleWindow {

//==============================================================================

PLUGININFO_FUNC PythonConsoleWindowPluginInfo();

//==============================================================================

class PythonConsoleWindow;

//==============================================================================

class PythonConsoleWindowPlugin : public QObject, public I18nInterface,
                                  public PluginInterface, public WindowInterface
{
    Q_OBJECT

    Q_PLUGIN_METADATA(IID "OpenCOR.PythonConsoleWindowPlugin" FILE "pythonconsolewindowplugin.json")

    Q_INTERFACES(OpenCOR::I18nInterface)
    Q_INTERFACES(OpenCOR::PluginInterface)
    Q_INTERFACES(OpenCOR::WindowInterface)

public:
#include "i18ninterface.inl"
#include "plugininterface.inl"
#include "windowinterface.inl"

private:
    QAction *mPythonConsoleWindowAction = nullptr;

    PythonConsoleWindow *mPythonConsoleWindowWindow = nullptr;

    void startConsole();
};

//==============================================================================

} // namespace PythonConsoleWindow
} // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================