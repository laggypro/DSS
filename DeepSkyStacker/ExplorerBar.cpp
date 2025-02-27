#include "stdafx.h"
#include <QStyleHints>
#include "ExplorerBar.h"
#include "ui/ui_ExplorerBar.h"

#include "Ztrace.h"
#include "zexcept.h"
#include "DeepSkyStacker.h"
#include "ProcessingDlg.h"
#include "Workspace.h"
#include "RegisterSettings.h"
#include "StackSettings.h"
#include "RawDDPSettings.h"
#include "RecommendedSettings.h"
#include "About.h"
#include "tracecontrol.h"

extern bool		g_bShowRefStars;
extern DSS::TraceControl traceControl;


#define dssApp DeepSkyStacker::instance()

static void makeLink(QLabel *label, QString color, QString text)
{
	label->setText(QString("<a href='.' style='text-decoration: none; color: %1'>%2</a>").arg(color, text));
}

static void makeLink(QLabel *label, QString color)
{
	makeLink(label, color, label->text());
}

ExplorerBar::ExplorerBar(QWidget *parent) :
	QDockWidget(parent),
	initialised{ false },
	ui(new Ui::ExplorerBar),
	windowColourName{ palette().color(QPalette::ColorRole::Window).name()},	// Default base window colour
	activeGroupColourName { "lightcyan" }
{
	ZTRACE_RUNTIME("Creating Explorer Bar");
	ui->setupUi(this);
#if QT_VERSION >= 0x060500
	//
	// Dark colour scheme?
	//
	if (Qt::ColorScheme::Dark == QGuiApplication::styleHints()->colorScheme())
		activeGroupColourName = "darkcyan";

	connect(QGuiApplication::styleHints(), &QStyleHints::colorSchemeChanged,
		this, &ExplorerBar::onColorSchemeChanged);
#endif

	ui->registerAndStack->setStyleSheet(QString("background-color: %1").arg(activeGroupColourName));
	ui->processing->setStyleSheet(QString("background-color: %1").arg(windowColourName));
	ui->options->setStyleSheet(QString("background-color: %1").arg(activeGroupColourName));

	bool value{ traceControl.deleteOnExit() };
	QString disposition;
	if (value)
		disposition = tr("deleted");
	else
		disposition = tr("kept");

	ui->traceFileDisposition->setText(tr("Trace File will be %1").arg(disposition));

	makeLinks();

	raise();
	show();
	activateWindow();

	ZTRACE_RUNTIME("Creating Explorer Bar - ok");
}

ExplorerBar::~ExplorerBar()
{
	delete ui;
}

void ExplorerBar::onInitDialog()
{
	QFont font(ui->registerAndStack->font());
	QFont bold{ font };
	bold.setWeight(QFont::Bold);

	ui->registerAndStack->setFont(bold);
	ui->frame_1->setFont(font);

	ui->frame_2->setFont(font);

	ui->frame_3->setFont(font);

	ui->processing->setFont(bold);
	ui->openPicture->setFont(font);
	ui->copyPicture->setFont(font);
	ui->doStarMask->setFont(font);
	ui->savePicture->setFont(font);

	ui->options->setFont(bold);
	ui->settings->setFont(font);
	ui->ddpSettings->setFont(font);
	ui->loadSettings->setFont(font);
	ui->saveSettings->setFont(font);
	ui->recommendedSettings->setFont(font);

	ui->about->setFont(font);
	ui->help->setFont(font);
	ui->traceFileDisposition->setFont(font);

	ui->enableSounds->setChecked(QSettings{}.value("Beep", false).toBool());

	connect(ui->enableSounds, &QCheckBox::stateChanged, this, &ExplorerBar::onEnableSoundsStateChanged);
}

