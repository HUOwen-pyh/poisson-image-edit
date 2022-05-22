#include "mainwindow.h"

using namespace std;

MainWindow::MainWindow(QWidget* parent)
	: QMainWindow(parent)
{
	//ui.setupUi(this);

	mdi_area_ = new QMdiArea;
	mdi_area_->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	mdi_area_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	setCentralWidget(mdi_area_);

	window_mapper_ = new QSignalMapper(this);
	connect(window_mapper_, SIGNAL(mapped(QWidget*)), this, SLOT(SetActiveSubWindow(QWidget*)));

	CreateActions();
	CreateMenus();
	CreateToolBars();
	CreateStatusBar();

}

MainWindow::~MainWindow()
{

}

void MainWindow::CreateActions()
{

	// File IO
	action_open_ = new QAction(QIcon("./images/open.jpg"), tr("&Open..."), this);
	action_open_->setShortcuts(QKeySequence::Open);
	action_open_->setStatusTip(tr("Open an existing file"));
	connect(action_open_, SIGNAL(triggered()), this, SLOT(Open()));

	action_save_ = new QAction(QIcon("./images/save.jpg"), tr("&Save"), this);
	action_save_->setShortcuts(QKeySequence::Save);
	action_save_->setStatusTip(tr("Save the document to disk"));
	connect(action_save_, SIGNAL(triggered()), this, SLOT(Save()));

	action_saveas_ = new QAction(tr("Save &As..."), this);
	action_saveas_->setShortcuts(QKeySequence::SaveAs);
	action_saveas_->setStatusTip(tr("Save the document under a new name"));
	connect(action_saveas_, SIGNAL(triggered()), this, SLOT(SaveAs()));

	// Image processing
	action_restore_ = new QAction(tr("Restore"), this);
	action_restore_->setStatusTip(tr("Show origin image"));
	connect(action_restore_, SIGNAL(triggered()), this, SLOT(Restore()));

	// Poisson image editting
	action_choose_polygon_ = new QAction(tr("RectChoose"), this);
	connect(action_choose_polygon_, SIGNAL(triggered()), this, SLOT(ChooseRect()));

	action_paste_ = new QAction(tr("Paste"), this);
	connect(action_paste_, SIGNAL(triggered()), this, SLOT(Paste()));
}

void MainWindow::CreateMenus()
{
	menu_file_ = menuBar()->addMenu(tr("&File"));
	menu_file_->setStatusTip(tr("File menu"));
	//	menu_file_->addAction(action_new_);
	menu_file_->addAction(action_open_);
	menu_file_->addAction(action_save_);
	menu_file_->addAction(action_saveas_);
}

void MainWindow::CreateToolBars()
{
	toolbar_file_ = addToolBar(tr("File"));
	// toolbar_file_->addAction(action_new_);
	toolbar_file_->addAction(action_open_);
	toolbar_file_->addAction(action_save_);

	// Add separator in toolbar 
	toolbar_file_->addSeparator();
	toolbar_file_->addAction(action_restore_);

	// Poisson Image Editing
	toolbar_file_->addSeparator();
	toolbar_file_->addAction(action_choose_polygon_);
	toolbar_file_->addAction(action_paste_);
}

void MainWindow::CreateStatusBar()
{
	statusBar()->showMessage(tr("Ready"));
}

void MainWindow::Open()
{
	QString filename = QFileDialog::getOpenFileName(this);
	if (!filename.isEmpty())
	{
		QMdiSubWindow* existing = FindChild(filename);

		if (existing)
		{
			mdi_area_->setActiveSubWindow(existing);
			return;
		}

		ChildWindow* child = CreateChildWindow();
		if (child->LoadFile(filename))
		{
			statusBar()->showMessage(tr("File loaded"), 2000);
			child->show();

			// Change size of the child window so it can fit image size
			int x = child->geometry().x();
			int y = child->geometry().y();
			int width = child->imagewidget_->ImageWidth();
			int height = child->imagewidget_->ImageHeight();
			mdi_area_->activeSubWindow()->setFixedSize(width + 2 * x, height + x + y);
		}
		else
		{
			child->close();
		}
	}
}

void MainWindow::Save()
{
	SaveAs();
}

ChildWindow* MainWindow::GetChildWindow() {
	QMdiSubWindow* activeSubWindow = mdi_area_->activeSubWindow();
	if (!activeSubWindow)
		return nullptr;

	return qobject_cast<ChildWindow*>(activeSubWindow->widget());
}

void MainWindow::SaveAs()
{
	ChildWindow* window = GetChildWindow();
	if (!window)
		return;
	window->imagewidget_->SaveAs();
}

ChildWindow* MainWindow::CreateChildWindow()
{
	ChildWindow* child = new ChildWindow;
	mdi_area_->addSubWindow(child);

	return child;
}

void MainWindow::SetActiveSubWindow(QWidget* window)
{
	if (!window)
	{
		return;
	}

	mdi_area_->setActiveSubWindow(qobject_cast<QMdiSubWindow*>(window));
}

void MainWindow::Restore()
{
	ChildWindow* window = GetChildWindow();
	if (!window)
		return;
	window->imagewidget_->Restore();
}

void MainWindow::ChooseRect()
{
	// Set source child window
	ChildWindow* window = GetChildWindow();
	if (!window)
		return;
	window->imagewidget_->set_draw_status_to_choose();
	child_source_ = window;
}

void MainWindow::Paste()
{
	// Paste image rect region to object image
	ChildWindow* window = GetChildWindow();
	if (!window)
		return;
	window->imagewidget_->set_source_window(child_source_);
	window->imagewidget_->set_draw_status_to_paste();
	
}

QMdiSubWindow* MainWindow::FindChild(const QString& filename)
{
	QString canonical_filepath = QFileInfo(filename).canonicalFilePath();

	foreach(QMdiSubWindow * window, mdi_area_->subWindowList())
	{
		ChildWindow* child = qobject_cast<ChildWindow*>(window->widget());
		if (child->current_file() == canonical_filepath)
		{
			return window;
		}
	}

	return 0;
}
