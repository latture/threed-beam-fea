#include <QtWidgets>
#include <assert.h>
#include <cmath>
#include <thread>
#include <chrono>
#include "boost/format.hpp"
#include "mainwindow.h"

#include "csv_parser.h"
#include "options.h"
#include "threed_beam_fea.h"

#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    createMenu();
    createChooseFilesGroupBox();
    createOptionsGroupBox();
    createSubmitGroupBox();
    createStatusBar();

    readSettings();

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setMenuBar(menuBar);
    mainLayout->addWidget(chooseFilesGroupBox);
    mainLayout->addWidget(optionsGroupBox);
    mainLayout->addWidget(submitGroupBox);

    QWidget *widget = new QWidget();
    widget->setLayout(mainLayout);
    setCentralWidget(widget);

    feaProgram = QCoreApplication::applicationDirPath().toStdString() + "/fea_cmd";
    feaTmpConfigFilename = QCoreApplication::applicationDirPath().toStdString() +"/tmp_config.json";
    setMinimumWidth(600);

    setWindowTitle(tr("3D Beam FEA"));
    setWindowIcon(QIcon(":images/logo_64x64.png"));
    setUnifiedTitleAndToolBarOnMac(true);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    writeSettings();
    event->accept();
}

void MainWindow::open() {
    QString filename = QFileDialog::getOpenFileName(this);
    if (!filename.isEmpty()) {

        try {
            rapidjson::Document config_doc = fea::parseJSONConfig(filename.toStdString());
            loadOptionsFromConfig(config_doc);
            statusBar()->showMessage(tr("File loaded"), 2000);
        }
        catch (std::exception &e) {
            std::cerr << "error: " << e.what() << std::endl;
            QMessageBox::critical(this, QString("Error"), QString(e.what()));
            statusBar()->showMessage(tr("Error loading file"), 2000);
        }
    }
}

void MainWindow::save() {
    QString filename = QFileDialog::getSaveFileName(this);
    if (!filename.isEmpty()) {
        rapidjson::Document config_doc = createConfigDoc();
        try {
            writeConfigDocToFile(config_doc, filename.toStdString());
            statusBar()->showMessage(tr("File saved"), 2000);
        }
        catch (std::exception &e) {
            std::cerr << "error: " << e.what() << std::endl;
            QString message_title("Error");
            QString message_text(e.what());
            QMessageBox::critical(this, message_title, message_text);
            setEnabled(true);
        }
    }
}

void MainWindow::handleFinishedFEA(int exitCode, QProcess::ExitStatus exitStatus) {
    if (exitStatus == QProcess::NormalExit && !feaTerminated) {
        QString message_text;
        if (progress) {
            progress->done(QDialog::Accepted);
            message_text = progress->labelText();
            delete progress;
        }
        removeTmpFiles();

        message_text.insert(0, "<pre>");
        message_text.append("</pre>");

        QMessageBox *message = new QMessageBox(QMessageBox::Information, "Summary", message_text, QMessageBox::Ok, this);
        message->exec();
        setEnabled(true);
    }
}

void MainWindow::handleCanceledFEA() {
    feaTerminated = true;
    QString error_text(feaProcess->readAllStandardError());
    feaProcess->kill();
    removeTmpFiles();
    if (progress) {
        progress->done(QDialog::Accepted);
        delete progress;
    }
    if (!error_text.isEmpty()) {
        QMessageBox::critical(this, "FEA exited with error(s)", error_text);
    }

    statusBar()->showMessage(tr("Analysis aborted"), 2000);
    setEnabled(true);
}

void MainWindow::submit() {
    if(checkFilesReady()) {
        progress = new QProgressDialog("Solving analysis...", "Abort", 0, 0);
        progress->setWindowModality(Qt::WindowModal);
        statusBar()->showMessage(tr("Analysis submitted"), 0);
        setEnabled(false);
        progress->show();

        solveFEA();
    }
}

void MainWindow::setNodesText() {
    QString filename = QFileDialog::getOpenFileName(this, tr("Select nodes"), QDir::currentPath(), tr("Data files (*.txt *.csv);; All files (*)"));
    if (!filename.isEmpty()) {
        nodesLineEdit->setText(filename);
    }
}

