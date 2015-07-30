/* kojarzenia_druzyn - wypisuje kojarzenia drużyn koszykówki
 * Copyright (C) 2012-2015 Paweł Zacharek
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
 * data: 2015-07-30
 * kompilacja: gcc -std=gnu11 -o kojarzenia_druzyn.elf kojarzenia_druzyn.c
 * 
 * TODO: zaimplementować obsługę świąt ruchomych:
 * - Wielkanoc
 * - Poniedziałek Wielkanocny
 * - Zielone Świątki
 * - Boże Ciało
 */

#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define WERSJA "0.6.7"

#define MAX_NAZWA 30  // największa możliwa liczba znaków w nazwie drużyny

#define STR(x) STR_2(x)
#define STR_2(x) # x

struct data {
	short rok;
	signed char miesiac, dzien;
};

signed char dzien_tygodnia_kalendarz_gregorianski(short rok, signed char miesiac, signed char dzien)
{
	short rok_licz;
	signed char miesiac_licz, dzien_licz, dzien_tygodnia;
	if (rok == 1582) {
		rok_licz = 1582;
		if (miesiac == 10) {
			miesiac_licz = 10;
			dzien_licz = 15;  // 1582-10-15
			dzien_tygodnia = 1;  // poniedziałek
		} else {
			miesiac_licz = 11;
			dzien_licz = 1;  // 1582-11-01
			dzien_tygodnia = 1;  // poniedziałek
		}
	} else if (rok < 2000) {
		rok_licz = 1583;
		miesiac_licz = 1;
		dzien_licz = 1;  // 1583-01-01
		dzien_tygodnia = 6;  // sobota
	} else {
		rok_licz = 2000;
		miesiac_licz = 1;
		dzien_licz = 1;  // 2000-01-01
		dzien_tygodnia = 6;  // sobota
	}
	for (; rok_licz < rok; ++rok_licz) {  // ustawianie roku
		if (rok_licz % 4 != 0 || (rok_licz % 100 == 0 && (rok_licz / 100) % 4 != 0)) {
			if (dzien_tygodnia == 7)  // niedziela
				dzien_tygodnia -= 6;  // poniedziałek
			else
				++dzien_tygodnia;
		} else {
			if (dzien_tygodnia > 5)  // niedziela lub sobota
				dzien_tygodnia -= 5;  // wtorek lub poniedziałek
			else
				dzien_tygodnia += 2;
		}
	}
	for (; miesiac_licz < miesiac; ++miesiac_licz) {  // ustawianie miesiąca
		if (miesiac_licz == 2) {
			if ((rok_licz % 4 == 0 && rok_licz % 100 != 0) || (rok_licz % 100 == 0 && (rok_licz / 100) % 4 == 0)) {
				if (dzien_tygodnia == 7)  // niedziela
					dzien_tygodnia -= 6;  // poniedziałek
				else
					++dzien_tygodnia;
			}
		} else if (miesiac_licz == 4 || miesiac_licz == 6 || miesiac_licz == 9 || miesiac_licz == 11) {
			if (dzien_tygodnia > 5)  // niedziela lub sobota
				dzien_tygodnia -= 5;  // wtorek lub poniedziałek
			else
				dzien_tygodnia += 2;
		} else {
			if (dzien_tygodnia > 4)  // niedziela, sobota lub piątek
				dzien_tygodnia -= 4;  // środa, wtorek lub poniedziałek
			else
				dzien_tygodnia += 3;
		}
	}
	for (; dzien_licz < dzien; ++dzien_licz)  // ustawianie dnia
		dzien_tygodnia = dzien_tygodnia % 7 + 1;
	return dzien_tygodnia;
}

bool wczytaj_bledne_dane(FILE *wejscie)
{
	fputs("Napotkano błędne dane. Ponawianie odczytu.\n", stderr);
	if (fscanf(wejscie, "%*s") == EOF) {
		fputs("Napotkano znak końca pliku. Kończenie.\n", stderr);
		return 0;
	}
	return 1;
}

