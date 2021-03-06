/*************************************************
* VERTVER, 2018 (C)
* X-RAY OXYGEN 1.7 PROJECT
*
* Edited: 19 April, 2018
* xrMain.h - Main header for compilation with Qt
* main namespace, classes and vars.
*************************************************/

#pragma once
/////////////////////////////////////////
#include "minimal_CPUID.h"
#include "ui_xrMain.h"
#include "ui_parametersdialog.h"
#include "aboutlauncher.h"
#include "ui_aboutlauncher.h"
#include "../../engine.vc2008/xrCore/xrCore.h"
#include "../../engine.vc2008/xrCore/cpuid.h"
/////////////////////////////////////////
#include <QMainWindow>
#include <QPushButton>
#include <windows.h>
#include <thread>
#include <QApplication>
#include <QDesktopServices>
#include <QUrl>
#include <QDialog>
#include <QMessageBox>
/////////////////////////////////////////
#define DLL_API __declspec(dllimport)
#define CXX 199711L
#define NOAWDA 1
#define NOXRCORE_LOAD 1
/////////////////////////////////////////
static const QStringList LIST_ITEMS = QStringList() 
<< "-r2a" << "-r2" << "-r2.5" <<  "-r3" << "-r4" << "-renderdebug" << "-nointro" << "-mblur";
/////////////////////////////////////////

namespace Ui {
class xrLaunch;			// main window
class xrDialogParam;	// parameters dialog
}

class xrDialogParam : public QDialog 
{
	Q_OBJECT
public: 	
	explicit xrDialogParam(QWidget *parent = NULL);
	~xrDialogParam();

private slots:
    void on_buttonBox_accepted();
	//

private:
	//! other field for other types
	Ui::Dialog	 *uiDialog;
};

class xrLaunch : public QMainWindow
{
    Q_OBJECT

public:
    explicit xrLaunch(QWidget *parent = 0);
	~xrLaunch();

private slots:
    void on_pushButton_clicked();			// Run
	void run_xrEngineRun();					// Run the xrEngine
	void on_actionExit_triggered();			// Menu->Launch->Exit
    void on_actionxrEngine_triggered();		// Menu->Launch->Game->xrEngine
    void on_actionxrCore_triggered();
    void on_actionOxygen_Team_triggered();
    void on_actionVertver_triggered();
    void on_actionGiperion_2_triggered();
    void on_actionForserX_triggered();
    void on_actionParametres_triggered();	
	void on_actionVertver_Github_triggered();
    void on_actionAbout_Oxygen_Team_triggered();
    void on_listWidget_itemPressed(QListWidgetItem *item);
	void status_render();
	void add_stringToList();
	void init_xrCore();

private:
    Ui::xrLaunch *ui;

};

