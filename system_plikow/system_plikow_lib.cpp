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
std::clog<<"system_plikow(std::string nazwa)"<<std::endl;
    std::string tmp;
    unsigned long int tmp2;
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
                f.seekg(4, f.beg);
                f.read((char*)&rozmiar_systemu, 4);
                tmp2=ceil(0.00476*rozmiar_systemu);
                rozm_fragm_dzien=tmp2+(tmp2%4);
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
std::clog<<"wczytany rozmiar systemu: "<<rozmiar_systemu<<std::endl;
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
std::clog<<"system_plikow(std::string nazwa, unsigned long int rozmiar)"<<std::endl;
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
std::clog<<"create_fs_file()"<<std::endl;
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
        file_exists=true;
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
std::clog<<"set_fs_size(unsigned long int rozmiar)"<<std::endl;
    unsigned long int tmp;
    if(!file_exists)
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
std::clog<<"delete_fs_file()"<<std::endl;
    if(remove(nazwa_systemu.c_str())!= 0)
    {
        std::cerr<<"nie udalo sie usunac dysku wirtualnego"<<std::endl;
        return false;
    }
    return true;
}

int system_plikow::cp_d_to_v(std::string nazwa)
{
std::clog<<"cp_d_to_v(std::string nazwa)"<<std::endl;
    std::fstream p;
    unsigned long int p_size=0;
    p.open(nazwa.c_str(), std::fstream::in | std::fstream::binary);
    if(!p.is_open())
    {
        std::cerr<<"błąd otwierania pliku: "<<nazwa<<std::endl;
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
std::clog<<"odpowiednie miejsce w dzienniku poz: "<<poz<<std::endl;
    p_nam_siz+=(p_nam_siz+9)%4;/**< naiwne wyrównanie */
    if(poz==0)
    {
        poz=2+powieksz_dziennik(rozm_fragm_dzien);
std::clog<<"utworzone miejsce w dzienniku poz: "<<poz<<std::endl;
    }
    f.seekg(poz, f.beg);
    f.write((char*)&p_nam_siz, 1);/**< długość nazwy */
    f.write(nazwa.c_str(), nazwa.size());/**< nazwa */
    //f.write((char*)&zero,(nazwa.size()+9)%4);/**< opcjonalne zera kończące nazwę */
    f.write((char*)&p_size, 4);/**< rozmiar pliku */
std::clog<<"p_size: "<<p_size<<std::endl;
    unsigned long int p_file_adr=0;
    unsigned long int tmp_adr=0;
    char* buf;
    best_fit bf;
    do
    {
        tmp_adr=f.tellg();/**< w tym miejscu wskazanie na następny fragment pliku */
        bf=znajdz_miejsce_na_plik(p_size+8);
        p_file_adr=bf.adres;
        f.seekg(tmp_adr, f.beg);
std::clog<<"bf.adres("<<f.tellg()<<"): "<<bf.adres<<std::endl;
std::clog<<"bf.rozmiar("<<f.tellg()<<"): "<<bf.rozmiar<<std::endl;
std::clog<<"bf.nadmiar: "<<bf.nadmiar<<std::endl;
        f.write((char*)&p_file_adr, 4);/**< adres pierwszego fragmentu pliku */
        buf=new char[bf.rozmiar];
        p.read(buf,bf.rozmiar-8);
        f.seekg(p_file_adr, f.beg);
        f.write((char*)&bf.rozmiar, 4);/**< rozmiar fragmentu pliku */
        f.write(buf,bf.rozmiar-8);/**< zapis danych z pliku */
        if(bf.nadmiar==0)
        {
            f.write((char*)&zero, 4);
        }
        delete[] buf;
    }while(bf.nadmiar!=0);
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
std::clog<<"znajdz_miejsce_w_dzienniku_wieksze_niz(unsigned long int rozmiar)"<<std::endl;
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
    while(i<length)/**< dziennik może być tylo w już napoczętej części */
    {
        f.seekg (i, f.beg);
        f.read((char*)&adr_fragm, 4);/**< wczytanie adresu fragmentu */
std::clog<<"sprawdzam fragment: "<<adr_fragm<<std::endl;
        if(adr_fragm==0)/**< nie znajdziemy takiego miejsca */
        {
            if(!stan)
            {
                f.close();
            }
            return 0;
        }
        f.seekg(adr_fragm, f.beg);
        f.read((char*)&rozm_fragm, 2);/**< wczytanie rozmiaru fragmentu */
std::clog<<"rozmiar sprawdzanego fragmentu: "<<rozm_fragm<<std::endl;
        licz_dl_fragm=0;
        for(unsigned long int j=0; j<rozm_fragm; j++)/**< szukanie wolnej przestrzeni we fragmencie */
        {
            //f.seekg (adr_fragm+2+j, f.beg);
            f.read((char*)&dl_nazwy, 1);
//std::clog<<"dlugosc nazwy: "<<(unsigned short)dl_nazwy<<std::endl;
//system("pause");
            if(dl_nazwy>0)
            {
std::clog<<"dlugosc nazwy: "<<(unsigned short)dl_nazwy<<std::endl;
                if(licz_dl_fragm>=rozmiar)/**< szukana wolna przestrzen znaleziona */
                {
                    if(!stan)
                    {
                        f.close();
                    }
                    return f.tellg()-licz_dl_fragm;/**< poczatek wolnego obszaru */
                }
                f.seekg (dl_nazwy+8, f.cur);/**< przeskakujemu nazwe, rozmiar i adres pliku */
                licz_dl_fragm=0;/**< pusty ciąg przerwany */
                continue;
            }
            licz_dl_fragm++;
            if(licz_dl_fragm>=rozmiar)/**< szukana wolna przestrzen znaleziona */
            {
                if(!stan)
                {
                    f.close();
                }
                return f.tellg()-licz_dl_fragm;/**< poczatek wolnego obszaru */
            }
std::clog<<licz_dl_fragm<<","<<j<<" ";
        }
        i=adr_fragm+rozm_fragm-4;
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
std::clog<<"powieksz_dziennik(unsigned long int rozm_fragm)"<<std::endl;
std::clog<<"rozm_fragm: "<<rozm_fragm<<std::endl;
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
    while(i<length && i<rozmiar_systemu)
    {
        i=next_i;
        f.seekg (i, f.beg);
        f.read((char*)&adr_fragm, 4);/**< wczytanie adresu fragmentu */
std::clog<<"adres fragmentu: "<<adr_fragm<<std::endl;
        next_i=adr_fragm+rozm_fragm;/**< jeśli chcemy zmiennego rozmiaru w danym pliku, to trzeba by zawsze wczytywać */
        if(adr_fragm==0)/**< koniec */
        {
            licz=0;
            if(f.tellg()==length)
            {
std::clog<<"f.tellg(): "<<f.tellg()<<std::endl;
std::clog<<"rozmiar systemu: "<<rozmiar_systemu<<std::endl;
                if(f.tellg()+rozm_fragm<rozmiar_systemu)
                {
                    adr_fragm=f.tellg();
                    f.write((char*)&rozm_fragm, 2);
                    for(unsigned long int j=2; j<rozm_fragm; j++)/**< nowy pusty fragment dziennika */
                    {
                        if(f.tellg()>=rozmiar_systemu)
                        {
                            throw new std::string("próba wykroczenia poza dysk");
                        }
                        f.write((char*)&zero,1);
                    }
std::clog<<"f.tellg(): "<<f.tellg()<<std::endl;
                    f.seekg (i, f.beg);/**< na pozycji gdzie było wskazanie na zero */
std::clog<<"f.tellg(): "<<f.tellg()<<std::endl;
                    f.write((char*)&adr_fragm, 4);/**< nadpisaniee poprzedniego wskazania nowym */
                    return adr_fragm;
                }
                else
                {
                    //std::cerr<<"nie ma wystarczająco miejsca, żeby dodać następny fragment dziennika"<<std::endl;
                    throw new std::string("nie ma wystarczająco miejsca, żeby dodać następny fragment dziennika");
                }
            }
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
                            if(f.tellg()>=rozmiar_systemu)
                            {
                                throw new std::string("próba wykroczenia poza dysk");
                            }
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

bool my_cmp(std::pair<unsigned long int, unsigned long int> a, std::pair<unsigned long int, unsigned long int> b)
{
    return a.first<b.first;
}

bool my_cmp2(std::pair<unsigned long int, unsigned long int> a, std::pair<unsigned long int, unsigned long int> b)
{
    return a.second<b.second;
}

/** \brief wyszukiwanie wolnych przestrzeni przez zmapowanie już zajętych
 *
 * \param rozmiar unsigned longint
 * \return system_plikow::best_fit
 *
 */
system_plikow::best_fit system_plikow::znajdz_miejsce_na_plik(unsigned long int rozmiar)
{
std::clog<<"znajdz_miejsce_na_plik(unsigned long int rozmiar)"<<std::endl;
    best_fit r;
    r.adres=0;
    r.rozmiar=0;
    r.nadmiar=rozmiar;
    bool stan=f.is_open();
    if(!stan)
    {
        f.open(nazwa_systemu.c_str(), std::fstream::in | std::fstream::out | std::fstream::binary);
    }
    f.seekg (0, f.end);
    unsigned long int length=f.tellg();
    unsigned long int najdluzy=0;
    std::vector< std::pair <unsigned long int, unsigned long int> > v;/**< wektor zakresów */
    unsigned long int adr_dz=0;
    unsigned short int dl_dz=0;
    unsigned long int adr_pl=0;
    unsigned long int dl_pl=0;
    unsigned long int adr_fr_pl=0;
    unsigned long int dl_fr_pl=0;
    unsigned char tmp=0;
    std::pair< unsigned long int, unsigned long int> tmp_pair;
    f.seekg(8, f.beg);
    f.read((char*)&adr_dz, 4);
std::clog<<"adres dziennika: "<<adr_dz<<std::endl;
    tmp_pair=std::make_pair(0,11);
    v.push_back(tmp_pair);
    while(adr_dz!=0)/**< spisywanie zajętych obszarów */
    {
        f.seekg(adr_dz, f.beg);
        f.read((char*)&dl_dz, 2);/**< długość fragmentu dziennika */
std::clog<<"długość fragmentu dziennika: "<<dl_dz<<std::endl;
        tmp_pair=std::make_pair(adr_dz, adr_dz+dl_dz);
        v.push_back(tmp_pair);/**< dodanie zakresu fragmentu dziennika */
        for(int i=0; i<dl_dz-14; i++)
        {
            f.seekg(adr_dz+2+i, f.beg);/**< tutaj szukamy początku info o pliku */
//std::clog<<"f.tellg(): "<<f.tellg()<<std::endl;
            f.read((char*)&tmp, 1);/**< szukanie informacji o pliku */
            if(tmp>0)/**< znaleźliśmy informację o pliku */
            {
std::clog<<"dlugosc nazwy: "<<(int)tmp<<std::endl;
std::clog<<"f.tellg(): "<<f.tellg()<<std::endl;
                i+=tmp+8;/**< przeskakujemy nazwę, rozmiar i adres */
                f.seekg(tmp-1, f.cur);
std::clog<<"f.tellg(): "<<f.tellg()<<std::endl;
                f.read((char*)&dl_pl, 4);/**< całkowita długość pliku */
                f.read((char*)&adr_pl, 4);/**< adres pierwszego fragmentu pliku */
                adr_fr_pl=adr_pl;
std::clog<<"adres pliku: "<<adr_fr_pl<<" rozmiar c pliku: "<<dl_pl<<std::endl;
                while(adr_fr_pl!=0)/**< szukanie fragmentów pliku */
                {
                    f.seekg(adr_fr_pl, f.beg);/**< skok do początka fragmentu pliku */
                    f.read((char*)&dl_fr_pl, 4);/**< dlugość fragmentu pliku */
                    tmp_pair=std::make_pair(adr_fr_pl,adr_fr_pl+dl_fr_pl);/**< zakres fragmentu */
                    v.push_back(tmp_pair);/**< dodanie kolejnego zajętego obszaru */
                    f.seekg(adr_fr_pl+dl_fr_pl-4, f.beg);/**< skok do wskaźnika na następny adres */
                    f.read((char*)&adr_fr_pl, 4);/**< wczytanie następnego adresu */
                }
            }
        }

        f.read((char*)&adr_dz, 4);/**< następny adres fragmentu dzinnika */
    }
    std::sort(v.begin(),v.end(),my_cmp);/**< uporządkowanie zajętych obszarów */
    //tmp_pair=std::make_pair(v.at(v.size()-1).second+1, rozmiar_systemu-1);/**< pozostała wolna przestrzeń */
    //v.push_back(tmp_pair);

    std::clog<<"zajęte fragmenty"<<std::endl;
    for(int i=0;i<v.size();i++)
    {
        std::clog<<v.at(i).first<<","<<v.at(i).second<<" ";
    }

    int tmp_dl;
    std::vector< std::pair<unsigned long int, unsigned long int> > spaces;/**< adres i rozmiar wolnej przestrzeni */
    for(int i=1;i<v.size();i++)
    {
        tmp_dl=v.at(i).first-v.at(i-1).second;/**< wyliczanie wolnych przestrzeni */
        tmp_pair=std::make_pair(v.at(i-1).second+1,tmp_dl-1);
        spaces.push_back(tmp_pair);
    }

    tmp_pair=std::make_pair(v.at(v.size()-1).second+1, rozmiar_systemu-v.at(v.size()-1).second);/**< pozostała wolna przestrzeń */
    spaces.push_back(tmp_pair);

    sort(spaces.begin(),spaces.end(),my_cmp2);/**< ułożenie wolnych przestrzeni po rozmiarach */

    std::clog<<"wolne fragmenty"<<std::endl;
    for(int i=0;i<spaces.size();i++)
    {
        std::clog<<spaces.at(i).first<<","<<spaces.at(i).second<<" ";
    }

    bool caly=false;
    for(int i=0;i<spaces.size();i++)
    {
        if(spaces.at(i).second>rozmiar)/**< pierwsza najmniejsza przestrzeń, gdzie zmieści się zadany rozmiar */
        {
            caly=true;
            r.adres=spaces.at(i).first;
            r.rozmiar=rozmiar;//spaces.at(i).second;
            r.nadmiar=0;
            return r;/**< znalezliśmy wpasowanie całosci */
        }
        if(spaces.at(i).second>najdluzy)
        {
            najdluzy=spaces.at(i).second;
            r.adres=spaces.at(i).first;
            r.rozmiar=spaces.at(i).second;
            r.nadmiar=rozmiar-r.rozmiar;
        }
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