void MainWindow::setElemsText() {
    QString filename = QFileDialog::getOpenFileName(this, tr("Select elements"), QDir::currentPath(), tr("Data files (*.txt *.csv);; All files (*)"));
    if (!filename.isEmpty()) {
        elemsLineEdit->setText(filename);
    }
}

void MainWindow::setPropsText() {
    QString filename = QFileDialog::getOpenFileName(this, tr("Select properties"), QDir::currentPath(), tr("Data files (*.txt *.csv);; All files (*)"));
    if (!filename.isEmpty()) {
        propsLineEdit->setText(filename);
    }
}

void MainWindow::setBCsText() {
    QString filename = QFileDialog::getOpenFileName(this, tr("Select boundary conditions"), QDir::currentPath(), tr("Data files (*.txt *.csv);; All files (*)"));
    if (!filename.isEmpty()) {
        bcsLineEdit->setText(filename);
    }
}

void MainWindow::setForcesText() {
    QString filename = QFileDialog::getOpenFileName(this, tr("Select forces"), QDir::currentPath(), tr("Data files (*.txt *.csv);; All files (*)"));
    if (!filename.isEmpty()) {
        forcesLineEdit->setText(filename);
    }
}

void MainWindow::setTiesText() {
    QString filename = QFileDialog::getOpenFileName(this, tr("Select ties"), QDir::currentPath(), tr("Data files (*.txt *.csv);; All files (*)"));
    if (!filename.isEmpty()) {
        tiesLineEdit->setText(filename);
    }
}

void MainWindow::updateProgressText() {
    if (progress) {
        progress->setLabelText(QString(feaProcess->readAllStandardOutput()));
    }
}

void MainWindow::createMenu()
{
    menuBar = new QMenuBar;

    fileMenu = new QMenu(tr("&File"), this);
    openAction = fileMenu->addAction(QIcon(":/images/default-document-open.png"), tr("&Open"));
    openAction->setStatusTip(tr("Open an existing file"));
    saveAction = fileMenu->addAction(QIcon(":/images/document-save.png"), tr("&Save"));
    saveAction->setStatusTip(tr("Save configuration file"));
    exitAction = fileMenu->addAction(QIcon(":/images/window-close.png"), tr("E&xit"));
    exitAction->setStatusTip(tr("Exit the application"));
    menuBar->addMenu(fileMenu);

    connect(openAction, SIGNAL(triggered()), this, SLOT(open()));
    connect(saveAction, SIGNAL(triggered()), this, SLOT(save()));
    connect(exitAction, SIGNAL(triggered()), this, SLOT(close()));
}

void MainWindow::createStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
}

void MainWindow::initializeChooseFilesRow(QGridLayout *glayout,
                                          QLineEdit* line_edit,
                                          QPushButton* button,
                                          int row_number) {
    line_edit->setPlaceholderText("No file chosen...");
    glayout->addWidget(button, row_number, 0);
    glayout->addWidget(line_edit, row_number, 1);
}

