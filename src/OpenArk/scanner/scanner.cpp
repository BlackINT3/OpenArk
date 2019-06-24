/****************************************************************************
**
** Copyright (C) 2019 BlackINT3
** Contact: https://github.com/BlackINT3/OpenArk
**
** GNU Lesser General Public License Usage (LGPL)
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
****************************************************************************/
#include "scanner.h"
#include "../common/common.h"
#include "../openark/openark.h"

#define PE_UNKNOWN_FILE L"Unknown file"
#define PE_FILE32 L"PE 32-bit"
#define PE_FILE64 L"PE 64-bit"

Scanner::Scanner(QWidget *parent) :
	pe_image_(NULL),
	parent_((OpenArk*)parent)
{
	ui.setupUi(this);
	connect(OpenArkLanguage::Instance(), &OpenArkLanguage::languageChaned, this, [this]() {ui.retranslateUi(this); });

	ui.tabWidget->setTabPosition(QTabWidget::West);
	ui.tabWidget->tabBar()->setStyle(new OpenArkTabStyle);
	setAcceptDrops(true);

	sumup_model_ = new QStandardItemModel;
	SetDefaultTableViewStyle(ui.summaryUpView, sumup_model_);
	sumup_model_->setHorizontalHeaderLabels(QStringList() << tr("Name") << tr("Value"));
	ui.summaryUpView->setColumnWidth(0, 120);
	sumdown_model_ = new QStandardItemModel;
	SetDefaultTableViewStyle(ui.summaryDownView, sumdown_model_);
	ui.summaryDownView->horizontalHeader()->hide();
	ui.summaryDownView->setColumnWidth(0, 120);
	ui.splitterSummary->setStretchFactor(0, 2);
	ui.splitterSummary->setStretchFactor(1, 1);

	headers_model_ = new QStandardItemModel;
	headers_model_->setHorizontalHeaderLabels(QStringList() << tr("Name") << tr("Value"));
	SetDefaultTreeViewStyle(ui.headersView, headers_model_);
	ui.headersView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
	ui.headersView->viewport()->installEventFilter(this);
	ui.headersView->installEventFilter(this);
	headers_menu_ = new QMenu();
	headers_menu_->addAction(WCharsToQ(L"ExpandAll"), this, SLOT(onExpandAll()));

	sections_model_ = new QStandardItemModel;
	SetDefaultTableViewStyle(ui.sectionsView, sections_model_);
	sections_model_->setHorizontalHeaderLabels(QStringList() << tr("Name") << tr("VirtualSize") << tr("VirtualAddress") << tr("SizeOfRawData") << tr("PointerToRawData") << tr("PointerToRelocations") << tr("PointerToLinenumbers") << tr("NumberOfRelocations") << tr("NumberOfLinenumbers") << tr("Characteristics"));
	
	imp_model_ = new QStandardItemModel;
	imp_func_model_ = new QStandardItemModel;
	SetDefaultTableViewStyle(ui.impView, imp_model_);
	SetDefaultTableViewStyle(ui.impFuncView, imp_func_model_);
	imp_model_->setHorizontalHeaderLabels(QStringList() << tr("Name") << tr("OriginalFirstThunk") << tr("TimeDateStamp") << tr("ForwarderChain") << tr("FirstThunk"));
	imp_func_model_->setHorizontalHeaderLabels(QStringList() << tr("ForwarderString") << tr("Function") << tr("Ordinal") << tr("AddressOfData") << tr("Hint") << tr("Name"));
	connect(ui.impView->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &Scanner::onImportChanged);

	exp_model_ = new QStandardItemModel;
	exp_func_model_ = new QStandardItemModel;
	SetDefaultTableViewStyle(ui.expView, exp_model_);
	SetDefaultTableViewStyle(ui.expFuncView, exp_func_model_);
	exp_model_->setHorizontalHeaderLabels(QStringList() << tr("Name") << tr("Value"));
	exp_func_model_->setHorizontalHeaderLabels(QStringList() << tr("FunctionAddr") << tr("Ordinal") << tr("Name"));
	
	reloc_model_ = new QStandardItemModel;
	reloc_item_model_ = new QStandardItemModel;
	SetDefaultTableViewStyle(ui.relocView, reloc_model_);
	SetDefaultTableViewStyle(ui.relocItemView, reloc_item_model_);
	reloc_model_->setHorizontalHeaderLabels(QStringList() << tr("VirtualAddress") << tr("SizeOfBlock") << tr("ItemCount"));
	reloc_item_model_->setHorizontalHeaderLabels(QStringList() << tr("Item") << tr("Address") << tr("Type"));
	connect(ui.relocView->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &Scanner::onRelocChanged);

	dbg_model_ = new QStandardItemModel;
	SetDefaultTableViewStyle(ui.debugView, dbg_model_);
	dbg_model_->setHorizontalHeaderLabels(QStringList() << tr("Name") << tr("Value"));

	connect(ui.baseEdit, SIGNAL(textChanged(const QString&)), this, SLOT(onTextChanged(const QString&)));
	connect(ui.rebaseEdit, SIGNAL(textChanged(const QString&)), this, SLOT(onTextChanged(const QString&)));
	connect(ui.vaEdit, SIGNAL(textChanged(const QString&)), this, SLOT(onTextChanged(const QString&)));
	connect(ui.revaEdit, SIGNAL(textChanged(const QString&)), this, SLOT(onTextChanged(const QString&)));
	connect(ui.rvaEdit, SIGNAL(textChanged(const QString&)), this, SLOT(onTextChanged(const QString&)));
	connect(ui.rawEdit, SIGNAL(textChanged(const QString&)), this, SLOT(onTextChanged(const QString&)));
}

Scanner::~Scanner()
{
}

bool Scanner::eventFilter(QObject *obj, QEvent *e)
{
	if (obj == ui.headersView->viewport()) {
		if (e->type() == QEvent::ContextMenu) {
			QContextMenuEvent* ctxevt = dynamic_cast<QContextMenuEvent*>(e);
			if (ctxevt != nullptr) {
				headers_menu_->move(ctxevt->globalPos());
				headers_menu_->show();
			}
		}
	}
	return QWidget::eventFilter(obj, e);
}

void Scanner::dragEnterEvent(QDragEnterEvent *event)
{
	if (event->mimeData()->hasFormat("text/uri-list"))
		event->acceptProposedAction();
}

void Scanner::dragMoveEvent(QDragMoveEvent *event)
{
}

void Scanner::dropEvent(QDropEvent *event)
{
	if (!event->mimeData()->hasUrls())
		return;
	QString& path = event->mimeData()->urls()[0].toLocalFile();
	onOpenFile(path);
}

