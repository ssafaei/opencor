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
// BioSignalML data store importer
//==============================================================================

#pragma once

//==============================================================================

#include "datastoreinterface.h"

//==============================================================================

namespace OpenCOR {
namespace BioSignalMLDataStore {

//==============================================================================

class BiosignalmlDataStoreImporterWorker : public DataStore::DataStoreImporterWorker
{
    Q_OBJECT

public:
    explicit BiosignalmlDataStoreImporterWorker(DataStore::DataStoreImportedData *pDataStoreImportedData);

public slots:
    void run() override;
};

//==============================================================================

class BiosignalmlDataStoreImporter : public DataStore::DataStoreImporter
{
    Q_OBJECT

protected:
    DataStore::DataStoreImporterWorker * workerInstance(DataStore::DataStoreImportedData *pDataStoreImportedData) override;
};

//==============================================================================

}   // namespace BioSignalMLDataStore
}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