void MainWindow::createChooseFilesGroupBox()
{
    chooseFilesGroupBox = new QGroupBox(tr("Choose files"));
    QGridLayout *glayout = new QGridLayout;
    int row_counter = 0;

    nodesLineEdit = new QLineEdit(this);
    loadNodesButton = new QPushButton("Nodes");
    loadNodesButton->setStatusTip(tr("Choose file containing nodal coordinates"));
    initializeChooseFilesRow(glayout,
                             nodesLineEdit,
                             loadNodesButton,
                             row_counter++);
    connect(loadNodesButton, &QPushButton::clicked, this, &MainWindow::setNodesText);

    elemsLineEdit = new QLineEdit();
    loadElemsButton = new QPushButton("Elements");
    loadElemsButton->setStatusTip(tr("Choose file containing element indices"));
    initializeChooseFilesRow(glayout,
                             elemsLineEdit,
                             loadElemsButton,
                             row_counter++);
    connect(loadElemsButton, &QPushButton::clicked, this, &MainWindow::setElemsText);

    propsLineEdit = new QLineEdit();
    loadPropsButton = new QPushButton("Properties");
    loadPropsButton->setStatusTip(tr("Choose file containing elemental properties"));
    initializeChooseFilesRow(glayout,
                             propsLineEdit,
                             loadPropsButton,
                             row_counter++);
    connect(loadPropsButton, &QPushButton::clicked, this, &MainWindow::setPropsText);

    bcsLineEdit = new QLineEdit();
    loadBCsButton = new QPushButton("Boundary conditions");
    loadBCsButton->setStatusTip(tr("Choose file containing boundary conditions"));
    initializeChooseFilesRow(glayout,
                             bcsLineEdit,
                             loadBCsButton,
                             row_counter++);
    connect(loadBCsButton, &QPushButton::clicked, this, &MainWindow::setBCsText);

    forcesLineEdit = new QLineEdit();
    loadForcesButton = new QPushButton("Prescribed forces");
    loadForcesButton->setStatusTip(tr("Choose file containing prescribed forces"));
    initializeChooseFilesRow(glayout,
                             forcesLineEdit,
                             loadForcesButton,
                             row_counter++);
    connect(loadForcesButton, &QPushButton::clicked, this, &MainWindow::setForcesText);

    tiesLineEdit = new QLineEdit();
    loadTiesButton = new QPushButton("Ties");
    loadTiesButton->setStatusTip(tr("Choose file containing ties"));
    initializeChooseFilesRow(glayout,
                             tiesLineEdit,
                             loadTiesButton,
                             row_counter++);
    connect(loadTiesButton, &QPushButton::clicked, this, &MainWindow::setTiesText);

    chooseFilesGroupBox->setLayout(glayout);
}

void MainWindow::createOptionsGroupBox()
{
    optionsGroupBox = new QGroupBox(tr("Options"));
    QGridLayout *glayout = new QGridLayout;

    int row_counter = 0;

    nodalDispCheckBox = new QCheckBox(tr("Save nodal displacements"));
    nodalDispCheckBox->setLayoutDirection(Qt::RightToLeft);
    nodalDispCheckBox->setChecked(false);
    nodalDispLineEdit = new QLineEdit(tr("nodal_displacements.csv"));
    nodalDispLineEdit->setDisabled(true);
    connect(nodalDispCheckBox, SIGNAL(toggled(bool)), nodalDispLineEdit, SLOT(setEnabled(bool)));
    glayout->addWidget(nodalDispCheckBox, row_counter, 0);
    glayout->addWidget(nodalDispLineEdit, row_counter, 1, 1, 5);

    nodalForcesCheckBox = new QCheckBox(tr("Save nodal forces"));
    nodalForcesCheckBox->setLayoutDirection(Qt::RightToLeft);
    nodalForcesCheckBox->setChecked(false);
    nodalForcesLineEdit = new QLineEdit(tr("nodal_forces.csv"));
    nodalForcesLineEdit->setDisabled(true);
    connect(nodalForcesCheckBox, SIGNAL(toggled(bool)), nodalForcesLineEdit, SLOT(setEnabled(bool)));
    glayout->addWidget(nodalForcesCheckBox, ++row_counter, 0);
    glayout->addWidget(nodalForcesLineEdit, row_counter, 1, 1, 5);

    tieForcesCheckBox = new QCheckBox(tr("Save tie forces"));
    tieForcesCheckBox->setLayoutDirection(Qt::RightToLeft);
    tieForcesCheckBox->setChecked(false);
    tieForcesLineEdit = new QLineEdit(tr("tie_forces.csv"));
    tieForcesLineEdit->setDisabled(true);
    connect(tieForcesCheckBox, SIGNAL(toggled(bool)), tieForcesLineEdit, SLOT(setEnabled(bool)));
    glayout->addWidget(tieForcesCheckBox, ++row_counter, 0);
    glayout->addWidget(tieForcesLineEdit, row_counter, 1, 1, 5);

    reportCheckBox = new QCheckBox(tr("Save report"));
    reportCheckBox->setLayoutDirection(Qt::RightToLeft);
    reportCheckBox->setChecked(false);
    reportLineEdit = new QLineEdit(tr("report.txt"));
    reportLineEdit->setDisabled(true);
    connect(reportCheckBox, SIGNAL(toggled(bool)), reportLineEdit, SLOT(setEnabled(bool)));
    glayout->addWidget(reportCheckBox, ++row_counter, 0);
    glayout->addWidget(reportLineEdit, row_counter, 1, 1, 5);

    epsilonLabel = new QLabel(tr("epsilon\t1E"));
    epsilonSpinBox = new QSpinBox();
    epsilonSpinBox->setMinimum(-16);
    epsilonSpinBox->setMaximum(0);
    epsilonSpinBox->setValue(-14);
    epsilonSpinBox->setMaximumWidth(50);

    precisionLabel = new QLabel(tr("csv precision"));
    precisionSpinBox = new QSpinBox();
    precisionSpinBox->setMinimum(0);
    precisionSpinBox->setMaximum(16);
    precisionSpinBox->setValue(8);
    precisionSpinBox->setMaximumWidth(50);

    delimiterLabel = new QLabel(tr("csv delimiter"));
    delimiterLineEdit = new QLineEdit(tr(","));
    delimiterLineEdit->setMaximumWidth(50);

    glayout->addWidget(epsilonLabel, ++row_counter, 0, Qt::AlignRight);
    glayout->addWidget(epsilonSpinBox, row_counter, 1);

    glayout->addWidget(precisionLabel, row_counter, 2, Qt::AlignRight);
    glayout->addWidget(precisionSpinBox, row_counter, 3);

    glayout->addWidget(delimiterLabel, row_counter, 4, Qt::AlignRight);
    glayout->addWidget(delimiterLineEdit, row_counter, 5);

    optionsGroupBox->setLayout(glayout);
}