void Scanner::onImportChanged(const QModelIndex &current, const QModelIndex &previous)
{
	ClearItemModelData(imp_func_model_);
	std::string dll = GetCurItemViewData(ui.impView, 0).toStdString();
	PIMAGE_IMPORT_DESCRIPTOR imp = (PIMAGE_IMPORT_DESCRIPTOR)UNONE::PeGetDataEntity(IMAGE_DIRECTORY_ENTRY_IMPORT, pe_image_);
	if (!imp) return;
	while (imp->Name != 0) {
		if ((pe_image_ + imp->Name) != dll) {
			imp++;
			continue;
		}
		if (UNONE::PeX64(pe_image_)) {
			PIMAGE_THUNK_DATA64 othunk = NULL;
			PIMAGE_THUNK_DATA64 fthunk = (PIMAGE_THUNK_DATA64)(pe_image_ + imp->FirstThunk);
			if (imp->OriginalFirstThunk != 0) othunk = (PIMAGE_THUNK_DATA64)(pe_image_ + imp->OriginalFirstThunk);
			else othunk = fthunk;
			while (othunk != NULL && othunk->u1.Ordinal != 0) {
				WORD hint = 0;
				std::string func_name;
				if (IMAGE_SNAP_BY_ORDINAL64(othunk->u1.Ordinal)) {
					func_name = UNONE::StrFormatA("Ordinal: %08X", othunk->u1.Ordinal & (~IMAGE_ORDINAL_FLAG64));
				} else {
					PIMAGE_IMPORT_BY_NAME imp_name = (PIMAGE_IMPORT_BY_NAME)(pe_image_ + othunk->u1.AddressOfData);
					func_name = (LPCSTR)(imp_name->Name);
					hint = imp_name->Hint;
				}
				InitTableItem(imp_func_model_);
				AppendTableItem(imp_func_model_, QWordToHexQ(othunk->u1.Ordinal));
				AppendTableItem(imp_func_model_, QWordToHexQ(othunk->u1.Ordinal));
				AppendTableItem(imp_func_model_, QWordToHexQ(othunk->u1.Ordinal));
				AppendTableItem(imp_func_model_, QWordToHexQ(othunk->u1.Ordinal));
				AppendTableItem(imp_func_model_, DWordToHexQ(hint));
				AppendTableItem(imp_func_model_, StrToQ(func_name));
				fthunk++;
				othunk++;
			}
		} else {
			PIMAGE_THUNK_DATA32 othunk = NULL;
			PIMAGE_THUNK_DATA32 fthunk = (PIMAGE_THUNK_DATA32)(pe_image_ + imp->FirstThunk);
			if (imp->OriginalFirstThunk != 0) othunk = (PIMAGE_THUNK_DATA32)(pe_image_ + imp->OriginalFirstThunk);
			else othunk = fthunk;
			while (othunk != NULL && othunk->u1.Ordinal != 0) {
				WORD hint = 0;
				std::string func_name;
				if (IMAGE_SNAP_BY_ORDINAL32(othunk->u1.Ordinal)) {
					func_name = UNONE::StrFormatA("Ordinal: %08X", othunk->u1.Ordinal & (~IMAGE_ORDINAL_FLAG32));
				} else {
					PIMAGE_IMPORT_BY_NAME imp_name = (PIMAGE_IMPORT_BY_NAME)(pe_image_ + othunk->u1.AddressOfData);
					func_name = (LPCSTR)(imp_name->Name);
					hint = imp_name->Hint;
				}
				InitTableItem(imp_func_model_);
				AppendTableItem(imp_func_model_, DWordToHexQ(othunk->u1.Ordinal));
				AppendTableItem(imp_func_model_, DWordToHexQ(othunk->u1.Ordinal));
				AppendTableItem(imp_func_model_, DWordToHexQ(othunk->u1.Ordinal));
				AppendTableItem(imp_func_model_, DWordToHexQ(othunk->u1.Ordinal));
				AppendTableItem(imp_func_model_, WordToHexQ(hint));
				AppendTableItem(imp_func_model_, StrToQ(func_name));
				fthunk++;
				othunk++;
			}
		}
		imp++;
	}
}