void zwieksz_date(struct data *data, signed char *dzien_tygodnia, short *dzien, short n)
{
	for (short i = 0; i < n; ++i) {
		if (data->dzien == 31) {  // dzień 31
			if (data->miesiac == 12) {  // 31 grudnia
				++data->rok;
				data->miesiac = 1;
				data->dzien = 1;
			} else {
				++data->miesiac;
				data->dzien = 1;
			}
		} else if (data->dzien == 30 && (data->miesiac == 4 || data->miesiac == 6 || data->miesiac == 9 || data->miesiac == 11)) {  // dzień 30
			++data->miesiac;
			data->dzien = 1;
		} else if (data->miesiac == 2 && data->dzien == 29) {  // 29 lutego
			++data->miesiac;
			data->dzien = 1;
		} else if (data->miesiac == 2 && data->dzien == 28 && (data->rok % 4 != 0 || (data->rok % 100 == 0 && data->rok % 400 != 0))) {  // 28 lutego roku nieprzestępnego
			++data->miesiac;
			data->dzien = 1;
		} else
			++data->dzien;
	}
	*dzien_tygodnia = ((*dzien_tygodnia - 1) + n) % 7 + 1;
	*dzien += n;
	return;
}

int main(int argc, char *argv[])
{
	if (argc < 2) {
		puts("kojarzenia_druzyn - wypisuje kojarzenia drużyn koszykówki\n"
		"Składnia: kojarzenia_druzyn [-d PLIK] [-i PLIK] [-o PLIK] [-v]\n"
		"Argumenty:\n"
		"  -d    określa plik, do którego mają zostać wypisane dane podawane na wejście\n"
		"  -i    określa plik z danymi wejściowymi\n"
		"  -o    określa plik, do którego ma zostać wypisane wyjście programu\n"
		"  -v    wyświetla informacje o wersji");
		fputs("Czy kontynuować działanie programu bez parametrów? [T/n] ", stdout);
		if (toupper(getchar()) != 'T')
			return 0;
	}
	struct struktura_parametry {
		char *czytaj_z_pliku, *pisz_do_pliku, *wypisz_dane_wejsciowe;
	} parametry = {NULL, NULL, NULL};
	for (int opcja; (opcja = getopt(argc, argv, "d:i:o:v")) != -1;)
		switch (opcja) {
			case 'd':
				parametry.wypisz_dane_wejsciowe = optarg;
				break;
			case 'i':
				parametry.czytaj_z_pliku = optarg;
				break;
			case 'o':
				parametry.pisz_do_pliku = optarg;
				break;
			case 'v':
				puts("kojarzenia_druzyn " WERSJA "\n"
				"Copyright (C) 2012-2015 Paweł Zacharek");
				return 0;
			default:
				return EINVAL;
		}
	FILE *wejscie = (parametry.czytaj_z_pliku == NULL ? stdin : fopen(parametry.czytaj_z_pliku, "r"));
	if (wejscie == NULL) {
		fprintf(stderr, "Nie udało się otworzyć pliku \"%.255s\".\n"
		"Czytanie ze standardowego wejścia.\n", parametry.czytaj_z_pliku);
		wejscie = stdin;
	}
	FILE *wyjscie = (parametry.pisz_do_pliku == NULL ? stdout : fopen(parametry.pisz_do_pliku, "w"));
	if (wyjscie == NULL) {
		fprintf(stderr, "Nie udało się otworzyć pliku \"%.255s\".\n"
		"Pisanie na standardowe wyjście.\n", parametry.pisz_do_pliku);
		wyjscie = stdout;
	}
	FILE *wyjscie_danych = (parametry.wypisz_dane_wejsciowe == NULL ? NULL : fopen(parametry.wypisz_dane_wejsciowe, "w"));
	if (wyjscie_danych == NULL && parametry.wypisz_dane_wejsciowe != NULL)
		fprintf(stderr, "Nie udało się otworzyć pliku \"%.255s\".\n"
		"Anulowano wypisywanie danych wejściowych.\n", parametry.wypisz_dane_wejsciowe);
	struct data data_poczatkowa, *wylaczenia = NULL;
	struct druzyna {
		char nazwa[MAX_NAZWA + 1];
		signed char dzien_tygodnia;
		char godzina[6];
	};
	int liczba_druzyn, liczba_wylaczen;
	signed char max_meczy_tygodniowo, prawdopodobienstwo;
	bool nie_podano_zadnych_sal = 1;
	if (wejscie == stdin)
		fputs("Podaj liczbę drużyn (MIN 2): ", stdout);
	while (fscanf(wejscie, "%d", &liczba_druzyn) != 1)
		if (wczytaj_bledne_dane(wejscie) == 0)  // pobiera z bufora błędne dane
			return 0;
	struct druzyna *druzyny = (struct druzyna *)malloc(liczba_druzyn * sizeof(struct druzyna));
	if (wejscie == stdin)
		fputs("Podaj optymalną liczbę meczy dla każdej drużyny w tygodniu (MAX 5): ", stdout);
	while (fscanf(wejscie, "%1hhd", &max_meczy_tygodniowo) != 1)
		if (wczytaj_bledne_dane(wejscie) == 0)
			return 0;
	if (wejscie == stdin)
		fputs("Podaj prawdopodobieństwo (w procentach), że drużyna może rozegrać\n"
		"o jeden mecz więcej w tygodniu: ", stdout);
	while (fscanf(wejscie, "%2hhd", &prawdopodobienstwo) != 1)
		if (wczytaj_bledne_dane(wejscie) == 0)
			return 0;
	if (wejscie == stdin)
		fputs("Podaj początkową datę (rok, miesiąc, dzień): ", stdout);
	while (fscanf(wejscie, "%4hd %2hhd %2hhd", &data_poczatkowa.rok, &data_poczatkowa.miesiac, &data_poczatkowa.dzien) != 3)
		if (wczytaj_bledne_dane(wejscie) == 0)
			return 0;
	if (wejscie == stdin)
		printf("Podaj nazwy %d drużyn:\n", liczba_druzyn);
	for (int i = 0; i < liczba_druzyn; ++i) {
		if (wejscie == stdin)
			printf("%d> ", i + 1);
		while (fscanf(wejscie, " %" STR(MAX_NAZWA) "[^\n]", druzyny[i].nazwa) != 1)
			if (wczytaj_bledne_dane(wejscie) == 0)
				return 0;
	}
	if (wejscie == stdin)
		puts("Podaj dzień tygodnia (w formie cyfry z przedziału 1-7) i godzinę,\n"
		"o której drużyny udostępniają salę (znak '-', jeśli brak):\n");
	for (int i = 0; i < liczba_druzyn; ++i) {
		if (wejscie == stdin)
			printf("%s> ", druzyny[i].nazwa);
		if (fscanf(wejscie, "%1hhd %5s", &druzyny[i].dzien_tygodnia, druzyny[i].godzina) != 2)
			druzyny[i].dzien_tygodnia = 0;
		else
			nie_podano_zadnych_sal = 0;
	}
	if (nie_podano_zadnych_sal == 1) {
		fputs("Nie ma gdzie rozgrywać meczy. Kończenie.\n", stderr);
		return 0;
	}
	if (wejscie == stdin)
		fputs("Podaj liczbę dodatkowych dni, w czasie których mecze nie będą rozgrywane: ", stdout);
	while (fscanf(wejscie, "%d", &liczba_wylaczen) != 1)
		if (wczytaj_bledne_dane(wejscie) == 0)
			return 0;
	if (liczba_wylaczen != 0) {
		wylaczenia = (struct data *)malloc(liczba_wylaczen * sizeof(struct data));
		if (wejscie == stdin)
			puts("Podaj te dni (rok, miesiąc, dzień):");
		for (int i = 0; i < liczba_wylaczen; ++i)
			while (fscanf(wejscie, "%4hd %2hhd %2hhd", &wylaczenia[i].rok, &wylaczenia[i].miesiac, &wylaczenia[i].dzien))
				if (wczytaj_bledne_dane(wejscie) == 0)
					return 0;
	}
	fclose(wejscie);
	if (wyjscie_danych != NULL) {  // wypisuje dane wejściowe do pliku
		fprintf(wyjscie_danych, "%d\n%hhd\n%hhd\n", liczba_druzyn, max_meczy_tygodniowo, prawdopodobienstwo);
		fprintf(wyjscie_danych, "%hd %hhd %hhd\n", data_poczatkowa.rok, data_poczatkowa.miesiac, data_poczatkowa.dzien);
		for (int i = 0; i < liczba_druzyn; ++i)
			fprintf(wyjscie_danych, "%s\n", druzyny[i].nazwa);
		for (int i = 0; i < liczba_druzyn; ++i) {
			if (druzyny[i].dzien_tygodnia == 0)
				fputs("-\n", wyjscie_danych);
			else
				fprintf(wyjscie_danych, "%hhd %s\n", druzyny[i].dzien_tygodnia, druzyny[i].godzina);
		}
		fprintf(wyjscie_danych, "%d\n", liczba_wylaczen);
		for (int i = 0; i < liczba_wylaczen; ++i)
			fprintf(wyjscie_danych, "%hd %hhd %hhd\n", wylaczenia[i].rok, wylaczenia[i].miesiac, wylaczenia[i].dzien);
		fclose(wyjscie_danych);
	}
	const char *Dni_Tygodnia[] = {"\x00", "Pn", "Wt", "Śr", "Cz", "Pt"};
	const int Liczba_Predefiniowanych_Wylaczen = 9;
	const struct miesiac_dzien {
		signed char miesiac, dzien;
	} Predefiniowane_Wylaczenia[] = {
		{1, 1},  // Nowy Rok
		{1, 6},  // Trzech Króli
		{5, 1},  // Święto Pracy
		{5, 3},  // Konstytucja 3 maja
		{8, 15},  // Święto Wojska Polskiego
		{11, 1},  // Wszystkich Świętych
		{11, 11},  // Święto Niepodległości
		{12, 25},  // Boże Narodzenie
		{12, 26}  // Boże Narodzenie
	};
	short *kojarzenia = (short *)malloc(liczba_druzyn * liczba_druzyn * sizeof(short));
	memset(kojarzenia, -1, sizeof(short) * liczba_druzyn * liczba_druzyn);  // bajt '-1' w kodzie U2 ma postać 0xFF
	bool *pierwszy_mecz = (bool *)calloc(liczba_druzyn * liczba_druzyn, sizeof(bool));
	bool *zajete_sale = (bool *)calloc(liczba_druzyn, sizeof(bool));
	bool *mecze_dzisiaj = (bool *)calloc(liczba_druzyn, sizeof(bool));
	signed char *mecze_w_tygodniu = (signed char *)calloc(liczba_druzyn, sizeof(signed char));
	struct data data = {data_poczatkowa.rok, data_poczatkowa.miesiac, data_poczatkowa.dzien};
	bool gramy_mecze_rewanzowe = 0;
	signed char dzien_tygodnia = dzien_tygodnia_kalendarz_gregorianski(data_poczatkowa.rok, data_poczatkowa.miesiac, data_poczatkowa.dzien);
	short dzien_rozgrywek = 0;
	int liczba_meczy = (liczba_druzyn - 1) * liczba_druzyn;
	int rozegrane_mecze = 0, liczba_losowan = 0, wiersz = 0, kolumna = 1;
	srand(time(NULL));
	fputs("-----KOJARZENIA DRUŻYN-----\n\n", wyjscie);
	while (rozegrane_mecze < liczba_meczy) {  // dopóki nie zapisano w tablicy dat wszystkich pierwszych spotkań
		if ((gramy_mecze_rewanzowe == 0 && kolumna == liczba_druzyn)
			|| (gramy_mecze_rewanzowe == 1 && wiersz == liczba_druzyn)
		) {  // wyszliśmy poza tablicę
			if ((gramy_mecze_rewanzowe == 0 && wiersz == 1)
				|| (gramy_mecze_rewanzowe == 1 && kolumna == 1)
			) {  // jeśli jest to ostatnia komórka połowy tablicy (w tym dniu nie rozegramy więcej meczy)
				if (gramy_mecze_rewanzowe == 0) {
					wiersz = 0;
					kolumna = 1;
				} else {
					kolumna = 0;
					wiersz = 1;
				}
				zwieksz_date(&data, &dzien_tygodnia, &dzien_rozgrywek, 1);  // zwiększa datę o 1 dzień
				memset(mecze_dzisiaj, 0, liczba_druzyn * sizeof(bool));  // wyzerowanie tablicy zawierającej dane o liczbie meczy rozegranych przez drużyny poprzedniego dnia (0 lub 1)
				memset(zajete_sale, 0, liczba_druzyn * sizeof(bool));  // wyzerowanie tablicy zawierającej informacje, czy w dniu wczorajszym sala była zajęta
				for (;;) {  // wykonuje się dopóki w danym dniu nie można zagrać żadnego meczu (czysto hipotetycznie)
					if (dzien_tygodnia == 6) {  // sobota
						zwieksz_date(&data, &dzien_tygodnia, &dzien_rozgrywek, 2);  // zwiększa datę o 2 dni (pomija weekend)
						memset(mecze_w_tygodniu, 0, liczba_druzyn * sizeof(signed char));  // wyzerowanie tablicy zawierającej dane o liczbie meczy rozegranych przez drużyny w ubiegłym tygodniu
						putc('\n', wyjscie);
						continue;
					}
					for (int i = 0; i < Liczba_Predefiniowanych_Wylaczen; ++i)
						if (Predefiniowane_Wylaczenia[i].miesiac == data.miesiac && Predefiniowane_Wylaczenia[i].dzien == data.dzien) {
							zwieksz_date(&data, &dzien_tygodnia, &dzien_rozgrywek, 1);  // zwiększa datę o 1 dzień (pomija święto)
							continue;
						}
					for (int i = 0; i < liczba_wylaczen; ++i)
						if (data.rok == wylaczenia[i].rok && data.miesiac == wylaczenia[i].miesiac && data.dzien == wylaczenia[i].dzien) {
							zwieksz_date(&data, &dzien_tygodnia, &dzien_rozgrywek, 1);  // zwiększa datę o 1 dzień (pomija święto)
							continue;
						}
					break;
				}
			} else {  // jeśli jest to ostatnia komórka diagonali
				if (gramy_mecze_rewanzowe == 0) {
					kolumna = (kolumna - wiersz) + 1;
					wiersz = 0;
				} else {
					wiersz = (wiersz - kolumna) + 1;
					kolumna = 0;
				}
			}
		}
		if (*(kojarzenia + wiersz * liczba_druzyn + kolumna) == -1 && mecze_dzisiaj[wiersz] == 0 && mecze_dzisiaj[kolumna] == 0 && (
			(mecze_w_tygodniu[wiersz] < max_meczy_tygodniowo && mecze_w_tygodniu[kolumna] < max_meczy_tygodniowo)
			|| (mecze_w_tygodniu[wiersz] <= max_meczy_tygodniowo && mecze_w_tygodniu[kolumna] < max_meczy_tygodniowo
				&& ++liczba_losowan && rand() % 100 + 1 <= prawdopodobienstwo)
			|| (mecze_w_tygodniu[wiersz] < max_meczy_tygodniowo && mecze_w_tygodniu[kolumna] <= max_meczy_tygodniowo
				&& ++liczba_losowan && rand() % 100 + 1 <= prawdopodobienstwo)
			|| (mecze_w_tygodniu[wiersz] <= max_meczy_tygodniowo && mecze_w_tygodniu[kolumna] <= max_meczy_tygodniowo
				&& ++liczba_losowan && (int)(rand() % 100 + 1) * (rand() % 100 + 1) <= (int)prawdopodobienstwo * 100)
		)) {  // jeszcze nie rozegrano tego meczu, a można by to zrobić w 'dzien_rozgrywek'
			bool warunki_sa_spelnione = 0;
			int gospodarz, gosc;
			char *godzina = NULL;
			if (druzyny[wiersz].dzien_tygodnia == 0 && druzyny[kolumna].dzien_tygodnia == 0) {  // drużyny nie mogą (nigdy) rozegrać meczu u siebie
				for (int i = 0; i < liczba_druzyn; ++i)
					if (druzyny[i].dzien_tygodnia == dzien_tygodnia && zajete_sale[i] == 0) {
						godzina = druzyny[i].godzina;
						zajete_sale[i] = 1;
						break;
					}
				if (godzina != NULL) {
					gospodarz = wiersz;
					gosc = kolumna;
					warunki_sa_spelnione = 1;
				}
			} else if (druzyny[wiersz].dzien_tygodnia != 0 && druzyny[kolumna].dzien_tygodnia != 0) {  // obie drużyny mogą być gospodarzami
				if (gramy_mecze_rewanzowe == 1) {
					if (*(pierwszy_mecz + wiersz * liczba_druzyn + kolumna) == 1 && druzyny[wiersz].dzien_tygodnia == dzien_tygodnia && zajete_sale[wiersz] == 0) {
						gospodarz = wiersz;
						gosc = kolumna;
						warunki_sa_spelnione = 1;
					} else if (*(pierwszy_mecz + wiersz * liczba_druzyn + kolumna) == 0 && druzyny[kolumna].dzien_tygodnia == dzien_tygodnia && zajete_sale[kolumna] == 0) {
						gospodarz = kolumna;
						gosc = wiersz;
						warunki_sa_spelnione = 1;
					}
				} else if (druzyny[wiersz].dzien_tygodnia == dzien_tygodnia && zajete_sale[wiersz] == 0
					&& druzyny[kolumna].dzien_tygodnia == dzien_tygodnia && zajete_sale[kolumna] == 0
				) {  // obie drużyny mogą zagrać mecz w swojej sali
					if (rand() % 2 == 1) {
						*(pierwszy_mecz + wiersz * liczba_druzyn + kolumna) = 1;
						*(pierwszy_mecz + kolumna * liczba_druzyn + wiersz) = 1;  // oznacza to, że następnym gospodarzem (w meczach rewanżowych) będzie przyszły 'wiersz' (teraz 'kolumna')
						gospodarz = wiersz;
						gosc = kolumna;
					} else {
						gospodarz = kolumna;
						gosc = wiersz;
					}
					++liczba_losowan;
					warunki_sa_spelnione = 1;
				} else if (druzyny[wiersz].dzien_tygodnia == dzien_tygodnia && zajete_sale[wiersz] == 0) {  // tylko drużyna 'wiersz' może zagrać mecz w swojej sali
					*(pierwszy_mecz + wiersz * liczba_druzyn + kolumna) = 1;
					*(pierwszy_mecz + kolumna * liczba_druzyn + wiersz) = 1;  // oznacza to, że następnym gospodarzem (w meczach rewanżowych) będzie przyszły 'wiersz' (teraz 'kolumna')
					gospodarz = wiersz;
					gosc = kolumna;
					warunki_sa_spelnione = 1;
				} else if (druzyny[kolumna].dzien_tygodnia == dzien_tygodnia && zajete_sale[kolumna] == 0) {  // tylko drużyna 'kolumna' może zagrać mecz w swojej sali
					gospodarz = kolumna;
					gosc = wiersz;
					warunki_sa_spelnione = 1;
				}
			} else if ((druzyny[wiersz].dzien_tygodnia == dzien_tygodnia && zajete_sale[wiersz] == 0)
				|| (druzyny[kolumna].dzien_tygodnia == dzien_tygodnia && zajete_sale[kolumna] == 0)
			) {  // tylko jedna drużyna może być gospodarzem, i to akurat w 'dzien_rozgrywek'
				if (druzyny[wiersz].dzien_tygodnia == 0) {
					gospodarz = kolumna;
					gosc = wiersz;
				} else {
					gospodarz = wiersz;
					gosc = kolumna;
				}
				warunki_sa_spelnione = 1;
			}
			if (warunki_sa_spelnione == 1) {
				*(kojarzenia + wiersz * liczba_druzyn + kolumna) = dzien_rozgrywek;
				++mecze_dzisiaj[wiersz];
				++mecze_dzisiaj[kolumna];
				++mecze_w_tygodniu[wiersz];
				++mecze_w_tygodniu[kolumna];
				fprintf(wyjscie, "%s\t-\t%s\t%hhd.%.2hhd.%hd\t%s\t%s\n", druzyny[gospodarz].nazwa, druzyny[gosc].nazwa, data.dzien, data.miesiac, data.rok, Dni_Tygodnia[dzien_tygodnia], godzina == NULL ? druzyny[gospodarz].godzina : godzina);
				++rozegrane_mecze;  // uzupełniono jedną komórkę tablicy
				if (rozegrane_mecze == liczba_meczy / 2) {
					fputs("\n-----MECZE REWANŻOWE-----\n\n", wyjscie);
					wiersz = 1;
					kolumna = 0;
					gramy_mecze_rewanzowe = 1;
					continue;
				}
			}
		}
		++wiersz;
		++kolumna;
	}
	free(druzyny);
	if (liczba_wylaczen != 0)
		free(wylaczenia);
	free(pierwszy_mecz);
	free(zajete_sale);
	free(mecze_dzisiaj);
	free(mecze_w_tygodniu);
	fputs("\n\n\n-----TABLICA KOJARZEŃ-----\n\n", wyjscie);
	for (int i = 0; i < liczba_druzyn; ++i) {  // wypisanie zawartości tablicy
		for (int j = 0; j < liczba_druzyn; ++j)
			fprintf(wyjscie, "%hd\t", *(kojarzenia + liczba_druzyn * j + i));
		putc('\n', wyjscie);
	}
	free(kojarzenia);
	fclose(wyjscie);
	printf("Raport został pomyślnie wygenerowany.\n"
	"Wykonano %d losowań.\n", liczba_losowan);
	return 0;
}
