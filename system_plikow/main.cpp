#include <iostream>
#include "system_plikow_lib.h"

using namespace std;

int main()
{
    cout << "Hello world!" << endl;
    cout<<"sizeof(unsigned long int): "<<sizeof(unsigned long int)<<" byc powinno: 4"<<endl;
    try
    {
        //system_plikow fs("drive1.tjfs", 100000);
        system_plikow fs("drive1.tjfs");
        cout<<fs.set_fs_size(100000)<<endl;;
        cout<<fs.create_fs_file()<<endl;
        fs.cp_d_to_v("test1.txt");
    }
    catch(string *s)
    {
        cout<<*s<<endl;
    }
    catch(exception& e)
    {
        cout<<e.what()<<endl;
    }
    catch(...)
    {
        cout<<"jakis nieznany wyjatek"<<endl;
    }
    /** \todo napisać osobne programy kożystające z mojej biblioteki realizujące operacje na dysku wirtualnym */
    return 0;
}
