#include <iostream>
#include <algorithm>
#include <fstream>
#include <vector>
#include <limits>
#include <sstream>
#include <string>
#include "windows.h"
#include "quicksort.h"
#include "bstr.h"


int main()
{

    //I begin by declaring some variables that will be used throughout the scope of this program.

    std::ifstream inputfile;
    std::ifstream data;
    std::ofstream results;
    std::ofstream outputflag;
    std::string datapath;
    std::string linestr;
    std::string directory;
    std::vector<std::string> allvarnames;
    std::vector<std::string> varnames;
    std::vector<std::vector<double> > uniquenums;
    char buffer[MAX_PATH];
    int cases = 0;
    int current_case;

    //Now I will obtain the directory of the program in order to locate our input files and place our output files.

    GetModuleFileName(NULL, buffer, MAX_PATH);
    directory = std::string(buffer).substr(0,std::string(buffer).find_last_of("\\/"));
    inputfile.open(directory + "\\uniquevaluesinput.txt");
    results.open(directory + "\\uniquevaluesoutput.txt");
    if (!inputfile.is_open())
    {
        std::cout << "inputfile does not exist" << std::endl;
        return 0;
    }

    //The first line of inputfile will be the path of the dataset that the user is currently using

    getline(inputfile,linestr);
    datapath = linestr;

    //The second line of inputfile will be the number of cases that the user wishes to analyze

    getline(inputfile,linestr);
    if(linestr != "Cases = ALL")
    {
        std::stringstream SS(linestr.substr(linestr.find("=") + 2));
        SS >> cases;
    }
    else{
        cases = -1; // we will set cases = -1 if the user wishes to view the entire dataset
    }

    //The third line of the inputfile will include the variablenames that the user has sent over. These will be in a comma-delimited format.

    getline(inputfile,linestr);
    varnames = split(linestr,',');
    inputfile.close(); // close the file.

    //Now we will open the dataset.

    data.open(datapath);
    if (!data.is_open())
    {
        std::cout << "data does not exist" << std::endl;
        return 0;
    }

    //Allocate space for our 2-d vector to hold the sets of unique numbers.

    uniquenums.resize(varnames.size());

    //The first line of the dataset will include the variable names of all variables in the dataset.

    getline(data,linestr);
    allvarnames = split(linestr,','); // This will be a container that holds all variable names.
    current_case = 0; // This variable will break the while loop if it reaches the max cases the user has specified.
    while(getline(data,linestr)) // Begin reading the dataset.
    {
        //These first lines will increment the current case by 1, declare an iterator for varnames vector, and establish the current case with a vector of strings.

        current_case++;
        std::vector<std::string> case_i = split(linestr,',');
        size_t i = 0;
        for(size_t varit=0 ; varit<varnames.size() ; ++varit)
        {

            //For each string in our case, a boolean variable will trigger if we find a value we have already seen for that variable.

            bool match = false;

            //This while loop will sync our case iterator with the current variables' location in the dataset.
            //Since allvarnames includes all variables and equals case_i in size, we will push i until allvarnames[i] equals varnames[varit].
            //We do not need to reset i to 0 because the list in varnames is forced to follow the same sequence as allvarnames.

            while(allvarnames[i] != varnames[varit])
            {
                ++i;
            }

            //Extract the number with a stringstream.
            if(case_i[i] != "")
            {
                std::stringstream numberstream(case_i[i]);
                double number = 0;
                numberstream >> number;

                //Here we will perform a series of searches and sorts, which I believe is optimal due to the amount of searches and sorts we will perform through the scope of this loop.

                if(std::binary_search(uniquenums[varit].begin(), uniquenums[varit].end(), number))
                    match = true;

                if(match == false)
                {
                    uniquenums[varit].push_back(number);
                    quicksort(&uniquenums[varit], 0, (uniquenums[varit].size() - 1));
                }
            }
        }

        // Break the loop if we reach the potentially specified limit.

        if(current_case >= cases && cases != -1) break;
    }

    //Close the dataset and print the results into our 'results' output txt file.

    data.close();
    if (uniquenums.size() > 0)
    {
        for(size_t i=0 ; i<uniquenums.size() ; ++i)
        {
            for(size_t j=0 ; j<uniquenums[i].size() ; ++ j)
            {
                if(j<uniquenums[i].size() - 1)
                    results << uniquenums[i].at(j) << ',';
                else
                    results << uniquenums[i].at(j) << std::endl;
            }
        }
    }
    results.close();
    outputflag.open(directory + "\\FLAG.txt"); // Create a flag for VBA to detect that the program here is complete.
    outputflag.close();
}