void ExplorerBar::makeLinks()
{
	QString defColour = palette().color(QPalette::ColorRole::WindowText).name();
	QString redColour = QColor(Qt::red).name();
	QString blueColour = QColor(Qt::blue).name();


	makeLink(ui->openLights, redColour);
	makeLink(ui->openDarks, defColour);
	makeLink(ui->openFlats, defColour);
	makeLink(ui->openDarkFlats, defColour);
	makeLink(ui->openBias, defColour);

	makeLink(ui->openFilelist, defColour);
	makeLink(ui->saveFilelist, defColour);

	makeLink(ui->clearList, defColour);

	makeLink(ui->checkAll, defColour);
	makeLink(ui->checkAbove, defColour);
	makeLink(ui->unCheckAll, defColour);

	makeLink(ui->registerChecked, redColour);
	makeLink(ui->computeOffsets, defColour);
	makeLink(ui->stackChecked, redColour);
	makeLink(ui->batchStacking, defColour);

	makeLink(ui->openPicture, defColour);
	makeLink(ui->copyPicture, defColour);
	makeLink(ui->doStarMask, defColour);
	makeLink(ui->savePicture, defColour);

	makeLink(ui->settings, defColour);
	makeLink(ui->ddpSettings, defColour);
	makeLink(ui->loadSettings, defColour);
	makeLink(ui->saveSettings, defColour);
	makeLink(ui->recommendedSettings, redColour);
	makeLink(ui->about, defColour);
	makeLink(ui->help, defColour);
	makeLink(ui->traceFileDisposition, blueColour);
}

//void ExplorerBar::linkActivated()
//{
//}

void ExplorerBar::onOpenLights()
{
	emit addImages(PICTURETYPE_LIGHTFRAME);
}
void ExplorerBar::onOpenDarks()
{
	emit addImages(PICTURETYPE_DARKFRAME);
}
void ExplorerBar::onOpenFlats()
{
	emit addImages(PICTURETYPE_FLATFRAME);
}
void ExplorerBar::onOpenDarkFlats()
{
	emit addImages(PICTURETYPE_DARKFLATFRAME);
}
void ExplorerBar::onOpenBias()
{
	emit addImages(PICTURETYPE_OFFSETFRAME);
}

/************************************************************************************/

void ExplorerBar::onOpenFilelist()
{
	QPoint point{ ui->openFilelist->mapToGlobal(QPoint(0, 2 + ui->openFilelist->height())) };
	emit loadList(point);
}
void ExplorerBar::onSaveFilelist()
{
	emit saveList();
}
void ExplorerBar::onClearList()
{
	emit clearList();
}

/************************************************************************************/

void ExplorerBar::onCheckAbove()
{
	emit checkAbove();
}
void ExplorerBar::onCheckAll()
{
	emit checkAll();
}
void ExplorerBar::onUncheckAll()
{
	emit unCheckAll();
}

/************************************************************************************/

void ExplorerBar::onRegisterChecked()
{
	emit registerCheckedImages();
}
void ExplorerBar::onComputeOffsets()
{
	emit computeOffsets();
}
void ExplorerBar::onStackChecked()
{
	emit stackCheckedImages();
}
void ExplorerBar::onBatchStacking()
{
	emit batchStack();
}

/************************************************************************************/

void ExplorerBar::onOpenPicture()
{
	dssApp->getProcessingDlg().OnLoaddsi();
}
void ExplorerBar::onCopyPicture()
{
	dssApp->getProcessingDlg().CopyPictureToClipboard();
}
void ExplorerBar::onDoStarMask()
{
	dssApp->getProcessingDlg().CreateStarMask();
}
void ExplorerBar::onSavePicture()
{
	dssApp->getProcessingDlg().SavePictureToFile();
}

/************************************************************************************/

void ExplorerBar::onSettings()
{
	QMenu menu(this);


	QAction *aRegisterSettings;
	QAction *aStackingSettings;

	aRegisterSettings = menu.addAction(tr("Register Settings...", "ID_EDITSETTINGS_REGISTERSETTINGS"));
	aStackingSettings = menu.addAction(tr("Stacking Settings...", "ID_EDITSETTINGS_STACKINGSETTINGS"));

	QPoint point(ui->settings->mapToGlobal(QPoint(0, 2 + ui->settings->height())));
	QAction *a = menu.exec(point);

	if (a == aRegisterSettings)
	{
		RegisterSettings dlg(this);
		dlg.setSettingsOnly(true);
		dlg.exec();
	}
	else if (a == aStackingSettings)
	{
		StackSettings dlg(this);
		dlg.setEnableAll(true);
		dlg.exec();
	}
}

