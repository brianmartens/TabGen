#include "CTable.h"
#include <utility>
#include <fstream>
#include <mutex>
#include "bmath.h"
#include "Algebra.h"
// forward declarations

// bstr
std::string bleft(const std::string &sourceStr, const size_t& index);
std::string bright(const std::string &sourceStr, const size_t& indexbegin);
std::string bmid(const std::string &sourceStr, const size_t& indexbegin, const size_t& indexend, bool inclusivity = false);
std::string filter(const std::string &sourceStr, const std::string& filterOut);
std::vector<std::string> split(const std::string& sourceStr, const char& delimiter, bool check_delimiters = true);
std::string splice(const std::string& sourceStr, const std::string& leftStr, const std::string& rightStr);
void allocated_split(const std::string& sourceStr, std::vector<std::string>& destination, const char& delimiter, bool check_delimiters = true);
int shared_print (std::mutex& locker, const char *__format, ...);

std::string ops[] = {"==","!=",">=","<=",">","<"};

infoSet::infoSet(const std::string& condstr, const std::vector<std::string>& variable_set){
    for(size_t i=0 ; i<6 ; ++i){
        if(condstr.find(ops[i]) != std::string::npos){
            operation = i;
            break;
        }
    }
    if(operation != 4 && operation != 5){
        if(is_numeric(bmid(condstr, condstr.find(ops[operation]) + 1, condstr.size()))){ // is_numeric returns true if string solely contains an int or double
            valuenum = std::stof(bmid(condstr, condstr.find(ops[operation]) + 1, condstr.size())); // operators with 2 chars
            valuestr.clear();
        }else{
            valuenum = std::numeric_limits<double>::max();
            valuestr = bmid(condstr, condstr.find(ops[operation]) + 1, condstr.size());
        }
    }else{
        if(is_numeric(bmid(condstr, condstr.find(ops[operation]), condstr.size()))){
            valuenum = std::stof(bmid(condstr, condstr.find(ops[operation]), condstr.size())); // operators with 1 char
            valuestr.clear();
        }else{
            valuenum = std::numeric_limits<double>::max();
            valuestr = bmid(condstr, condstr.find(ops[operation]), condstr.size());
        }
    }
    std::string name = bleft(condstr, condstr.find(ops[operation]) - 1);
    for(size_t i=0 ; i<variable_set.size() ; ++i){
        if(name == variable_set[i])
            varLoc = i;
    }
}

const size_t& infoSet::getVloc() const { return varLoc;}
const size_t& infoSet::getOP() const { return operation;}
const float& infoSet::getNum() const { return valuenum;}
const std::string& infoSet::getVstr() const { return valuestr;}

condition_set::condition_set(){
    std::vector<std::vector<infoSet> >();
}

condition_set::condition_set(const std::string& condstr, const std::vector<std::string>& variable_set){
    std::vector<std::string> or_split = split(condstr, ',');
    infoMap.reserve(or_split.size());
    for(size_t i=0 ; i<or_split.size() ; ++i){
        std::vector<std::string> and_split = split(or_split[i], '&');
        std::vector<infoSet> iSets;
        if(and_split.size() > 1){
            iSets.reserve(and_split.size());
            for(size_t j=0 ; j<and_split.size() ; ++j)
                iSets.push_back(infoSet(and_split[j], variable_set));
        }else{
            iSets.reserve(1);
            iSets.push_back(infoSet(or_split[i], variable_set));
        }
        infoMap.push_back(iSets);
    }
}

