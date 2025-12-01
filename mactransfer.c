#include <stdio.h>
#include <math.h>

// funktionsprototyper
int sirModel(float S, float E, float I, float R);
int sirModelToByer(float S_AAL, float E_AAL, float I_AAL, float R_AAL, float S_KBH, float E_kbh, float I_KBH, float R_KBH);

// startværdier vi bruger og løbende vil ændre på for Aalborg
float S0_AAL = 121878;
float S_AAL;
float E_AAL;
float E0_AAL = 0;
float I0_AAL = 10;
float I_AAL;
float R0_AAL = 0;
float R_AAL;
float beta_AAL = 0.3247;
float gamma_AAL = 0.143;
float sigma_AAL = 1.666;

// startværdier vi bruger og løbende vil ændre på for København
float S0_KBH = 667099;
float S_KBH;
float E_KBH;
float E0_KBH = 0;
float I0_KBH = 50;
float I_KBH;
float R0_KBH = 0;
float R_KBH;
float beta_KBH = 0.4;
float gamma_KBH = 0.143;
float sigma_KBH = 1.666;

float days = 1.0;
float N_AAL = 121888;
float N_KBH = 667149;

// her sætter jeg transferraten til 0,1%
float t = 0.001;

// main funktion
int main(void)
{
    // kalder sir model function for begge byer
    sirModelToByer(S0_AAL, E0_AAL, I0_AAL, R0_AAL, S0_KBH, E0_KBH, I0_KBH, R0_KBH);
    return 0;
}

