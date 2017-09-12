#ifndef BMATH_H_INCLUDED
#define BMATH_H_INCLUDED
#include "math.h"
#include <vector>
#include <thread>
#include <clocale>
#include <cwctype>

const double ln_sqrt_2_pi = 0.91893853320467274178;
const double g_pi = 3.14159265358979323846;
const double num_e = 2.71828182845904523536;

double abs(const double& x){
    if (x >= 0)
        return x;
    else
        return -1*x;
}

double lanczos_ln_gamma(double z){ // z is used as a variable in the function, so const reference cannot be used
    double LG_g = 7.0;
    size_t LG_N = 9;

    const double lct[LG_N+1] = {
        0.9999999999998099322768470047347,
        676.520368121885098567009190444019,
       -1259.13921672240287047156078755283,
        771.3234287776530788486528258894,
        -176.61502916214059906584551354,
        12.507343278686904814458936853,
        -0.13857109526572011689554707,
        9.984369578019570859563e-6,
        1.50563273514931155834e-7
    };
    double sum;
    double base;
    int i;
    if (z < 0.5) {
      // Use Euler's reflection formula:
      // Gamma(z) = Pi / [Sin[Pi*z] * Gamma[1-z]];
      return log(g_pi / sin(g_pi * z)) - lanczos_ln_gamma(1.0 - z);
    }
    z = z - 1.0;
    base = z + LG_g + 0.5;  // Base of the Lanczos exponential
    sum = 0;
    // We start with the terms that have the smallest coefficients and largest
    // denominator.
    for(i=LG_N; i>=1; i--) {
      sum += lct[i] / (z + ((double) i));
    }
    sum += lct[0];
    // Gamma[z] = Sqrt(2*Pi) * sum * base^[z + 0.5] / E^base
    return ((ln_sqrt_2_pi + log(sum)) - base) + log(base)*(z+0.5);
}

  // Compute the Gamma function, which is e to the power of ln_gamma.