const bool condition_set::test(const std::vector<std::string>& dataline, std::mutex& mu) const{
    if(infoMap.size() > 0){ // just in case a calculated element is sent for testing
        for(size_t i=0 ; i<infoMap.size() ; ++i){ // For each Or set
            bool AndCheck = true; // AndCheck will be false if one of the AND conditions fails
            float datanum; // this value will be the numeric value in the dataline through std::stod
            for(size_t j=0 ; j<infoMap[i].size() ; ++j){ // For each AND condition within the OR
                if(is_numeric(dataline[infoMap[i][j].getVloc()])){
                    datanum = std::stof(dataline[infoMap[i][j].getVloc()]);
                }else{
                    datanum = std::numeric_limits<float>::max();
                }
                // {"==","!=",">=","<=",">","<"};
                if(datanum != std::numeric_limits<float>::max()){ // if data value was found to be numeric, then this will not be max value
                    if(infoMap[i][j].getNum() != std::numeric_limits<float>::max()){ // if conditional value was found to be numeric, then this will not be max value
                        if(infoMap[i][j].getOP() == 0){ // ==
                            if(infoMap[i][j].getNum() != datanum)
                                AndCheck = false;
                        }else if(infoMap[i][j].getOP() == 1){ // !=
                            if(infoMap[i][j].getNum() == datanum)
                                AndCheck = false;
                        }else if(infoMap[i][j].getOP() == 2){ // >=
                            if(infoMap[i][j].getNum() < datanum)
                                AndCheck = false;
                        }else if(infoMap[i][j].getOP() == 3){ // <=
                            if(infoMap[i][j].getNum() > datanum)
                                AndCheck = false;
                        }else if(infoMap[i][j].getOP() == 4){ // >
                            if(infoMap[i][j].getNum() <= datanum)
                                AndCheck = false;
                        }else if(infoMap[i][j].getOP() == 5){ // <
                            if(infoMap[i][j].getNum() >= datanum)
                                AndCheck = false;
                        }else
                            AndCheck=false;
                    }else{
                        AndCheck = false; // comparing numeric vs. non-numeric
                    }
                }else{ // if datavalue is non-numeric
                    if(infoMap[i][j].getNum() != std::numeric_limits<float>::max()){ // if InfoSet value is numeric
                        AndCheck = false; // comparison b/w numeric and non-numeric data
                        break; // continue to next OR
                    }else{ // comparing non-numerics lexicographically
                        if(infoMap[i][j].getOP() == 0){ // ==
                            if(infoMap[i][j].getVstr() != dataline[infoMap[i][j].getVloc()])
                                AndCheck = false;
                        }else if(infoMap[i][j].getOP() == 1){ // !=
                            if(infoMap[i][j].getVstr() == dataline[infoMap[i][j].getVloc()])
                                AndCheck = false;
                        }else if(infoMap[i][j].getOP() == 2){ // >=
                            if(infoMap[i][j].getVstr() < dataline[infoMap[i][j].getVloc()])
                                AndCheck = false;
                        }else if(infoMap[i][j].getOP() == 3){ // <=
                            if(infoMap[i][j].getVstr() > dataline[infoMap[i][j].getVloc()])
                                AndCheck = false;
                        }else if(infoMap[i][j].getOP() == 4){ // >
                            if(infoMap[i][j].getVstr() <= dataline[infoMap[i][j].getVloc()])
                                AndCheck = false;
                        }else if(infoMap[i][j].getOP() == 5){ // <
                            if(infoMap[i][j].getVstr() >= dataline[infoMap[i][j].getVloc()])
                                AndCheck = false;
                        }else
                            AndCheck=false;
                    }
                }
                if(!AndCheck) break;
            }
            if(AndCheck){ // if after we fully check the AND vector, all checks are satisfied, then an OR condition has resolved to true and we can return true
                return true;
            }
        }
    }
    return false; // we return false if the set of OR conditions returns false for all conditions
}

size_t condition_set::MapSize() const {
    size_t msize = 0;
    for(size_t i=0 ; i<infoMap.size() ; ++i)
        msize += infoMap[i].size();
    return msize;
}

void condition_set::printMap() const {
    for(auto it=infoMap.begin() ; it!=infoMap.end() ; ++it){ // for each OR vector
        for(auto it2=it->begin() ; it2!=it->end() ; ++it2){ // for each AND infoSet
            if(it2->getNum() != std::numeric_limits<double>::max())
                printf("%zu\t%s\t%f\n",it2->getVloc(),ops[it2->getOP()].c_str(),it2->getNum());
            else
                printf("%zu\t%s\t%s\n",it2->getVloc(),ops[it2->getOP()].c_str(),it2->getVstr().c_str());
        }
    }
}

void condition_set::printMap(std::mutex& mu) const {
    for(auto it=infoMap.begin() ; it!=infoMap.end() ; ++it){ // for each OR vector
        for(auto it2=it->begin() ; it2!=it->end() ; ++it2){ // for each AND infoSet
            if(it2->getNum() != std::numeric_limits<double>::max())
                shared_print(mu, "%zu\t%s\t%f\n",it2->getVloc(),ops[it2->getOP()].c_str(),it2->getNum());
            else
                shared_print(mu, "%zu\t%s\t%s\n",it2->getVloc(),ops[it2->getOP()].c_str(),it2->getVstr().c_str());
        }
    }
}

