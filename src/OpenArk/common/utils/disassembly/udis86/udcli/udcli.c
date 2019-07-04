/* udis86 - udcli/udcli.c
 *
 * Copyright (c) 2002-2013 Vivek Thampi
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 * 
 *     * Redistributions of source code must retain the above copyright notice, 
 *       this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright notice, 
 *       this list of conditions and the following disclaimer in the documentation 
 *       and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR 
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#ifdef _MSC_VER
#include "..\udis86.h"
#define PACKAGE_STRING "udis86 pre-1.8"
#else
#include <udis86.h>
#include <config.h>
#endif

#if defined(__APPLE__)
# define FMT64 "ll" 
#elif defined(__amd64__) || defined(__x86_64__)
# define FMT64 "l" 
# else
# define FMT64 "ll" 
#endif

#if defined(__DJGPP__) || defined(_WIN32)
# include <io.h>
# include <fcntl.h>
#endif 

#ifdef __DJGPP__
# include <unistd.h>  /* for isatty() */
# define _setmode setmode
# define _fileno fileno
# define _O_BINARY O_BINARY
#endif

/* help string */
static char help[] = 
{
  "Usage: %s [-option[s]] file\n"
  "Options:\n"
  "    -16      : Set the disassembly mode to 16 bits. \n"
  "    -32      : Set the disassembly mode to 32 bits. (default)\n"
  "    -64      : Set the disassembly mode to 64 bits.\n"
  "    -intel   : Set the output to INTEL (NASM like) syntax. (default)\n"
  "    -att     : Set the output to AT&T (GAS like) syntax.\n"
  "    -v <v>   : Set vendor. <v> = {intel, amd}.\n"
  "    -o <pc>  : Set the value of program counter to <pc>. (default = 0)\n"
  "    -s <n>   : Set the number of bytes to skip before disassembly to <n>.\n"
  "    -c <n>   : Set the number of bytes to disassemble to <n>.\n"
  "    -x       : Set the input mode to whitespace separated 8-bit numbers in\n"
  "               hexadecimal representation. Example: 0f 01 ae 00\n"
  "    -noff    : Do not display the offset of instructions.\n"
  "    -nohex   : Do not display the hexadecimal code of instructions.\n"
  "    -h       : Display this help message.\n"
  "    --version: Show version.\n"
  "\n"
  "Udcli is a front-end to the Udis86 Disassembler Library.\n" 
  "http://udis86.sourceforge.net/\n"
};

FILE* fptr = NULL;
uint64_t o_skip = 0;
uint64_t o_count = 0;
unsigned char o_do_count= 0;
unsigned char o_do_off = 1;
unsigned char o_do_hex = 1;
unsigned char o_do_x = 0;
unsigned o_vendor = UD_VENDOR_AMD;

int input_hook_x(ud_t* u);
int input_hook_file(ud_t* u);

