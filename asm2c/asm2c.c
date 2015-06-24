/* asm2c - konwertuje kod źródłowy języka asembler do kodu źródłowego C
 * Copyright (C) 2014-2015 Paweł Zacharek
 * 
 * -------------------------------------------------------------------
 * Niniejszy program jest wolnym oprogramowaniem; możesz go
 * rozprowadzać dalej i/lub modyfikować na warunkach Powszechnej
 * Licencji Publicznej GNU, wydanej przez Fundację Wolnego
 * Oprogramowania - według wersji 2-giej tej Licencji lub którejś
 * z późniejszych wersji.
 * 
 * Niniejszy program rozpowszechniany jest z nadzieją, iż będzie on
 * użyteczny - jednak BEZ JAKIEJKOLWIEK GWARANCJI, nawet domyślnej
 * gwarancji PRZYDATNOŚCI HANDLOWEJ albo PRZYDATNOŚCI DO OKREŚLONYCH
 * ZASTOSOWAŃ. W celu uzyskania bliższych informacji - Powszechna
 * Licencja Publiczna GNU.
 * 
 * Z pewnością wraz z niniejszym programem otrzymałeś też egzemplarz
 * Powszechnej Licencji Publicznej GNU (GNU General Public License);
 * jeśli nie - napisz do Free Software Foundation, Inc., 675 Mass Ave,
 * Cambridge, MA 02139, USA.
 * -------------------------------------------------------------------
 * 
 * data: 2015-03-05
 * kompilacja: gcc -o asm2c.elf asm2c.c
 */

#include <errno.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[])
{
	if (argc > 2) {
		fputs("Podano za dużo argumentów.\n", stderr);
		return EINVAL;
	}
	if (argc == 2 && (strncmp(argv[1], "-h", 2) == 0 || strncmp(argv[1], "--help", 6) == 0)) {
		puts("Program konwertuje kod źródłowy języka asembler do kodu źródłowego C.\n"
		"Składnia: asm2c [plik wejściowy]");
		return 0;
	}
	FILE *dokument = (argc == 2 ? fopen(argv[1], "r") : stdin);
	if (dokument == '\0') {
		fprintf(stderr, "Nie udało się otworzyć pliku \"%.255s\".\n", argv[1]);
		return ENOENT;
	}
	char znaki[2];
	fputs("asm(\"", stdout);
	for (znaki[0] = '\0'; fscanf(dokument, "%c", &znaki[1]) == 1; znaki[0] = znaki[1]) {  // pobiera po kolei każdy znak z wejścia
		if (znaki[0] == '\n')  // po znaku '\n' znajduje się inny znak
			fputs("\\n\"\n\"", stdout);  // kończy starą i rozpoczyna nową linię
		if (znaki[0] == '\\') {  // aktualny znak jest znakiem specjalnym
			printf("%c%c", znaki[0], znaki[1]);
			znaki[1] = '\0';
		} else if (znaki[1] == '"')
			fputs("\\\"", stdout);
		else if (znaki[1] != '\n' && znaki[1] != '\\')
			putchar(znaki[1]);
	}
	fclose(dokument);
	if (znaki[0] == '\\')  // ostatni znak wejścia to '\\'
		putchar('\\');
	fputs("\");", stdout);
	if (znaki[0] == '\n')  // ostatni znak wejścia to '\n'
		putchar('\n');
	return 0;
}
