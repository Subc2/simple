/* konwersja - konwertuje polskie znaki kodowane w CP852/ISO 8859-2/UTF-8/Windows-1250
 * Copyright (C) 2014 Paweł Zacharek
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
 * data: 2014-11-13
 * kompilacja: gcc -std=gnu11 -o konwersja.elf konwersja.c
 */

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define SPRAWDZAJ_KOMENTARZE

const int A_Poprawne = 0;  // do funkcji 'wczytaj_argumenty'
const int A_Blad = 1;
const int A_Pomoc = 2;

#define ILOSC_LITER 18  // do tablicy 'Mapa_Znakow'
const int Kodowanie_CP852 = 0;
const int Kodowanie_ISO88592 = 1;
const int Kodowanie_UTF8 = 2;
const int Kodowanie_Windows1250 = 3;

const int L_Brak = 0;  // do zamiany znaków końca linii
const int L_UNIX = 1;
const int L_Windows = 2;

const int K_Brak = 0;  // do wyszukiwania komentarzy
const int K_Cytat = 1;
const int K_Krotki = 2;
const int K_Dlugi = 3;

struct struktura_parametry {
	signed char wejscie, wyjscie;
	bool komentarze, kod_zrodlowy;
	bool czytaj_z_pliku;
	signed char koniec_linii;
	bool napraw;
} parametry = {-1, -1, 1, 1, 0, 0, 0};

signed char wczytaj_argumenty(int ilosc, const char *Argumenty[])
{
	if (ilosc < 2)
		return A_Pomoc;
	for (int i = 1; i < ilosc; ++i) {  // wczytuje parametry uruchomieniowe
		if (strncmp(&Argumenty[i][0], "-h", 2) == 0 || strncmp(&Argumenty[i][0], "--help", 6) == 0)
			return A_Pomoc;
		else if (Argumenty[i][0] == '-') {
			for (int j = 1; Argumenty[i][j] != '\0'; ++j)
				switch (Argumenty[i][j]) {
					case 'i':
					case 'o':
						if (strncmp(&Argumenty[i][j], "i=c", 3) == 0)
							parametry.wejscie = Kodowanie_CP852;
						else if (strncmp(&Argumenty[i][j], "i=i", 3) == 0)
							parametry.wejscie = Kodowanie_ISO88592;
						else if (strncmp(&Argumenty[i][j], "i=u", 3) == 0)
							parametry.wejscie = Kodowanie_UTF8;
						else if (strncmp(&Argumenty[i][j], "i=w", 3) == 0)
							parametry.wejscie = Kodowanie_Windows1250;
						else if (strncmp(&Argumenty[i][j], "o=c", 3) == 0)
							parametry.wyjscie = Kodowanie_CP852;
						else if (strncmp(&Argumenty[i][j], "o=i", 3) == 0)
							parametry.wyjscie = Kodowanie_ISO88592;
						else if (strncmp(&Argumenty[i][j], "o=u", 3) == 0)
							parametry.wyjscie = Kodowanie_UTF8;
						else if (strncmp(&Argumenty[i][j], "o=w", 3) == 0)
							parametry.wyjscie = Kodowanie_Windows1250;
						else
							return A_Blad;
						j += 2;
						break;
					case 'k':
						parametry.komentarze = 1;
						parametry.kod_zrodlowy = 0;
						break;
					case 'z':
						parametry.komentarze = 0;
						parametry.kod_zrodlowy = 1;
						break;
					case 'u':
						parametry.koniec_linii = L_UNIX;
						break;
					case 'w':
						parametry.koniec_linii = L_Windows;
						break;
					case 'n':
						parametry.napraw = 1;
						break;
					default:
						return A_Blad;
				}
		} else if (i == ilosc - 1)  // jeśli jest to ostatni parametr (nazwa pliku)
			parametry.czytaj_z_pliku = 1;
		else
			return A_Blad;
	}
	return A_Poprawne;
}

