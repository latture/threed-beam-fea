#ifndef DIALOG_H
#define DIALOG_H

#include <QMainWindow>
#include <QProcess>
#include "setup.h"
#include "summary.h"

class QAction;
class QGroupBox;
class QLabel;
class QLineEdit;
class QMenu;
class QMenuBar;
class QPushButton;
class QTextEdit;
class QCheckBox;
class QSpinBox;
class QGridLayout;
class QProgressDialog;
class QColor;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

protected:
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;

private slots:
    void open();
    void save();
    void handleFinishedFEA(int exitCode, QProcess::ExitStatus exitStatus);
    void handleCanceledFEA();
    void submit();
    void setNodesText();
    void setElemsText();
    void setPropsText();
    void setBCsText();
    void setForcesText();
    void setTiesText();
    void updateProgressText();

private:
    void createMenu();
    void createStatusBar();
    void initializeChooseFilesRow(QGridLayout *glayout,
                                  QLineEdit* line_edit,
                                  QPushButton* button,
                                  int row_number);
    void createChooseFilesGroupBox();
    void createOptionsGroupBox();
    void createSubmitGroupBox();

    bool checkFileOpens(const std::string &filename);
    bool checkFilesReady();
    rapidjson::Document createConfigDoc();
    void addMemberToDoc(rapidjson::Document &doc,
                        const std::string &key,
                        const std::string& value);
    void addOptionsToDoc(rapidjson::Document &doc);

    void writeConfigDocToFile(const rapidjson::Document &doc,
                              const std::string &filename);

    void readSettings();
    void writeSettings();

    void removeTmpFiles();
    void setLineEditTextFromConfig(QLineEdit *ledit,
                                   const std::string &variable,
                                   const rapidjson::Document &config_doc);
    void loadOptionsFromConfig(const rapidjson::Document &config_doc);

    void solveFEA();

    QMenuBar *menuBar;

    QGroupBox *chooseFilesGroupBox;
    QGroupBox *optionsGroupBox;
    QGroupBox *submitGroupBox;

    QPushButton *loadNodesButton;
    QPushButton *loadElemsButton;
    QPushButton *loadPropsButton;
    QPushButton *loadBCsButton;
    QPushButton *loadForcesButton;
    QPushButton *loadTiesButton;
    QPushButton *submitButton;

    QLineEdit *nodesLineEdit;
    QLineEdit *elemsLineEdit;
    QLineEdit *propsLineEdit;
    QLineEdit *bcsLineEdit;
    QLineEdit *forcesLineEdit;
    QLineEdit *tiesLineEdit;

    QCheckBox *nodalDispCheckBox;
    QCheckBox *nodalForcesCheckBox;
    QCheckBox *tieForcesCheckBox;
    QCheckBox *reportCheckBox;

    QLineEdit *nodalDispLineEdit;
    QLineEdit *nodalForcesLineEdit;
    QLineEdit *tieForcesLineEdit;
    QLineEdit *reportLineEdit;

    QLabel *epsilonLabel;
    QSpinBox *epsilonSpinBox;

    QLabel *precisionLabel;
    QSpinBox *precisionSpinBox;

    QLabel *delimiterLabel;
    QLineEdit *delimiterLineEdit;

    QMenu *fileMenu;
    QAction *exitAction;
    QAction *openAction;
    QAction *saveAction;
    QProgressDialog *progress;

    QProcess *feaProcess;
    std::string feaProgram;
    std::string feaTmpConfigFilename;
    bool feaTerminated;
};

#endif // DIALOG_H
