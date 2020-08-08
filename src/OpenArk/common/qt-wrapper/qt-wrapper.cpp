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
#include "qt-wrapper.h"
#include "../common/common.h"

QSize OpenArkTabStyle::sizeFromContents(ContentsType type, const QStyleOption *option, const QSize &size, const QWidget *widget) const
{
	QSize s = QProxyStyle::sizeFromContents(type, option, size, widget);
	if (type == QStyle::CT_TabBarTab) {
		s.transpose();
		s.rwidth() = 140;
		s.rheight() = 30;
	}
	return s;
}
void OpenArkTabStyle::drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
	if (element == CE_TabBarTabLabel) {
		const QStyleOptionTab* tabopt = reinterpret_cast<const QStyleOptionTab*>(option);
		if (tabopt) {
			QRect rect = tabopt->rect;
			if (tabopt->state & QStyle::State_Selected) {
				painter->save();
				painter->setPen(0xb9b9b9);
				painter->setBrush(QBrush(0xffffff));
				if (tabopt->position != QStyleOptionTab::End) {
					painter->drawRect(rect.adjusted(0, 0, 20, 0));
				}	else {
					painter->drawRect(rect.adjusted(0, 0, 20, -1));
				}
				painter->restore();
				painter->save();
				painter->setPen(0x00868b);
				QTextOption option;
				option.setAlignment(Qt::AlignCenter);
				painter->setFont(QFont("", 11, QFont::Bold));
				painter->drawText(rect, tabopt->text, option);
				painter->restore();
			} else {
				painter->save();
				painter->setPen(0xb9b9b9);
				painter->setBrush(QBrush(0xf0f0f0));
				if (tabopt->position != QStyleOptionTab::End) {
					painter->drawRect(rect.adjusted(0, 0, 20, 0));
				}	else {
					painter->drawRect(rect.adjusted(0, 0, 20, -1));
				}
				painter->restore();
				painter->save();
				QTextOption option;
				option.setAlignment(Qt::AlignCenter);
				painter->setFont(QFont("", 11));
				painter->drawText(rect, tabopt->text, option);
				painter->restore();
			}
			return;
		}
	}
	else if (element == CE_TabBarTab) {
		QProxyStyle::drawControl(element, option, painter, widget);
	}
}

OpenArkLanguage* OpenArkLanguage::langobj_ = nullptr;
OpenArkLanguage* OpenArkLanguage::Instance()
{
	if (langobj_) return langobj_;
	langobj_ = new OpenArkLanguage;
	return langobj_;
}
void OpenArkLanguage::ChangeLanguage(int lang)
{
	curlang_ = lang;
	switch (lang) {
	case -1:
		if (QLocale::system().language() == QLocale::Chinese) {
			return ChangeLanguage(1);
		} else {
			return ChangeLanguage(0);
		}
		break;
	case 0:
		if (app_tr) {
			app->removeTranslator(app_tr);
			//emit languageChaned();
		}
		break;
	case 1:
		if (app_tr) {
			app_tr->load(":/OpenArk/lang/openark_zh.qm");
			app->installTranslator(app_tr);
			//emit languageChaned();
		}
	default:
		break;
	}
}

QIcon LoadIcon(QString file_path)
{
	static struct {
		QMutex lck;
		QMap<QString, QIcon> d;
	} icon_cache;
	QMutexLocker locker(&icon_cache.lck);
	if (icon_cache.d.contains(file_path)) {
		auto it = icon_cache.d.find(file_path);
		return it.value();
	}
	QFileInfo file_info(file_path);
	QFileIconProvider provider;
	QIcon &ico = provider.icon(file_info);
	for (auto qs : ico.availableSizes()) {
		if (!ico.pixmap(qs).isNull()) {
			icon_cache.d.insert(file_path, ico);
			return ico;
		}
	}
	ico = QIcon(":/OpenArk/revtools/default.ico");
	icon_cache.d.insert(file_path, ico);
	return ico;
}

bool IsContainAction(QMenu *menu, QObject *obj)
{
	QAction *action = qobject_cast<QAction*>(obj);
	for (auto a : menu->actions()) {
		if (action == a) {
			return true;
		}
	}
	return false;
}

bool ExploreFile(QString file_path)
{
	QString cmdline = "explorer.exe /select," + file_path;
	return UNONE::PsCreateProcessW(cmdline.toStdWString(), SW_SHOW, NULL);
}