void ExplorerBar::onDDPSettings()
{
	RawDDPSettings dlg(this);

	dlg.exec();
}

void ExplorerBar::onLoadSettings()
{
	ZFUNCTRACE_RUNTIME();
	Workspace	workspace;
	QMenu menu(this);

	if (mruPath.paths.size() != 0)
	{
		for (size_t i = 0; i != mruPath.paths.size(); i++)
		{
			menu.addAction(QString::fromStdU16String(mruPath.paths[i].generic_u16string()));
		}
		menu.addSeparator();
	}

	QAction* restoreSettings =
		menu.addAction(tr("Restore Default settings", "ID_LOADSETTINGS_RESTOREDEFAULTSETTINGS"));
	QAction* loadLiveSettings =
		menu.addAction(tr("Load DeepSkyStacker Live settings", "ID_LOADSETTINGS_LOADDEEPSKYSTACKERLIVESETTINGS"));

	menu.addSeparator();
	QAction* loadAnother =
		menu.addAction(tr("Load...", "ID_LOADSETTINGS_LOAD"));

	QPoint point(ui->settings->mapToGlobal(QPoint(0, 2 + ui->loadSettings->height())));
	QAction* a = menu.exec(point);
	if (a != nullptr)
	{
		if (a == restoreSettings)
		{
			ZTRACE_RUNTIME("Restoring default settings.");
			workspace.ResetToDefault();
			workspace.saveSettings();
		}
		else if (a == loadLiveSettings)
		{
			// Read the DSSLive setting file from the folder %AppData%/DeepSkyStacker/DeepSkyStacker5
			QString directory = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

			fs::path fileName(directory.toStdU16String());
			create_directories(fileName);		// In case they don't exist

			fileName /= "DSSLive.settings";		// Append the filename with a path separator
			ZTRACE_RUNTIME("Loading DSSLive settings from: %s", fileName.generic_string().c_str());
			workspace.ReadFromFile(fileName);
			workspace.saveSettings();
		}
		else if (a == loadAnother)
		{
			ZTRACE_RUNTIME("Invoking dialog to load another settings file.");
			LoadSettingFile();
		}
		else
		{
			ZTRACE_RUNTIME("Loading settings file: %s", a->text().toLocal8Bit().constData());
			//
			// One of the paths in the mruPath must have been selected
			// In which case the action's text string is the fully qualified name of the file to load
			//
			fs::path fileName(a->text().toStdU16String());
			workspace.ReadFromFile(fileName);
			workspace.saveSettings();
			mruPath.Add(fileName);
			mruPath.saveSettings();
		}
	}
}
void ExplorerBar::onSaveSettings()
{
	ZFUNCTRACE_RUNTIME();
	Workspace	workspace;
	QMenu menu(this);

	if (mruPath.paths.size() != 0)
	{
		for (size_t i = 0; i != mruPath.paths.size(); i++)
		{
			menu.addAction(QString::fromStdU16String(mruPath.paths[i].generic_u16string()));
		}
		menu.addSeparator();
	}

	QAction* saveLiveSettings =
		menu.addAction(tr("Save as DeepSkyStacker Live settings", "ID_SAVESETTINGS_SAVEASDEEPSKYSTACKERLIVESETTINGS"));

	menu.addSeparator();
	QAction* saveAnother =
		menu.addAction(tr("Save as...", "ID_SAVESETTINGS_SAVEAS"));

	QPoint point(ui->settings->mapToGlobal(QPoint(0, 2 + ui->loadSettings->height())));
	QAction* a = menu.exec(point);
	if (a != nullptr)
	{
		if (a == saveLiveSettings)
		{
			// Save the DSSLive setting file from the folder %AppData%/DeepSkyStacker/DeepSkyStacker5
			QString directory = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

			fs::path fileName(directory.toStdU16String());
			create_directories(fileName);		// In case they don't exist

			fileName /= "DSSLive.settings";		// Append the filename with a path separator
			ZTRACE_RUNTIME("Saving DSSLive settings to: %s", fileName.generic_string().c_str());
			workspace.SaveToFile(fileName);
		}
		else if (a == saveAnother)
		{
			ZTRACE_RUNTIME("Invoking dialog to save another settings file.");
			SaveSettingFile();
		}
		else
		{
			ZTRACE_RUNTIME("Saving settings to file: %s", a->text().toLocal8Bit().constData());
			//
			// One of the paths in the mruPath must have been selected
			// In which case the action's text string is the fully qualified name of the file to load
			//
			fs::path fileName(a->text().toStdU16String());

			//
			// Save workspace settings
			//
			workspace.SaveToFile(fileName);

			mruPath.Add(fileName);
			mruPath.saveSettings();
		}
	}
}

