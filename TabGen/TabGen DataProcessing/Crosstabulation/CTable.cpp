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

# define MaxSize_t std::numeric_limits<size_t>::max()

infoSet::infoSet(const std::string& condstr, const std::vector<std::string>& variable_set){
    for(size_t i=0 ; i<6 ; ++i){
        if(condstr.find(ops[i]) != std::string::npos){
            operation = i;
            break;
        }
    }
    if(operation != 4 && operation != 5){
        if(is_numeric(bmid(condstr, condstr.find(ops[operation]) + 1, condstr.size()))){ // is_numeric returns true if string solely contains an int or double
            valuenum = std::atof(bmid(condstr, condstr.find(ops[operation]) + 1, condstr.size()).c_str()); // operators with 2 chars
            valuestr.clear();
        }else{
            valuenum = std::numeric_limits<double>::max();
            valuestr = bmid(condstr, condstr.find(ops[operation]) + 1, condstr.size());
        }
    }else{
        if(is_numeric(bmid(condstr, condstr.find(ops[operation]), condstr.size()))){
            valuenum = std::atof(bmid(condstr, condstr.find(ops[operation]), condstr.size()).c_str()); // operators with 1 char
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
const double& infoSet::getNum() const { return valuenum;}
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

bool condition_set::test(const std::vector<std::string>& dataline, std::mutex& mu) const{
    if(infoMap.size() > 0){ // just in case a calculated element is sent for testing
        for(size_t i=0 ; i<infoMap.size() ; ++i){ // For each Or set
            bool AndCheck = true; // AndCheck will be false if one of the AND conditions fails
            float datanum; // this value will be the numeric value in the dataline through std::stod
            for(size_t j=0 ; j<infoMap[i].size() ; ++j){ // For each AND condition within the OR
                if(is_numeric(dataline[infoMap[i][j].getVloc()])){
                    datanum = std::atof(dataline[infoMap[i][j].getVloc()].c_str());
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
                            if(infoMap[i][j].getNum() > datanum)
                                AndCheck = false;
                        }else if(infoMap[i][j].getOP() == 3){ // <=
                            if(infoMap[i][j].getNum() < datanum)
                                AndCheck = false;
                        }else if(infoMap[i][j].getOP() == 4){ // >
                            if(infoMap[i][j].getNum() >= datanum)
                                AndCheck = false;
                        }else if(infoMap[i][j].getOP() == 5){ // <
                            if(infoMap[i][j].getNum() <= datanum)
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
                            if(infoMap[i][j].getVstr() > dataline[infoMap[i][j].getVloc()])
                                AndCheck = false;
                        }else if(infoMap[i][j].getOP() == 3){ // <=
                            if(infoMap[i][j].getVstr() < dataline[infoMap[i][j].getVloc()])
                                AndCheck = false;
                        }else if(infoMap[i][j].getOP() == 4){ // >
                            if(infoMap[i][j].getVstr() >= dataline[infoMap[i][j].getVloc()])
                                AndCheck = false;
                        }else if(infoMap[i][j].getOP() == 5){ // <
                            if(infoMap[i][j].getVstr() <= dataline[infoMap[i][j].getVloc()])
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
            printf("%zu\t%s\t%f\n",it2->getVloc(),ops[it2->getOP()].c_str(),it2->getNum());
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

row_group::row_group(const std::string& reportline, const std::vector<std::string> varlist, const size_t& tabgroupcount, const size_t& totaltabs){
    if(splice(reportline, "Profile: ", " Control Variable:") != "NULL"){
        std::vector<std::string> profstr = split(splice(reportline, "Profile: ", " Control Variable:"), ',');
        for(size_t i=0 ; i<profstr.size() ; ++i){
            size_t j = profstr[i].find(':');
            profile.push_back(std::pair<std::string, float>(bmid(profstr[i], 1, j-1, true),
                                                            std::atof(bmid(profstr[i], j, profstr[i].size() - 1).c_str())
                                                            ));
        }
    }
    cvar =  bmid(reportline, reportline.find_last_of(':') + 1, reportline.size());
    if(cvar.size() > 0){
        for(size_t i=0 ; i<varlist.size() ; ++i){
            if(cvar == varlist[i]){
                 cvLoc = i;
                 break;
            }
        }
    }else{
        cvLoc = MaxSize_t;
    }
    name = splice(reportline, "GroupName: ", " Profile:");
    anova_results.resize(tabgroupcount);
    chi2_results.resize(tabgroupcount);
    uTab_Totals.resize(totaltabs);
    wTab_Totals.resize(totaltabs);
    uTab_PTotals.resize(totaltabs);
    wTab_PTotals.resize(totaltabs);
    ETbase = ubaseSize = wbaseSize = Tmean = wTmean = Tvar = wTvar = Tdev = wTdev = 0;
    ETdata.push_back(0.0);
    ETdata.push_back(0.0);
    ECbase.resize(totaltabs);
    ECdata.reserve(totaltabs);
    for(size_t i=0 ; i<totaltabs ; ++i){
        ECdata.push_back(std::vector<double>());
        ECdata[i].push_back(double(0.0));
        ECdata[i].push_back(double(0.0));
    }
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
    wmeandata.resize(totaltabs);
    wvardata.resize(totaltabs);
    wdevdata.resize(totaltabs);
    ucounts.resize(totaltabs);
    wcounts.resize(totaltabs);
    upercs.resize(totaltabs);
    wpercs.resize(totaltabs);
}

void row_group::add_element(const std::string & reportline, const std::vector<std::string>& variable_set, const size_t& index){
    erows.push_back(row_element(reportline, variable_set, index));
    uRow_Totals.push_back(size_t(0));
    wRow_Totals.push_back(double(0.0));
    uRow_PTotals.push_back(double(0.0));
    wRow_PTotals.push_back(double(0.0));
    ERbase.push_back(double(0.0));
    ERdata.push_back(std::vector<double>(0.0, 0.0));
    for(size_t i=0 ; i<ucounts.size() ; ++i){
        ucounts[i].push_back(size_t(0));
        wcounts[i].push_back(double(0.0));
        upercs[i].push_back(double(0.0));
        wpercs[i].push_back(double(0.0));
        pwwztest[i].push_back(std::string());
        ppwztest[i].push_back(std::string());
        pwuztest[i].push_back(std::string());
        ppuztest[i].push_back(std::string());
    }
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

 void row_group::setUPTabTotal(const size_t& tindex, const double& value){
    uTab_PTotals[tindex] = value;
}

void row_group::setWPTabTotal(const size_t& tindex, const double& value){
    wTab_PTotals[tindex] = value;
}

void row_group::setUPRowTotal(const size_t& rindex, const double& value){
    uRow_PTotals[rindex] = value;
}

void row_group::setWPRowTotal(const size_t& rindex, const double& value){
    wRow_PTotals[rindex] = value;
}

void row_group::onlineTMVS(const double& x, const double& weight){
    double temp;
    if(weight != std::numeric_limits<double>::max()){
        if(wbaseSize==0){
            wTmean = x;
            wTvar = 0.0;
        }else{
            temp = wTmean;
            wTmean = temp + ((weight/wbaseSize)*(x - temp));
            wTvar = ((wbaseSize - weight)*wTvar) + (weight*(x - temp)*(x - wTmean));
        }
        ETdata[0] += weight;
        ETdata[1] += pow(weight,2);
    }
    if(ubaseSize==0){
        Tmean = x;
        Tvar = 0.0;
    }else{
        temp = Tmean;
        Tmean = temp + ((x - temp) / ubaseSize);
        Tvar = ((ubaseSize - 1)*Tvar) + ((x - temp)*(x - Tmean));
    }
}

void row_group::onlineCMVS(const double& x, const double& weight, const size_t& tindex){
    double temp;
    if(weight != std::numeric_limits<double>::max()){
        if(wTab_Totals[tindex]==0){
            wmeandata[tindex] = x;
            wvardata[tindex] = 0.0;
        }else{
            temp = wmeandata[tindex];
            wmeandata[tindex] = temp + ((weight/wbaseSize)*(x - temp));
            wvardata[tindex] = ((wbaseSize - weight)*wvardata[tindex]) + (weight*(x - temp)*(x - wmeandata[tindex]));
        }
        ECdata[tindex][0] += weight;
        ECdata[tindex][1] += pow(weight,2);
    }
    if(uTab_Totals[tindex]==0){
        meandata[tindex] = x;
        vardata[tindex] = 0.0;
    }else{
        temp = meandata[tindex];
        meandata[tindex] = temp + ((x - temp) / ubaseSize);
        vardata[tindex] = ((ubaseSize - 1)*vardata[tindex]) + ((x - temp)*(x - meandata[tindex]));
    }
}

void row_group::setucount(const size_t& rindex, const size_t& tindex, const size_t& value){
    ucounts[tindex][rindex] = value;
}

void row_group::setwcount(const size_t& rindex, const size_t& tindex, const double& value){
    wcounts[tindex][rindex] = value;
}

void row_group::calctotals(const std::vector<std::string>& rnames, const std::vector<std::string>& cnames, const std::vector<tab_group>& Tabs, const bool& isweight){
    if(isweight){
        for(size_t i=0 ; i<ECbase.size() ; ++i){
            ECdata[i][0] = pow(ECdata[i][0],2);
            ECbase[i] = ECdata[i][0] / ECdata[i][1];
        }
        ETdata[0] = pow(ETdata[0],2);
        ETbase = ETdata[0] / ETdata[1];
    }
    for(size_t i=0 ; i<erows.size() ; ++i){
        if(erows[i].calc()){
            uRow_Totals[i] = Algebra(erows[i].getCondstr(), rnames, uRow_Totals);
            if(isweight) wRow_Totals[i] = Algebra(erows[i].getCondstr(), rnames, wRow_Totals);
        }
    }
    size_t counter = 0;
    for(size_t i=0 ; i<Tabs.size() ; ++i){
        for(size_t j=0 ; j<Tabs[i].tabcount() ; ++j){
            if(Tabs[i].getTab(j).calc()){
                uTab_Totals[counter] = Algebra(Tabs[i].getTab(j).getCondstr(), cnames, uTab_Totals);
                if(isweight) wTab_Totals[counter] = Algebra(Tabs[i].getTab(j).getCondstr(), cnames, wTab_Totals);
            }
            ++counter;
        }
    }
}

void row_group::ubase(){
    ++ubaseSize;
}

void row_group::wbase(const double& basevalue){
    wbaseSize += basevalue;
}

void row_group::setuperc(const size_t& rindex, const size_t& tindex, const double& value){
    upercs[tindex][rindex] = value;
}

void row_group::setwperc(const size_t& rindex, const size_t& tindex, const double& value){
    wpercs[tindex][rindex] = value;
}

void row_group::setuTvar(const double& var){
    Tvar = var;
}

void row_group::setwTvar(const double& wvar){
    wTvar = wvar;
}

void row_group::setTmean(const double& mean){
    Tmean = mean;
}

void row_group::setwTmean(const double& wmean){
    wTmean = wmean;
}

void row_group::setuRvar(const double& var, const size_t& rindex){
    vardata[rindex] = var;
}

void row_group::setwRvar(const double& wvar, const size_t& rindex){
    wvardata[rindex] = wvar;
}

void row_group::setRmean(const double& mean, const size_t& rindex){
    meandata[rindex] = mean;
}

void row_group::setwRmean(const double& wmean, const size_t& rindex){
    wmeandata[rindex] = wmean;
}

void row_group::runstats(const bool& ztpp, const bool& ztpw, const bool& ttpp, const bool& ttpw, const bool& Chi2, const bool& Anova){

}

const row_element& row_group::getRow(const size_t& index) const{
    return erows[index];
}

const std::string& row_group::Gname() const{
    return name;
}

size_t row_group::rowcount() const{
    return erows.size();
}

const std::string& row_group::getCvar() const{
    return cvar;
}

const size_t& row_group::CVLoc() const{
    return cvLoc;
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

const std::vector<double>& row_group::getuPRowTotals() const{
    return uRow_PTotals;
}

const std::vector<double>& row_group::getwPRowTotals() const{
    return wRow_PTotals;
}

const std::vector<double>& row_group::getuPTabTotals() const{
    return uTab_PTotals;
}

const std::vector<double>& row_group::getwPTabTotals() const{
    return wTab_PTotals;
}

const std::vector<std::pair<std::string, double> >& row_group::getprofile() const{
    return profile;
}

const double& row_group::getwBase() const{
    return wbaseSize;
}

const size_t& row_group::getuBase() const{
    return ubaseSize;
}

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

const double& row_group::getTmean() const{
    return Tmean;
}

const double& row_group::getTvar() const{
    return Tvar;
}

const double& row_group::getTdev() const{
    return Tdev;
}

const double& row_group::getwTmean() const{
    return wTmean;
}

const double& row_group::getwTvar() const{
    return wTvar;
}

const double& row_group::getwTdev() const{
    return wTdev;
}

const std::vector<double>& row_group::getmeandata() const{
    return meandata;
}

const std::vector<double>& row_group::getvardata() const{
    return vardata;
}

const std::vector<double>& row_group::getdevdata() const{
    return devdata;
}

const std::vector<double>& row_group::getwmeandata() const{
    return wmeandata;
}

const std::vector<double>& row_group::getwvardata() const{
    return wvardata;
}

const std::vector<double>& row_group::getwdevdata() const{
    return wdevdata;
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
    g_test = *(tablebools + 10);
    Tlink = *(tablebools + 11);
    Tablename = Tname;
    for(size_t i=0 ; i<3 ; ++i){
        Clevels[i] = *(confLvls + i);
    }
    totalrows = 0;
    totaltabs = 0;
}

void CTable::addRowGroup(const std::string& reportline, const std::vector<std::string> varlist, const size_t& tabgroupcount, const size_t& totaltabs){
    Rows.push_back(row_group(reportline, varlist, tabgroupcount, totaltabs));
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

const std::string& CTable::getTname() const{
    return Tablename;
}

void CTable::RunCounts(const std::vector<std::string>& dataline, const size_t& weightLoc, std::mutex& mu){
    double weight = 0.0;
    if(weightLoc != MaxSize_t){
        weight = std::atof(dataline[weightLoc].c_str());
    }else{
        weight = std::numeric_limits<double>::max();
    }
    bool totaled = false;
    bool ctotaled[Tabs.size() - 1];
    for(size_t i=0 ; i<Tabs.size() ; ++i){
        ctotaled[i] = false;
    }
    for(size_t i=0 ; i<Rows.size() ; ++i){
        size_t tindices[Tabs.size() - 1];
        if(!Tlink){
            totaled = false;
        }
        bool circuit1 = false;
        if(Rows[i].CVLoc() != MaxSize_t){
            if(dataline[Rows[i].CVLoc()].size() > 0){
                if(!totaled){
                    totaled = true;
                    if(Tlink){
                        for(size_t j=0 ; j<Rows.size() ; ++j){
                            Rows[j].ubase();
                            if(weightLoc != MaxSize_t){
                                Rows[j].wbase(weight);
                            }
                        }
                    }else{
                        Rows[i].ubase();
                        if(weightLoc != MaxSize_t){
                            Rows[i].wbase(weight);
                        }
                    }
                    for(size_t j=0 ; j<Tabs.size() ; ++j){
                        bool circuit2 = false;
                        for(size_t k=0 ; k<Tabs[j].tabcount() ; ++k){
                            if(Tabs[j].getTab(k).calc()) continue;
                            if(circuit2) break;
                            if(Tabs[j].getTab(k).getConds().test(dataline, mu)){
                                circuit2 = true;
                                tindices[j] = Tabs[j].getTab(k).getLoc();
                                if(Tlink){
                                    if(!ctotaled[j]){
                                        for(size_t R=0 ; R<Rows.size() ; ++R){
                                            Rows[R].UCTotal(Tabs[j].getTab(k).getLoc());
                                            if(weightLoc != MaxSize_t)
                                                Rows[R].WCTotal(Tabs[j].getTab(k).getLoc(), weight);
                                        }
                                        ctotaled[j] = true;
                                    }
                                }else{
                                    Rows[i].UCTotal(Tabs[j].getTab(k).getLoc());
                                    if(weightLoc != MaxSize_t)
                                        Rows[i].WCTotal(Tabs[j].getTab(k).getLoc(), weight);
                                }
                            }
                        }
                    }
                }
                if(Means || Var || StdDv){
                    if(Rows[i].getprofile().size() > 0){
                        for(size_t j=0 ; j<Rows[i].getprofile().size() ; ++j){
                            if(dataline[Rows[i].CVLoc()] == Rows[i].getprofile()[j].first){
                                Rows[i].onlineTMVS(Rows[i].getprofile()[j].second, weight);
                            }
                        }
                    }
                }
            }
        }
        for(size_t j=0 ; j<Rows[i].rowcount() ; ++j){
            if(circuit1) break;
            if(Rows[i].getRow(j).calc()) continue;
            if(Rows[i].getRow(j).getCset().test(dataline, mu)){
                if(!totaled){
                    for(size_t k=0 ; k<Tabs.size() ; ++k){
                        bool circuit2 = false;
                        for(size_t l=0 ; l<Tabs[k].tabcount() ; ++l){
                            if(Tabs[k].getTab(l).calc()) continue;
                            if(circuit2) break;
                            if(Tabs[k].getTab(l).getConds().test(dataline, mu)){
                                circuit2 = true;
                                tindices[k] = Tabs[k].getTab(l).getLoc();
                                if(Tlink){
                                    if(!ctotaled[k]){
                                        for(size_t R=0 ; R<Rows.size() ; ++R){
                                            Rows[R].UCTotal(Tabs[k].getTab(l).getLoc());
                                            if(weightLoc != MaxSize_t)
                                                Rows[R].WCTotal(Tabs[k].getTab(l).getLoc(), weight);
                                        }
                                        ctotaled[k] = true;
                                    }
                                }else{
                                    Rows[i].UCTotal(Tabs[k].getTab(l).getLoc());
                                    if(weightLoc != MaxSize_t)
                                        Rows[i].WCTotal(Tabs[k].getTab(l).getLoc(), weight);
                                }
                            }
                        }
                    }
                    if(Tlink){
                        for(size_t k=0 ; k<Rows.size() ; ++k){
                            Rows[k].ubase();
                            if(weightLoc != MaxSize_t){
                                Rows[k].wbase(weight);
                            }
                        }
                    }else{
                        Rows[i].ubase();
                        if(weightLoc != MaxSize_t){
                            Rows[i].wbase(weight);
                        }
                    }
                    totaled = true;
                }
                circuit1 = true;
                Rows[i].URTotal(j);
                if(weightLoc != MaxSize_t){
                    Rows[i].WRTotal(j, weight);
                    for(size_t k=0 ; k<Tabs.size() ; ++k){
                        if(tindices[k] != MaxSize_t){
                            Rows[i].ucount(j, tindices[k]);
                            if(weightLoc != MaxSize_t){
                                Rows[i].wcount(j, tindices[k], weight);
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
        Rows[i].calctotals(rnames, cnames, Tabs, weightLoc != MaxSize_t);
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
                            if(weightLoc != MaxSize_t){
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
                            if(weightLoc != MaxSize_t){
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

void CTable::RunPercents(const bool& weight, std::mutex& mu){
    for(size_t i=0 ; i<Rows.size() ; ++i){
        for(size_t j=0 ; j<Rows[i].getucounts().size() ; ++j){
            for(size_t k=0 ; k<Rows[i].getucounts()[j].size() ; ++k){
                if(Rows[i].getuTabTotals()[j] > 0)
                    Rows[i].setuperc(k,j, (double(Rows[i].getucounts()[j][k]) / double(Rows[i].getuTabTotals()[j])));
                else
                    Rows[i].setuperc(k,j, 0.0);
                if(weight){
                    if(Rows[i].getwTabTotals()[j] > 0)
                        Rows[i].setwperc(k,j, (Rows[i].getwcounts()[j][k] / Rows[i].getwTabTotals()[j]));
                    else
                        Rows[i].setwperc(k,j, 0.0);
                }
            }
        }
        for(size_t j=0 ; j<Rows[i].getuPRowTotals().size() ; ++j){
            if(Rows[i].getuRowTotals()[j] > 0){
                Rows[i].setUPRowTotal(j, (double(Rows[i].getuRowTotals()[j]) / double(Rows[i].getuBase())));
                if(weight){
                    Rows[i].setWPRowTotal(j, (Rows[i].getwRowTotals()[j] / Rows[i].getwBase()));
                }
            }else{
                Rows[i].setUPRowTotal(j, 0.0);
                if(weight){
                    Rows[i].setWPRowTotal(j, 0.0);
                }
            }
        }
        for(size_t j=0 ; j<Rows[i].getuPTabTotals().size() ; ++j){
            if(Rows[i].getuTabTotals()[j] > 0){
                Rows[i].setUPTabTotal(j, (double(Rows[i].getuTabTotals()[j]) / double(Rows[i].getuBase())));
                if(weight){
                    Rows[i].setWPTabTotal(j, (Rows[i].getwTabTotals()[j] / Rows[i].getwBase()));
                }
            }else{
                Rows[i].setUPTabTotal(j, 0.0);
                if(weight){
                    Rows[i].setWPTabTotal(j, 0.0);
                }
            }
        }
    }
}

void CTable::RunStats(){

}

void CTable::TableReport(const std::string& outputDIR, const bool counts, const bool percs, const bool stats, const bool& weight) const{
    std::ofstream rfile(outputDIR + Tablename + ".txt");
    rfile.precision(15);
    if(!(counts) && !(percs) && !(stats)) return;
    rfile << "CTable: " << Tablename << " {\n";
    if(counts){
        rfile << "\tCounts: {\n";
        for(size_t i=0 ; i<Rows.size() ; ++i){
            rfile << "\t\t" << "Row Group: " << Rows[i].Gname() << " {\n";
            rfile << "\t\t\tuBaseSize: {" << Rows[i].getuBase() << "}\n";
            if(weight) rfile << "\t\t\twBaseSize: {" << Rows[i].getwBase() << "}\n";
            rfile << "\t\t\tuTabTotals: {";
            for(size_t j=0 ; j<Rows[i].getuTabTotals().size() ; ++j){
                if(j >= Rows[i].getuTabTotals().size()-1)
                    rfile << Rows[i].getuTabTotals()[j] << "}\n";
                else
                    rfile << Rows[i].getuTabTotals()[j] << ',';
            }
            if(weight){
                rfile << "\t\t\twTabTotals: {";
                for(size_t j=0 ; j<Rows[i].getwTabTotals().size() ; ++j){
                    if(j >= Rows[i].getwTabTotals().size()-1)
                        rfile << Rows[i].getwTabTotals()[j] << "}\n";
                    else
                        rfile << Rows[i].getwTabTotals()[j] << ',';
                }
            }
            rfile << "\t\t\tuRowTotals: {";
            for(size_t j=0 ; j<Rows[i].getuRowTotals().size() ; ++j){
                if(j >= Rows[i].getuRowTotals().size()-1)
                    rfile << Rows[i].getuRowTotals()[j] << "}\n";
                else
                    rfile << Rows[i].getuRowTotals()[j] << ',';
            }
            if(weight){
                rfile << "\t\t\twRowTotals: {";
                for(size_t j=0 ; j<Rows[i].getwRowTotals().size() ; ++j){
                    if(j >= Rows[i].getwRowTotals().size()-1)
                        rfile << Rows[i].getwRowTotals()[j] << "}\n";
                    else
                        rfile << Rows[i].getwRowTotals()[j] << ',';
                }
            }
            rfile << "\t\t\tuCounts: {\n";
            for(size_t j=0 ; j<Rows[i].rowcount() ; ++j){
                rfile << "\t\t\t\t" << Rows[i].getRow(j).Ename() << ": {";
                for(size_t k=0 ; k<Rows[i].getucounts().size() ; ++k){
                    if(k >= Rows[i].getucounts().size()-1)
                        rfile << Rows[i].getucounts()[k][j] << "}\n";
                    else
                        rfile << Rows[i].getucounts()[k][j] << ',';
                }
            }
            rfile << "\t\t\t}\n";
            if(weight){
                rfile << "\t\t\twCounts: {\n";
                for(size_t j=0 ; j<Rows[i].rowcount() ; ++j){
                    rfile << "\t\t\t\t" << Rows[i].getRow(j).Ename() << ": {";
                    for(size_t k=0 ; k<Rows[i].getwcounts().size() ; ++k){
                        if(k >= Rows[i].getwcounts().size()-1)
                            rfile << Rows[i].getwcounts()[k][j] << "}\n";
                        else
                            rfile << Rows[i].getwcounts()[k][j] << ',';
                    }
                }
                rfile << "\t\t\t}\n";
            }
            rfile << "\t\t}\n";
        }
        rfile << "\t}\n";
    }
    if(percs){
        rfile << "\tPercents: {\n";
        for(size_t i=0 ; i<Rows.size() ; ++i){
            rfile << "\t\t" << "Row Group: " << Rows[i].Gname() << " {\n";
            rfile << "\t\t\tuPTabTotals: {";
            for(size_t j=0 ; j<Rows[i].getuPTabTotals().size() ; ++j){
                if(j >= Rows[i].getuPTabTotals().size()-1)
                    rfile << Rows[i].getuPTabTotals()[j] << "}\n";
                else
                    rfile << Rows[i].getuPTabTotals()[j] << ',';
            }
            if(weight){
                rfile << "\t\t\twPTabTotals: {";
                for(size_t j=0 ; j<Rows[i].getwPTabTotals().size() ; ++j){
                    if(j >= Rows[i].getwPTabTotals().size()-1)
                        rfile << Rows[i].getwPTabTotals()[j] << "}\n";
                    else
                        rfile << Rows[i].getwPTabTotals()[j] << ',';
                }
            }
            rfile << "\t\t\tuPRowTotals: {";
            for(size_t j=0 ; j<Rows[i].getuPRowTotals().size() ; ++j){
                if(j >= Rows[i].getuPRowTotals().size()-1)
                    rfile << Rows[i].getuPRowTotals()[j] << "}\n";
                else
                    rfile << Rows[i].getuPRowTotals()[j] << ',';
            }
            if(weight){
                rfile << "\t\t\twPRowTotals: {";
                for(size_t j=0 ; j<Rows[i].getwPRowTotals().size() ; ++j){
                    if(j >= Rows[i].getwPRowTotals().size()-1)
                        rfile << Rows[i].getwPRowTotals()[j] << "}\n";
                    else
                        rfile << Rows[i].getwPRowTotals()[j] << ',';
                }
            }
            rfile << "\t\t\tupercs: {\n";
            for(size_t j=0 ; j<Rows[i].rowcount() ; ++j){
                rfile << "\t\t\t\t" << Rows[i].getRow(j).Ename() << ": {";
                for(size_t k=0 ; k<Rows[i].getupercs().size() ; ++k){
                    if(k >= Rows[i].getupercs().size()-1)
                        rfile << Rows[i].getupercs()[k][j] << "}\n";
                    else
                        rfile << Rows[i].getupercs()[k][j] << ',';
                }
            }
            rfile << "\t\t\t}\n";
            if(weight){
                rfile << "\t\t\twpercs: {\n";
                for(size_t j=0 ; j<Rows[i].rowcount() ; ++j){
                    rfile << "\t\t\t\t" << Rows[i].getRow(j).Ename() << ": {";
                    for(size_t k=0 ; k<Rows[i].getwpercs().size() ; ++k){
                        if(k >= Rows[i].getwpercs().size()-1)
                            rfile << Rows[i].getwpercs()[k][j] << "}\n";
                        else
                            rfile << Rows[i].getwpercs()[k][j] << ',';
                    }
                }
                rfile << "\t\t\t}\n";
            }
            rfile << "\t\t}\n";
        }
        rfile << "\t}\n";
    }

    if(stats){

    }
    rfile << "}\n";
    rfile.close();
}

void runfctor::operator()(std::vector<CTable>& TableContainer, const size_t index_begin, const size_t length,
                            const std::string& datapath, const size_t& weightLoc, std::mutex& mu){
    std::ifstream datafile(datapath);
    std::string linestr;
    std::vector<std::string> dataline;
    getline(datafile, linestr); // skip variable names
    dataline = split(linestr,',');
    while(getline(datafile, linestr)){
        allocated_split(linestr, dataline, ',');
        for(size_t i=index_begin ; i<index_begin + length ; ++i){
            TableContainer[i].RunCounts(dataline, weightLoc, mu);
        }
    }
    for(size_t i=index_begin ; i<index_begin + length ; ++i){
        TableContainer[i].calcmembers(weightLoc, mu);
    }
    for(size_t i=index_begin ; i<index_begin+length ; ++i){
        TableContainer[i].RunPercents((weightLoc != std::numeric_limits<size_t>::max()), mu);
    }
}

void statfctor::operator()(){

}