tab_element::tab_element(const std::string& reportline, const std::vector<std::string>& variable_set, const size_t& index){
    name = splice(reportline, "Element Name: ", " Calculated:");
    tag = splice(reportline, "E-tag: ", " Parent Group Number:");
    calculated = (splice(reportline, "Calculated: ", " Conditions")=="True");
    if(!calculated)
        Cset = condition_set(splice(reportline, "Conditions: ", " Parent:"), variable_set);
    else
        condstr = splice(reportline, "Conditions: ", " Parent:");
    indexLoc = index;
}

const condition_set& tab_element::getConds() const{
    return Cset;
}

const std::string& tab_element::getCondstr() const{
    return condstr;
}

const std::string& tab_element::Ename() const{
    return name;
}

const std::string& tab_element::Etag() const{
    return tag;
}

const bool& tab_element::calc() const{
    return calculated;
}

const size_t& tab_element::getLoc() const{
    return indexLoc;
}

row_element::row_element(const std::string& reportline, const std::vector<std::string>& variable_set, const size_t& index){
    name = splice(reportline, "Element Name: ", " Calculated:");
    calculated = (splice(reportline, "Calculated: ", " Conditions:") == "True");
    if(!calculated)
        Cset = condition_set(splice(reportline, "Conditions: ", " Parent:"), variable_set);
    else
        condstr = splice(reportline, "Conditions: ", " Parent:");
    indexLoc = index;
}

const condition_set& row_element::getCset() const{
    return Cset;
}

const std::string& row_element::getCondstr() const{
    return condstr;
}

const std::string& row_element::Ename() const{
    return name;
}

const bool& row_element::calc() const{
    return calculated;
}

const size_t& row_element::getLoc() const{
    return indexLoc;
}

tab_group::tab_group(const std::string& reportline){
    name = splice(reportline, "GroupName: ", " E-tags:");
}

const std::string& tab_group::Gname() const{
    return name;
}

size_t tab_group::tabcount() const{
    return etabs.size();
}

const tab_element& tab_group::getTab(const size_t& index) const{
    return etabs[index];
}

void tab_group::add_element(const std::string& reportline, const std::vector<std::string>& variable_set, const size_t& index){
    etabs.push_back(tab_element(reportline, variable_set, index));
}

row_group::row_group(const std::string& reportline, const size_t& tabgroupcount, const size_t& totaltabs){
    name = splice(reportline, "GroupName: ", " Profile:");
    anova_results.resize(tabgroupcount);
    chi2_results.resize(tabgroupcount);
    uTab_Totals.resize(totaltabs);
    wTab_Totals.resize(totaltabs);
    ppuztest.resize(totaltabs);
    pwuztest.resize(totaltabs);
    pputtest.resize(totaltabs);
    pwuttest.resize(totaltabs);
    ppwztest.resize(totaltabs);
    pwwztest.resize(totaltabs);
    ppwttest.resize(totaltabs);
    pwwttest.resize(totaltabs);
    meandata.resize(totaltabs);
    vardata.resize(totaltabs);
    devdata.resize(totaltabs);
    ucounts.resize(totaltabs);
    wcounts.resize(totaltabs);
    upercs.resize(totaltabs);
    wpercs.resize(totaltabs);
}

const row_element& row_group::getRow(const size_t& index) const{
    return erows[index];
}

const std::string& row_group::Gname() const{
    return name;
}

void row_group::add_element(const std::string & reportline, const std::vector<std::string>& variable_set, const size_t& index){
    erows.push_back(row_element(reportline, variable_set, index));
    uRow_Totals.push_back(size_t());
    wRow_Totals.push_back(double());
    for(size_t i=0 ; i<ucounts.size() ; ++i){
        ucounts[i].push_back(size_t());
        wcounts[i].push_back(double());
        upercs[i].push_back(double());
        wpercs[i].push_back(double());
        pwwztest[i].push_back(std::string());
        ppwztest[i].push_back(std::string());
        pwuztest[i].push_back(std::string());
        ppuztest[i].push_back(std::string());
    }
}

size_t row_group::rowcount() const{
    return erows.size();
}

void row_group::ucount(const size_t& rindex, const size_t& tindex){
    ++ucounts[tindex][rindex];
}

