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
#include "about.h"
#include "../common/common.h"

About::About(QWidget* parent)
{
	ui.setupUi(this);
	connect(OpenArkLanguage::Instance(), &OpenArkLanguage::languageChaned, this, [this]() {ui.retranslateUi(this); });

	setAttribute(Qt::WA_ShowModal, true);
	setAttribute(Qt::WA_DeleteOnClose);
	setWindowFlags(windowFlags()& ~(Qt::WindowMaximizeButtonHint| Qt::WindowMinimizeButtonHint)| Qt::MSWindowsFixedSizeDialogHint);
	connect(ui.pushButton, SIGNAL(clicked()), this, SLOT(close()));
}

About::~About()
{
}