void MainWindow::createSubmitGroupBox() {
    submitGroupBox = new QGroupBox();
    QGridLayout *glayout = new QGridLayout();
    submitButton = new QPushButton("Submit");
    glayout->addWidget(submitButton, 0, 0, Qt::AlignRight);
    submitGroupBox->setLayout(glayout);
    submitGroupBox->setMaximumHeight(80);
    connect(submitButton, SIGNAL(clicked()), this, SLOT(submit()));
}

bool MainWindow::checkFileOpens(const std::string &filename) {
    FILE* file_ptr = fopen(filename.c_str(), "r");
    if (!file_ptr) {
        return false;
    }
    else {
        fclose(file_ptr);
        return true;
    }
}

bool MainWindow::checkFilesReady() {
    int errorCounter = 0;
    QString message_text("");

    if(!checkFileOpens(feaProgram)){
        ++errorCounter;
        message_text.append("Unable to find command line application `fea_gui`.\n"
                            "The command line application should be in the same"
                            "directory as the gui.\n\n");
    }
    if (nodesLineEdit->displayText().isEmpty()) {
        ++errorCounter;
        message_text.append("No file for nodes selected.\n");
    }
    else {
        std::string filename = nodesLineEdit->displayText().toStdString();
        if (!checkFileOpens(filename))
        {
            ++errorCounter;
            message_text.append("Unable to open file selected for nodes.\n");
        }
    }
    if (elemsLineEdit->displayText().isEmpty()) {
        ++errorCounter;
        message_text.append("No file for elements selected.\n");
    }
    else {
        std::string filename = elemsLineEdit->displayText().toStdString();
        if (!checkFileOpens(filename))
        {
            ++errorCounter;
            message_text.append("Unable to open file selected for elements.\n");
        }
    }
    if (propsLineEdit->displayText().isEmpty()) {
        ++errorCounter;
        message_text.append("No file for properties selected.\n");
    }
    else {
        std::string filename = propsLineEdit->displayText().toStdString();
        if (!checkFileOpens(filename))
        {
            ++errorCounter;
            message_text.append("Unable to open file selected for properties.\n");
        }
    }
    if (bcsLineEdit->displayText().isEmpty() && forcesLineEdit->displayText().isEmpty()) {
        ++errorCounter;
        message_text.append("No prescribed boundary conditions or forces.\n");
    }
    if (!bcsLineEdit->displayText().isEmpty())
    {
        std::string filename = bcsLineEdit->displayText().toStdString();
        if (!checkFileOpens(filename))
        {
            ++errorCounter;
            message_text.append("Unable to open file selected for boundary conditions.\n");
        }
    }
    if (!forcesLineEdit->displayText().isEmpty())
    {
        std::string filename = forcesLineEdit->displayText().toStdString();
        if (!checkFileOpens(filename))
        {
            ++errorCounter;
            message_text.append("Unable to open file selected for forces.\n");
        }
    }
    if (!tiesLineEdit->displayText().isEmpty())
    {
        std::string filename = tiesLineEdit->displayText().toStdString();
        if (!checkFileOpens(filename))
        {
            ++errorCounter;
            message_text.append("Unable to open file selected for ties.\n");
        }
    }

    bool isReady = true;
    if(errorCounter > 0) {
        QString message_title(tr("%1 Error(s) found.").arg(QString::number(errorCounter)));
        QMessageBox::critical(this, message_title, message_text);
        isReady = false;
    }

    return isReady;
}