void row_group::wcount(const size_t& rindex, const size_t& tindex, const double& weightvalue){
    wcounts[tindex][rindex] += weightvalue;
}

void row_group::URTotal(const size_t& rindex){
    ++uRow_Totals[rindex];
}

void row_group::WRTotal(const size_t& rindex, const double& weightvalue){
    wRow_Totals[rindex] += weightvalue;
}

void row_group::UCTotal(const size_t& tindex){
    ++uTab_Totals[tindex];
}

void row_group::WCTotal(const size_t& tindex, const double& weightvalue){
    wTab_Totals[tindex] += weightvalue;
}

const std::vector<size_t>& row_group::getuRowTotals() const{
    return uRow_Totals;
}

const std::vector<double>& row_group::getwRowTotals() const{
    return wRow_Totals;
}

const std::vector<size_t>& row_group::getuTabTotals() const{
    return uTab_Totals;
}

const std::vector<double>& row_group::getwTabTotals() const{
    return wTab_Totals;
}

void row_group::setucount(const size_t& rindex, const size_t& tindex, const size_t& value){
    ucounts[tindex][rindex] = value;
}

void row_group::setwcount(const size_t& rindex, const size_t& tindex, const double& value){
    wcounts[tindex][rindex] = value;
}

void row_group::ubase(const size_t& basevalue){
    ubaseSize = basevalue;
}

void row_group::wbase(const double& basevalue){
    wbaseSize = basevalue;
}

void row_group::runstats(const bool& ztpp, const bool& ztpw, const bool& ttpp, const bool& ttpw, const bool& Chi2, const bool& Anova){}

const std::vector<std::vector<size_t> >& row_group::getucounts() const{
    return ucounts;
}

const std::vector<std::vector<double> >& row_group::getwcounts() const{
    return wcounts;
}

const std::vector<std::vector<double> >& row_group::getupercs() const{
    return upercs;
}

const std::vector<std::vector<double> >& row_group::getwpercs() const{
    return wpercs;
}

CTable::CTable(const double* confLvls, const bool* tablebools, const std::string& Tname){ // tells CTable how much memory to allocate for vectors
    RowTotal = *(tablebools + 0);
    TabTotal = *(tablebools + 1);
    Means = *(tablebools + 2);
    Var = *(tablebools + 3);
    StdDv = *(tablebools + 4);
    ZtestPP = *(tablebools + 5);
    TtestPP = *(tablebools + 6);
    ZtestPW = *(tablebools + 7);
    TtestPW = *(tablebools + 8);
    ANOVA = *(tablebools + 9);
    ChiSqr = *(tablebools + 10);
    Tablename = Tname;
    for(size_t i=0 ; i<3 ; ++i){
        Clevels[i] = *(confLvls + i);
    }
    totalrows = 0;
    totaltabs = 0;
}

void CTable::addRowGroup(const std::string& reportline, const size_t& tabgroupcount, const size_t& totaltabs){
    Rows.push_back(row_group(reportline, tabgroupcount, totaltabs));
}

void CTable::addTabGroup(const std::string& reportline){
    Tabs.push_back(tab_group(reportline));
}

void CTable::addRowElement(const std::string& reportline, const std::vector<std::string>& variable_set, const size_t& index){
    for(size_t i=0 ; i<Rows.size() ; ++i){
        if(Rows[i].Gname() == splice(reportline, "Parent: ", " Parent Row Number:")){
            Rows[i].add_element(reportline, variable_set, index);
            ++totalrows;
            return;
        }
    }
}

void CTable::addTabElement(const std::string& reportline, const std::vector<std::string>& variable_set, const size_t& index){
    for(size_t i=0 ; i<Tabs.size() ; ++i){
        if(Tabs[i].Gname() == splice(reportline, "Parent: ", " E-tag:")){
            Tabs[i].add_element(reportline, variable_set, index);
            ++totaltabs;
            return;
        }
    }
}

const size_t CTable::tabgroupcount() const{
    return Tabs.size();
}

const size_t CTable::rowgroupcount() const{
    return Rows.size();
}

const row_group& CTable::getRowGroup(const size_t& index) const{
    return Rows[index];
}

const tab_group& CTable::getTabGroup(const size_t& index) const{
    return Tabs[index];
}

