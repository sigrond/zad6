/** \file system_plikow_lib.cpp
 * \author Tomasz Jakubczyk
 * \brief implementacje metod biblioteki system plikow
 *
 *
 */

#include "system_plikow_lib.h"
#include<cstdlib>
#include<cstring>
#include<cstdio>
#include<cmath>
#include<vector>
#include<algorithm>
#include<iostream>

/** \brief konstruktor
 * sprawdza nazwe, zeby wiedziec, czy dzialamy na istniejacym pliku
 * \param nazwa std::string
 *
 */
system_plikow::system_plikow(std::string nazwa) : nazwa_systemu(nazwa)
{
    std::string tmp;
    file_exists=false;
    rozmiar_systemu=0;
    f.open(nazwa_systemu.c_str(), std::fstream::in | std::fstream::out);
    if(f.is_open())/**< czy plik juz istnieje */
    {
        tmp.resize(4);
        f.read(&tmp[0],4);
        if(f)
        {
            if(tmp.compare("TJFS")==0)
            {
                f.close();
                file_exists=true;
                f.open(nazwa_systemu.c_str(), std::fstream::in | std::fstream::binary);
                f.read((char*)&rozmiar_systemu, 4);
                f.close();
            }
            else
            {
                f.close();
                file_exists=false;
                throw new std::string(
                    "taki plik juz istnieje, ale nie jest to nasz dysk wirtualny");
            }
        }
        else
        {
            f.close();
            file_exists=false;
            throw new std::string(
                "taki plik juz istnieje, ale nie udalo sie odczytac pierwszych 4 znakow. prawdopodobnie nie jest to nasz plik, albo dzieje sie z nim cos niedobrego");
        }
    }
    else
    {
        file_exists=false;
    }
}

/** \brief konstruktor
 * wola konstruktor powyzej. ustawia rozmiar
 * \param nazwa std::string
 * \param rozmiar unsigned longint
 *
 */
system_plikow::system_plikow(std::string nazwa, unsigned long int rozmiar)
    : system_plikow::system_plikow(nazwa)
{
    unsigned long int tmp;
    if(file_exists)
    {
        throw new std::string("prubujesz ustawic rozmiar plikowi, ktory ma juz ustawiony rozmiar");
    }
    else
    {
        rozmiar_systemu=rozmiar;
        tmp=ceil(0.00476*rozmiar_systemu);
        rozm_fragm_dzien=tmp+(tmp%4);
        if(rozm_fragm_dzien>65532)/**< najwiekszy możliwy fragment dziennika,
        ze względu na zakres rozmiaru fragmentu i wyrównanie naturalne */
        {
            rozm_fragm_dzien=65532;
        }
        else if(rozm_fragm_dzien<270)/**< najmiejszy bezpieczny fragment,
            ze względu na maksymalną długość napisu */
        {
            rozm_fragm_dzien=270;
        }
    }
}


/** \brief tworzy plik wirtualnego dysku
 *
 * \return bool
 *
 */
bool system_plikow::create_fs_file()
{
    unsigned long int tmp=0;
    if(file_exists)
    {
        return false;
    }
    f.open(nazwa_systemu.c_str(), std::fstream::out | std::fstream::binary | std::fstream::app);
    if(f)
    {
        f.write("TJFS", 4);
        f.write((char*)&rozmiar_systemu, 4);
        f.write((char*)&tmp, 4);
        f.close();
        return true;
    }
    return false;
}

/** \brief ustawienie rozmiaru wirtualnego dysku w bajtach
 *
 * \param rozmiar unsigned longint
 * \return bool
 *
 */
bool system_plikow::set_fs_size(unsigned long int rozmiar)
{
    if(!file_exists)
    {
        rozmiar_systemu=rozmiar;
        return true;
    }
    return false;
}

/** \brief kasowanie pliku dysku wirtualnego
 *
 * \return bool
 *
 */
bool system_plikow::delete_fs_file()
{
    if(remove(nazwa_systemu.c_str())!= 0)
    {
        std::cerr<<"nie udalo sie usunac dysku wirtualnego"<<std::endl;
        return false;
    }
    return true;
}

