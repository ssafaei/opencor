#ifndef PLUGININTERFACE_H
#define PLUGININTERFACE_H

#include <QtPlugin>

#include <QMap>
#include <QStringList>

namespace OpenCOR {

#ifdef Q_WS_WIN
    #define PLUGININFO_FUNC extern "C" __declspec(dllexport) PluginInfo
#else
    #define PLUGININFO_FUNC extern "C" PluginInfo
#endif

typedef QMap<QString, QString> PluginInfoDescriptions;

class PluginInfo
{
    friend class Plugin;

public:
    enum PluginType
    {
        Undefined,
        General,
        Console,
        Gui
    };

    explicit PluginInfo(const PluginType &pType = Undefined,
                        const bool &pManageable = false,
                        const QStringList &pDependencies = QStringList(),
                        const PluginInfoDescriptions &pDescriptions = PluginInfoDescriptions());

    PluginType type() const;
    bool manageable() const;
    QStringList dependencies() const;
    QStringList fullDependencies() const;
    QString description(const QString &pLocale = "en") const;

private:
    PluginType mType;
    bool mManageable;
    QStringList mDependencies;
    QStringList mFullDependencies;
    PluginInfoDescriptions mDescriptions;
};

class PluginInterface : public QObject
{
public:
    virtual void initialize();
    virtual void finalize();
};

}

Q_DECLARE_INTERFACE(OpenCOR::PluginInterface, "OpenCOR.PluginInterface")

#endif
