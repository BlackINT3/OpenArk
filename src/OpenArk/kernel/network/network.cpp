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
#include <WinSock2.h>

//https://social.msdn.microsoft.com/Forums/windowsdesktop/en-US/8fd93a3d-a794-4233-9ff7-09b89eed6b1f/compiling-with-wfp?forum=wfp
#include "include/fwpmu.h"
#include "network.h"
#pragma comment(lib, "fwpuclnt.lib")
#pragma comment(lib, "Rpcrt4.lib")


BOOLEAN GuidEqual(_In_ const GUID* pGUIDAlpha, _In_ const GUID* pGUIDOmega)
{
	RPC_STATUS status = RPC_S_OK;
	UINT32     areEqual = FALSE;

	do
	{
		if (pGUIDAlpha == 0 ||
			pGUIDOmega == 0)
		{
			if ((pGUIDAlpha == 0 &&
				pGUIDOmega) ||
				(pGUIDAlpha &&
					pGUIDOmega == 0))
				break;
		}

		if (pGUIDAlpha == 0 &&
			pGUIDOmega == 0)
		{
			areEqual = TRUE;
			break;
		}

		areEqual = UuidEqual((UUID*)pGUIDAlpha,
			(UUID*)pGUIDOmega,
			&status);

	} while (false);

	return (BOOLEAN)areEqual;
}

bool EnumWfpCallouts(std::vector<CALLOUT_INFO>& CalloutIDs)
{
	bool Result = false;
	HANDLE EngineHandle = NULL;
	UINT32       status = NO_ERROR;
	FWPM_SESSION session = { 0 };
	HANDLE EnumHandle = NULL;
	FWPM_CALLOUT_ENUM_TEMPLATE* pCalloutEnumTemplate = NULL;
	session.displayData.name = L"WFPSampler's User Mode Session";
	session.flags = 0;

	do
	{
		status = FwpmEngineOpen0(0,
			RPC_C_AUTHN_WINNT,
			0,
			&session,
			&EngineHandle);
		if (status != NO_ERROR) {
			break;
		}

		status = FwpmCalloutCreateEnumHandle(EngineHandle,
			pCalloutEnumTemplate,
			&EnumHandle);
		if (status != NO_ERROR) {
			break;
		}

		UINT32 NumEntries = 0;
		FWPM_CALLOUT** ppCallouts = 0;
		status = FwpmCalloutEnum0(EngineHandle,
			EnumHandle,
			0xFFFFFFFF,
			&ppCallouts,
			&NumEntries);
		if (status != NO_ERROR) {
			break;
		}

		if (ppCallouts)
		{
			for (DWORD Index = 0; Index < NumEntries; Index++)
			{
				CALLOUT_INFO CalloutInfo;
				CalloutInfo.CalloutId = ppCallouts[Index]->calloutId;
				RtlCopyMemory(&CalloutInfo.CalloutKey, &ppCallouts[Index]->calloutKey, sizeof(GUID));
				CalloutIDs.push_back(CalloutInfo);
			}
			Result = true;
		}

	} while (false);

	if (EnumHandle) {
		FwpmCalloutDestroyEnumHandle(EngineHandle, EnumHandle);
	}

	if (EngineHandle) {
		status = FwpmEngineClose(EngineHandle);
	}

	if (pCalloutEnumTemplate) {
		delete pCalloutEnumTemplate;
		pCalloutEnumTemplate = NULL;
	}

	return Result;
}

UINT64 GetFilterIDByCalloutKey(const GUID* CalloutKey)
{
	UINT64 Result = 0;
	HANDLE EngineHandle = NULL;
	LONG  Status = NO_ERROR;
	FWPM_SESSION Session = { 0 };
	HANDLE EnumHandle = 0;
	FWPM_FILTER_ENUM_TEMPLATE* pFilterEnumTemplate = NULL;
	Session.displayData.name = L"WFPSampler's User Mode Session";
	Session.flags = 0;

	do
	{
		Status = FwpmEngineOpen(0,
			RPC_C_AUTHN_WINNT,
			0,
			&Session,
			&EngineHandle);
		if (Status != NO_ERROR)
		{
			break;
		}

		UINT32                               NumEntries = 0;
		FWPM_FILTER**                        ppFilters = 0;
		Status = FwpmFilterCreateEnumHandle0(EngineHandle,
			pFilterEnumTemplate,
			&EnumHandle);
		if (Status != NO_ERROR)
		{
			break;
		}

		Status = FwpmFilterEnum0(EngineHandle,
			EnumHandle,
			0xFFFFFFFF,
			&ppFilters,
			&NumEntries);
		if (Status != NO_ERROR)
		{
			break;
		}

		if (Status == NO_ERROR && ppFilters && NumEntries)
		{
			for (UINT32 Index = 0; Index < NumEntries; Index++)
			{
				if (GuidEqual(&ppFilters[Index]->action.calloutKey,
					CalloutKey))
				{
					Result = ppFilters[Index]->filterId;
					break;
				}
			}
		}

	} while (false);

	if (EngineHandle) {
		Status = FwpmEngineClose(EngineHandle);
	}

	if (pFilterEnumTemplate) {
		delete pFilterEnumTemplate;
		pFilterEnumTemplate = NULL;
	}

	return Result;
}

