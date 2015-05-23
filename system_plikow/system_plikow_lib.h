/** \file system_plikow_lib.h
 * \author Tomasz Jakubczyk
 * \brief plik naglowkowy biblioteki systemu plikow
 */
/** \mainpage
 * \author Tomasz Jakubczyk
 * \section struktura Struktura Dysku Wirtualnego
 \subsection opis_naglowka Opis Nagłówka
 Na samym początku pliku występuje nagłówek dysku.
 \subsubsection naglowek Nagłówek:
    - 4 bajty napis "TJFS" do rozpoznawania mojego pliku.
    - 4 bajty (unsigned long int) rozmiar pliku (a zarazem jego koniec) w bajtach.
    - 4 bajty adres pierwszego fragmentu dziennika.
    Jeśli jest zero, to znaczy, że nie ma utworzonego dziennika, a zatem też żadnych plików.
 \subsection opis_dziennika Opis Dziennika
 Na powyższym adresie zaczyna się pierwszy fragment dziennika.
 Dziennik może składać się z wielu fragmentów i jest swego rodzaju listą jednokierunkową.
 \subsubsection dziennik Fragment dziennika:
    - 2 bajty długość fragmentu dziennika w bajtach (najprawdopodobniej będzie wyrównana do dword'a)
    Po informacji o fragmencie dziennika następują informacje o plikach.
    - 1 bajt długość nazwy pliku w bajtach.
    - n bajtów nazwy pliku
    (zapewne na końcu uzupełniona zerami -napisy są zwykle null terminated-, tak aby uzyskać globalne dopełnienie do dword'a).
    - 4 bajty rozmiar pliku w bajtach.
    - 4 bajty adres pierwszego fragmentu pliku.
    Pliki mogą być podzielone na fragmenty tworzące listę jednokierunkową.
    - Teraz mogą wystąpić informacje o innych plikach w takim samym formacie,
    lub przynajmniej 4 następne bajty mają wartość 0.
    - 4 bajty wskazujące na ewentualny następny fragment dziennika lub 0 znaczące,
    że to ostatni fragment dziennika. Ten dword najlpiej jeśli będzie wyrównany globalnie do dworda w prawo.
 \subsection opis_plikow Opis Plików
 Pod adresem z dziennika znajduje się początek listy fragmentów zawierającyh plik.
 \subsubsection pliki Fragmenty plików:
    - 4 bajty na rozmiar danego fragmentu pliku. Z uwzględnieniem wyrównania ostatniego dword'a
    - bajty pliku
    - 4 bajty adres następnego fragmentu pliku, lub 0 jeśli to ostatni fragment.
 \section znaczenia Znaczenia Rozmiarów
Bajty pełniące funkcje dworda trzymającego rozmiar lub adres dobrze,
żeby miały wyrównanie naturalne do 4, bo wtedy łatwiej jest oglądać dysk w hex edytorze,
lub wykonywać obliczenia oparte o system dwujkowy.
Jeśli odnoszę się do rozmairu fragmentu, to mam na mysil cały rozmiar fragmentu, tak,
żeby bez dodatkowych operacji wyliczyć koniec fragmentu.
Dword rozmiaru pliku zawiera w sobie tylko i wyłącznie rozmiar pliku, taki, jak odczytany,
przy jego kopiowaniu/otwieraniu.
 \section szacowanie Szacowanie Rozmiarów Fragmentów Dziennika
 Niech najmniejszy dysk będzie miał 100kB i będzie zapełniony plikami po 4kB.
 wychodzi, że nie zmieści się więcej niż 25 plików -1 na trzymanie nazw, rozmiarów i adresów.
 To przy założeniu, że nazwa ma średnio koło 10 znaków.
 24pliki * ( 19B na nazwę, długości i adres ) + 18 na nagówek z jednym fragmentem dziennika = 474B
 Czyli rozmiar bloku przyjmijmy 476B razem z wyrównaniem.
 Od tego momentu ze wzrostem rozmiaru dysku rozmiar fragmentu powinien wzrastać proporcjonalnie,
 aż do osiągnięcia maksymalenj długości pojedyńczego fragmentu.
 Czyli średnio przy 4,6MB będzie potrzebny następny blok.
 y - rozmiar fragmentu dziennika
 x - rzozmiar pliku
 y:=ceil(0.00476*x)+ceil(0.00476*x)%4
 jeśli y > 65532
 to y:=65532
 \section ulozenie_plikow Ułożenie Plików
 Jeśli będzie to możliwe, to pliki będą zapisywane w jednym fragmenci, jeśli nie,
 to będą one podzielone na najwieksze możliwe pasujące w wolne miejsca fragmenty.
 \section dlaczego Dlaczego tak?
    Jak powszechnie wiadomo jeśli elementy nie są do siebie idealnie wpasowane,
    to zajmują więcej miejsca (fragmentacja), jeśli mamy wiele elmentów przynajmniej dwóch różnych
    rozmiarów, to zajmą one przestrzeń lepiej niż taka sama obętość identycznych elementów.
    To troche tak jak chaotyczne pakowanie rzeczy do plecaka.
    Jeśli skasujemy jakiś plik i na jego miejsce wsadzimy inny, to powstanie przerwa, którą trudno
    by było wypełnić jeszcze innym plikiem, ale przychodzi nam z pomocą mniejszy fragment danych
    dziennika, które z pewością tam się zmieszczą. :-)

    W taki rozwiązanie jest też chyba dość ciekawe.

 *
 */

#include<string>
#include<fstream>

/** \class system_plikow
 * \brief abstrakcyjna struktura
 * trzymajaca w pamieci informacje o systemie plikow (wirtualny dysk z katalogiem)
 * jesli chcemy cos z nim zrobic
 */
class system_plikow
{
private:
    std::string nazwa_systemu;/**< nazwa trwale identyfikuje obiekt */
    unsigned long int rozmiar_systemu;/**< rozmiar pliku z systemem plikow w bajtach */
    bool file_exists;/**< czy mamy juz plik naszego wirtualnego dysku */
    std::fstream f;/**< strumien na plik dysku wirtualnego */
    unsigned long int rozm_fragm_dzien;/**< szacowany_najlepszy_rozmiar_fragmentu_dziennika */
    unsigned long int znajdz_miejsce_w_dzienniku_wieksze_niz(unsigned long int rozmiar);
    unsigned long int powieksz_dziennik(unsigned long int);
    best_fit znajdz_miejsce_na_plik(unsigned long int rozmiar);
public:
    system_plikow(std::string nazwa);/**< podstawowy konstruktor wiążący nazwę */
    system_plikow(std::string nazwa, unsigned long int rozmiar);
    bool create_fs_file();/**< utworzenie wirtualnego dysku */
    bool delete_fs_file();/**< usuniecie wirtualnego dysku */
    int cp_d_to_v(std::string nazwa);/**< kopiowanie plikow na dysk wirtualny */
    int cp_v_to_d(std::string nazwa);/**< kopiowanie plikow z dysku wirtualnego */
    int delete_file(std::string nazwa);
    std::string show_files();
    std::string show_fs_map();
    bool set_fs_size(unsigned long int rozmiar);
    struct/**< adres i rozmiar wolnego miejsca */
    {
        unsigned long int adres;/**< tu się zmieści */
        unsigned long int rozmiar;/**< tyle się zmieści */
        unsigned long int nadmiar;/**< tyle się nie zmieści */
    }typedef best_fit;
};