int system_plikow::cp_d_to_v(std::string nazwa)
{
    std::fstream p;
    unsigned long int p_size=0;
    p.open(nazwa.c_str(), std::fstream::in | std::fstream::binary);
    if(!p.is_open())
    {
        return 0;
    }
    if(!file_exists)
    {
        p.close();
        return 0;
    }
    p.seekg (0, f.end);
    p_size=p.tellg();/**< długość pliku */
    f.open(nazwa_systemu.c_str(), std::fstream::in | std::fstream::out | std::fstream::binary);
    unsigned long int poz=0;
    unsigned char p_nam_siz=nazwa.size();/**< zakładam, że się nie przepełni */
    unsigned long int zero=0;
    poz=znajdz_miejsce_w_dzienniku_wieksze_niz(p_nam_siz+9+(p_nam_siz+9)%4);
    p_nam_siz+=(p_nam_siz+9)%4;/**< naiwne wyrównanie */
    if(poz==0)
    {
        poz=2+powieksz_dziennik(rozm_fragm_dzien);
    }
    f.seekg(poz, f.beg);
    f.write((char*)&p_nam_siz, 1);/**< długość nazwy */
    f.write(nazwa.c_str(), nazwa.size());/**< nazwa */
    f.write((char*)&zero,(nazwa.size()+9)%4);/**< opcjonalne zera kończące nazwę */
    f.write((char*)&p_size, 4);/**< rozmiar pliku */
    unsigned long int p_file_adr=0;

    f.write((char*)&p_file_adr, 4);/**< adres pierwszego fragmentu pliku */
    f.close();
    p.close();
    return 1;
}


/** \brief znajduje pierwszy odpowiednio duży fragment dziennika na informację o pliku
 *
 * \param rozmiar unsigned longint
 * \return unsigned long int adres wolnej przestrzeni dziennika
 *
 */
unsigned long int system_plikow::znajdz_miejsce_w_dzienniku_wieksze_niz(unsigned long int rozmiar)
{
    bool stan=f.is_open();
    if(!stan)
    {
        f.open(nazwa_systemu.c_str(), std::fstream::in | std::fstream::out | std::fstream::binary);
    }
    f.seekg (0, f.end);
    unsigned long int length=f.tellg();
    unsigned long int i=8;/**< adres pierwszego fragmentu dziennika */
    unsigned long int adr_fragm=0;
    unsigned short int rozm_fragm=0;
    unsigned char dl_nazwy=0;
    unsigned short int licz_dl_fragm=0;
    //unsigned long int rozm_pliku=0;
    while(i<length)
    {
        f.seekg (i, f.beg);
        f.read((char*)&adr_fragm, 4);/**< wczytanie adresu fragmentu */
        if(adr_fragm==0)/**< nie znajdziemy takiego miejsca */
        {
            if(!stan)
            {
                f.close();
            }
            return 0;
        }
        f.read((char*)&rozm_fragm, 2);/**< wczytanie rozmiaru fragmentu */
        licz_dl_fragm=0;
        for(unsigned long int j=0; j<rozm_fragm; j++)/**< szukanie wolnej przestrzeni we fragmencie */
        {
            //f.seekg (adr_fragm+2+j, f.beg);
            f.read((char*)&dl_nazwy, 1);
            if(dl_nazwy>0)
            {
                if(j>=rozmiar)/**< szukana wolna przestrzen znaleziona */
                {
                    if(!stan)
                    {
                        f.close();
                    }
                    return j-licz_dl_fragm;/**< poczatek wolnego obszaru */
                }
                f.seekg (dl_nazwy+8, f.cur);/**< przeskakujemu nazwe, rozmiar i adres pliku */
                licz_dl_fragm=0;/**< pusty ciąg przerwany */
                continue;
            }
            licz_dl_fragm++;
        }
        i=adr_fragm+rozm_fragm;
    }
    if(!stan)
    {
        f.close();
    }
    return 0;
}

/** \brief dodanie nowego fragmentu dzienika
 *
 * \param rozm_fragm=rozm_fragm_dzien unsigned longint
 * \return unsigned long int adres początku nowego fragmentu dziennika
 *
 */