int main(int argc, char **argv)
{
  char *prog_path = *argv;
  char *s;
  ud_t ud_obj;

  /* initialize */
  ud_init(&ud_obj);
  ud_set_mode(&ud_obj, 32);
  ud_set_syntax(&ud_obj, UD_SYN_INTEL);

#ifdef __DJGPP__
  if ( !isatty( fileno( stdin ) ) )
#endif
#if defined(__DJGPP) || defined(_WIN32)
  _setmode(_fileno(stdin), _O_BINARY);
#endif  

  fptr = stdin;

  argv++;

  /* loop through the args */
  while(--argc > 0) {
    if (strcmp(*argv, "-h") == 0) {
        printf(help, prog_path);
        exit(EXIT_SUCCESS);
    } else if (strcmp(*argv,"-16") == 0) {
		ud_set_mode(&ud_obj, 16);
	} else if (strcmp(*argv,"-32") == 0) {
		ud_set_mode(&ud_obj, 32);
	} else if (strcmp(*argv,"-64") == 0) {
		ud_set_mode(&ud_obj, 64);
	} else if (strcmp(*argv,"-intel") == 0)
		ud_set_syntax(&ud_obj, UD_SYN_INTEL);
	else if (strcmp(*argv,"-att") == 0)
		ud_set_syntax(&ud_obj, UD_SYN_ATT);
	else if (strcmp(*argv,"-noff") == 0)
		o_do_off = 0;
	else if (strcmp(*argv,"-nohex") == 0)
		o_do_hex = 0;
	else if (strcmp(*argv,"-x") == 0)
		o_do_x = 1;
	else if (strcmp(*argv,"-s") == 0)
		if (--argc) {
			s = *(++argv);
			if (sscanf(s, "%"  FMT64 "u", &o_skip) == 0)
				fprintf(stderr, "Invalid value given for -s.\n");
		} else { 
			fprintf(stderr, "No value given for -s.\n");
			printf(help, prog_path);
			exit(EXIT_FAILURE);
		}
	else if (strcmp(*argv,"-c") == 0)
		if (--argc) {
			o_do_count= 1;
			s = *(++argv);
			if (sscanf(s, "%" FMT64 "u", &o_count) == 0)
				fprintf(stderr, "Invalid value given for -c.\n");
		} else { 
			fprintf(stderr, "No value given for -c.\n");
			printf(help, prog_path);
			exit(EXIT_FAILURE);
		}
	else if (strcmp(*argv,"-v") == 0)
		if (--argc) {
			s = *(++argv);
			if (*s == 'i')
				ud_set_vendor(&ud_obj, UD_VENDOR_INTEL);
		} else { 
			fprintf(stderr, "No value given for -v.\n");
			printf(help, prog_path);
			exit(EXIT_FAILURE);
		}
	else if (strcmp(*argv,"-o") == 0) {
		if (--argc) {
			uint64_t pc = 0;
			s = *(++argv);
			if (sscanf(s, "%" FMT64 "x", &pc) == 0)
				fprintf(stderr, "Invalid value given for -o.\n");
			ud_set_pc(&ud_obj, pc);
		} else { 
			fprintf(stderr, "No value given for -o.\n");
			printf(help, prog_path);
			exit(EXIT_FAILURE);
		}
	} else if ( strcmp( *argv, "--version" ) == 0 ) {
		fprintf(stderr, "%s\n", PACKAGE_STRING );
		exit(0);
	} else if((*argv)[0] == '-') {
		fprintf(stderr, "Invalid option %s.\n", *argv);
		printf(help, prog_path);
		exit(EXIT_FAILURE);
	} else {
		static int i = 0;
		s = *argv;
		if (i) {
			fprintf(stderr, "Multiple files specified.\n");
			exit(EXIT_FAILURE);
		} else i = 1;
		if ((fptr = fopen(s, "rb")) == NULL) {
			fprintf(stderr, "Failed to open file: %s.\n", s);
				exit(EXIT_FAILURE);
		}
	}
	argv++;
  }

  if (o_do_x)
	ud_set_input_hook(&ud_obj, input_hook_x);
  else	ud_set_input_hook(&ud_obj, input_hook_file);	

  if (o_skip) {
	o_count += o_skip;
	ud_input_skip(&ud_obj, o_skip);
  }

  /* disassembly loop */
  while (ud_disassemble(&ud_obj)) {
	if (o_do_off)
		printf("%016" FMT64 "x ", ud_insn_off(&ud_obj));
	if (o_do_hex) {
		const char* hex1, *hex2;
		hex1 = ud_insn_hex(&ud_obj);
		hex2 = hex1 + 16;
		printf("%-16.16s %-24s", hex1, ud_insn_asm(&ud_obj));
		if (strlen(hex1) > 16) {
			printf("\n");
			if (o_do_off)
				printf("%15s -", "");
			printf("%-16s", hex2);
		}
	} 
	else printf(" %-24s", ud_insn_asm(&ud_obj));

	printf("\n");
  }
  
  exit(EXIT_SUCCESS);
  return 0;
}

int input_hook_x(ud_t* u)
{
  unsigned int c, i;

  if (o_do_count) {
	if (! o_count)
		return UD_EOI;
	else --o_count;
  }

  i = fscanf(fptr, "%x", &c);

  if (i == EOF)
	return UD_EOI;
  if (i == 0) {
	fprintf(stderr, "Error: Invalid input, should be in hexadecimal form (8-bit).\n");
	return UD_EOI;
  }
  if (c > 0xFF)
	fprintf(stderr, "Warning: Casting non-8-bit input (%x), to %x.\n", c, c & 0xFF);
  return (int) (c & 0xFF);
}	

int input_hook_file(ud_t* u)
{
  int c;

  if (o_do_count) {
	  if (! o_count) {
		return -1;
	  } else o_count -- ;
  }

  if ((c = fgetc(fptr)) == EOF)
	return UD_EOI;
  return c;
}