rapidjson::Document MainWindow::createConfigDoc() {

    char json[] = "{}";
    rapidjson::Document config_doc;
    config_doc.ParseInsitu(json);

    addMemberToDoc(config_doc, "nodes", nodesLineEdit->displayText().toStdString());
    addMemberToDoc(config_doc, "elems", elemsLineEdit->displayText().toStdString());
    addMemberToDoc(config_doc, "props", propsLineEdit->displayText().toStdString());

    if (!bcsLineEdit->displayText().isEmpty()) {
        addMemberToDoc(config_doc, "bcs", bcsLineEdit->displayText().toStdString());
    }
    if (!forcesLineEdit->displayText().isEmpty()) {
        addMemberToDoc(config_doc, "forces", forcesLineEdit->displayText().toStdString());
    }
    if (!tiesLineEdit->displayText().isEmpty()) {
        addMemberToDoc(config_doc, "ties", tiesLineEdit->displayText().toStdString());
    }

    addOptionsToDoc(config_doc);

    return config_doc;
}

void MainWindow::addMemberToDoc(rapidjson::Document &doc,
                                const std::string &key,
                                const std::string &value) {

    rapidjson::Value rj_key;
    rj_key.SetString(key.c_str(), key.length(), doc.GetAllocator());

    rapidjson::Value rj_val;
    rj_val.SetString(value.c_str(), value.length(), doc.GetAllocator());

    doc.AddMember(rj_key, rj_val, doc.GetAllocator());
}

void MainWindow::addOptionsToDoc(rapidjson::Document &doc) {
    rapidjson::Value options(rapidjson::kObjectType);

    if (nodalDispCheckBox->isChecked()) {
        options.AddMember("save_nodal_displacements", true, doc.GetAllocator());
        rapidjson::Value rj_val;
        std::string val = nodalDispLineEdit->displayText().toStdString();
        rj_val.SetString(val.c_str(), val.length(), doc.GetAllocator());
        options.AddMember("nodal_displacements_filename", rj_val, doc.GetAllocator());
    }
    if (nodalForcesCheckBox->isChecked()) {
        options.AddMember("save_nodal_forces", true, doc.GetAllocator());
        rapidjson::Value rj_val;
        std::string val = nodalForcesLineEdit->displayText().toStdString();
        rj_val.SetString(val.c_str(), val.length(), doc.GetAllocator());
        options.AddMember("nodal_forces_filename", rj_val, doc.GetAllocator());
    }
    if (tieForcesCheckBox->isChecked()) {
        options.AddMember("save_tie_forces", true, doc.GetAllocator());
        rapidjson::Value rj_val;
        std::string val = tieForcesLineEdit->displayText().toStdString();
        rj_val.SetString(val.c_str(), val.length(), doc.GetAllocator());
        options.AddMember("ties_forces_filename", rj_val, doc.GetAllocator());
    }
    if (reportCheckBox->isChecked()) {
        options.AddMember("save_report", true, doc.GetAllocator());
        rapidjson::Value rj_val;
        std::string val = tieForcesLineEdit->displayText().toStdString();
        rj_val.SetString(val.c_str(), val.length(), doc.GetAllocator());
        options.AddMember("report_filename", rj_val, doc.GetAllocator());
    }
    options.AddMember("verbose", true, doc.GetAllocator());
    doc.AddMember("options", options, doc.GetAllocator());
}

void MainWindow::writeConfigDocToFile(const rapidjson::Document &doc,
                                      const std::string &filename) {
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    doc.Accept(writer);

    std::ofstream output_file;
    output_file.open(filename);

    if (!output_file.is_open()) {
        throw std::runtime_error(
                (boost::format("Could not open file %s.") % filename).str()
        );
    }
    else {
        output_file << buffer.GetString();
        output_file.close();
    }
}

