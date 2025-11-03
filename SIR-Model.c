#include <stdio.h>
#include <math.h>

float Differentiationer(float S, float I, float R);

float S0 = 500;

float S;
float I0 = 2;
float I;
float R0 = 0;
float R;
float betaYAY = 1.4247;
float gammaYAY = 0.143;
float days = 1.0;
float N = 502;
int main(void)

{

    Differentiationer(S0, I0, R0);
    // printf("%d",S0);

    // printf("skriv parameter for beta og gamma");
    return 0;
}

float Differentiationer(float S, float I, float R)
{

    for (int n = 0; n < 100; n++)
    {
        float dt = 1.0f;
        float dSdT = -betaYAY * S * I / N;
        float dIdT = betaYAY * S / N - gammaYAY * I;
        float dRdt = gammaYAY * I;

        S += dSdT * dt;
        I += dIdT * dt;
        R += dRdt * dt;

        printf(" S = %.2f\n I = %.2f\n R = %.2f\n", S, I, R);
    }
}