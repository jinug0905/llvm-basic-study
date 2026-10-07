#include <stdio.h>

int main() {
    int a = 17, b = 5;

    int sum  = a + b;
    int diff = a - b;
    int prod = a * b;
    int quot = a / b;  
    int rem  = a % b;   

    unsigned int ua = 17, ub = 5;
    unsigned int uquot = ua / ub;  

    int band = a & b;
    int bor  = a | b;
    int bxor = a ^ b;
    int lsh  = a << 2;
    int rsh  = a >> 2;              
    unsigned int ursh = ua >> 2; 
    int lt = a < b;    
    int eq = a == b;      

    double x = 3.14, y = 2.0;
    int flt_lt = x < y;  

    int neg  = -a;    
    int bnot = ~a;     
    int lnot = !a;     

    double dneg  = -x;    
    double dsum  = x + y;
    double ddiff = x - y;
    double dprod = x * y;
    double dquot = x / y;

    unsigned int uadd = ua + 1; 
    int sadd = a + 1;  
    int ssub = a - 1;  
    int smul = a * 2; 

    return sum;
}