int main(int argc, char *argv[])
{
	signed char wartosc = wczytaj_argumenty(argc, (const char **)argv);
	if (wartosc == A_Pomoc) {
		puts("Program konwertuje polskie znaki kodowane w CP852/ISO 8859-2/UTF-8/Windows-1250.\n"
		"Składnia: konwersja [argumenty] [plik wejściowy]\n"
		"\n"
		"Argumenty:\n"
		"  -h, --help    wypisanie tekstu pomocy na standardowe wyjście\n"
		"                i zakończenie programu\n"
		"  -i=KODOWANIE  kodowanie wejściowe (input)\n"
		"  -o=KODOWANIE  kodowanie wyjściowe (output)\n"
#ifdef SPRAWDZAJ_KOMENTARZE
		"  -k            tylko tekst w komentarzach\n"
		"                (od \"//\" do końca linii lub pomiędzy \"/*\" i \"*/\")\n"
		"  -z            tylko tekst kodu źródłowego (bez komentarzy)\n"
#endif
		"  -u            konwertuje znaki końca linii z formatu Windows na format UNIX\n"
		"                (CR+LF -> LF)\n"
		"  -w            konwertuje znaki końca linii z formatu UNIX na format Windows\n"
		"                (LF -> CR+LF)\n"
		"  -n            nie wypisuje ostatniego znaku z pliku, jeśli jest uszkodzony\n"
		"\n"
		"KODOWANIE przyjmuje wartości:\n"
		"  c     dla kodowania CP852\n"
		"  i     dla kodowania ISO 8859-2\n"
		"  u     dla kodowania UTF-8\n"
		"  w     dla kodowania Windows-1250\n"
		"\n"
		"Wywołanie równocześnie z parametrami "
#ifdef SPRAWDZAJ_KOMENTARZE
		"\'k\' i \'z\' lub "
#endif
		"\'u\' i \'w\' uwzględnia\n"
		"tylko ostatni z nich, podobnie jak kilkukrotne wywołanie z \"i=x\" lub \"o=x\".\n"
		"Parametry można łączyć, np. \"./konwersja.elf -i=uo=cw plik.c\"");
		return 0;
	} else if (wartosc == A_Blad) {
		fputs("Znaleziono niewłaściwe argumenty.\n", stderr);
		return EINVAL;
	}
	if (parametry.wejscie == -1 || parametry.wyjscie == -1) {
		fprintf(stderr, "Nie podano kodowania %s.\n", parametry.wejscie == -1 ? (parametry.wyjscie == -1 ? "wejściowego i wyjściowego" : "wejściowego") : "wyjściowego");
		return ENOEXEC;
	}
	FILE *dokument = (parametry.czytaj_z_pliku == 0 ? stdin : fopen(argv[argc - 1], "rb"));
	if (dokument == '\0') {
		fprintf(stderr, "Nie udało się otworzyć pliku \"%.255s\".\n", argv[argc - 1]);
		return ENOENT;
	}
	const char *Mapa_Znakow[4][ILOSC_LITER] = {
		{  // kodowanie CP852
			"\xA4",  // Ą
			"\xA5",  // ą
			"\x8F",  // Ć
			"\x86",  // ć
			"\xA8",  // Ę
			"\xA9",  // ę
			"\x9D",  // Ł
			"\x88",  // ł
			"\xE3",  // Ń
			"\xE4",  // ń
			"\xE0",  // Ó
			"\xA2",  // ó
			"\x97",  // Ś
			"\x98",  // ś
			"\x8D",  // Ź
			"\xAB",  // ź
			"\xBD",  // Ż
			"\xBE"  // ż
		}, {  // kodowanie ISO 8859-2
			"\xA1",  // Ą
			"\xB1",  // ą
			"\xC6",  // Ć
			"\xE6",  // ć
			"\xCA",  // Ę
			"\xEA",  // ę
			"\xA3",  // Ł
			"\xB3",  // ł
			"\xD1",  // Ń
			"\xF1",  // ń
			"\xD3",  // Ó
			"\xF3",  // ó
			"\xA6",  // Ś
			"\xB6",  // ś
			"\xAC",  // Ź
			"\xBC",  // ź
			"\xAF",  // Ż
			"\xBF"  // ż
		}, {  // kodowanie UTF-8
			"\xC4\x84",  // Ą
			"\xC4\x85",  // ą
			"\xC4\x86",  // Ć
			"\xC4\x87",  // ć
			"\xC4\x98",  // Ę
			"\xC4\x99",  // ę
			"\xC5\x81",  // Ł
			"\xC5\x82",  // ł
			"\xC5\x83",  // Ń
			"\xC5\x84",  // ń
			"\xC3\x93",  // Ó
			"\xC3\xB3",  // ó
			"\xC5\x9A",  // Ś
			"\xC5\x9B",  // ś
			"\xC5\xB9",  // Ź
			"\xC5\xBA",  // ź
			"\xC5\xBB",  // Ż
			"\xC5\xBC"  // ż
		}, {  // kodowanie Windows-1250
			"\xA5",  // Ą
			"\xB9",  // ą
			"\xC6",  // Ć
			"\xE6",  // ć
			"\xCA",  // Ę
			"\xEA",  // ę
			"\xA3",  // Ł
			"\xB3",  // ł
			"\xD1",  // Ń
			"\xF1",  // ń
			"\xD3",  // Ó
			"\xF3",  // ó
			"\x8C",  // Ś
			"\x9C",  // ś
			"\x8F",  // Ź
			"\x9F",  // ź
			"\xAF",  // Ż
			"\xBF"  // ż
		}
	};
	struct bufor_na_2_znaki {
		signed char przechowano;
		char znaki[2];
	} znak_zlozony, komentarz, koniec_wiersza;
	znak_zlozony.przechowano = komentarz.przechowano = koniec_wiersza.przechowano = 0;
	signed char jaki_komentarz = K_Brak;  // rodzaj komentarza, w którym aktualnie czytamy znaki
	char nowy_znak;  // przechowuje po kolei każdy znak z wejścia
	bool juz_wypisano_lub_przechowano_znak = 0;  // zmienna uniemożliwiająca kilkukrotne wypisanie tego samego znaku
	bool czy_konwertowac = parametry.kod_zrodlowy;  // włącza/wyłącza konwersję w zależności od obecności komentarzy (tylko z parametrami 'k' lub 'z')
	while (fscanf(dokument, "%c", &nowy_znak) == 1) {  // pobiera po kolei każdy znak z wejścia
#ifdef SPRAWDZAJ_KOMENTARZE
		if (parametry.koniec_linii != L_Brak && znak_zlozony.przechowano == 0 && komentarz.przechowano == 0) {  // wyszukuje znaki końca linii
#else
		if (parametry.koniec_linii != L_Brak && znak_zlozony.przechowano == 0) {  // wyszukuje znaki końca linii
#endif
			koniec_wiersza.znaki[koniec_wiersza.przechowano] = nowy_znak;
			if (parametry.koniec_linii == L_UNIX && koniec_wiersza.przechowano == 0 && koniec_wiersza.znaki[0]  == '\r') {  // nie mamy jeszcze drugiego bajtu
				++koniec_wiersza.przechowano;
				juz_wypisano_lub_przechowano_znak = 1;  // uniemożliwia ponowne wyszukiwanie znaku
			} else if (parametry.koniec_linii == L_Windows && koniec_wiersza.przechowano == 0 && koniec_wiersza.znaki[0]  == '\n') {
				fputs("\r\n", stdout);
				juz_wypisano_lub_przechowano_znak = 1;
#ifdef SPRAWDZAJ_KOMENTARZE
				if (jaki_komentarz == K_Krotki) {  // skończył się krótki komentarz
					jaki_komentarz = K_Brak;
					czy_konwertowac = (jaki_komentarz == K_Brak || jaki_komentarz == K_Cytat ? parametry.kod_zrodlowy : parametry.komentarze);
				}
#endif
			} else if (parametry.koniec_linii == L_UNIX && koniec_wiersza.przechowano == 1 && strncmp(koniec_wiersza.znaki, "\r\n", 2) == 0) {
				putchar('\n');
				juz_wypisano_lub_przechowano_znak = 1;
				koniec_wiersza.przechowano = 0;
#ifdef SPRAWDZAJ_KOMENTARZE
				if (jaki_komentarz == K_Krotki) {  // skończył się krótki komentarz
					jaki_komentarz = K_Brak;
					czy_konwertowac = (jaki_komentarz == K_Brak || jaki_komentarz == K_Cytat ? parametry.kod_zrodlowy : parametry.komentarze);
				}
#endif
			}
		}
#ifdef SPRAWDZAJ_KOMENTARZE
		if (juz_wypisano_lub_przechowano_znak == 0 && znak_zlozony.przechowano == 0 && (parametry.komentarze == 0 || parametry.kod_zrodlowy == 0)) {  // wyszukuje komentarze
			komentarz.znaki[komentarz.przechowano] = nowy_znak;
			if (komentarz.przechowano == 1 && (strncmp(komentarz.znaki, "**", 2) == 0 || strncmp(komentarz.znaki, "\\\\", 2) == 0)) {  // szczególne przypadki, gdy wczytano tekst:  '/* **/'  lub '" \\"'
				putchar(komentarz.znaki[0]);  // wypisuje pierwszy znak
				komentarz.znaki[0] = komentarz.znaki[1];  // traktuje drugi znak jako pierwszy znak
				komentarz.przechowano = 0;
			}
			if (komentarz.przechowano == 0) {
				if ((komentarz.znaki[0] == '/' && jaki_komentarz == K_Brak) || (komentarz.znaki[0] == '*' && jaki_komentarz == K_Dlugi) || (komentarz.znaki[0] == '\\' && jaki_komentarz == K_Cytat)) {  // nie wczytano całości początku/końca komentarza lub znaku specjalnego
					++komentarz.przechowano;
					juz_wypisano_lub_przechowano_znak = 1;  // uniemożliwia ponowne wyszukiwanie znaku w tablicy znakow
				} else if (komentarz.znaki[0] == '\n' && jaki_komentarz == K_Krotki) {  // skończył się krótki komentarz
					jaki_komentarz = K_Brak;
					putchar(komentarz.znaki[0]);
					juz_wypisano_lub_przechowano_znak = 1;
				} else if (komentarz.znaki[0] == '\"' && (jaki_komentarz == K_Brak || jaki_komentarz == K_Cytat)) {  // zaczął/skończył się cytat (w kodzie źródłowym)
					jaki_komentarz = (jaki_komentarz == K_Brak ? K_Cytat : K_Brak);
					putchar(komentarz.znaki[0]);
					juz_wypisano_lub_przechowano_znak = 1;
				}
			} else {  // komentarz.przechowano == 1
				if (strncmp(komentarz.znaki, "\\\"", 2) == 0) {  // znaleziono cudzysłów nie otwierający/zamykający cytatu
					printf("%.*s", 2, komentarz.znaki);
					juz_wypisano_lub_przechowano_znak = 1;
				} else if (strncmp(komentarz.znaki, "//", 2) == 0 && jaki_komentarz == K_Brak) {  // zaczął się krótki komentarz
					jaki_komentarz = K_Krotki;
					printf("%.*s", 2, komentarz.znaki);
					juz_wypisano_lub_przechowano_znak = 1;
				} else if (strncmp(komentarz.znaki, "/*", 2) == 0 && jaki_komentarz == K_Brak) {  // zaczął się długi komentarz
					jaki_komentarz = K_Dlugi;
					printf("%.*s", 2, komentarz.znaki);
					juz_wypisano_lub_przechowano_znak = 1;
				} else if (strncmp(komentarz.znaki, "*/", 2) == 0 && jaki_komentarz == K_Dlugi) {  // skończył się długi komentarz
					jaki_komentarz = K_Krotki;
					printf("%.*s", 2, komentarz.znaki);
					juz_wypisano_lub_przechowano_znak = 1;
				} else  // nie rozpoczął/zakończył się żaden komentarz
					putchar(komentarz.znaki[0]);  // tylko tyle, ponieważ następny znak może być początkiem znaku złożonego
				komentarz.przechowano = 0;
			}
			czy_konwertowac = (jaki_komentarz == K_Brak || jaki_komentarz == K_Cytat ? parametry.kod_zrodlowy : parametry.komentarze);
		}
		if (juz_wypisano_lub_przechowano_znak == 0 && czy_konwertowac == 1) {
#else
		if (juz_wypisano_lub_przechowano_znak == 0) {
#endif
			znak_zlozony.znaki[znak_zlozony.przechowano] = nowy_znak;
			for (signed char ktora_litera = 0; ktora_litera < ILOSC_LITER; ++ktora_litera) {  // wyszukuje znaków złożonych
				if (strncmp(znak_zlozony.znaki, Mapa_Znakow[parametry.wejscie][ktora_litera], znak_zlozony.przechowano + 1) == 0) {  // dotychczas wczytane bajty znaku zgadzają się z 'Mapa_Znakow'
					if (Mapa_Znakow[parametry.wejscie][ktora_litera][znak_zlozony.przechowano + 1] == '\0') {  // znaleziono znak złożony
						fputs(Mapa_Znakow[parametry.wyjscie][ktora_litera], stdout);  // wypisuje znak złożony
						znak_zlozony.przechowano = 0;
					} else  // nie wczytano całego znaku złożonego
						++znak_zlozony.przechowano;
					juz_wypisano_lub_przechowano_znak = 1;  // uniemożliwia wypisanie znaku już przechowanego/wypisanego
					break;
				}
			}
			if (juz_wypisano_lub_przechowano_znak == 0 && znak_zlozony.przechowano > 0) {  // wypisz znak z bufora (nie znaleziono znaku złożonego)
				putchar(znak_zlozony.znaki[0]);  // drugi znak zostanie wypisany po wyjściu z instrukcji warunkowej
				znak_zlozony.przechowano = 0;
			}
		}
		if (juz_wypisano_lub_przechowano_znak == 0)  // jeśli nigdzie nie przechowano/wypisano znaku 'nowy_znak'
			putchar(nowy_znak);
		else
			juz_wypisano_lub_przechowano_znak = 0;
	}
	fclose(dokument);
	if (koniec_wiersza.przechowano > 0)  // jeśli wczytano tylko pierwszy bajt końca linii Windows (CR)
		putchar(koniec_wiersza.znaki[0]);
	else if (komentarz.przechowano > 0)  // jeśli w buforze pozostał niewypisany znak
		putchar(komentarz.znaki[0]);
	else if (znak_zlozony.przechowano > 0 && parametry.napraw == 0)  // jeśli wczytano tylko pierwszy bajt znaku UTF-8
		putchar(znak_zlozony.znaki[0]);
	return 0;
}
