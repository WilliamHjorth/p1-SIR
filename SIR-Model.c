#include <stdio.h>
#include <math.h>

    int time = 0;
float S0 = 500;
 
int S;
float I0 = 2;
float Is;
int R0=0;
float R;
float betaYAY = 1.4247;
float gammaYAY = 0.143;
float days = 1.0;
int main (void)

{

float function(float x);

 
 function(S0);
 // printf("%d",S0);

    
 // printf("skriv parameter for beta og gamma");
return 0;
}

 float function(float x){

    for (int i = 0; i <100; i++)
    {
    printf("dag : %d var der : %.2f suceetible\n", i, x);

    float dsdt = -betaYAY*I0*x;
       x = x+ dsdt*days;
        
    }
 
}