void ExplorerBar::onRecommendedSettings()
{
	RecommendedSettings		dlg(this);

	dlg.exec();
}

/************************************************************************************/

void ExplorerBar::onAbout()
{
	About dlg(this);

	dlg.exec();
}

void ExplorerBar::onHelp()
{
	QString helpFile = QCoreApplication::applicationDirPath() + 
		"/" + tr("DeepSkyStacker Help.chm", "IDS_HELPFILE");

	//
	// Invoking HtmlHelp works fine on Windows but ...
	//
	::HtmlHelp(::GetDesktopWindow(), CString((wchar_t*)helpFile.utf16()), HH_DISPLAY_TOPIC, 0);
}

void ExplorerBar::onToggleDeletion()
{
	bool value{ !traceControl.deleteOnExit() };
	traceControl.setDeleteOnExit(value);

	QString disposition;
	if (value)
		disposition = tr("deleted");
	else
		disposition = tr("kept");

	ui->traceFileDisposition->setText(tr("Trace File will be %1").arg(disposition));
	QString blueColour = QColor(Qt::blue).name();
	makeLink(ui->traceFileDisposition, blueColour);

	update();
}

void ExplorerBar::onEnableSoundsStateChanged(int state)
{
	Qt::CheckState checked{ static_cast<Qt::CheckState>(state) };
	QSettings{}.setValue("Beep", (Qt::Checked == checked));

}

#if QT_VERSION >= 0x060500
void ExplorerBar::onColorSchemeChanged(Qt::ColorScheme scheme)
{
	//
	// Dark colour scheme?
	//
	if (Qt::ColorScheme::Dark == scheme)
		activeGroupColourName = "darkcyan";
	else
		activeGroupColourName = "lightcyan";

	windowColourName = palette().color(QPalette::ColorRole::Window).name();	// Default base window colour

	const auto tabID = dssApp->tab();
	if (IDD_REGISTERING == tabID)
	{
		ui->registerAndStack->setStyleSheet(QString("background-color: %1").arg(activeGroupColourName));
		ui->processing->setStyleSheet(QString("background-color: %1").arg(windowColourName));
	}
	else
	{
		ui->registerAndStack->setStyleSheet(QString("background-color: %1").arg(windowColourName));
		ui->processing->setStyleSheet(QString("background-color: %1").arg(activeGroupColourName));
	}
	ui->options->setStyleSheet(QString("background-color: %1").arg(activeGroupColourName));

	makeLinks();
}
#endif

