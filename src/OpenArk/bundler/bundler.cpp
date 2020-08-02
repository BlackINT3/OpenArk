#include "bundler.h"
#include "../common/common.h"
#include "../openark/openark.h"
#include "pack/pack.h"
#include "icons/icons.h"
#include <QFile>

Bundler::Bundler(QWidget *parent) :
	parent_((OpenArk*)parent)
{
	ui.setupUi(this);
	connect(OpenArkLanguage::Instance(), &OpenArkLanguage::languageChaned, this, [this]() {ui.retranslateUi(this); });

	ui.folderLabel->installEventFilter(this);
	ui.folderLabel->setCursor(Qt::PointingHandCursor);
	setAcceptDrops(true);

	QTableView *filesView = ui.filesView;
	files_model_ = new QStandardItemModel;
	SetDefaultTableViewStyle(filesView, files_model_);
	filesView->viewport()->installEventFilter(this);
	files_model_->setHorizontalHeaderLabels(QStringList() << tr("Name") << tr("Size(KB)") << tr("Path"));

	files_menu_ = new QMenu();
	files_menu_->addAction(tr("Use the ICON"), this, SLOT(onUseIcon()));

	connect(ui.openBtn, SIGNAL(clicked()), this, SLOT(onOpenFolder()));
	connect(ui.selectIconBtn, SIGNAL(clicked()), this, SLOT(onSelectIcon()));
	connect(ui.saveBtn, SIGNAL(clicked()), this, SLOT(onSaveTo()));
}
 
Bundler::~Bundler()
{
}

bool Bundler::eventFilter(QObject *obj, QEvent *e)
{
	if (obj == ui.folderLabel) {
		if (e->type() == QEvent::MouseButtonPress) {
			ShellExecuteW(NULL, L"open", files_folder_.toStdWString().c_str(), NULL, NULL, SW_SHOW);
		}
	}	else if (obj == ui.filesView->viewport()) {
		if (e->type() == QEvent::ContextMenu) {
			QContextMenuEvent* ctxevt = dynamic_cast<QContextMenuEvent*>(e);
			if (ctxevt != nullptr) {
				files_menu_->move(ctxevt->globalPos());
				files_menu_->show();
			}
		}
	}
	return QWidget::eventFilter(obj, e);
}

void Bundler::dragEnterEvent(QDragEnterEvent *event)
{
	if (event->mimeData()->hasFormat("text/uri-list"))
		event->acceptProposedAction();
}

void Bundler::dropEvent(QDropEvent *event)
{
	if (!event->mimeData()->hasUrls())
		return;
	QString& path = event->mimeData()->urls()[0].toLocalFile();
	if (!UNONE::FsIsDirW(path.toStdWString()))
		return;
	OpenFolderImpl(path);
}

void Bundler::onRefresh()
{
	OpenFolderImpl(files_folder_);
}

void Bundler::onOpenFolder()
{
	QString folder = QFileDialog::getExistingDirectory(this, tr("Open Folder"), "");
	if (folder.isEmpty()) return;
	OpenFolderImpl(folder);
}

void Bundler::onSelectIcon()
{
	QString file = QFileDialog::getOpenFileName(this, tr("Select ICON"), "", tr("ico/exe (*.exe;*.ico)"));
	if (file.isEmpty()) return;
	SelectIconImpl(file);
}

void Bundler::onUseIcon()
{
	auto idx = ui.filesView->currentIndex();
	auto name = GetItemModelData(files_model_, idx.row(), 2);
	auto path = files_folder_ + "/" + name.replace("./", "");
	SelectIconImpl(path);
}

void Bundler::onSaveTo()
{
	QString bundle = QFileDialog::getSaveFileName(this,tr("Save to"),"",tr("BundleFile(*.exe)"));
	if (bundle.isEmpty()) return;
	
	std::vector<std::wstring> files;
	std::vector<std::wstring> names;
	for (int i = 0; i < files_model_->rowCount(); i++) {
		QString name = GetItemModelData(files_model_, i, 2);
		name.replace("./", "");
		names.push_back(name.toStdWString());
		QString path = files_folder_  + "/" + name;
		files.push_back(path.toStdWString());
	}
	std::wstring script = ui.scriptEdit->toPlainText().toStdWString();
	std::string bdata;
	BundleError err = BundlePack(UNONE::FsPathToNameW(files_folder_.toStdWString()), files, names, script, bdata);
	if (err != BERR_OK) {
		qDebug() << tr("Build err") << err;
		MsgBoxError(tr("Build bundle file err"));
		return;
	}

	QFile f(":/OpenArk/BundlerShell.exe");
	f.open(QIODevice::ReadOnly);
	auto&& shell = f.readAll().toStdString();
	UNONE::FsWriteFileDataW(bundle.toStdWString(), shell);

	std::string icondata;
	if (GetIconData(icon_file_.toStdWString(), icondata) && !icondata.empty()) {
		SetIconData(bundle.toStdWString(), icondata);
	}

	HANDLE upt = BeginUpdateResourceW(bundle.toStdWString().c_str(), FALSE);
	if (!upt) {
		qDebug() << tr("BeginUpdateResourceW err") << GetLastError();
		MsgBoxError(tr("BeginUpdateResourceW err"));
		return;
	}
	if (!UpdateResourceW(upt, IDS_BUDE, MAKEINTRESOURCEW(IDX_BUDE), MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT),
		(LPVOID)bdata.data(), bdata.size())) {
		qDebug() << tr("UpdateResourceW err") << GetLastError();
		MsgBoxError(tr("UpdateResourceW err"));
		return;
	}
	EndUpdateResource(upt, FALSE);

	MsgBoxInfo(QString(tr("Build %1 ok").arg(bundle)));

	UNONE::FsWriteFileDataA("c:/xxx.bd", bdata);
	/*
	std::string d1;
	std::wstring sc;
	std::wstring outdir = L"i:/5566";
	BundleUnpack(outdir, bdata.c_str(), sc);
*/
}

void Bundler::OpenFolderImpl(QString folder)
{
	if (folder.isEmpty()) return;
	//folder.replace("/", "\\");
	files_folder_ = folder;
	ui.folderLabel->setText(folder);
	ClearItemModelData(files_model_);
	UNONE::DirEnumCallbackW fcb = [&](wchar_t* path, wchar_t* name, void* param)->bool {
		if (UNONE::FsIsDirW(path)) {
			UNONE::FsEnumDirectoryW(path, fcb, param);;
			return true;
		}
		InitTableItem(files_model_);
		AppendTableIconItem(files_model_, LoadIcon(WCharsToQ(path)), WCharsToQ(name));
		DWORD64 size;
		UNONE::FsGetFileSizeW(path, size);
		AppendTableItem(files_model_, WStrToQ(UNONE::StrFormatW(L"%.2f", ((double)size) / 1024)));
		std::wstring p(path);
		UNONE::StrReplaceW(p, folder.toStdWString(), L".");
		UNONE::StrReplaceW(p, L"\\", L"/");
		AppendTableItem(files_model_, WStrToQ(p));
		return true;
	};
	UNONE::FsEnumDirectoryW(folder.toStdWString(), fcb);
}

void Bundler::SelectIconImpl(QString icon)
{
	icon_file_ = icon;
	ui.iconLabel->setPixmap(LoadIcon(icon).pixmap(24, 24));
}