bool DeleteFilterById(UINT64 FilterId)
{
	bool Result = false;
	HANDLE EngineHandle = NULL;
	LONG       Status = NO_ERROR;
	FWPM_SESSION Session = { 0 };
	Session.displayData.name = L"WFPSampler's User Mode Session";
	Session.flags = 0;

	do
	{
		Status = FwpmEngineOpen(0,
			RPC_C_AUTHN_WINNT,
			0,
			&Session,
			&EngineHandle);
		if (Status != NO_ERROR) {
			break;
		}

		Status = FwpmFilterDeleteById(EngineHandle, FilterId);
		if (Status != NO_ERROR) {
			break;
		}

		Result = true;
	} while (false);

	if (EngineHandle) {
		Status = FwpmEngineClose(EngineHandle);
	}

	return Result;
}

bool WfpSortFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const {
	auto s1 = sourceModel()->data(left); auto s2 = sourceModel()->data(right);
	return QString::compare(s1.toString(), s2.toString(), Qt::CaseInsensitive) < 0;
}

KernelNetwork::KernelNetwork()
{

}

KernelNetwork::~KernelNetwork()
{

}

void KernelNetwork::onTabChanged(int index)
{
	switch (index) {
	case 0:
		ShowWfpInfo();
		break;
	default:
		break;
	}
	CommonTabObject::onTabChanged(index);
}

bool KernelNetwork::EventFilter()
{
	return true;
}

void KernelNetwork::ModuleInit(Ui::Kernel *ui, Kernel *kernel)
{
	this->ui = ui;

	wfp_model_ = new QStandardItemModel;
	QTreeView *view = ui->wfpView;
	proxy_wfp_ = new WfpSortFilterProxyModel(view);
	proxy_wfp_->setSourceModel(wfp_model_);
	proxy_wfp_->setDynamicSortFilter(true);
	proxy_wfp_->setFilterKeyColumn(1);
	view->setModel(proxy_wfp_);
	view->selectionModel()->setModel(proxy_wfp_);
	view->header()->setSortIndicator(-1, Qt::AscendingOrder);
	view->setSortingEnabled(true);
	view->viewport()->installEventFilter(kernel);
	view->installEventFilter(kernel);
	std::pair<int, QString> colum_layout[] = {
		{ 130, tr("ID") },
		{ 100, tr("Key") },
		{ 200, tr("Name") },
	};
	QStringList name_list;
	for (auto p : colum_layout) {
		name_list << p.second;
	}
	wfp_model_->setHorizontalHeaderLabels(name_list);
	for (int i = 0; i < _countof(colum_layout); i++) {
		view->setColumnWidth(i, colum_layout[i].first);
	}
	view->setEditTriggers(QAbstractItemView::NoEditTriggers);
	wfp_menu_ = new QMenu();
	wfp_menu_->addAction(tr("Refresh"), kernel, [&] {  });

	Init(ui->tabNetwork, TAB_KERNEL, TAB_KERNEL_NETWORK);
}

void KernelNetwork::ShowWfpInfo()
{
	DISABLE_RECOVER();
	ClearItemModelData(wfp_model_, 0);

	std::vector<CALLOUT_INFO> infos;
	EnumWfpCallouts(infos);

	for (auto item : infos) {
		auto id_item = new QStandardItem(DWordToHexQ(item.CalloutId));
		auto key_item = new QStandardItem(DWordToHexQ(item.CalloutKey));
		auto name_item = new QStandardItem(item.ModuleName);
		auto count = wfp_model_->rowCount();
		wfp_model_->setItem(count, 0, id_item);
		wfp_model_->setItem(count, 1, key_item);
		wfp_model_->setItem(count, 2, name_item);
	}
}