void ExplorerBar::LoadSettingFile()
{
	ZFUNCTRACE_RUNTIME();
	QFileDialog			fileDialog;
	Workspace			workspace;
	QString				directory;
	QString				extension("settings");
	bool				fileLoaded(false);

	// Read the DSSLive setting file from the folder %AppData%/DeepSkyStacker/DeepSkyStacker5
	// create the directory to avoid surprises
	directory = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
	create_directories(fs::path{ directory.toStdU16String() });

	fileDialog.setWindowTitle(tr("Load DeepSkyStacker Settings", "IDS_TITLE_LOADSETTINGS"));
	fileDialog.setDefaultSuffix(extension);
	fileDialog.setFileMode(QFileDialog::ExistingFile);

	fileDialog.setNameFilter(tr("DSS Settings Files (*.dsssettings)", "IDS_FILTER_SETTINGFILE"));
	fileDialog.selectFile(QString());		// No file(s) selected
	fileDialog.setDirectory(directory);

	ZTRACE_RUNTIME("About to show file open dlg");
	if (QDialog::Accepted == fileDialog.exec())
	{
		QStringList files = fileDialog.selectedFiles();

		if (files.size() != 0)
		{
			fs::path fileName(files.at(0).toStdU16String());		// as UTF-16
			if (status(fileName).type() == fs::file_type::regular)
			{
				ZTRACE_RUNTIME("Loading settings file: %s", fileName.generic_string().c_str());
				QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

				workspace.ReadFromFile(fileName);
				workspace.saveSettings();
				mruPath.Add(fileName);
				mruPath.saveSettings();
				fileLoaded = true;
				QGuiApplication::restoreOverrideCursor();
			}
		}
	}
	if (!fileLoaded) ZTRACE_RUNTIME("No settings file selected to load");
}

void ExplorerBar::SaveSettingFile()
{
	ZFUNCTRACE_RUNTIME();
	QFileDialog			fileDialog;
	Workspace			workspace;
	QString				directory;
	QString				extension("settings");
	bool				fileSaved(false);

	// Save the DSSLive setting file from the folder %AppData%/DeepSkyStacker/DeepSkyStacker5
	// create the directory to avoid surprises
	directory = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
	create_directories(fs::path{ directory.toStdU16String() });

	fileDialog.setWindowTitle(tr("Save DeepSkyStacker Settings", "IDS_TITLE_SAVESETTINGS"));
	fileDialog.setDefaultSuffix(extension);
	fileDialog.setFileMode(QFileDialog::AnyFile);

	fileDialog.setNameFilter(tr("DSS Settings Files (*.dsssettings)"));
	fileDialog.setDirectory(directory);

	ZTRACE_RUNTIME("About to show file open dlg");
	if (QDialog::Accepted == fileDialog.exec())
	{
		QString file = fileDialog.selectedFiles()[0];

		if (!file.isEmpty())
		{
			fs::path fileName(file.toStdU16String());		// as UTF-16
			ZTRACE_RUNTIME("Saving settings file: %s", fileName.generic_string().c_str());
			QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

			workspace.SaveToFile(fileName);
			mruPath.Add(fileName);
			mruPath.saveSettings();
			fileSaved = true;
			QGuiApplication::restoreOverrideCursor();
		}
	}
	if (!fileSaved) ZTRACE_RUNTIME("No file specified to save settings");
}

void ExplorerBar::changeEvent(QEvent* event)
{
	if (event->type() == QEvent::LanguageChange)
	{
		ui->retranslateUi(this);

		//
		// The Labels are now plain text labels, so make them into links
		// just as done by the ctor.
		//
		makeLinks();
	}

	Inherited::changeEvent(event);
}

void ExplorerBar::showEvent(QShowEvent* event)
{
	if (!event->spontaneous())
	{
		if (!initialised)
		{
			initialised = true;
			onInitDialog();
		}
	}
	// Invoke base class showEvent()
	return Inherited::showEvent(event);
}
void ExplorerBar::mousePressEvent(QMouseEvent *event)
{
	if (Qt::LeftButton == event->buttons())
	{
		const auto dwTabID = dssApp->tab();
		if ((ui->registerAndStack->underMouse()) && (dwTabID != IDD_REGISTERING) && (dwTabID != IDD_STACKING))
		{
			ui->registerAndStack->setStyleSheet(QString("background-color: %1").arg(activeGroupColourName));
			ui->processing->setStyleSheet(QString("background-color: %1").arg(windowColourName));
			// Change tab to stacking
			dssApp->setTab(IDD_STACKING);
		}
		else if (ui->processing->underMouse() && (dwTabID != IDD_PROCESSING))
		{
			ui->registerAndStack->setStyleSheet(QString("background-color: %1").arg(windowColourName));
			ui->processing->setStyleSheet(QString("background-color: %1").arg(activeGroupColourName));
			// Change tab to processing
			dssApp->setTab(IDD_PROCESSING);
		};
	}
	Inherited::mousePressEvent(event);
}