void Scanner::onRelocChanged(const QModelIndex &current, const QModelIndex &previous)
{
	ClearItemModelData(reloc_item_model_);
	int row = GetCurViewRow(ui.relocView);
	PIMAGE_BASE_RELOCATION reloc = (PIMAGE_BASE_RELOCATION)UNONE::PeGetDataEntity(IMAGE_DIRECTORY_ENTRY_BASERELOC, pe_image_);
	if (!reloc) return;
	PIMAGE_DATA_DIRECTORY dir = UNONE::PeGetDataDirectory(IMAGE_DIRECTORY_ENTRY_BASERELOC, pe_image_);
	DWORD reloc_size = dir->Size;
	DWORD reloc_rva = dir->VirtualAddress;
	DWORD item_size = 0;
	int idx = 0;
	while (item_size < reloc_size) {
		DWORD itemcnt = (reloc->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(USHORT);
		PUSHORT items = (PUSHORT)(reloc + 1);
		if (idx++ == row) {
			for (int i = 0; i < itemcnt; i++) {
				USHORT offset = items[i] & 0xFFF;
				DWORD rva = reloc->VirtualAddress + offset;
				CHAR *type = NULL;
				switch (items[i] >> 12) {
				case IMAGE_REL_BASED_ABSOLUTE:
					type = "IMAGE_REL_BASED_ABSOLUTE";
					break;
				case IMAGE_REL_BASED_HIGHLOW:
					type = "IMAGE_REL_BASED_HIGHLOW";
					break;
				case IMAGE_REL_BASED_HIGH:
					type = "IMAGE_REL_BASED_HIGH";
					break;
				case IMAGE_REL_BASED_LOW:
					type = "IMAGE_REL_BASED_LOW";
					break;
				case IMAGE_REL_BASED_HIGHADJ:
					type = "IMAGE_REL_BASED_HIGHADJ";
					break;
				case IMAGE_REL_BASED_DIR64:
					type = "IMAGE_REL_BASED_DIR64";
					break;
				default:
					type = "IMAGE_REL_BASED_UNKNOWN";
					break;
				}
				InitTableItem(reloc_item_model_);
				AppendTableItem(reloc_item_model_, DWordToHexQ(items[i]));
				AppendTableItem(reloc_item_model_, DWordToHexQ(rva));
				AppendTableItem(reloc_item_model_, CharsToQ(type));
			}
			break;
		}
		item_size += reloc->SizeOfBlock;
		reloc = (PIMAGE_BASE_RELOCATION)(pe_image_ + reloc_rva + item_size);
	}
}

void Scanner::onExpandAll()
{
	ExpandTreeView(ui.headersView->currentIndex(), ui.headersView);
}

void Scanner::onTextChanged(const QString& text)
{
	QLineEdit* sender = qobject_cast<QLineEdit*>(QObject::sender());
	auto SetEditValue = [&](QLineEdit* edit, int64_t val){
		if (sender != edit)
			edit->setText(StrToQ(UNONE::StrFormatA("%llX", val)));
	};
	auto GetEditValue = [](QLineEdit* edit) {
		return UNONE::StrToHex64A(edit->text().toStdString());
	};
	try {
		std::string str = sender->text().toStdString();
		auto InputFilter = [&](std::string& input) {
			UNONE::StrReplaceA(input, "-");
			UNONE::StrReplaceA(input, " ");
			UNONE::StrReplaceA(input, "0x");
			UNONE::StrReplaceA(input, "h");
			UNONE::StrReplaceA(input, "\\x");
			input = UNONE::StrTrimLeftA(input, "0");
			UNONE::StrUpperA(input);
			sender->setText(StrToQ(input));
		};
		InputFilter(str);
		std::unique_lock<std::mutex> guard(upt_mutex_, std::try_to_lock);
		if (!guard.owns_lock()) return;
		int64_t hex = UNONE::StrToHex64A(str);
		int64_t base, va, reva, rva, raw;

		if (sender == ui.baseEdit) {
			va = hex + GetEditValue(ui.revaEdit) - GetEditValue(ui.rebaseEdit);
			SetEditValue(ui.vaEdit, va);
		} else if (sender == ui.vaEdit) {
__va:
			reva = hex - GetEditValue(ui.baseEdit) + GetEditValue(ui.rebaseEdit);
			SetEditValue(ui.revaEdit, reva);
			rva = hex - GetEditValue(ui.baseEdit);
			SetEditValue(ui.rvaEdit, rva);
			hex = rva;
			goto __rva;
		} else if (sender == ui.rebaseEdit) {
			reva = GetEditValue(ui.vaEdit) - GetEditValue(ui.baseEdit) + hex;
			SetEditValue(ui.revaEdit, reva);
			hex = reva;
			goto __reva;
		}	else if (sender == ui.revaEdit) {
__reva:
			va = hex - GetEditValue(ui.rebaseEdit) + GetEditValue(ui.baseEdit);
			SetEditValue(ui.vaEdit, va);
			hex = va;
			goto __va;
		} else if (sender == ui.rvaEdit) {
__rva:
			va = hex + GetEditValue(ui.baseEdit);
			SetEditValue(ui.vaEdit, va);
			reva = va - GetEditValue(ui.baseEdit) + GetEditValue(ui.rebaseEdit);
			SetEditValue(ui.revaEdit, reva);
			if (pe_image_) {
				raw = UNONE::PeRvaToRaw(pe_image_, hex);
				SetEditValue(ui.rawEdit, raw);
			}
			hex = raw;
			goto __raw;
		} else if (sender == ui.rawEdit) {
__raw:
			if (pe_image_) {
				rva = UNONE::PeRawToRva(pe_image_, hex);
				if (rva != 0) {
					SetEditValue(ui.rvaEdit, rva);
					hex = rva + GetEditValue(ui.baseEdit);
/*
					if (va != hex)
						goto __va;*/
				}
			}
		}
	}
	catch (...) {
	}
}

void Scanner::onOpenFile(const QString& file)
{
	if (file.isEmpty()) {
		ERR(tr("file path is empty").toStdWString().c_str());
		return;
	}

	bool pe_valid = false;
	std::wstring path = file.toStdWString();
	pe_file_ = file;
	if (pe_image_) UnmapPe();
	if (MapPe(path)) pe_valid = true;

	RefreshSummary(path);
	if (!pe_valid) return;

	RefreshHeaders();
	RefreshSections();
	RefreshImport();
	RefreshExport();
	RefreshResource();
	RefreshRelocation();
	RefreshDebug();
	RefreshRva();
}

void Scanner::onRefresh()
{
	onOpenFile(pe_file_);
}

std::wstring Scanner::GuessFileType()
{
	if (!pe_image_) return PE_UNKNOWN_FILE;
	std::wstring filetype;
	if (UNONE::PeX64(pe_image_)) {
		filetype.append(PE_FILE64);
	}	else {
		filetype.append(PE_FILE32);
	}
	return filetype.empty() ? PE_UNKNOWN_FILE : filetype;
}

bool Scanner::CheckIsPe()
{
	return pe_valid_;
}

bool Scanner::MapPe(const std::wstring& path)
{
	pe_valid_ = false;
	pe_image_ = UNONE::PeMapImageByPathW(path);
	if (!pe_image_) return false;
	pe_valid_ = UNONE::PeValid(pe_image_) == TRUE;
	pe_x64_ = UNONE::PeX64(pe_image_) == TRUE;
	return true;
}

bool Scanner::UnmapPe()
{
	bool ret = UNONE::PeUnmapImage(pe_image_);
	pe_image_ = NULL;
	ClearItemModelData(sumup_model_);
	ClearItemModelData(sumdown_model_);
	ClearItemModelData(headers_model_);
	ClearItemModelData(sections_model_);
	ClearItemModelData(imp_model_);
	ClearItemModelData(imp_func_model_);
	ClearItemModelData(exp_model_);
	ClearItemModelData(exp_func_model_);
	ClearItemModelData(reloc_model_);
	ClearItemModelData(reloc_item_model_);
	ClearItemModelData(dbg_model_);
	return ret;
}

void Scanner::RefreshSummary(const std::wstring& path)
{
	int up_seq = 0, down_seq = 0;
	double kbytes, mbytes;
	DWORD64 size;
	UNONE::FsGetFileSizeW(path, (DWORD64&)size);
	kbytes = size / 1024;
	mbytes = size / 1024 / 1024;
	std::wstring formed = UNONE::StrFormatW(L"%.2f MB | %.2f KB | %d B", mbytes, kbytes, size);

	auto AddSummaryUpItem = [&](QString name, QString value) {
		sumup_model_->setItem(up_seq, 0, new QStandardItem(name));
		sumup_model_->setItem(up_seq, 1, new QStandardItem(value));
		up_seq++;
	};

	auto AddSummaryDownItem = [&](QString name, QString value) {
		sumdown_model_->setItem(down_seq, 0, new QStandardItem(name));
		sumdown_model_->setItem(down_seq, 1, new QStandardItem(value));
		down_seq++;
	};

	AddSummaryUpItem(tr("File Path"), WStrToQ(UNONE::FsPathStandardW(path)));
	AddSummaryUpItem(tr("File Type"), WStrToQ(GuessFileType()));
	AddSummaryUpItem(tr("File Size"), WStrToQ(formed));

	LONGLONG create_tm, access_tm, modify_tm;
	UNONE::FsGetFileTimeW(path, &create_tm, &access_tm, &modify_tm);
	AddSummaryUpItem(tr("Created Time"), MsToTime(create_tm));
	AddSummaryUpItem(tr("Modified Time"), MsToTime(access_tm));
	AddSummaryUpItem(tr("Accessed Time"), MsToTime(modify_tm));

	auto &&temp = UNONE::StrToA(path);
	auto crc32 = Cryptor::GetCRC32ByFile(temp);
	auto md5 = UNONE::StrStreamToHexStrA(Cryptor::GetMD5ByFile(temp));
	auto sha1 = UNONE::StrStreamToHexStrA(Cryptor::GetSHA1ByFile(temp));
	AddSummaryUpItem(tr("CRC32"), StrToQ(UNONE::StrFormatA("%08x", crc32)));
	AddSummaryUpItem(tr("MD5"), StrToQ(md5));
	AddSummaryUpItem(tr("SHA1"), StrToQ(sha1));

	if (!CheckIsPe()) return;

	std::wstring file_ver, prod_ver, prod_name, cright, origin, inner, corp, desc;
	UNONE::FsGetFileInfoW(path, L"FileVersion", file_ver);
	UNONE::FsGetFileInfoW(path, L"ProductVersion", prod_ver);
	UNONE::FsGetFileInfoW(path, L"ProductName", prod_name);
	UNONE::FsGetFileInfoW(path, L"LegalCopyright", cright);
	UNONE::FsGetFileInfoW(path, L"OriginalFileName", origin);
	UNONE::FsGetFileInfoW(path, L"InternalName", inner);
	UNONE::FsGetFileInfoW(path, L"CompanyName", corp);
	UNONE::FsGetFileInfoW(path, L"FileDescription", desc);

	AddSummaryUpItem(tr("File Version"), WStrToQ(file_ver));
	AddSummaryUpItem(tr("ProductVersion"), WStrToQ(prod_ver));
	AddSummaryUpItem(tr("ProductName"), WStrToQ(prod_name));
	AddSummaryUpItem(tr("LegalCopyright"), WStrToQ(cright));
	AddSummaryUpItem(tr("OriginalFileName"), WStrToQ(origin));
	AddSummaryUpItem(tr("InternalName"), WStrToQ(inner));
	AddSummaryUpItem(tr("CompanyName"), WStrToQ(corp));
	AddSummaryUpItem(tr("FileDescription"), WStrToQ(desc));

	std::wstring pdb, cptime, cpver;
	pdb = UNONE::StrToW(UNONE::PeGetPdb(pe_image_));
	cptime = UNONE::TmFormatUnixTimeW((time_t)PE_NT_HEADER(pe_image_)->FileHeader.TimeDateStamp, L"Y-M-D H:W:S");
	DWORD link_major, link_minor;
	if (UNONE::PeX64(pe_image_)) {
		AddSummaryDownItem(tr("ImageBase"), QWordToHexQ(PE_OPT_HEADER64(pe_image_)->ImageBase));
		AddSummaryDownItem(tr("ImageSize"), DWordToHexQ(PE_OPT_HEADER64(pe_image_)->SizeOfImage));
		AddSummaryDownItem(tr("OEP"), DWordToHexQ(PE_OPT_HEADER64(pe_image_)->AddressOfEntryPoint));
		link_major = PE_OPT_HEADER64(pe_image_)->MajorLinkerVersion;
		link_minor = PE_OPT_HEADER64(pe_image_)->MinorLinkerVersion;
	}	else {
		AddSummaryDownItem(tr("ImageBase"), DWordToHexQ(PE_OPT_HEADER32(pe_image_)->ImageBase));
		AddSummaryDownItem(tr("ImageSize"), DWordToHexQ(PE_OPT_HEADER32(pe_image_)->SizeOfImage));
		AddSummaryDownItem(tr("OEP"), DWordToHexQ(PE_OPT_HEADER32(pe_image_)->AddressOfEntryPoint));
		link_major = PE_OPT_HEADER32(pe_image_)->MajorLinkerVersion;
		link_minor = PE_OPT_HEADER32(pe_image_)->MinorLinkerVersion;
	}

	struct { int major; int minor; wchar_t* info; } linkers[] = {
		{ 5, -1, L"vc50 (5.0)" },
		{ 6, -1, L"vc60 (6.0)" },
		{ 7, -1, L"vc70 (2003)" },
		{ 8, -1, L"vc80 (2005)" },
		{ 9, -1, L"vc90 (2008)" },
		{ 10, -1, L"vc100 (2010)" },
		{ 11, -1, L"vc110 (2012)" },
		{ 12, -1, L"vc120 (2013)" },
		{ 14, 0, L"vc140 (2015)" },
		{ 14, 0x10, L"vc141 (2017)" },
		{ 14, 0x10, L"vc142 (2019)" },
	};
	cpver = UNONE::StrFormatW(L"vc%d%d (unknown)", link_major, link_minor);
	for (int i = 0; i < _countof(linkers); i++) {
		if (linkers[i].major == link_major && (linkers[i].minor == link_minor || linkers[i].minor == -1)) {
			cpver = linkers[i].info;
			break;
		}
	}
	AddSummaryDownItem(tr("Linker"), WStrToQ(cpver));
	AddSummaryDownItem(tr("CompileTime"), WStrToQ(cptime));
	AddSummaryDownItem(tr("PDB File"), WStrToQ(pdb));

	std::vector<UNONE::CertInfoW> infos;
	std::wstring sign, sn;
	bool ret = UNONE::SeGetCertInfoW(path, infos);
	if (!ret) {
		DWORD err = GetLastError();
		UNONE::StrFormatW(sign, L"%X %s", err, UNONE::OsDosErrorMsgW(err).c_str());
	} else {
		sign = infos[0].owner;
		sn = infos[0].sn;
	}
	AddSummaryDownItem(tr("Cert Owner"), WStrToQ(sign));
	AddSummaryDownItem(tr("Cert SN"), WStrToQ(sn));
}

void Scanner::RefreshHeaders()
{
	int row = 0;
	QStandardItem *item, *value;
	auto dos_item = new QStandardItem("IMAGE_DOS_HEADER");
	headers_model_->appendRow(dos_item);
	PIMAGE_DOS_HEADER dos_hdr = PE_DOS_HEADER(pe_image_);
	AppendTreeItem(dos_item, "e_magic", WordToHexQ(dos_hdr->e_magic));
	AppendTreeItem(dos_item, "e_cblp", WordToHexQ(dos_hdr->e_cblp));
	AppendTreeItem(dos_item, "e_cp", WordToHexQ(dos_hdr->e_cp));
	AppendTreeItem(dos_item, "e_crlc", WordToHexQ(dos_hdr->e_crlc));
	AppendTreeItem(dos_item, "e_cparhdr", WordToHexQ(dos_hdr->e_cparhdr));
	AppendTreeItem(dos_item, "e_minalloc", WordToHexQ(dos_hdr->e_minalloc));
	AppendTreeItem(dos_item, "e_maxalloc", WordToHexQ(dos_hdr->e_maxalloc));
	AppendTreeItem(dos_item, "e_ss", WordToHexQ(dos_hdr->e_ss));
	AppendTreeItem(dos_item, "e_sp", WordToHexQ(dos_hdr->e_sp));
	AppendTreeItem(dos_item, "e_csum", WordToHexQ(dos_hdr->e_csum));
	AppendTreeItem(dos_item, "e_ip", WordToHexQ(dos_hdr->e_ip));
	AppendTreeItem(dos_item, "e_cs", WordToHexQ(dos_hdr->e_cs));
	AppendTreeItem(dos_item, "e_lfarlc", WordToHexQ(dos_hdr->e_lfarlc));
	AppendTreeItem(dos_item, "e_ovno", WordToHexQ(dos_hdr->e_ovno));
	AppendTreeItem(dos_item, "e_res", WordArrayToHexQ(dos_hdr->e_res, 4));
	AppendTreeItem(dos_item, "e_oemid", WordToHexQ(dos_hdr->e_oemid));
	AppendTreeItem(dos_item, "e_oeminfo", WordToHexQ(dos_hdr->e_oeminfo));
	AppendTreeItem(dos_item, "e_res2", WordArrayToHexQ(dos_hdr->e_res2, 10));
	AppendTreeItem(dos_item, "e_lfanew", DWordToHexQ(dos_hdr->e_lfanew));

	row = 0;
	auto nt_item = new QStandardItem(pe_x64_ ? "IMAGE_NT_HEADERS64":"IMAGE_FILE_HEADER32");
	headers_model_->appendRow(nt_item);
	PIMAGE_NT_HEADERS nt_hdr = PE_NT_HEADER(pe_image_);
	AppendTreeItem(nt_item, "Signature", DWordToHexQ(nt_hdr->Signature));

	PIMAGE_FILE_HEADER file_hdr = &nt_hdr->FileHeader;
	auto file_item = new QStandardItem("FileHeader");
	nt_item->appendRow(file_item);
	nt_item->setChild(row++, 1, new QStandardItem("IMAGE_FILE_HEADER"));

	row = 0;
	AppendTreeItem(file_item, "Machine", WordToHexQ(file_hdr->Machine));
	AppendTreeItem(file_item, "NumberOfSections", WordToHexQ(file_hdr->NumberOfSections));
	AppendTreeItem(file_item, "TimeDateStamp", DWordToHexQ(file_hdr->TimeDateStamp));
	AppendTreeItem(file_item, "PointerToSymbolTable", DWordToHexQ(file_hdr->PointerToSymbolTable));
	AppendTreeItem(file_item, "NumberOfSymbols", DWordToHexQ(file_hdr->NumberOfSymbols));
	AppendTreeItem(file_item, "SizeOfOptionalHeader", WordToHexQ(file_hdr->SizeOfOptionalHeader));
	AppendTreeItem(file_item, "Characteristics", WordToHexQ(file_hdr->Characteristics));

	PIMAGE_OPTIONAL_HEADER32 opt_hdr32;
	PIMAGE_OPTIONAL_HEADER64 opt_hdr64;
	char* opt_title;
	if (pe_x64_) {
		opt_title = "IMAGE_FILE_HEADER64";
		opt_hdr64 = PE_OPT_HEADER64(pe_image_);
	}	else {
		opt_title = "IMAGE_FILE_HEADER32";
		opt_hdr32 = PE_OPT_HEADER32(pe_image_);
	}
	row = 0;
	auto opt_item = new QStandardItem("OptionalHeader");
	nt_item->appendRow(opt_item);
	nt_item->setChild(2, 1, new QStandardItem(opt_title));

	if (pe_x64_) {
		AppendTreeItem(opt_item, "Magic", WordToHexQ(opt_hdr64->Magic));
		AppendTreeItem(opt_item, "MajorLinkerVersion", ByteToHexQ(opt_hdr64->MajorLinkerVersion));
		AppendTreeItem(opt_item, "MinorLinkerVersion", ByteToHexQ(opt_hdr64->MinorLinkerVersion));
		AppendTreeItem(opt_item, "SizeOfCode", DWordToHexQ(opt_hdr64->SizeOfCode));
		AppendTreeItem(opt_item, "SizeOfInitializedData", DWordToHexQ(opt_hdr64->SizeOfInitializedData));
		AppendTreeItem(opt_item, "SizeOfUninitializedData", DWordToHexQ(opt_hdr64->SizeOfUninitializedData));
		AppendTreeItem(opt_item, "AddressOfEntryPoint", DWordToHexQ(opt_hdr64->AddressOfEntryPoint));
		AppendTreeItem(opt_item, "BaseOfCode", DWordToHexQ(opt_hdr64->BaseOfCode));
		AppendTreeItem(opt_item, "ImageBase", QWordToHexQ(opt_hdr64->ImageBase));
		AppendTreeItem(opt_item, "SectionAlignment", DWordToHexQ(opt_hdr64->SectionAlignment));
		AppendTreeItem(opt_item, "FileAlignment", DWordToHexQ(opt_hdr64->FileAlignment));
		AppendTreeItem(opt_item, "MajorOperatingSystemVersion", WordToHexQ(opt_hdr64->MajorOperatingSystemVersion));
		AppendTreeItem(opt_item, "MinorOperatingSystemVersion", WordToHexQ(opt_hdr64->MinorOperatingSystemVersion));
		AppendTreeItem(opt_item, "MajorImageVersion", WordToHexQ(opt_hdr64->MajorImageVersion));
		AppendTreeItem(opt_item, "MinorImageVersion", WordToHexQ(opt_hdr64->MinorImageVersion));
		AppendTreeItem(opt_item, "MajorSubsystemVersion", WordToHexQ(opt_hdr64->MajorSubsystemVersion));
		AppendTreeItem(opt_item, "MinorSubsystemVersion", WordToHexQ(opt_hdr64->MinorSubsystemVersion));
		AppendTreeItem(opt_item, "Win32VersionValue", DWordToHexQ(opt_hdr64->Win32VersionValue));
		AppendTreeItem(opt_item, "SizeOfImage", DWordToHexQ(opt_hdr64->SizeOfImage));
		AppendTreeItem(opt_item, "SizeOfHeaders", DWordToHexQ(opt_hdr64->SizeOfHeaders));
		AppendTreeItem(opt_item, "CheckSum", DWordToHexQ(opt_hdr64->CheckSum));
		AppendTreeItem(opt_item, "Subsystem", WordToHexQ(opt_hdr64->Subsystem));
		AppendTreeItem(opt_item, "DllCharacteristics", WordToHexQ(opt_hdr64->DllCharacteristics));
		AppendTreeItem(opt_item, "SizeOfStackReserve", QWordToHexQ(opt_hdr64->SizeOfStackReserve));
		AppendTreeItem(opt_item, "SizeOfStackCommit", QWordToHexQ(opt_hdr64->SizeOfStackCommit));
		AppendTreeItem(opt_item, "SizeOfHeapReserve", QWordToHexQ(opt_hdr64->SizeOfHeapReserve));
		AppendTreeItem(opt_item, "SizeOfHeapCommit", QWordToHexQ(opt_hdr64->SizeOfHeapCommit));
		AppendTreeItem(opt_item, "LoaderFlags", DWordToHexQ(opt_hdr64->LoaderFlags));
		AppendTreeItem(opt_item, "NumberOfRvaAndSizes", DWordToHexQ(opt_hdr64->NumberOfRvaAndSizes));
	}
	else {
		AppendTreeItem(opt_item, "Magic", WordToHexQ(opt_hdr32->Magic));
		AppendTreeItem(opt_item, "MajorLinkerVersion", ByteToHexQ(opt_hdr32->MajorLinkerVersion));
		AppendTreeItem(opt_item, "MinorLinkerVersion", ByteToHexQ(opt_hdr32->MinorLinkerVersion));
		AppendTreeItem(opt_item, "SizeOfCode", DWordToHexQ(opt_hdr32->SizeOfCode));
		AppendTreeItem(opt_item, "SizeOfInitializedData", DWordToHexQ(opt_hdr32->SizeOfInitializedData));
		AppendTreeItem(opt_item, "SizeOfUninitializedData", DWordToHexQ(opt_hdr32->SizeOfUninitializedData));
		AppendTreeItem(opt_item, "AddressOfEntryPoint", DWordToHexQ(opt_hdr32->AddressOfEntryPoint));
		AppendTreeItem(opt_item, "BaseOfCode", DWordToHexQ(opt_hdr32->BaseOfCode));
		AppendTreeItem(opt_item, "BaseOfData", DWordToHexQ(opt_hdr32->BaseOfData));
		AppendTreeItem(opt_item, "ImageBase", DWordToHexQ(opt_hdr32->ImageBase));
		AppendTreeItem(opt_item, "SectionAlignment", DWordToHexQ(opt_hdr32->SectionAlignment));
		AppendTreeItem(opt_item, "FileAlignment", DWordToHexQ(opt_hdr32->FileAlignment));
		AppendTreeItem(opt_item, "MajorOperatingSystemVersion", WordToHexQ(opt_hdr32->MajorOperatingSystemVersion));
		AppendTreeItem(opt_item, "MinorOperatingSystemVersion", WordToHexQ(opt_hdr32->MinorOperatingSystemVersion));
		AppendTreeItem(opt_item, "MajorImageVersion", WordToHexQ(opt_hdr32->MajorImageVersion));
		AppendTreeItem(opt_item, "MinorImageVersion", WordToHexQ(opt_hdr32->MinorImageVersion));
		AppendTreeItem(opt_item, "MajorSubsystemVersion", WordToHexQ(opt_hdr32->MajorSubsystemVersion));
		AppendTreeItem(opt_item, "MinorSubsystemVersion", WordToHexQ(opt_hdr32->MinorSubsystemVersion));
		AppendTreeItem(opt_item, "Win32VersionValue", DWordToHexQ(opt_hdr32->Win32VersionValue));
		AppendTreeItem(opt_item, "SizeOfImage", DWordToHexQ(opt_hdr32->SizeOfImage));
		AppendTreeItem(opt_item, "SizeOfHeaders", DWordToHexQ(opt_hdr32->SizeOfHeaders));
		AppendTreeItem(opt_item, "CheckSum", DWordToHexQ(opt_hdr32->CheckSum));
		AppendTreeItem(opt_item, "Subsystem", WordToHexQ(opt_hdr32->Subsystem));
		AppendTreeItem(opt_item, "DllCharacteristics", WordToHexQ(opt_hdr32->DllCharacteristics));
		AppendTreeItem(opt_item, "SizeOfStackReserve", DWordToHexQ(opt_hdr32->SizeOfStackReserve));
		AppendTreeItem(opt_item, "SizeOfStackCommit", DWordToHexQ(opt_hdr32->SizeOfStackCommit));
		AppendTreeItem(opt_item, "SizeOfHeapReserve", DWordToHexQ(opt_hdr32->SizeOfHeapReserve));
		AppendTreeItem(opt_item, "SizeOfHeapCommit", DWordToHexQ(opt_hdr32->SizeOfHeapCommit));
		AppendTreeItem(opt_item, "LoaderFlags", DWordToHexQ(opt_hdr32->LoaderFlags));
		AppendTreeItem(opt_item, "NumberOfRvaAndSizes", DWordToHexQ(opt_hdr32->NumberOfRvaAndSizes));
	}

	auto dir_item = new QStandardItem("DataDirectory[]");
	opt_item->appendRow(dir_item);
	opt_item->setChild(row++, 1, new QStandardItem("IMAGE_DATA_DIRECTORY"));

	QStandardItem* subdir_item;
	PIMAGE_DATA_DIRECTORY dir;

	struct {char* name;	int value;} dir_arr[] = {
		"IMAGE_DIRECTORY_ENTRY_EXPORT", IMAGE_DIRECTORY_ENTRY_EXPORT,
		"IMAGE_DIRECTORY_ENTRY_IMPORT", IMAGE_DIRECTORY_ENTRY_IMPORT,
		"IMAGE_DIRECTORY_ENTRY_RESOURCE", IMAGE_DIRECTORY_ENTRY_RESOURCE,
		"IMAGE_DIRECTORY_ENTRY_EXCEPTION", IMAGE_DIRECTORY_ENTRY_EXCEPTION,
		"IMAGE_DIRECTORY_ENTRY_SECURITY", IMAGE_DIRECTORY_ENTRY_SECURITY,
		"IMAGE_DIRECTORY_ENTRY_BASERELOC", IMAGE_DIRECTORY_ENTRY_BASERELOC,
		"IMAGE_DIRECTORY_ENTRY_DEBUG", IMAGE_DIRECTORY_ENTRY_DEBUG,
		"IMAGE_DIRECTORY_ENTRY_ARCHITECTURE", IMAGE_DIRECTORY_ENTRY_ARCHITECTURE,
		"IMAGE_DIRECTORY_ENTRY_GLOBALPTR", IMAGE_DIRECTORY_ENTRY_GLOBALPTR,
		"IMAGE_DIRECTORY_ENTRY_TLS", IMAGE_DIRECTORY_ENTRY_TLS,
		"IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG", IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG,
		"IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT", IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT,
		"IMAGE_DIRECTORY_ENTRY_IAT", IMAGE_DIRECTORY_ENTRY_IAT,
		"IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT", IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT,
		"IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR", IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR
	};
	
	for (int i = 0; i < _countof(dir_arr); i++) {
		row = 0;
		subdir_item = new QStandardItem(dir_arr[i].name);
		dir_item->appendRow(subdir_item);
		dir_item->setChild(dir_arr[i].value, 1, new QStandardItem(StrToQ(UNONE::StrFormatA("%d", dir_arr[i].value))));
		dir = UNONE::PeGetDataDirectory(dir_arr[i].value, pe_image_);
		AppendTreeItem(subdir_item, "VirtualAddress",DWordToHexQ(dir->VirtualAddress));
		AppendTreeItem(subdir_item, "Size", DWordToHexQ(dir->Size));
	}
}

void Scanner::RefreshSections()
{
	PIMAGE_NT_HEADERS nt_header = PE_NT_HEADER(pe_image_);
	PIMAGE_SECTION_HEADER header = IMAGE_FIRST_SECTION(nt_header);
	for (DWORD i = 0; i < nt_header->FileHeader.NumberOfSections; i++) {
		char name[IMAGE_SIZEOF_SHORT_NAME+1] = { 0 };
		memcpy(name, header->Name, IMAGE_SIZEOF_SHORT_NAME);
		InitTableItem(sections_model_);
		AppendTableItem(sections_model_, CharsToQ(name));
		AppendTableItem(sections_model_, DWordToHexQ(header->Misc.VirtualSize));
		AppendTableItem(sections_model_, DWordToHexQ(header->VirtualAddress));
		AppendTableItem(sections_model_, DWordToHexQ(header->SizeOfRawData));
		AppendTableItem(sections_model_, DWordToHexQ(header->PointerToRawData));
		AppendTableItem(sections_model_, DWordToHexQ(header->PointerToRelocations));
		AppendTableItem(sections_model_, DWordToHexQ(header->PointerToLinenumbers));
		AppendTableItem(sections_model_, DWordToHexQ(header->NumberOfRelocations));
		AppendTableItem(sections_model_, DWordToHexQ(header->NumberOfLinenumbers));
		AppendTableItem(sections_model_, DWordToHexQ(header->Characteristics));
		header++;
	}
}

void Scanner::RefreshImport()
{
	auto imp = (PIMAGE_IMPORT_DESCRIPTOR)UNONE::PeGetDataEntity(IMAGE_DIRECTORY_ENTRY_IMPORT, pe_image_);
	if (!imp) return;
	while (imp->Name != 0) {
		InitTableItem(imp_model_);
		AppendTableItem(imp_model_, CharsToQ(pe_image_ + imp->Name));
		AppendTableItem(imp_model_, DWordToHexQ(imp->OriginalFirstThunk));
		AppendTableItem(imp_model_, DWordToHexQ(imp->TimeDateStamp));
		AppendTableItem(imp_model_, DWordToHexQ(imp->ForwarderChain));
		AppendTableItem(imp_model_, DWordToHexQ(imp->FirstThunk));
		imp++;
	}
}

void Scanner::RefreshExport()
{
#define AppendExpKvItem(key, value) \
	item = new QStandardItem(key); \
	count = exp_model_->rowCount();\
	exp_model_->setItem(count, 0, item); \
	item = new QStandardItem(value); \
	exp_model_->setItem(count, 1, item);
	
	int row = 0, count = 0;
	QStandardItem *item;

	auto exp = (PIMAGE_EXPORT_DIRECTORY)UNONE::PeGetDataEntity(IMAGE_DIRECTORY_ENTRY_EXPORT, pe_image_);
	if (!exp) return;
	AppendExpKvItem("Characteristics", DWordToHexQ(exp->Characteristics));
	AppendExpKvItem("TimeDateStamp", DWordToHexQ(exp->TimeDateStamp));
	AppendExpKvItem("MajorVersion", WordToHexQ(exp->MajorVersion));
	AppendExpKvItem("MinorVersion", WordToHexQ(exp->MinorVersion));
	AppendExpKvItem("Name", DWordToHexQ(exp->Name));
	AppendExpKvItem("Base", DWordToHexQ(exp->Base));
	AppendExpKvItem("NumberOfFunctions", DWordToHexQ(exp->NumberOfFunctions));
	AppendExpKvItem("NumberOfNames", DWordToHexQ(exp->NumberOfNames));
	AppendExpKvItem("AddressOfFunctions", DWordToHexQ(exp->AddressOfFunctions));
	AppendExpKvItem("AddressOfNames", DWordToHexQ(exp->AddressOfNames));
	AppendExpKvItem("AddressOfNameOrdinals", DWordToHexQ(exp->AddressOfNameOrdinals));

	PIMAGE_DATA_DIRECTORY dir = UNONE::PeGetDataDirectory(IMAGE_DIRECTORY_ENTRY_EXPORT, pe_image_);
	PDWORD addr_names = (PDWORD)(exp->AddressOfNames + pe_image_);
	PDWORD addr_funcs = (PDWORD)(exp->AddressOfFunctions + pe_image_);
	PWORD addr_ordinals = (PWORD)(exp->AddressOfNameOrdinals + pe_image_);
	DWORD cnt_names = exp->NumberOfNames;
	DWORD cnt_ordinals = exp->NumberOfFunctions;
	DWORD base_ordinal = exp->Base;
	for (DWORD i = 0; i < cnt_names; i++) {
		DWORD idx = addr_ordinals[i];
		DWORD func_addr = addr_funcs[idx];
		CHAR* func_name = pe_image_ + addr_names[i];
		InitTableItem2(exp_func_model_, exp_func_model_->rowCount());
		AppendTableItem(exp_func_model_, DWordToHexQ(func_addr));
		AppendTableItem(exp_func_model_, DWordToHexQ(idx));
		AppendTableItem(exp_func_model_, CharsToQ(func_name));
	}
}

void Scanner::RefreshResource()
{

}

void Scanner::RefreshRelocation()
{
	auto reloc = (PIMAGE_BASE_RELOCATION)UNONE::PeGetDataEntity(IMAGE_DIRECTORY_ENTRY_BASERELOC, pe_image_);
	if (!reloc) return;
	PIMAGE_DATA_DIRECTORY dir = UNONE::PeGetDataDirectory(IMAGE_DIRECTORY_ENTRY_BASERELOC, pe_image_);
	DWORD reloc_size = dir->Size;
	DWORD reloc_rva = dir->VirtualAddress;
	DWORD item_size = 0;
	while (item_size < reloc_size) {
		DWORD itemcnt = (reloc->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(USHORT);
		InitTableItem(reloc_model_);
		AppendTableItem(reloc_model_, DWordToHexQ(reloc->VirtualAddress));
		AppendTableItem(reloc_model_, DWordToHexQ(reloc->SizeOfBlock));
		AppendTableItem(reloc_model_, DWordToDecQ(itemcnt));
		item_size += reloc->SizeOfBlock;
		reloc = (PIMAGE_BASE_RELOCATION)(pe_image_ + reloc_rva + item_size);
	}
}

void Scanner::RefreshDebug()
{
	ClearItemModelData(dbg_model_);
	PIMAGE_DEBUG_DIRECTORY dbg = (PIMAGE_DEBUG_DIRECTORY)UNONE::PeGetDataEntity(IMAGE_DIRECTORY_ENTRY_DEBUG, pe_image_);
	if (dbg == NULL || dbg->Type != IMAGE_DEBUG_TYPE_CODEVIEW ||
		!dbg->AddressOfRawData || !dbg->SizeOfData) {
		return;
	}
	UNONE::CV_HEADER* cv_hdr = (UNONE::CV_HEADER*)(pe_image_ + dbg->AddressOfRawData);
	DWORD cv_size = dbg->SizeOfData;
	if (!UNONE::PeRegionValid(pe_image_, cv_hdr, cv_size))
		return;
	int count = 0;
	AppendNameValue(dbg_model_, tr("Characteristics"), DWordToHexQ(dbg->Characteristics));
	AppendNameValue(dbg_model_, tr("TimeDateStamp"), DWordToHexQ(dbg->TimeDateStamp));
	AppendNameValue(dbg_model_, tr("MajorVersion"), WordToHexQ(dbg->MajorVersion));
	AppendNameValue(dbg_model_, tr("MinorVersion"), WordToHexQ(dbg->MinorVersion));
	AppendNameValue(dbg_model_, tr("Type"), DWordToHexQ(dbg->Type));
	AppendNameValue(dbg_model_, tr("SizeOfData"), DWordToHexQ(dbg->SizeOfData));
	AppendNameValue(dbg_model_, tr("AddressOfRawData"), DWordToHexQ(dbg->AddressOfRawData));
	AppendNameValue(dbg_model_, tr("PointerToRawData"), DWordToHexQ(dbg->PointerToRawData));

	DWORD age = 0;
	CHAR *sig = "";
	std::string pdb;
	std::string guidsig;
	std::string symid;
	if (cv_hdr->Signature == NB10_SIG) {
		sig = "NB10";
		age = ((UNONE::CV_INFO_PDB20*)cv_hdr)->Age;
		auto name = (CHAR*)((UNONE::CV_INFO_PDB20*)cv_hdr)->PdbFileName;
		auto name_size = (DWORD)strlen(name);
		if (name_size && name_size < MAX_PATH) pdb = name;
	} else if (cv_hdr->Signature == RSDS_SIG) {
		sig = "RSDS";
		auto name = (CHAR*)((UNONE::CV_INFO_PDB70*)cv_hdr)->PdbFileName;
		auto name_size = (DWORD)strlen(name);
		if (name_size && name_size < MAX_PATH) pdb = name;
		GUID guid = ((UNONE::CV_INFO_PDB70*)cv_hdr)->Signature;
		age = ((UNONE::CV_INFO_PDB70*)cv_hdr)->Age;
		guidsig = UNONE::StrFormatA("{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
			guid.Data1,
			guid.Data2,
			guid.Data3,
			guid.Data4[0],
			guid.Data4[1],
			guid.Data4[2],
			guid.Data4[3],
			guid.Data4[4],
			guid.Data4[5],
			guid.Data4[6],
			guid.Data4[7]);
		symid = UNONE::StrFormatA("%08X%04X%04X%02X%02X%02X%02X%02X%02X%02X%02X%X",
			guid.Data1, guid.Data2, guid.Data3,
			guid.Data4[0],
			guid.Data4[1],
			guid.Data4[2],
			guid.Data4[3],
			guid.Data4[4],
			guid.Data4[5],
			guid.Data4[6],
			guid.Data4[7],
			age);
		pdb = UNONE::StrUTF8ToACP(name);
	}

	AppendNameValue(dbg_model_, tr("Signature"), CharsToQ(sig));
	AppendNameValue(dbg_model_, tr("Age"), DWordToHexQ(age));
	AppendNameValue(dbg_model_, tr("GUID"), StrToQ(guidsig));
	AppendNameValue(dbg_model_, tr("SymbolID"), StrToQ(symid));
	AppendNameValue(dbg_model_, tr("PDB"), WStrToQ(UNONE::StrToW(pdb)));
}

void Scanner::RefreshRva()
{
	connect(ui.baseEdit, SIGNAL(textChanged(const QString&)), this, SLOT(onTextChanged(const QString&)));
	connect(ui.rebaseEdit, SIGNAL(textChanged(const QString&)), this, SLOT(onTextChanged(const QString&)));
	connect(ui.vaEdit, SIGNAL(textChanged(const QString&)), this, SLOT(onTextChanged(const QString&)));
	connect(ui.revaEdit, SIGNAL(textChanged(const QString&)), this, SLOT(onTextChanged(const QString&)));
	connect(ui.rvaEdit, SIGNAL(textChanged(const QString&)), this, SLOT(onTextChanged(const QString&)));
	connect(ui.rawEdit, SIGNAL(textChanged(const QString&)), this, SLOT(onTextChanged(const QString&)));

	if (pe_x64_) {
		ULONGLONG base = PE_OPT_HEADER64(pe_image_)->ImageBase;
		base_prev_ = base;
		QString basestr = StrToQ(UNONE::StrFormatA("%llX", base));
		ui.baseEdit->setText(basestr);
		ui.vaEdit->setText(basestr);
		ui.rebaseEdit->setText(basestr);
		ui.revaEdit->setText(basestr);
	} else {
		ULONG base = PE_OPT_HEADER32(pe_image_)->ImageBase;
		base_prev_ = base;
		QString basestr = StrToQ(UNONE::StrFormatA("%X", base));
		ui.baseEdit->setText(basestr);
		ui.vaEdit->setText(basestr);
		ui.rebaseEdit->setText(basestr);
		ui.revaEdit->setText(basestr);
	}
	ui.rvaEdit->setText("0");
	ui.rawEdit->setText("0");
}