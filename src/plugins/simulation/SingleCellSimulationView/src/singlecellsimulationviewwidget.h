//==============================================================================
// Single cell simulation view widget
//==============================================================================

#ifndef SINGLECELLSIMULATIONVIEWWIDGET_H
#define SINGLECELLSIMULATIONVIEWWIDGET_H

//==============================================================================

#include "solverinterface.h"
#include "viewwidget.h"

//==============================================================================

class QFrame;
class QLabel;
class QProgressBar;
class QSettings;
class QSplitter;
class QTextEdit;

//==============================================================================

class QwtSlider;

//==============================================================================

namespace Ui {
    class SingleCellSimulationViewWidget;
}

//==============================================================================

namespace OpenCOR {
namespace SingleCellSimulationView {

//==============================================================================

class SingleCellSimulationViewContentsWidget;
class SingleCellSimulationViewProgressBarWidget;
class SingleCellSimulationViewSimulation;
class SingleCellSimulationViewSimulationInformationWidget;
class SingleCellSimulationViewSimulationWorker;

//==============================================================================

class SingleCellSimulationViewWidget : public Core::ViewWidget
{
    Q_OBJECT

public:
    explicit SingleCellSimulationViewWidget(QWidget *pParent = 0);
    ~SingleCellSimulationViewWidget();

    virtual void retranslateUi();

    void addSolverInterface(SolverInterface *pSolverInterface);

    virtual void loadSettings(QSettings *pSettings);
    virtual void saveSettings(QSettings *pSettings) const;

    void initialize(const QString &pFileName);

    QIcon fileTabIcon(const QString &pFileName) const;

protected:
    virtual void paintEvent(QPaintEvent *pEvent);

private:
    Ui::SingleCellSimulationViewWidget *mGui;

    bool mCanSaveSettings;

    SolverInterfaces mSolverInterfaces;

    SingleCellSimulationViewSimulation *mSimulation;
    QMap<QString, SingleCellSimulationViewSimulation *> mSimulations;

    QList<SingleCellSimulationViewSimulation *> mStoppedSimulations;

    QwtSlider *mDelaySlider;
    QLabel *mDelaySliderValue;

    QSplitter *mSplitter;

    SingleCellSimulationViewContentsWidget *mContentsWidget;
    QTextEdit *mOutput;

    SingleCellSimulationViewProgressBarWidget *mProgressBar;

    QMap<QString, int> mProgresses;

    void output(const QString &pMessage);

    void setSimulationMode(const bool &pEnabled, const bool &pRunVisible);

    void clearGraphPanels();
    void clearActiveGraphPanel();

    int tabBarIconSize() const;

private Q_SLOTS:
    void on_actionRun_triggered();
    void on_actionPause_triggered();
    void on_actionStop_triggered();

    void on_actionDebugMode_triggered();

    void on_actionAdd_triggered();
    void on_actionRemove_triggered();

    void on_actionCsvExport_triggered();

    void updateSliderValue(const double &pSliderValue);

    void simulationRunning();
    void simulationPausing();
    void simulationStopped(const int &pElapsedTime);

    void resetProgressBar();
    void resetFileTabIcon();

    void simulationProgress(const double &pProgress,
                            SingleCellSimulationViewSimulation *pSimulation = 0);

    void simulationError(const QString &pError);
};

//==============================================================================

}   // namespace SingleCellSimulationView
}   // namespace OpenCOR

//==============================================================================

#endif

//==============================================================================
// End of file
//==============================================================================