void CTable::RunCounts(const std::vector<std::string>& dataline, const size_t& weightLoc, std::mutex& mu){
    double weight = 0.0;
    if(weightLoc != std::numeric_limits<size_t>::max())
        weight = std::stod(dataline[weightLoc]);
    for(size_t i=0 ; i<Rows.size() ; ++i){
        bool RGFLAG = false;
        for(size_t j=0 ; j<Rows[i].rowcount() ; ++j){
            if(RGFLAG) break;
            if(Rows[i].getRow(j).calc()) continue;
            if(Rows[i].getRow(j).getCset().test(dataline, mu)){
                RGFLAG = true;
                Rows[i].URTotal(j);
                if(weightLoc != std::numeric_limits<size_t>::max())
                    Rows[i].WRTotal(j, weight);
            }
            if(RGFLAG){
                for(size_t k=0 ; k<Tabs.size() ; ++k){
                    bool TGFLAG = false;
                    for(size_t l=0 ; l<Tabs[k].tabcount() ; ++l){
                        if(TGFLAG) break;
                        if(Tabs[k].getTab(l).calc()) continue;
                        if(Tabs[k].getTab(l).getConds().test(dataline, mu)){
                            TGFLAG = true;
                            Rows[i].ucount(j,Tabs[k].getTab(l).getLoc());
                            Rows[i].UCTotal(Tabs[k].getTab(l).getLoc());
                            if(weightLoc != std::numeric_limits<size_t>::max()){
                                Rows[i].wcount(j, Tabs[k].getTab(l).getLoc(), weight);
                                Rows[i].WCTotal(Tabs[k].getTab(l).getLoc(), weight);
                            }
                        }
                    }
                }
            }
        }
    }
}

void CTable::calcmembers(const size_t& weightLoc, std::mutex& mu){
    std::vector<std::string> rnames;
    std::vector<std::string> cnames;
    for(size_t i=0 ; i<Rows.size() ; ++i){
        for(size_t j=0 ; j<Rows[i].rowcount() ; ++j){
            rnames.push_back(Rows[i].getRow(j).Ename());
        }
    }
    for(size_t i=0 ; i<Tabs.size() ; ++i){
        for(size_t j=0 ; j<Tabs[i].tabcount() ; ++j){
            cnames.push_back(Tabs[i].getTab(j).Ename());
        }
    }
    for(size_t i=0 ; i<Rows.size() ; ++i){
        for(size_t j=0 ; j<Rows[i].rowcount() ; ++j){
            if(Rows[i].getRow(j).calc()){
                for(size_t L=0 ; L<Tabs.size() ; ++L){
                    for(size_t M=0 ; M<Tabs[L].tabcount() ; ++M){
                        if(!(Tabs[L].getTab(M).calc())){
                            Rows[i].setucount(j,
                                              Tabs[L].getTab(M).getLoc(),
                                              Algebra(Rows[i].getRow(j).getCondstr(),
                                                      rnames,
                                                      Rows[i].getwcounts()[Tabs[L].getTab(M).getLoc()]
                                                      )
                                              );
                            if(weightLoc != std::numeric_limits<size_t>::max()){
                                Rows[i].setwcount(j,
                                                  Tabs[L].getTab(M).getLoc(),
                                                  Algebra(Rows[i].getRow(j).getCondstr(),
                                                          rnames,
                                                          Rows[i].getwcounts()[Tabs[L].getTab(M).getLoc()]
                                                          )
                                                  );
                            }
                        }
                    }
                    for(size_t M=0 ; M<Tabs[L].tabcount() ; ++M){
                        if(Tabs[L].getTab(M).calc()){
                            Rows[i].setucount(j,
                                              Tabs[L].getTab(M).getLoc(),
                                              C_Algebra(Tabs[L].getTab(M).getCondstr(),
                                                        cnames,
                                                        Rows[i].getucounts(),
                                                        j)
                                              );
                            if(weightLoc != std::numeric_limits<size_t>::max()){
                                Rows[i].setwcount(j,
                                                  Tabs[L].getTab(M).getLoc(),
                                                  C_Algebra(Tabs[L].getTab(M).getCondstr(),
                                                            cnames,
                                                            Rows[i].getwcounts(),
                                                            j)
                                                  );
                            }
                        }
                    }
                }
            }
        }
    }
}

void CTable::RunPercents(){

}

void CTable::RunStats(){

}