unsigned long int system_plikow::powieksz_dziennik(unsigned long int rozm_fragm)
{
    bool stan=f.is_open();
    if(!stan)
    {
        f.open(nazwa_systemu.c_str(), std::fstream::in | std::fstream::out | std::fstream::binary);
    }
    f.seekg (0, f.end);
    unsigned long int length=f.tellg();
    unsigned long int i=8;/**< adres pierwszego fragmentu dziennika */
    unsigned long int adr_fragm=0;
    unsigned short int zero=0;
    unsigned long int licz=0;
    unsigned long int next_i=8;
    unsigned char tmp=0;
    while(i<length)
    {
        i=next_i;
        f.seekg (i, f.beg);
        f.read((char*)&adr_fragm, 4);/**< wczytanie adresu fragmentu */
        next_i=adr_fragm+rozm_fragm;/**< jeśli chcemy zmiennego rozmiaru w danym pliku, to trzeba by zawsze wczytywać */
        if(adr_fragm==0)/**< koniec */
        {
            licz=0;
            for(unsigned long int j=f.tellg();j<length;j++)/**< szukanie wolnego miejsca */
            {
                f.read((char*)&tmp, 1);
                if(tmp==0)
                {
                    licz++;
                    if(licz==rozm_fragm)
                    {
                        adr_fragm=j-licz;/**< początek naszego pustego obszaru */
                        f.write((char*)&rozm_fragm, 2);
                        for(unsigned long int j=2; j<rozm_fragm; j++)/**< nowy pusty fragment dziennika */
                        {
                            f.write((char*)&zero,1);
                        }
                        f.seekg (i, f.beg);/**< na pozycji gdzie było wskazanie na zero */
                        f.write((char*)&adr_fragm, 4);/**< nadpisaniee poprzedniego wskazania nowym */
                    }
                }
                else
                {
                    licz=0;
                }
            }
            if(f.tellg()==length)/**< dopisanie na koniec pliku */
            {
                adr_fragm=f.tellg();/**< początek naszego pustego obszaru */
                f.write((char*)&rozm_fragm, 2);
                for(unsigned long int j=2; j<rozm_fragm; j++)/**< nowy pusty fragment dziennika */
                {
                    f.write((char*)&zero,1);
                }
                f.seekg (i, f.beg);/**< na pozycji gdzie było wskazanie na zero */
                f.write((char*)&adr_fragm, 4);/**< nadpisaniee poprzedniego wskazania nowym */
            }
            else
            {
                std::cerr<<"wskaznik pisania nie doszedł prawidłowo do końca pliku"<<std::endl;
            }
            if(!stan)
            {
                f.close();
            }
            return 0;
        }
    }
    if(!stan)
    {
        f.close();
    }
    return adr_fragm;
}

/** \brief wyszukiwanie wolnych przestrzeni przez zmapowanie już zajętych
 *
 * \param rozmiar unsigned longint
 * \return system_plikow::best_fit
 *
 */
system_plikow::best_fit system_plikow::znajdz_miejsce_na_plik(unsigned long int rozmiar)
{
    best_fit r;
    bool stan=f.is_open();
    if(!stan)
    {
        f.open(nazwa_systemu.c_str(), std::fstream::in | std::fstream::out | std::fstream::binary);
    }
    f.seekg (0, f.end);
    unsigned long int length=f.tellg();
    unsigned long int najdluzy=0;
    unsigned long int najkrutszy_dluzszy=0;
    unsigned long int najdluzy_krutszy=0;
    std::vector<std::pair <unsigned long int, unsigned long int> > v;/**< wektor zakresów */
    unsigned long int adr_dz=0;
    unsigned short int dl_dz=0;
    unsigned long int adr_pl=0;
    unsigned long int dl_pl=0;
    unsigned char tmp=0;
    std::pair tmp_pair;
    f.seekg(8, f.beg);
    f.read((char*)&adr_dz, 4);
    while(adr_dz!=0)
    {
        f.seek(adr_dz, f.beg);
        f.read((char*)&dl_dz, 2);
        tmp_pair=std::make_pair(adr_dz, adr_dz+dl_dz);
        v.push_back(tmp_pair);/**< dodanie zakresu fragmentu dziennika */
        for(unsigned long int i=0; i<dl_dz-14; i++)
        {
            f.read((char*)&tmp, 1);
            if(tmp>0)
            {
                i+=tmp;
                f.seek(tmp, f.cur);
                f.read((char*)&dl_pl, 4);
                f.read((char*)&adr_pl, 4);
                /**< \todo napisać znajdowanie zkaresów fragmentów plików */
            }
        }

        f.read((char*)&adr_dz, 4);
    }

    if(!stan)
    {
        f.close();
    }
    return r;
}

/**< \todo napisać kopiowanie z dysku wirtualnego na zwykły, powinno być łatwiejsze */
/**< \todo napisać wyświetlanie plików */
/**< \todo napisać wyświetlanie mapy dysku */