// funktion der plotter en seir model for to byer ud fra givne startværdier
int sirModelToByer(float S_AAL_start, float E_AAL_start, float I_AAL_start, float R_AAL_start, float S_KBH_start, float E_KBH_start, float I_KBH_start, float R_KBH_start)
{
    int valid = 0;
    char valg = 0;

    // spørger brugeren om man bruger Windows eller Mac (fortsætter indtil rigtigt svar)
    while (!valid)
    {
        printf("Bruger du Windows eller Mac? (w/m): ");
        scanf(" %c", &valg);

        if (valg == 'w' || valg == 'W')
        {
#ifdef _WIN32
            valid = 1;
#else
            printf("Windows valgt, men du kompilerer med macOS\n");
#endif
        }
        else if (valg == 'm' || valg == 'M')
        {
#ifndef _WIN32
            valid = 1;
#else
            printf("Mac valgt, men du kompilerer med Windows\n");
#endif
        }
        else
            printf("Ugyldigt input! Prøv igen.\n");
    }

    // laver en pipe til gnuplot
    char gnuplot_path[512];
    printf("Enter full path to gnuplot: ");
    scanf(" %[^\n]", gnuplot_path);

    // åbner pipe til filen
    char command[600];
    sprintf(command, "\"%s\" -persistent", gnuplot_path);

    FILE *pipe;

#ifdef _WIN32
    pipe = _popen(command, "w");
#else
    pipe = popen(command, "w");
#endif

    // tjekker at pipen er åbnet ellers fejl
    if (!pipe)
    {
        perror("_popen failed");
        return 1;
    }

    // laver text fil
    char const *file_name = "data_file.txt";
    FILE *file = fopen(file_name, "w");

    if (file == NULL)
    {
        printf("couldnot open the file %s", file_name);
        return 1;
    }

    // initialiserer variabler for Aalborg
    S_AAL = S_AAL_start;
    E_AAL = E_AAL_start;
    I_AAL = I_AAL_start;
    R_AAL = R_AAL_start;

    // initialiserer variabler for København
    S_KBH = S_KBH_start;
    E_KBH = E_KBH_start;
    I_KBH = I_KBH_start;
    R_KBH = R_KBH_start;

    for (int n = 0; n < 150; n++)
    {
        float dt = 1.0f;

        // her laver jeg en ny kategori til hver bogstav, hvor jeg tilføjer ud på deres navn, fordi det er dem der rejser ud af deres egen by
        float S_ud_AAL = t * S_AAL;
        float E_ud_AAL = t * E_AAL;
        float I_ud_AAL = t * I_AAL;
        float R_ud_AAL = t * R_AAL;

        float S_ud_KBH = t * S_KBH;
        float E_ud_KBH = t * E_KBH;
        float I_ud_KBH = t * I_KBH;
        float R_ud_KBH = t * R_KBH;

        // differentialligninger for sir modelen - AALBORG, forskellen her, er jeg tækker udrejsende fra aalborg fra og tilføjer indrejserne fra københavn til aalborg
        float dSdT_AAL = -beta_AAL * S_AAL * I_AAL / N_AAL - S_ud_AAL + S_ud_KBH;
        float dEdT_AAL = beta_AAL * S_AAL * I_AAL / N_AAL - sigma_AAL * E_AAL - E_ud_AAL + E_ud_KBH;
        float dIdT_AAL = sigma_AAL * E_AAL - gamma_AAL * I_AAL - I_ud_AAL + I_ud_KBH;
        float dRdt_AAL = gamma_AAL * I_AAL - R_ud_AAL + R_ud_KBH;

        // differentialligninger for sir modelen - KØBENHAVN, her trækker jeg så udrejsende fra københavn fra og tilføjer indrejsende fra aalborg til københavn
        float dSdT_KBH = -beta_KBH * S_KBH * I_KBH / N_KBH - S_ud_KBH + S_ud_AAL;
        float dEdT_KBH = beta_KBH * S_KBH * I_KBH / N_KBH - sigma_KBH * E_KBH - E_ud_KBH + E_ud_AAL;
        float dIdT_KBH = sigma_KBH * E_KBH - gamma_KBH * I_KBH - I_ud_KBH + I_ud_AAL;
        float dRdt_KBH = gamma_KBH * I_KBH - R_ud_KBH + R_ud_AAL;

        // differentialligninger integreret med eulers metode - AALBORG
        S_AAL += dSdT_AAL * dt;
        E_AAL += dEdT_AAL * dt;
        I_AAL += dIdT_AAL * dt;
        R_AAL += dRdt_AAL * dt;

        // differentialligninger integreret med eulers metode - KØBENHAVN
        S_KBH += dSdT_KBH * dt;
        E_KBH += dEdT_KBH * dt;
        I_KBH += dIdT_KBH * dt;
        R_KBH += dRdt_KBH * dt;

        // printer resultater
        printf("Day %d: AAL(S=%.0f, E=%.0f, I=%.0f, R=%.0f) KBH(S=%.0f, E=%.0f I=%.0f, R=%.0f)\n", n, S_AAL, E_AAL, I_AAL, R_AAL, S_KBH, E_KBH, I_KBH, R_KBH);

        fprintf(file, "%d %f %f %f %f %f %f %f %f\n", n, S_AAL, E_AAL, I_AAL, R_AAL, S_KBH, E_KBH, I_KBH, R_KBH);
    }

    // lukker fil
    fclose(file);

    // gnuplot commands
    fprintf(pipe, "set title 'SIR Model Simulation: Aalborg og Copenhagen med transferrate'\n");
    fprintf(pipe, "set xlabel 'Days'\n");
    fprintf(pipe, "set ylabel 'Population'\n");

    fprintf(pipe, "plot 'data_file.txt' using 1:2 with lines lw 2 title 'AAL Susceptible', \\\n");
    fprintf(pipe, "     'data_file.txt' using 1:3 with lines lw 2 title 'AAL Exposed', \\\n");
    fprintf(pipe, "     'data_file.txt' using 1:4 with lines lw 2 title 'AAL Infected', \\\n");
    fprintf(pipe, "     'data_file.txt' using 1:5 with lines lw 2 title 'AAL Recovered', \\\n");
    fprintf(pipe, "     'data_file.txt' using 1:6 with lines lw 2 title 'KBH Susceptible', \\\n");
    fprintf(pipe, "     'data_file.txt' using 1:7 with lines lw 2 title 'KBH Exposed', \\\n");
    fprintf(pipe, "     'data_file.txt' using 1:8 with lines lw 2 title 'KBH Infected', \\\n");
    fprintf(pipe, "     'data_file.txt' using 1:9 with lines lw 2 title 'KBH Recovered'\n");

    fflush(pipe);

#ifdef _WIN32
    _pclose(pipe);
#else
    pclose(pipe);
#endif

    return 0;
}
 