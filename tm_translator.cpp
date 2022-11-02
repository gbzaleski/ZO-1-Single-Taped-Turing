#include <iostream>
#include <sstream>
#include <cstddef>
#include <cstdlib>
#include <fstream>
#include "turing_machine.h"
#include "tm_convert.h"

using namespace std;

int main(int argc, char* argv[]) 
{
    if (argc < 2)
    {
        cerr << "ERROR: .tm file not provided!" << endl;
        return 1;
    }

    string filename = argv[1];
    string outputname = argc > 2 ? argv[2] : "one_taped_translation.tm";

    FILE *f = fopen(filename.c_str(), "r");
    if (!f) 
    {
        cerr << "ERROR: File " << filename << " does not exist" << endl;
        return 1;
    }
    TuringMachine tm = read_tm_from_file(f);

    TuringMachine one_taped_tm = tm_convert(tm);

    ofstream one_taped_tm_file;
    one_taped_tm_file.open(outputname);
    one_taped_tm_file << one_taped_tm;
    one_taped_tm_file.close();

    cout << one_taped_tm;
}
 