QString GetItemModelData(QAbstractItemModel *model, int row, int column)
{
	return model->data(model->index(row, column)).toString();
	//auto idx = model->index(row, column);
	//return idx.sibling(row, column).data().toString();
}

QString GetItemViewData(QAbstractItemView *view, int row, int column)
{
	auto idx = view->rootIndex();
	return idx.sibling(row, column).data().toString();
}

int GetCurViewRow(QAbstractItemView *view)
{
	return view->currentIndex().row();
}

int GetCurViewColumn(QAbstractItemView *view)
{
	return view->currentIndex().column();
}

QModelIndex GetCurItemView(QAbstractItemView *view, int column)
{
	auto idx = view->currentIndex();
	return idx.sibling(idx.row(), column);
}

QString GetCurItemViewData(QAbstractItemView *view, int column)
{
	auto idx = view->currentIndex();
	return idx.sibling(idx.row(), column).data().toString();
}

void SetCurItemViewData(QAbstractItemView *view, int column, QString val)
{
	auto idx = view->currentIndex();
	view->model()->setData(idx.sibling(idx.row(), column), val);
}

void ExpandTreeView(const QModelIndex& index, QTreeView* view)
{
	if (!index.isValid()) {
		return;
	}
	int childCount = index.model()->rowCount(index);
	for (int i = 0; i < childCount; i++) {
		const QModelIndex &child = index.child(i, 0);
		ExpandTreeView(child, view);
	}
	if (!view->isExpanded(index)) {
		view->expand(index);
	}
}

void ClearItemModelData(QStandardItemModel* model, int pos)
{
	model->removeRows(pos, model->rowCount());
}

QString MsToTime(LONGLONG ms)
{
	return WStrToQ(UNONE::TmFormatMsW(ms, L"Y-M-D H:W:S", NULL));
}

void SetDefaultTableViewStyle(QTableView* view, QStandardItemModel* model)
{
	view->setModel(model);
	view->setSortingEnabled(true);
	view->verticalHeader()->hide();
	view->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
	view->horizontalHeader()->setStretchLastSection(true);
	view->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	view->horizontalHeader()->setMinimumSectionSize(100);
	view->verticalHeader()->setDefaultSectionSize(25);
	view->selectionModel()->selectedIndexes();
}

void SetDefaultTreeViewStyle(QTreeView* view, QStandardItemModel* model)
{
	view->setModel(model);
	view->header()->setDefaultAlignment(Qt::AlignLeft);
	//view->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
	view->header()->setMinimumSectionSize(100);
	view->setSortingEnabled(true);
}

void SetLineBgColor(QStandardItemModel *model, int row, const QBrush &abrush)
{
	for (int i = 0; i < model->columnCount(); i++) {
		model->item(row, i)->setBackground(abrush);
	}
}

void SetLineHidden(QTreeView *view, int row, bool hide)
{
	view->setRowHidden(row, view->rootIndex(), hide);
}

bool JsonParse(const QByteArray &data, QJsonObject &obj)
{
	QJsonParseError err;
	QJsonDocument doc;
	doc = QJsonDocument::fromJson(data, &err);
	if (err.error != QJsonParseError::NoError) {
		return false;
	}
	if (!doc.isObject()) {
		return false;
	}
	obj = doc.object();
	return true;
}

bool JsonGetValue(const QJsonObject &obj, const QString &key, QJsonValue &val)
{
	if (!obj.contains(key)) {
		return false;
	}
	val = obj[key];
	return true;
}

bool JsonGetValue(const QByteArray &data, const QString &key, QJsonValue &val)
{
	QJsonObject obj;
	if (!JsonParse(data, obj)) {
		return false;
	}
	if (!JsonGetValue(obj, key, val)) {
		return false;
	}
	return JsonGetValue(obj, key, val);
}

void ShellOpenUrl(QString url)
{
	ShellExecuteW(NULL, L"open", url.toStdWString().c_str(), NULL, NULL, SW_SHOW);
}

void ShellRun(QString cmdline, QString param)
{
	ShellExecuteW(NULL, L"open", cmdline.toStdWString().c_str(), param.toStdWString().c_str(), NULL, SW_SHOW);
}

QString PidFormat(DWORD pid)
{
	if (pid == -1) return "N/A";
	return QString("%1").arg(pid);
}

QString NameFormat(QString name)
{
	return name.replace(" *32", "");
}