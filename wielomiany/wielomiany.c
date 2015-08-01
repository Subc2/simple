/* wielomiany - oblicza wymierne pierwiastki wielomianu całkowitego
 * Copyright (C) 2015 Paweł Zacharek
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
 * data: 2015-08-01
 * kompilacja: gcc -std=gnu11 -o wielomiany.elf wielomiany.c
 */

#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_STOPIEN_WIELOMIANU 10

int *liczby_pierwsze;

struct ulamek_niewlasciwy {
	int licznik, mianownik;
};

struct wielomian_calkowity {
	signed char stopien;
	int wspolczynniki[MAX_STOPIEN_WIELOMIANU + 1];
};

int porownaj_ulamki(const void *wsk_1, const void *wsk_2)
{
	const struct ulamek_niewlasciwy *a = (const struct ulamek_niewlasciwy *)wsk_1;
	const struct ulamek_niewlasciwy *b = (const struct ulamek_niewlasciwy *)wsk_2;
	double x = (double)a->licznik / a->mianownik;
	double y = (double)b->licznik / b->mianownik;
	return x < y ? -1 : x > y;
}

void skroc_ulamek(struct ulamek_niewlasciwy *ulamek)
{
	if (ulamek->mianownik < 0) {
		ulamek->licznik *= -1;
		ulamek->mianownik *= -1;
	}
	if (abs(ulamek->licznik) == ulamek->mianownik) {  // skracanie przez liczby większe od tych z tablicy 'liczby_pierwsze'
		ulamek->licznik /= ulamek->mianownik;
		ulamek->mianownik /= ulamek->mianownik;
	} else  // skracanie przez pozostałe liczby
		for (int i = 0; liczby_pierwsze[i] <= abs(ulamek->licznik) && liczby_pierwsze[i] <= ulamek->mianownik && liczby_pierwsze[i] != 0; ++i)
			while (ulamek->licznik % liczby_pierwsze[i] == 0 && ulamek->mianownik % liczby_pierwsze[i] == 0) {
				ulamek->licznik /= liczby_pierwsze[i];
				ulamek->mianownik /= liczby_pierwsze[i];
			}
	return;
}

int reszta_z_dzielenia(const struct wielomian_calkowity *wielomian, struct ulamek_niewlasciwy zmienna)
{
	skroc_ulamek(&zmienna);
	struct ulamek_niewlasciwy reszta = {wielomian->wspolczynniki[wielomian->stopien], 1};
	for (signed char i = wielomian->stopien - 1; i >= 0; --i) {
		reszta.licznik *= zmienna.licznik;
		reszta.mianownik *= zmienna.mianownik;
		skroc_ulamek(&reszta);
		reszta.licznik += wielomian->wspolczynniki[i] * reszta.mianownik;
		skroc_ulamek(&reszta);
	}
	return reszta.licznik;
}

// USAGE: swap_pointers((void **)&pointer_1, (void **)&pointer_2);
static inline void swap_pointers(void **pointer_1, void **pointer_2)
{
	void *store = *pointer_1;
	*pointer_1 = *pointer_2;
	*pointer_2 = store;
	return;
}

