static PyObject * openSimulation(PyObject *pSelf, PyObject *pArgs)
{
    Q_UNUSED(pSelf)

    // Open a simulation

    PyObject *bytes;

    if (PyArg_ParseTuple(pArgs, "O&", PyUnicode_FSConverter, &bytes) == 0) { // NOLINT(cppcoreguidelines-pro-type-vararg)
#include "pythonbegin.h"
        Py_RETURN_NONE;
#include "pythonend.h"
    }

    char *string;
    Py_ssize_t len;

    PyBytes_AsStringAndSize(bytes, &string, &len);

    bool isLocalFile;
    QString fileNameOrUrl;

    Core::checkFileNameOrUrl(QString::fromUtf8(string, int(len)), isLocalFile, fileNameOrUrl);

#include "pythonbegin.h"
    Py_DECREF(bytes);
#include "pythonend.h"

    QString error = isLocalFile?
#ifdef GUI_SUPPORT
                        Core::centralWidget()->openFile(fileNameOrUrl,
                                                        Core::File::Type::Local,
                                                        QString(), false):
                        Core::centralWidget()->openRemoteFile(fileNameOrUrl, false);
#else
                        Core::openFile(fileNameOrUrl):
                        Core::openRemoteFile(fileNameOrUrl);
#endif

    if (!error.isEmpty()) {
        PyErr_SetString(PyExc_IOError, qPrintable(error));

        return nullptr;
    }

    return initializeSimulation(isLocalFile?
                                    fileNameOrUrl:
                                    Core::FileManager::instance()->fileName(fileNameOrUrl));
}