void MainWindow::readSettings()
{
    QSettings settings("Latture", "beam-fea");
    QPoint pos = settings.value("pos", QPoint(200, 200)).toPoint();
    QSize size = settings.value("size", QSize(640, 480)).toSize();
    resize(size);
    move(pos);
}

void MainWindow::writeSettings()
{
    QSettings settings("Latture", "beam-fea");
    settings.setValue("pos", pos());
    settings.setValue("size", size());
}

void MainWindow::removeTmpFiles() {
    remove(feaTmpConfigFilename.c_str());
}

void MainWindow::setLineEditTextFromConfig(QLineEdit *ledit, const std::string &variable, const rapidjson::Document &config_doc) {
    if (config_doc.HasMember(variable.c_str())) {
        if (!config_doc[variable.c_str()].IsString()){
            throw std::runtime_error(
                    (boost::format("Value associated with variable %s is not a string.") % variable).str()
            );
        }
        ledit->setText(tr(config_doc[variable.c_str()].GetString()));
    }
}

void MainWindow::loadOptionsFromConfig(const rapidjson::Document &config_doc) {

    try {
        setLineEditTextFromConfig(nodesLineEdit, "nodes", config_doc);
        setLineEditTextFromConfig(elemsLineEdit, "elems", config_doc);
        setLineEditTextFromConfig(propsLineEdit, "props", config_doc);
        setLineEditTextFromConfig(bcsLineEdit, "bcs", config_doc);
        setLineEditTextFromConfig(tiesLineEdit, "ties", config_doc);
        setLineEditTextFromConfig(forcesLineEdit, "forces", config_doc);

        fea::Options options = fea::createOptionsFromJSON(config_doc);

        epsilonSpinBox->setValue(std::log10(options.epsilon));
        precisionSpinBox->setValue(options.csv_precision);
        delimiterLineEdit->setText(tr(options.csv_delimiter.c_str()));
        if (options.save_nodal_displacements) {
            nodalDispCheckBox->setChecked(true);
            nodalDispLineEdit->setText(tr(options.nodal_displacements_filename.c_str()));
        }
        else {
            nodalDispCheckBox->setChecked(false);
        }
        if (options.save_nodal_forces) {
            nodalForcesCheckBox->setChecked(true);
            nodalForcesLineEdit->setText(tr(options.nodal_forces_filename.c_str()));
        }
        else {
            nodalForcesCheckBox->setChecked(false);
        }
        if (options.save_tie_forces) {
            tieForcesCheckBox->setChecked(true);
            tieForcesLineEdit->setText(tr(options.tie_forces_filename.c_str()));
        }
        else {
            tieForcesCheckBox->setChecked(false);
        }
        if (options.save_report) {
            reportCheckBox->setChecked(true);
            reportLineEdit->setText(tr(options.report_filename.c_str()));
        }
        else {
            reportCheckBox->setChecked(false);
        }
    }
    catch (std::exception &e) {
        throw;
    }
}

void MainWindow::solveFEA() {
    rapidjson::Document configDoc = createConfigDoc();
    feaTerminated = false;
    try {
        writeConfigDocToFile(configDoc, feaTmpConfigFilename);
        QStringList feaProgramArgs;
        feaProgramArgs << "-c" << feaTmpConfigFilename.c_str();
        feaProcess = new QProcess(this);

        connect(progress, SIGNAL(canceled()), this, SLOT(handleCanceledFEA()));
        connect(feaProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(handleFinishedFEA(int, QProcess::ExitStatus)));
        connect(feaProcess, SIGNAL(readyReadStandardOutput()), this, SLOT(updateProgressText()));
        connect(feaProcess, SIGNAL(readyReadStandardError()), this, SLOT(handleCanceledFEA()));

        feaProcess->start(QString::fromStdString(feaProgram), feaProgramArgs);
    }
    catch (std::exception &e) {
        std::cerr << "error: " << e.what();
        QString message_title("Error");
        QString message_text(e.what());
        QMessageBox::critical(this, message_title, message_text);
        setEnabled(true);
    }
}
