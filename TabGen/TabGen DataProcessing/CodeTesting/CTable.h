#ifndef CTABLE_H_INCLUDED
#define CTABLE_H_INCLUDED
#include <string>
#include <mutex>
#include <vector>
#include <limits>

struct infoSet{
protected:
    std::string valuestr;
    float valuenum;
    size_t varLoc;
    size_t operation;
public:
    infoSet(const std::string& condstr, const std::vector<std::string>& variable_set);
    const size_t& getVloc() const;
    const size_t& getOP() const;
    const float& getNum() const;
    const std::string& getVstr() const;
};

struct condition_set{ // holds sets of conditional statements (G==7 && I<=9,a>4,b!=5) in a format that makes them easy to evaluate
protected:
    std::vector<std::vector<infoSet> > infoMap;
public:
    condition_set();
    condition_set(const std::string& condset, const std::vector<std::string>& variable_set);
    const bool test(const std::vector<std::string>& dataline, std::mutex& mu) const;
    size_t MapSize() const;
    void printMap(std::mutex& mu) const;
    void printMap() const;
};

struct tab_element{
public:
    tab_element(const std::string& reportline, const std::vector<std::string>& variable_set, const size_t& index);
    const condition_set& getConds() const;
    const std::string& getCondstr() const;
    const std::string& Ename() const;
    const std::string& Etag() const;
    const bool& calc() const;
    const size_t& getLoc() const;
protected:
    std::string name;
    condition_set Cset;
    std::string tag;
    std::string condstr;
    bool calculated;
    size_t indexLoc;
};

struct row_element{
public:
    row_element(const std::string& reportline, const std::vector<std::string>& variable_set, const size_t& index);
    const condition_set& getCset() const;
    const std::string& getCondstr() const;
    const std::string& Ename() const;
    const bool& calc() const;
    const size_t& getLoc() const;
protected:
    condition_set Cset;
    std::string condstr;
    std::string name;
    bool calculated;
    size_t indexLoc;
};

struct tab_group{
public:
    tab_group(const std::string& reportline);
    const std::string& Gname() const;
    size_t tabcount() const;
    const tab_element& getTab(const size_t& index) const;
    void add_element(const std::string& reportline, const std::vector<std::string>& variable_set, const size_t& index);
protected:
    std::string name;
    std::vector<tab_element> etabs;
};

struct row_group{
public:
    row_group(const std::string& reportline, const size_t& tabgroupcount, const size_t& totaltabs);
    const row_element& getRow(const size_t& index) const;
    const std::string& Gname() const;
    void add_element(const std::string & reportline, const std::vector<std::string>& variable_set, const size_t& index);
    size_t rowcount() const;
    void ucount(const size_t& rindex, const size_t& tindex);
    void wcount(const size_t& rindex, const size_t& tindex, const double& weightvalue);
    void setucount(const size_t& rindex, const size_t& tindex, const size_t& value);
    void setwcount(const size_t& rindex, const size_t& tindex, const double& value);
    void runstats(const bool& ztpp, const bool& ztpw, const bool& ttpp, const bool& ttpw, const bool& Chi2, const bool& Anova);
    void URTotal(const size_t& rindex);
    void WRTotal(const size_t& rindex, const double& weightvalue);
    void UCTotal(const size_t& tindex);
    void WCTotal(const size_t& tindex, const double& weightvalue);
    void ubase(const size_t& basevalue);
    void wbase(const double& basevalue);
    const std::vector<std::vector<size_t> >& getucounts() const;
    const std::vector<std::vector<double> >& getwcounts() const;
    const std::vector<std::vector<double> >& getupercs() const;
    const std::vector<std::vector<double> >& getwpercs() const;
    const std::vector<size_t>& getuRowTotals() const;
    const std::vector<double>& getwRowTotals() const;
    const std::vector<size_t>& getuTabTotals() const;
    const std::vector<double>& getwTabTotals() const;
protected:
    std::string name;
    std::vector<std::string> chi2_results;
    std::vector<std::string> anova_results;
    std::vector<row_element> erows;
    std::vector<std::vector<size_t> > ucounts;
    std::vector<std::vector<double> > wcounts;
    std::vector<std::vector<double> > upercs;
    std::vector<std::vector<double> > wpercs;
    std::vector<size_t> uTab_Totals;
    std::vector<double> wTab_Totals;
    std::vector<size_t> uRow_Totals;
    std::vector<double> wRow_Totals;
    size_t ubaseSize;
    double wbaseSize;
    std::vector<double> meandata;
    std::vector<double> vardata;
    std::vector<double> devdata;
    std::vector<std::vector<std::string> > ppuztest;
    std::vector<std::vector<std::string> > pwuztest;
    std::vector<std::string> pputtest;
    std::vector<std::string> pwuttest;
    std::vector<std::vector<std::string> > ppwztest;
    std::vector<std::vector<std::string> > pwwztest;
    std::vector<std::string> ppwttest;
    std::vector<std::string> pwwttest;
};

struct CTable{
public:
    CTable(const double* confLvls, const bool* tablebools, const std::string& Tname);
    void addRowGroup(const std::string& reportline, const size_t& tabgroupcount, const size_t& totaltabs);
    void addTabGroup(const std::string& reportline);
    void addTabElement(const std::string& reportline, const std::vector<std::string>& variable_set, const size_t& index);
    void addRowElement(const std::string& reportline, const std::vector<std::string>& variable_set, const size_t& index);
    void RunCounts(const std::vector<std::string>& dataline, const size_t& weightLoc, std::mutex& mu);
    void calcmembers(const size_t& weightLoc, std::mutex& mu);
    void RunPercents();
    void RunStats();
    const size_t tabgroupcount() const;
    const size_t rowgroupcount() const;
    const row_group& getRowGroup(const size_t& index) const;
    const tab_group& getTabGroup(const size_t& index) const;
    void TableReport(const std::string& outputDIR, const bool counts, const bool percs, const bool stats, const bool& weight) const;
protected:
    bool RowTotal, TabTotal, Means, Var, StdDv, ZtestPP, TtestPP, ZtestPW, TtestPW, ANOVA, ChiSqr;
    double Clevels[3];
    size_t totalrows, totaltabs, BASE_SIZE;
    std::string Tablename;
    std::vector<tab_group> Tabs;
    std::vector<row_group> Rows;
};

class runfctor{
public:
    void operator()(std::vector<CTable>& TableContainer, const size_t index_begin, const size_t length,
                    const std::string& datapath, const size_t& weightLoc, std::mutex& mu);
};

class perfctor{
public:
    void operator()(std::vector<CTable>& tablecontainer, const size_t index_begin, const size_t length, std::mutex& mu);
};
#endif // CTABLE_H_INCLUDED