void CTable::TableReport(const std::string& outputDIR, const bool counts, const bool percs, const bool stats, const bool& weight) const{
    for(size_t i=0 ; i<Tabs.size() ; ++i){
        for(size_t j=0 ; j<Tabs[i].tabcount() ; ++j)
            Tabs[i].getTab(j).getConds().printMap();
    }
    std::ofstream rfile(outputDIR + Tablename + ".txt");
    rfile.precision(10);
    if(!(counts) && !(percs) && !(stats)) return;
    rfile << "TableName: " << Tablename << '\n';
    if(counts){
        for(size_t i=0 ; i<Rows.size() ; ++i){
            if(TabTotal){
                rfile << "uTabTotals{";
                for(size_t j=0 ; j<Rows[i].getuTabTotals().size() ; ++j){
                    if(j >= Rows[i].getuTabTotals().size()-1)
                        rfile << Rows[i].getuTabTotals()[j] << '}' << '\n';
                    else
                        rfile << Rows[i].getuTabTotals()[j] << ',';
                }
                if(weight){
                    rfile << "wTabTotals{";
                    for(size_t j=0 ; j<Rows[i].getwTabTotals().size() ; ++j){
                        if(j >= Rows[i].getwTabTotals().size()-1)
                            rfile << Rows[i].getwTabTotals()[j] << '}' << '\n';
                        else
                            rfile << Rows[i].getwTabTotals()[j] << ',';
                    }
                }
            }
            if(RowTotal){
                rfile << "uRowTotals{";
                for(size_t j=0 ; j<Rows[i].getuRowTotals().size() ; ++j){
                    if(j >= Rows[i].getuRowTotals().size()-1)
                        rfile << Rows[i].getuRowTotals()[j] << '}' << '\n';
                    else
                        rfile << Rows[i].getuRowTotals()[j] << ',';
                }
                if(weight){
                    rfile << "wRowTotals{";
                    for(size_t j=0 ; j<Rows[i].getwRowTotals().size() ; ++j){
                        if(j >= Rows[i].getwRowTotals().size()-1)
                            rfile << Rows[i].getwRowTotals()[j] << '}' << '\n';
                        else
                            rfile << Rows[i].getwRowTotals()[j] << ',';
                    }
                }
            }
            rfile << "uCounts:\n";
            for(size_t j=0 ; j<Rows[i].rowcount() ; ++j){
                rfile << Rows[i].getRow(j).Ename() << '{';
                for(size_t k=0 ; k<Rows[i].getucounts().size() ; ++k){
                    if(k >= Rows[i].getucounts().size()-1)
                        rfile << Rows[i].getucounts()[k][j] << '}' << '\n';
                    else
                        rfile << Rows[i].getucounts()[k][j] << ',';
                }
            }
            if(weight){
                rfile << "wCounts:\n";
                for(size_t j=0 ; j<Rows[i].rowcount() ; ++j){
                    rfile << Rows[i].getRow(j).Ename() << '{';
                    for(size_t k=0 ; k<Rows[i].getwcounts().size() ; ++k){
                        if(k >= Rows[i].getwcounts().size()-1)
                            rfile << Rows[i].getwcounts()[k][j] << '}' << '\n';
                        else
                            rfile << Rows[i].getwcounts()[k][j] << ',';
                    }
                }
            }
        }
    }
    if(percs){

    }
    if(stats){

    }
    rfile.close();
}

void runfctor::operator()(std::vector<CTable>& TableContainer, const size_t index_begin, const size_t length,
                            const std::string& datapath, const size_t& weightLoc, std::mutex& mu){
    std::ifstream datafile(datapath);
    std::string linestr;
    std::vector<std::string> dataline;
    getline(datafile, linestr); // skip variable names
    dataline = split(linestr,',');
    size_t counter = 0;
    while(getline(datafile, linestr)){
        ++counter;
        if(counter%10000 == 0) shared_print(mu, "Line %zu\n", counter);
        allocated_split(linestr, dataline, ',');
        for(size_t i=index_begin ; i<index_begin + length ; ++i){
            TableContainer[i].RunCounts(dataline, weightLoc, mu);
        }
    }
}

void perfctor::operator()(std::vector<CTable>& tablecontainer, const size_t index_begin, const size_t length, std::mutex& mu){
    for(size_t i=index_begin ; i<index_begin+length ; ++i){

    }
}