int main(void)
{
	liczby_pierwsze = (int *)malloc((USHRT_MAX + 1) * sizeof(int));
	for (int i = 0; i <= USHRT_MAX; ++i)
		liczby_pierwsze[i] = i;
	liczby_pierwsze[0] = liczby_pierwsze[1] = 0;
	for (int i = 2; i <= USHRT_MAX; ++i)
		for (int j = 2 * i; j <= USHRT_MAX; j += i)
			liczby_pierwsze[j] = 0;
	for (int i = 0, j = 0; i <= USHRT_MAX; ++i)
		if (liczby_pierwsze[i] != 0)
			liczby_pierwsze[j++] = liczby_pierwsze[i];
	struct wielomian_calkowity wielomian_1, wielomian_2;
	struct wielomian_calkowity *wielomian = &wielomian_1, *nowy_wielomian = &wielomian_2;
	struct ulamek_niewlasciwy dzielniki[MAX_STOPIEN_WIELOMIANU], dzielnik;
	signed char liczba_dzielnikow;
	puts("Program oblicza wymierne pierwiastki wielomianu całkowitego.");
	for (;;) {
		printf("Podaj stopień wielomianu (od 0 do %d): ", MAX_STOPIEN_WIELOMIANU);
		if (scanf("%hhd", &wielomian->stopien) != 1 || wielomian->stopien < 0 || wielomian->stopien > MAX_STOPIEN_WIELOMIANU)
			break;
		puts("Podaj współczynniki przy kolejnych (malejących) potęgach zmiennej:");
		for (signed char i = wielomian->stopien; i >= 0; --i)
			if (scanf("%d", &wielomian->wspolczynniki[i]) != 1)
				goto wyjscie;
		if (wielomian->stopien == 0) {
			printf("%d = 0\n", wielomian->wspolczynniki[0]);
			continue;
		}
		liczba_dzielnikow = 0;
szukaj:
		if (wielomian->stopien > 0)
			for (dzielnik.licznik = 1; dzielnik.licznik <= abs(wielomian->wspolczynniki[0]); ++dzielnik.licznik) {  // szukamy dzielników wielomianu
				if (wielomian->wspolczynniki[0] % dzielnik.licznik != 0)
					continue;
				for (dzielnik.mianownik = 1; dzielnik.mianownik <= abs(wielomian->wspolczynniki[wielomian->stopien]); ++dzielnik.mianownik) {
					if (wielomian->wspolczynniki[wielomian->stopien] % dzielnik.mianownik != 0)
						continue;
					for (signed char i = 0; i < 2; ++i, dzielnik.licznik *= -1)
						if (reszta_z_dzielenia(wielomian, dzielnik) == 0) {  // znaleźliśmy miejsce zerowe
							skroc_ulamek(&dzielnik);
							dzielniki[liczba_dzielnikow].licznik = dzielnik.licznik;
							dzielniki[liczba_dzielnikow].mianownik = dzielnik.mianownik;
							++liczba_dzielnikow;
							nowy_wielomian->stopien = wielomian->stopien - 1;
							nowy_wielomian->wspolczynniki[nowy_wielomian->stopien] = wielomian->wspolczynniki[wielomian->stopien];  // wylicza współczynnik przy najwyższej potędze
							for (signed char i = nowy_wielomian->stopien - 1; i >= 0; --i)  // wylicza pozostałe współczynniki ilorazu
								nowy_wielomian->wspolczynniki[i] = wielomian->wspolczynniki[i + 1] + nowy_wielomian->wspolczynniki[i + 1] * dzielnik.licznik / dzielnik.mianownik;
							swap_pointers((void **)&wielomian, (void **)&nowy_wielomian);
							goto szukaj;
						}
				}
			}
		qsort(dzielniki, liczba_dzielnikow, sizeof(struct ulamek_niewlasciwy), porownaj_ulamki);
		if (wielomian->stopien == 0 && wielomian->wspolczynniki[0] != 1)
			printf("%d * ", wielomian->wspolczynniki[0]);
		for (signed char i = 0; i < liczba_dzielnikow; ++i) {  // wypisuje dzielniki wielomianu
			if (i != 0)
				fputs(" * ", stdout);
			if (dzielniki[i].mianownik == 1)
				printf("(x %c %d)", dzielniki[i].licznik < 0 ? '+' : '-', abs(dzielniki[i].licznik));
			else
				printf("(x %c %d/%d)", dzielniki[i].licznik < 0 ? '+' : '-', abs(dzielniki[i].licznik), dzielniki[i].mianownik);
		}
		if (wielomian->stopien != 0) {  // wypisuje nierozłożony wielomian
			bool wypisano_pierwszy_wspolczynnik = 0;
			if (liczba_dzielnikow != 0)
				fputs(" * (", stdout);
			for (int i = wielomian->stopien; i >= 0; --i) {
				if (wielomian->wspolczynniki[i] == 0)
					continue;
				printf(wypisano_pierwszy_wspolczynnik == 0 ? "%d" : "%+d", wielomian->wspolczynniki[i]);
				if (i >= 1)
					fputs("*x", stdout);
				if (i > 1)
					printf("^%d", i);
				wypisano_pierwszy_wspolczynnik = 1;
			}
			if (liczba_dzielnikow != 0)
				putchar(')');
		}
		puts(" = 0");
	}
wyjscie:
	free(liczby_pierwsze);
	putchar('\n');
	return 0;
}