double lanczos_gamma(const double& z){
    return(exp(lanczos_ln_gamma(z)));
}
double low_incomplete_gamma(const double& s, const double& z){
    double epsilon = .0000000001;
    size_t MAXIT = 100;
    size_t i = 1;
    double initial = (pow(z,s)*pow(num_e,-z))/s; // z^0 == 1
    for(;;)
    {
        double denom = s;
        for(size_t j=1 ; j<=i ; ++j)
        {
            denom = denom*(s+j);
        }
        double num = pow(z,s)*pow(num_e,-1*z)*pow(z,i);
        double test = num/denom;
        if(abs(test)<epsilon)
        {
            initial += test;
            break;
        }
        else
        {
            initial += test;
            ++i;
        }
        if(i > MAXIT) break;
    }
    return initial;
}
double regularized_gamma(const double& s, const double& z){
    double inc = low_incomplete_gamma(s,z);
    double gam = lanczos_gamma(s);
    return inc/gam;
}
double error_function(const double& x){
    return x < 0.0 ? -regularized_gamma(.5,x*x) : regularized_gamma(.5,x*x);
}
double Tdist(const double& t, const int& df){
    double num = lanczos_gamma((df+1)/(double)2);
    double den = sqrt(g_pi*df)*lanczos_gamma(df/(double)2);
    double mult = (1 + pow(t,2)/(double)df);
    double exp =  (-1/(double)2)*(df+1);
    return (num/den)*(pow(mult,exp));
}
double Beta(const double& x, const double& y){
    double gamx = lanczos_gamma(x);
    double gamy = lanczos_gamma(y);
    double gamxy = lanczos_gamma(x+y);
    return (gamx*gamy)/gamxy;
}
double i_beta_Lentz(const double& a, const double& b, const double& x){
    size_t MAXIT = 100;
    double EPS = 3.0e-7;
    double FPMIN = 1.0e-30;
    size_t m,m2;
    double aa,c,d,del,h,qab,qam,qap;
    qab=a+b;
    qap=a+1.0;
    qam=a-1.0;
    c=1.0;
    d=1.0-qab*x/qap;
    if (fabs(d) < FPMIN) d=FPMIN;
    d=1.0/d;
    h=d;
    for (m=1;m<=MAXIT;m++) {
        m2=2*m;
        aa=m*(b-m)*x/((qam+m2)*(a+m2));
        d=1.0+aa*d;
        if (fabs(d) < FPMIN) d=FPMIN;
        c=1.0+aa/c;
        if (fabs(c) < FPMIN) c=FPMIN;
        d=1.0/d;
        h *= d*c;
        aa = -(a+m)*(qab+m)*x/((a+m2)*(qap+m2));
        d=1.0+aa*d;
        if (fabs(d) < FPMIN) d=FPMIN;
        c=1.0+aa/c;
        if (fabs(c) < FPMIN) c=FPMIN;
        d=1.0/d;
        del=d*c;
        h *= del;
        if (fabs(del-1.0) < EPS) break;
    }
    if (m > MAXIT)
    {
       return 0.0;
    }
    else
    {
        return h;
    }
}
double regularized_beta(const double& a, const double& b, const double& x){
    double bt;
    if (x < 0.0 || x > 1.0) return 0.0;
    if (x == 0.0 || x == 1.0) bt=0.0;
    else
        bt=exp(lanczos_ln_gamma(a+b)-lanczos_ln_gamma(a)-lanczos_ln_gamma(b)+a*log(x)+b*log(1.0-x));
    if (x < (a+1.0)/(a+b+2.0)) // Use continued fraction directly.
        return bt*i_beta_Lentz(a,b,x)/a;
    else // Use continued fraction after making the symmetry transformation.
        return 1.0-bt*i_beta_Lentz(b,a,1.0-x)/b;
}
double t_dist_CDF(const double& t, const double& df){
    double x = df/(pow(t,2)+df);
    double a = df/(double)2;
    double b = 1/(double)2;
    return double(1) - .5*regularized_beta(a,b,x);
}
double std_normal_CDF(const double& z){
    return .5*(1.0 + error_function(z/sqrt(2.0)));
}
double f_dist_CDF(const double& v1, const double& v2, const double& f){ // right tail probability
    return regularized_beta(v2/(double)2, v1/(double)2, (v2/(v2+v1*f)));
}
double chi_2_CDF(const double& df, const double& x){ // returns p-value for given chi-2 test statistic(x) and degrees of freedom(df)
    return regularized_gamma(df/(double)2,x/(double)2);
}
double t_testUE_uwpp(const double& xbar1, const double& xbar2, const double& var1, const double& var2, const size_t& n1, const size_t& n2){
    double meandiff = xbar1 - xbar2;
    double SE = sqrt((var1/n1) + (var2/n2));
    double t = meandiff/SE;
    double term1 = var1/n1;
    double term2 = var2/n2;
    double numerator = pow((term1 + term2),2);
    double denominator = ((1.0 / (n1 - 1.0)) * pow(term1,2)) + ((1.0 / (n2 - 1.0)) * pow(term2,2));
    double df = numerator / denominator;
    return t_dist_CDF(t,df);
}
double t_testE_uwpp(const double& xbar1, const double& xbar2, const double& var1, const double& var2, const size_t& n1, const size_t& n2){
    double meandiff = xbar1 - xbar2;
    double cvar_numerator = (n1 - 1.0)*var1 + (n2 - 1.0)*var2;
    double cvar_denominator = n1 + n2 - 2;
    double cvar = cvar_numerator / cvar_denominator;
    double cstd = sqrt(cvar);
    double SE = cstd * sqrt((1.0 / n1) + (1.0 / n2));
    double t = meandiff / SE;
    return t_dist_CDF(t,(n1+n2-2));
}
double t_testUE_uwpw(const double& x_m , const double& x_n, const double& s_m, const double& s_n, const size_t& m, const size_t& n){
    double meandiff = x_m - x_n;
    double SE = sqrt((((1.0 / (double)m) - (1.0 / (double)n)) * m * s_m) + (((n - m) / pow(n,2)) * s_n));
    double t = meandiff/SE;
    double numerator = (m - 1) * (n - m - 1);
    double c = (s_m / m) / ((s_m / m) + (s_n / (n - m)));
    double denominator = ((m - 1) * pow((1 - c),2)) + ((n - m - 1) * pow(c,2));
    double df = numerator / denominator;
    return t_dist_CDF(t,df);
}
double t_testE_uwpw(const double& pbar, const double& wbar, const double& svar, const size_t& m, const size_t& n){
    double meandiff = pbar - wbar;
    double SE = sqrt(svar) * sqrt((1.0 / m) - (1.0 / n));
    double t = meandiff / SE;
    return t_dist_CDF(t,(n-1));
}
double t_testUE_wpp(const double& xbar1, const double& xbar2, const double& var1, const double& var2, const double& e1, const double& e2,const size_t& n1, const size_t& n2){
    double meandiff = xbar1 - xbar2;
    double SE = sqrt((var1/e1) + (var2/e2));
    double t = meandiff/SE;
    double term1 = var1/n1;
    double term2 = var2/n2;
    double numerator = pow((term1 + term2),2);
    double denominator = ((1.0 / (n1 - 1.0)) * pow(term1,2)) + ((1.0 / (n2 - 1.0)) * pow(term2,2));
    double df = numerator / denominator;
    return t_dist_CDF(t,df);
}
double t_testE_wpp(const double& xbar1, const double& xbar2, const double& var1, const double& var2, const size_t& n1, const size_t& n2, const double& e1, const double& e2){
    double meandiff = xbar1 - xbar2;
    double cvar_numerator = (n1 - 1.0)*var1 + (n2 - 1.0)*var2;
    double cvar_denominator = n1 + n2 - 2;
    double cvar = cvar_numerator / cvar_denominator;
    double cstd = sqrt(cvar);
    double SE = cstd * sqrt((1.0 / e1) + (1.0 / e2));
    double t = meandiff / SE;
    return t_dist_CDF(t,(n1+n2-2));
}
double t_testUE_wpw(const double& x_m , const double& x_n, const double& s_m, const double& s_n, const size_t& m, const size_t& n, const double& ewhole, const double& epart){
    double meandiff = x_m - x_n;
    double s_m_n = 0;
    double dterm1 = ((1.0 / epart) - (1.0 / ewhole)) * epart * s_m;
    double dterm2 = ((ewhole - epart) / pow(ewhole,2)) * s_m_n;
    double SE = sqrt(dterm1 + dterm2);
    double t = meandiff/SE;
    double numerator = (m - 1.0) * (n - m - 1.0);
    double c = (s_m / m) / ((s_m / m) + (s_n / (n - m)));
    double denominator = ((m - 1.0) * pow((1.0 - c),2)) + ((n - m - 1.0) * pow(c,2));
    double df = numerator / denominator;
    return t_dist_CDF(t,df);
}
double t_testE_wpw(const double& pbar, const double& wbar, const double& svar, const double& ewhole, const double& epart, const size_t& n){
    double meandiff = pbar - wbar;
    double SE = sqrt(svar) * sqrt((1.0 / epart) - (1.0 / ewhole));
    double t = meandiff / SE;
    return t_dist_CDF(t,(n-1.0));
}
double z_test_uwupoolpp(const double& p1, const double& p2, const size_t& n1, const size_t& n2){
    double numerator = p1 - p2;
    double dterm1 = (p1*(1.0-p1))/n1;
    double dterm2 = (p2*(1.0-p2))/n2;
    double denominator = sqrt(dterm1 + dterm2);
    double z = numerator / denominator;
    return std_normal_CDF(z);
}
double z_test_uwpoolpp(const double& p1, const size_t& n1, const double& p2, const size_t& n2){
    double p_hat = ((p1*n1) + (p2*n2))/(n1+n2);
    double SE = sqrt(p_hat*(1.0-p_hat)*((1.0/n1) + (1.0/n2)));
    double z = (p1-p2)/SE;
    return std_normal_CDF(z);
}
double z_test_uwupoolpw(const double& p1, const double& p2, const size_t& n, const size_t& m){ // p2 is whole proportion, n is whole sample size
    double numerator = p1 - p2;
    double dterm1 = pow(n-m,2);
    dterm1 *= (p1*(1.0-p1));
    dterm1 /= (m*pow(n,2));
    double p_star = (n*p1-m*p2)/(n-m);
    double dterm2 = ((n-m)*p_star*(1.0-p_star))/pow(n,2);
    double denominator = sqrt(dterm1 + dterm2);
    double z = numerator / denominator;
    return std_normal_CDF(z);
}
double z_test_uwpoolpw(const double& p1, const double& p2, const double& n, const size_t& m){  // p2 is whole proportion, n is whole sample size
    double numerator = p1 - p2;
    double denominator = sqrt((p2*(1.0-p2))*((1.0/m) - (1.0/n)));
    double z = numerator / denominator;
    return std_normal_CDF(z);
}
double z_test_wupoolpp(const double& p1w, const double& p2w, const double& p1u, const double& p2u, const double& e1, const double& e2){
    double numerator = p1w - p2w;
    double dterm1 = (p1u*(1.0 - p1u))/e1;
    double dterm2 = (p2u*(1.0 - p2u))/e2;
    double denominator = sqrt(dterm1 + dterm2);
    double z = numerator / denominator;
    return std_normal_CDF(z);
}
double z_test_wupoolpw(const double& p1w, const double& p2w, const double& p1u, const double& p2u, const double& e1, const double& e2){ // e2 is whole sample size
    double numerator = p1w - p2w;
    double dterm1 = pow(e2 - e1, 2) * (p1u *(1.0 - p2u));
    dterm1 /= (e1 * pow(e2,2));
    double p_star = (e2*p2u-e1*p1u)/(e2-e1);
    double dterm2 = ((e2-e1)*p_star*(1.0-p_star))/pow(e2,2);
    double denominator = sqrt(dterm1 + dterm2);
    double z = numerator / denominator;
    return std_normal_CDF(z);
}
double z_test_wpoolpp(const double& p1w, const double& p2w, const double& p1u, const double& p2u, const double& e1, const double& e2, const size_t& n1, const size_t& n2){
    double numerator = p1w - p2w;
    double p_hat = (n1*p1u + n2*p2u) / (n1 + n2);
    double denominator = sqrt(p_hat*(1.0 - p_hat)*((1.0 / e1) + (1.0 / e2)));
    double z = numerator / denominator;
    return std_normal_CDF(z);
}
double z_test_wpoolpw(const double& p1w, const double& p2w, const double& pu, const double& e1, const double& e2){
    double numerator = p1w - p2w;
    double denominator = sqrt(pu*(1.0 - pu)*((1.0 / e1) - (1.0 / e2)));
    double z = numerator / denominator;
    return std_normal_CDF(z);
}
#endif // BMATH_H_INCLUDED
