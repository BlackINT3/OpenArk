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
#include <unone.h>
#include "disassembly.h"
#include "udis86/udis86.h"

std::string DisasmMemory(ULONG64 pc, char *mem, ULONG memsize, int bits /*= 64*/)
{
	if (mem == nullptr) return "??";

	std::string disasm;
	ud_t ud_obj;

	/* initialize */
	ud_init(&ud_obj);
	ud_set_mode(&ud_obj, bits);
	ud_set_syntax(&ud_obj, UD_SYN_INTEL);

	unsigned char o_do_off = 1;
	unsigned char o_do_hex = 1;
	ud_set_pc(&ud_obj, pc);
	ud_set_input_buffer(&ud_obj, (const uint8_t *)mem, memsize);

	/* disassembly loop */
	while (ud_disassemble(&ud_obj)) {
		if (o_do_off)
			disasm += UNONE::StrFormatA("%016llX ", ud_insn_off(&ud_obj));
		if (o_do_hex) {
			const char* hex1, *hex2;
			hex1 = ud_insn_hex(&ud_obj);
			hex2 = hex1 + 16;
			disasm += UNONE::StrFormatA("%-16.16s %-24s\n", hex1, ud_insn_asm(&ud_obj));
/*
			if (strlen(hex1) > 16) {
				disasm += UNONE::StrFormatA("\n");
				if (o_do_off)
					disasm += UNONE::StrFormatA("%15s -", "");
				disasm += UNONE::StrFormatA("%-16s", hex2);
			}*/
		}
		else disasm += UNONE::StrFormatA(" %-24s", ud_insn_asm(&ud_obj));
	}
	return disasm;
}

std::string HexDumpMemory(ULONG64 pc, char *mem, ULONG memsize)
{
	auto hexchars = [&](std::string data)->std::string{
		std::string prints;
		std::string stream;
		if (mem != nullptr) {
			stream = UNONE::StrHexStrToStreamA(data);;
		} else {
			stream.resize(data.size()/2);
		}
		for (unsigned char ch : stream) {
			if (mem == nullptr) {
				prints.push_back('?');
				continue;
			}
			if (!isprint(ch)) prints.push_back('.');
			else prints.push_back(ch);
		}
		return prints;
	};
	std::string str;
	if (mem == nullptr) {
		str.assign(memsize*2, '?');
	} else {
		str = UNONE::StrStreamToHexStrA(std::string(mem, memsize));
	}
	std::string formated;
	auto &&newbuf = UNONE::StrInsertA(str, 32, "\n");
	std::vector<std::string> lines;
	UNONE::StrSplitLinesA(newbuf, lines);
	for (auto line : lines) {
		std::string data;
		data = UNONE::StrInsertA(line, 2, " ");
		if (line.size() < 32) {
			auto padsize = (32 - (line.size() % 32));
			data += " " + UNONE::StrInsertA(std::string(padsize, ' '), 2, " ");
		}
		formated += UNONE::StrFormatA("%016llX  %s  %s\n", pc, data.c_str(), hexchars(line).c_str());
		pc += line.size() / 2;
	}
	return formated;
}