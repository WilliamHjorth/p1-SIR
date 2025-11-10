#include <stdio.h>
#include <math.h>

float Differentiationer(float S, float I, float R);
void func();

float S0 = 500;

float S;
float I0 = 2;
float I;
float R0 = 0;
float R;
float betaYAY = 0.3247;
float gammaYAY = 0.143;
float days = 1.0;
float N = 502;
int main(void)
{

    char gnuplot_path[512];
    printf("Enter full path to gnuplot.exe: ");
    scanf(" %[^\n]", gnuplot_path);

    char command[600];
    sprintf(command, "\"%s\" -persistent", gnuplot_path);

    FILE *pipe = _popen(command, "w");
    if (!pipe)
    {
        perror("_popen failed");
        return 1;
    }

    S = S0;
    I = I0;
    R = R0;

    char const *file_name = "data_file.txt";

    FILE *file = fopen(file_name, "w");

    if (file == NULL)
    {
        printf("couldnot open the file %s", file_name);
        return 1;
    }

    for (int n = 0; n < 100; n++)
    {
        float dt = 1.0f;
        float dSdT = -betaYAY * S * I / N;
        float dIdT = betaYAY * S * I / N - gammaYAY * I;
        float dRdt = gammaYAY * I;

        S += dSdT * dt;
        I += dIdT * dt;
        R += dRdt * dt;

        printf("Day %d: S = %.2f, I = %.2f, R = %.2f\n", n, S, I, R);
        fprintf(file, "%d %f %f %f\n", n, S, I, R);
    }
    fclose(file);

    fprintf(pipe, "set title 'SIR Model Simulation'\n");
    fprintf(pipe, "set xlabel 'Days'\n");
    fprintf(pipe, "set ylabel 'Population'\n");
    fprintf(pipe, "plot 'data_file.txt' using 1:2 with lines lw 2 title 'Susceptible', \\\n");
    fprintf(pipe, "     'data_file.txt' using 1:3 with lines lw 2 title 'Infected', \\\n");
    fprintf(pipe, "     'data_file.txt' using 1:4 with lines lw 2 title 'Recovered'\n");
    fflush(pipe);
    _pclose(pipe);

    //  Differentiationer(S0, I0, R0);

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

        printf("hej");
        printf("hej med dig");
    }
}

void func()
{
    /*
    random vrøvl


    */
}
