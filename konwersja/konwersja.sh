#!/bin/bash
# konwersja - zmienia kodowanie i/lub koniec linii wejścia
# Copyright (C) 2015 Paweł Zacharek
# 
# -------------------------------------------------------------------
# Niniejszy program jest wolnym oprogramowaniem; możesz go
# rozprowadzać dalej i/lub modyfikować na warunkach Powszechnej
# Licencji Publicznej GNU, wydanej przez Fundację Wolnego
# Oprogramowania - według wersji 2-giej tej Licencji lub którejś
# z późniejszych wersji.
# 
# Niniejszy program rozpowszechniany jest z nadzieją, iż będzie on
# użyteczny - jednak BEZ JAKIEJKOLWIEK GWARANCJI, nawet domyślnej
# gwarancji PRZYDATNOŚCI HANDLOWEJ albo PRZYDATNOŚCI DO OKREŚLONYCH
# ZASTOSOWAŃ. W celu uzyskania bliższych informacji - Powszechna
# Licencja Publiczna GNU.
# 
# Z pewnością wraz z niniejszym programem otrzymałeś też egzemplarz
# Powszechnej Licencji Publicznej GNU (GNU General Public License);
# jeśli nie - napisz do Free Software Foundation, Inc., 675 Mass Ave,
# Cambridge, MA 02139, USA.
# -------------------------------------------------------------------
# 
# data: 2015-06-24

HELP='Program zmienia kodowanie i/lub koniec linii wejścia.
Składnia: konwersja -[IO][E] [plik wejściowy]
    albo: konwersja [-h|--help]

Argumenty:
  -h, --help    wypisanie tekstu pomocy na standardowe wyjście
                i zakończenie programu
  -[IO][E]      konwersja z kodowania I do O i/lub
                zamiana znaku końca wiersza na E

I/O przyjmuje wartości:
  c     dla kodowania CP852
  i     dla kodowania ISO 8859-2
  u     dla kodowania UTF-8
  w     dla kodowania Windows-1250

E przyjmuje wartości:
  u     konwertuje znaki końca linii z formatu DOS na format UNIX
        (CR+LF -> LF)
  d     konwertuje znaki końca linii z formatu UNIX na format DOS
        (LF -> CR+LF)'

if [ "$#" -eq 0 ]; then
	echo "$HELP"
	exit 0
fi

DOS2UNIX='dos2unix'
UNIX2DOS='unix2dos'

if [ -z "$(which dos2unix)" ]; then
	if [ -z "$(which fromdos)" ]; then
		DOS2UNIX="sed 's/\r//'"
		UNIX2DOS="sed 's/$/\r/'"
	else
		DOS2UNIX='fromdos'
		UNIX2DOS='todos'
	fi
fi

CONVERT='cat'
INPUT_ENCODING=""
OUTPUT_ENCODING=""
FILE=""
FURTHER_CHANGES='cat'

for I in "$@"; do
	case "$I" in
		-h|--help)
			echo "$HELP"
			exit 0 ;;
		-*)
			if [ "${#I}" -eq 2 ]; then
				case "${I:1:1}" in
					u)
						FURTHER_CHANGES="$DOS2UNIX" ;;
					d)
						FURTHER_CHANGES="$UNIX2DOS" ;;
					*)
						echo "Błędny argument \"$I\"."
						exit 1 ;;
				esac
			else
				CONVERT='iconv -c'
				case "${I:1:1}" in
					c)
						INPUT_ENCODING='-f cp852' ;;
					i)
						INPUT_ENCODING='-f ISO-8859-2' ;;
					u)
						INPUT_ENCODING='-f UTF8' ;;
					w)
						INPUT_ENCODING='-f WINDOWS-1250' ;;
					*)
						echo "Błędny argument \"$I\"."
						exit 1 ;;
				esac
				case "${I:2:1}" in
					c)
						OUTPUT_ENCODING='-t cp852' ;;
					i)
						OUTPUT_ENCODING='-t ISO-8859-2' ;;
					u)
						OUTPUT_ENCODING='-t UTF8' ;;
					w)
						OUTPUT_ENCODING='-t WINDOWS-1250' ;;
					*)
						echo "Błędny argument \"$I\"."
						exit 1 ;;
				esac
				if [ -n "${I:3:1}" ]; then
					case "${I:3:1}" in
						u)
							FURTHER_CHANGES="$DOS2UNIX" ;;
						d)
							FURTHER_CHANGES="$UNIX2DOS" ;;
						*)
							echo "Błędny argument \"$I\"."
							exit 1 ;;
					esac
				fi
			fi ;;
		*)
			FILE="$I" ;;
	esac
done

eval "$CONVERT $INPUT_ENCODING $OUTPUT_ENCODING \"$FILE\" | $FURTHER_CHANGES"
exit 0
