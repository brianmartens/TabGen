#include "windows.h"
#include "CTable.h"
#include "bstr.h"
#include <thread>
#include <iostream>
#include <fstream>

// for thread safety
std::mutex guard;

int main()
{
    std::ofstream finalflag;
    std::ifstream inputfile, datafile;
    std::string run_type, run_name, datapath, appdir, weightname, lineStr;
    size_t weightLoc, Tcount,  TperThread, CTremainder;
    std::vector<CTable> CTables;
    std::vector<std::string> variablenames;
    char buffer[MAX_PATH];
    bool isweight;
    size_t concurrency = std::thread::hardware_concurrency();

    GetModuleFileName(NULL, buffer, MAX_PATH);
    appdir = std::string(buffer).substr(0,std::string(buffer).find_last_of("\\/"));
    inputfile.open(appdir + "\\InputReport.txt"); // open the input report from VBA
    if(!inputfile.is_open()){ // if the input report errors out, then return
        std::runtime_error("Input report not found: ");
        return 0;
    }
    for(size_t i=0 ; i<4 ; ++i)
        getline(inputfile,lineStr); // skip application filepath, run name, run type, and consolidate tables

    getline(inputfile,lineStr);
    if(bmid(lineStr, lineStr.find('=') + 1, lineStr.size()) == "True")
        isweight = true;
    else
        isweight = false;

    // extract our data path and weight name and weight variable location
    getline(inputfile,lineStr);
    datapath = bmid(lineStr, lineStr.find('=') + 1, lineStr.size());
    datafile.open(datapath);
    if(!datafile.is_open()){
        std::runtime_error("Datafile not found: ");
        return 0;
    }
    getline(datafile,lineStr);
    variablenames = split(lineStr,',');
    getline(inputfile,lineStr);
    if(isweight){
        weightname = bmid(lineStr, lineStr.find('=') + 1, lineStr.size());
        for(size_t i=0 ; i<variablenames.size() ; ++i){
            if(variablenames[i] == weightname)
                weightLoc = i;
        }
    }else{
        weightLoc = std::numeric_limits<size_t>::max();
    }
    getline(inputfile,lineStr);
    Tcount = std::atol(bmid(lineStr, lineStr.find(':') + 1, lineStr.size()).c_str());
    CTables.reserve(Tcount); // reserve capacity for CTables
    if(concurrency>=Tcount){
        TperThread = 1;
        CTremainder = 0;
    }else{
        TperThread = Tcount / concurrency; // set tables per thread
        CTremainder = Tcount % concurrency; // see how many tables are left over
    }
    for(size_t i=0 ; i<3 ; ++i)
        getline(inputfile,lineStr); // reads blank line, dotted line, and blank line

    // now we will begin looping through our tables in the input file

    for(size_t i=0 ; i<Tcount ; ++i){
        size_t tgcount, rgcount, totaltabs, totalrows, rcount, ccount;
        tgcount = rgcount = totalrows = totaltabs = rcount = ccount = 0;
        bool tableoptions[12]; // array to organize and store optional checkbox values from report
        double CLevels[3]; // 3 confidence levels
        std::string tname;

        getline(inputfile,lineStr); // skip table name
        tname = bmid(lineStr, lineStr.find_first_of(':') + 1, lineStr.size());
        getline(inputfile,lineStr); // skip blank line
        // extract boolean values
        for(size_t j=0 ; j<12 ; ++j){
            getline(inputfile,lineStr);
            tableoptions[j] = (bmid(lineStr, lineStr.find('=') + 1, lineStr.size()) == "True") ? true : false;
        }

        getline(inputfile,lineStr); // read confidence levels
        std::vector<std::string> split1 = split(bmid(lineStr, lineStr.find(':') + 1, lineStr.size()),',');
        for(size_t j=0 ; j<3 ; ++j){
            CLevels[j] = std::atof(bmid(split1[j], 0, split1[j].size() - 1).c_str());
        }

        // extract counts
        getline(inputfile, lineStr);
            totaltabs = std::atol(bmid(lineStr, lineStr.find('=') + 1, lineStr.size()).c_str());
        getline(inputfile, lineStr);
            totalrows = std::atol(bmid(lineStr, lineStr.find('=') + 1, lineStr.size()).c_str());
        getline(inputfile, lineStr);
            tgcount = std::atol(bmid(lineStr, lineStr.find('=') + 1, lineStr.size()).c_str());
        getline(inputfile, lineStr);
            rgcount = std::atol(bmid(lineStr, lineStr.find('=') + 1, lineStr.size()).c_str());
        CTable CT(CLevels, tableoptions, tname);

        for(size_t j=0 ; j<3 ; ++j)
            getline(inputfile,lineStr); // reads: blank,"tabsbegin:",blank

        for(size_t j=0 ; j<tgcount ; ++j){ // extract tabgroups
            getline(inputfile,lineStr);
            if(splice(lineStr, "GroupName: ", " E-tags:") == "Row Totals") continue;
            CT.addTabGroup(lineStr);
        }

        for(size_t j=0 ; j<totaltabs ; ++j){ // extract conditions for our ctab elements and conditions
            getline(inputfile, lineStr);
            if(splice(lineStr, "Parent: ", " E-tag:") == "Row Totals") continue;
            CT.addTabElement(lineStr, variablenames, ccount);
            ++ccount;
        }

        for(size_t j=0 ; j<4 ; ++j)
            getline(inputfile, lineStr); // reads: "TabsEnd", blank, "RowsBegin:", blank

        for(size_t j=0 ; j<rgcount ; ++j){ // extract information for our row groups
            getline(inputfile,lineStr);
            std::string gname = splice(lineStr, "GroupName: ", " Profile:");
            if((gname == "Tab Totals") || (gname == "Descriptives")) continue;
            CT.addRowGroup(lineStr, variablenames, tgcount, ccount);
        }

        for(size_t j=0 ; j<totalrows ; ++j){ // extract information for row elements and conditions
            getline(inputfile,lineStr);
            std::string gname = splice(lineStr, "Parent: ", " Parent Row Number:");
            if((gname == "Tab Totals") || (gname == "Descriptives")) continue;
            CT.addRowElement(lineStr, variablenames, rcount);
            ++rcount;
        }

        for(size_t j=0 ; j<4 ; ++j)
            getline(inputfile,lineStr); // reads: "RowsEnd", blank, "End Table", blank

        CTables.push_back(CT); // add CT to CTables container
    }
    inputfile.close(); // don't need inputfile open anymore
    // end input report----------------------------------------------------------------------------------

    // move appdir to tablereports
    appdir = bleft(appdir, std::string(buffer).find("Crosstabulation") + 15) + "Table Reports\\";
    std::thread threads[concurrency];
    size_t t = 0;
    for(size_t i=0 ; i<concurrency ; ++i){
        if(CTremainder>0){ // make sure we account for left over tables (e.g. 5 tables with 4 threads leaves 1 left over)
            ++TperThread;
            threads[i] = std::thread((runfctor()), std::ref(CTables), t, TperThread, std::ref(datapath), weightLoc, std::ref(guard));
            --CTremainder;
            t += TperThread;
            --TperThread;
            if(t>=Tcount) break;
        }else{
            threads[i] = std::thread((runfctor()), std::ref(CTables), t, TperThread, std::ref(datapath), weightLoc, std::ref(guard));
            t += TperThread;
            if(t>=Tcount) break;
        }
    }
    for(size_t i=0 ; i<concurrency ; ++i){
        if(threads[i].joinable()){
            threads[i].join();
            shared_print(guard, "thread %zu joined successfully\n", i);
        }
    }
    if(concurrency>=Tcount){
        CTremainder = 0;
    }else{
        CTremainder = Tcount % concurrency; // see how many tables are left over
    }
    t = 0;
    for(size_t i=0 ; i<CTables.size() ; ++i){
        CTables[i].TableReport(appdir, true, true, false, isweight);
    }
    appdir = bleft(appdir, appdir.find("Crosstabulation\\") + std::string("Crosstabulation\\").size() - 1);
    finalflag.open(appdir + "FLAG.txt");
    // Finished table counts-----------------------------------------------------------------------------
    return 0;
}
