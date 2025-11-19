#include <stdio.h>
#include <math.h>

// funktionsprototyper
int sirModel(float S, float E, float I, float R);

// startværdier vi bruger og løbende vil ændre på
float S0 = 500;

float S;
float I0 = 2;
float E;
float E0 = 0;
float I;
float R0 = 0;
float R;
float betaYAY = 0.3247;
float gammaYAY = 0.143;
float sigmaYAY = 0.1666; // værdien for en inkubatiostid på 6 dage (sigmma= 1/inkubationstid)
float days = 1.0;
float N = 502;

// main funktion

int main(void)
{

    // kalder sir model function
    sirModel(S0, E0, I0, R0);

    return 0;
}

// funktion der plotter en sir model ud fra 3 givne startværdier
int sirModel(float S, float E, float I, float R) // kun til windows indtilvidere
{
    // laver en pipe til gnuplot.exe

    char gnuplot_path[512];
    printf("Enter full path to gnuplot.exe: ");
    scanf(" %[^\n]", gnuplot_path);

    // åbner pipe til filen
    char command[600];
    sprintf(command, "\"%s\" -persistent", gnuplot_path);

    FILE *pipe = _popen(command, "w");

    // tjekker at pipen er åbnet ellers fejl
    if (!pipe)
    {
        perror("_popen failed");
        return 1;
    }

    // laver en text fil
    char const *file_name = "data_file.txt";
    // åbner så vi kan skrive i den
    FILE *file = fopen(file_name, "w");

    if (file == NULL)
    {
        printf("couldnot open the file %s", file_name);
        return 1;
    }

    for (int n = 0; n < 100; n++)
    {
        // differentialligninger for sir modelen
        float dt = 1.0f;
        float dSdT = -betaYAY * S * I / N;
        float dEdt = betaYAY * S * I / N - sigmaYAY * E;
        float dIdT = sigmaYAY * E - gammaYAY * I; // "betaYAY * S * I / N" -- erstattet
        float dRdt = gammaYAY * I;
        // differentialligninger integreret med eulors metode, altså ændring hver dag
        S += dSdT * dt;
        E += dEdt * dt;
        I += dIdT * dt;
        R += dRdt * dt;
        // printer S I R ud hver dag i terminal og i text fil
        printf("Day %d: S = %.2f, E = %.2f, I = %.2f, R = %.2f\n", n, S, E, I, R);
        fprintf(file, "%d %f %f %f %f\n", n, S, E, I, R);
    }
    // lukker filen efter vi har printet tallene ind i filen.
    fclose(file);
    // gnuplot commands der laver headers og labels.
    fprintf(pipe, "set title 'SIR Model Simulation'\n");
    fprintf(pipe, "set xlabel 'Days'\n");
    fprintf(pipe, "set ylabel 'Population'\n");

    // plotter data_file.txt data og lukker pipelinen
    fprintf(pipe, "plot 'data_file.txt' using 1:2 with lines lw 2 title 'Susceptible', \\\n");
    fprintf(pipe, "     'data_file.txt' using 1:3 with lines lw 2 title 'Exposed', \\\n");
    fprintf(pipe, "     'data_file.txt' using 1:4 with lines lw 2 title 'Infected', \\\n");
    fprintf(pipe, "     'data_file.txt' using 1:5 with lines lw 2 title 'Recovered'\n");
    fflush(pipe);
    _pclose(pipe);
}
