#include <stdio.h>
#include <math.h>

// funktionsprototyper
int sirModel(float S, float E, float I, float R);
int sirModelToByer(void);

#define Alders_GRUPPER 4

typedef enum
{
    AGE_0_18,
    AGE_19_35,
    AGE_36_50,
    AGE_50_PLUS
} ALDER;

// startværdier vi bruger og løbende vil ændre på for Aalborg

float S0_AAL = 121878;
float S_AAL[Alders_GRUPPER];
float E_AAL[Alders_GRUPPER];
float E0_AAL = 0;
float I0_AAL = 10;
float I_AAL[Alders_GRUPPER];
float R0_AAL = 0;
float R_AAL[Alders_GRUPPER];
float beta_AAL = 0.3247;
float gamma_AAL = 0.143;
float sigma_AAL = 1.666;

// startværdier vi bruger og løbende vil ændre på for København
float S0_KBH = 667099;
float S_KBH[Alders_GRUPPER];
float E_KBH[Alders_GRUPPER];
float E0_KBH = 0;
float I0_KBH = 50;
float I_KBH[Alders_GRUPPER];
float R0_KBH = 0;
float R_KBH[Alders_GRUPPER];
float beta_KBH = 0.4;
float gamma_KBH = 0.143;
float sigma_KBH = 1.666; // værdien for en inkubatiostid på 6 dage (sigmma= 1/inkubationstid)

float days = 1.0;
float N_AAL = 121888;
float N_KBH = 667149;

// sætter parametre der indflyder hvad smitte raten/ helbreds raten, inkobuations raten etc.. for de fire alders grupper
float age_beta_factor[Alders_GRUPPER] = {0.8, 1.0, 1.2, 1.5};
float age_sigma_factor[Alders_GRUPPER] = {1.0, 1.0, 0.9, 0.8};
float age_gamma_factor[Alders_GRUPPER] = {1.2, 1.0, 0.9, 0.7};

// main funktion

int main(void)
{
    char city;
    printf("Choose city (a=Aalborg, b=Koebenhavn): \n");
    scanf(" %C", &city);

    if (city == 'a' || city == 'A')
    {
        // startværdierene samlet bliver fordelt ligelidt ud på de fire aldersgruppe (Kan ændres hvis vi vil have det skal være anderledes)
        for (int i = 0; i < Alders_GRUPPER; i++)
        {
            S_AAL[i] = S0_AAL / Alders_GRUPPER;
            E_AAL[i] = E0_AAL / Alders_GRUPPER;
            I_AAL[i] = I0_AAL / Alders_GRUPPER;
            R_AAL[i] = R0_AAL / Alders_GRUPPER;

            // forstår ikke hvorfor vi gør dette mvh William, men det gjorde vi i tidligere version
            S_KBH[i] = 0;
            E_KBH[i] = 0;
            I_KBH[i] = 0;
            R_KBH[i] = 0;
        }
    }
    else if (city == 'b' || city == 'B')
    {
        for (int u = 0; u < Alders_GRUPPER; u++)
        {
            S_KBH[u] = S0_KBH / Alders_GRUPPER;
            E_KBH[u] = E0_KBH / Alders_GRUPPER;
            I_KBH[u] = I0_KBH / Alders_GRUPPER;
            R_KBH[u] = R0_KBH / Alders_GRUPPER;

            S_AAL[u] = 0;
            E_AAL[u] = 0;
            I_AAL[u] = 0;
            R_AAL[u] = 0;
        }
    }
    else
    {
        printf("invalid city!\n");
        return 1;
    }

    char season;
    printf("Choose season (s=summer, v=vinter): \n");
    scanf(" %c", &season);

    if (season == 's' || season == 'S')
    {
        beta_AAL *= 0.8;
        gamma_AAL *= 0.8;
        sigma_AAL *= 0.8;

        beta_KBH *= 0.8;
        gamma_KBH *= 0.8;
        sigma_KBH *= 0.8;
    }
    else if (season == 'v' || season == 'V')
    {
        beta_KBH *= 1.2;
        gamma_KBH *= 1.2;
        sigma_KBH *= 1.2;

        beta_AAL *= 1.2;
        gamma_AAL *= 1.2;
        sigma_AAL *= 1.2;
    }
    else
    {
        printf("Invalid season!\n");
        return 1;
    }
    // kalder sir model function for begge byer
    sirModelToByer();
    return 0;
}

// funktion der plotter en sir model for to byer ud fra givne startværdier
int sirModelToByer(void) // kun til windows indtilvidere
{
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
            {
                printf("Ugyldigt input! Prøv igen.\n");
            }
        }
        // laver en pipe til gnuplot.exe

        char gnuplot_path[512];
        printf("Enter full path to gnuplot.exe: ");
        scanf(" %[^\n]", gnuplot_path);

        // åbner pipe til filen
        char command[600];
        sprintf(command, "\"%s\" -persistent", gnuplot_path);

        FILE *pipe = _popen(command, "w");

        // Her åbnes filen med specifikke kommandoer til hhv mac (popen) og windows (_popen)
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

        // laver en text fil
        char const *file_name = "data_file.txt";
        // åbner så vi kan skrive i den
        FILE *file = fopen(file_name, "w");

        if (file == NULL)
        {
            printf("couldnot open the file %s", file_name);
            return 1;
        }

        // initialiserer variabler for København med de modtagne parametre

        float dS_AAL[Alders_GRUPPER];
        float dE_AAL[Alders_GRUPPER];
        float dI_AAL[Alders_GRUPPER];
        float dR_AAL[Alders_GRUPPER];

        float dS_KBH[Alders_GRUPPER];
        float dE_KBH[Alders_GRUPPER];
        float dI_KBH[Alders_GRUPPER];
        float dR_KBH[Alders_GRUPPER];

        for (int n = 0; n < 150; n++)
        {

            float total_I_AAL = 0.0f;
            float total_I_KBH = 0.0f;

            for (int i = 0; i < Alders_GRUPPER; i++)
            {
                total_I_AAL += I_AAL[i];
                total_I_KBH += I_KBH[i];
            }

            // differentialligninger for sir modelen - AALBORG
            float dt = 1.0f;

            for (int i = 0; i < Alders_GRUPPER; i++)
            {
                float beta_i = beta_AAL * age_beta_factor[i];
                float sigma_i = sigma_AAL * age_sigma_factor[i];
                float gamma_i = gamma_AAL * age_gamma_factor[i];

                dS_AAL[i] = -beta_i * S_AAL[i] * total_I_AAL / N_AAL;
                dE_AAL[i] = beta_i * S_AAL[i] * total_I_AAL / N_AAL - sigma_i * E_AAL[i];
                dI_AAL[i] = sigma_i * E_AAL[i] - gamma_i * I_AAL[i];
                dR_AAL[i] = gamma_i * I_AAL[i];
            }

            // differentialligninger for sir modelen - KØBENHAVN
            for (int i = 0; i < Alders_GRUPPER; i++)
            {
                float beta_i = beta_KBH * age_beta_factor[i];
                float sigma_i = sigma_KBH * age_sigma_factor[i];
                float gamma_i = gamma_KBH * age_gamma_factor[i];

                dS_KBH[i] = -beta_i * S_KBH[i] * total_I_KBH / N_KBH;
                dE_KBH[i] = beta_i * S_KBH[i] * total_I_KBH / N_KBH - sigma_i * E_KBH[i];
                dI_KBH[i] = sigma_i * E_KBH[i] - gamma_i * I_KBH[i];
                dR_KBH[i] = gamma_i * I_KBH[i];
            }

            // differentialligninger integreret med eulors metode, altså ændring hver dag - AALBORG
            // 2) Opdatér ALLE grupper samtidig
            for (int i = 0; i < Alders_GRUPPER; i++)
            {
                S_AAL[i] += dS_AAL[i];
                E_AAL[i] += dE_AAL[i];
                I_AAL[i] += dI_AAL[i];
                R_AAL[i] += dR_AAL[i];

                S_KBH[i] += dS_KBH[i];
                E_KBH[i] += dE_KBH[i];
                I_KBH[i] += dI_KBH[i];
                R_KBH[i] += dR_KBH[i];
            }

            float sum_S_AAL = 0, sum_E_AAL = 0, sum_I_AAL = 0, sum_R_AAL = 0;
            float sum_S_KBH = 0, sum_E_KBH = 0, sum_I_KBH = 0, sum_R_KBH = 0;

            for (int i = 0; i < Alders_GRUPPER; i++)
            {
                sum_S_AAL += S_AAL[i];
                sum_E_AAL += E_AAL[i];
                sum_I_AAL += I_AAL[i];
                sum_R_AAL += R_AAL[i];

                sum_S_KBH += S_KBH[i];
                sum_E_KBH += E_KBH[i];
                sum_I_KBH += I_KBH[i];
                sum_R_KBH += R_KBH[i];
            }

            // differentialligninger integreret med eulors metode, altså ændring hver dag - KØBENHAVN

            // printer S I R ud hver dag i terminal og i text fil for begge byer
            printf("Day %d:  AAL(S=%.0f, E=%.0f, I=%.0f, R=%.0f)  |  KBH(S=%.0f, E=%.0f, I=%.0f, R=%.0f)\n",
                   n,
                   sum_S_AAL, sum_E_AAL, sum_I_AAL, sum_R_AAL,
                   sum_S_KBH, sum_E_KBH, sum_I_KBH, sum_R_KBH);
            fprintf(file, "%d %f %f %f %f %f %f %f %f\n",
                    n,
                    sum_S_AAL, sum_E_AAL, sum_I_AAL, sum_R_AAL,
                    sum_S_KBH, sum_E_KBH, sum_I_KBH, sum_R_KBH);
        }
        // lukker filen efter vi har printet tallene ind i filen.
        fclose(file);
        // gnuplot commands der laver headers og labels.
        fprintf(pipe, "set title 'SIR Model Simulation: Aalborg og Copenhagen'\n");
        fprintf(pipe, "set xlabel 'Days'\n");
        fprintf(pipe, "set ylabel 'Population'\n");

        // plotter data_file.txt data og lukker pipelinen for begge byer
        fprintf(pipe, "plot 'data_file.txt' using 1:2 with lines lw 2 title 'AAL Susceptible', \\\n");
        fprintf(pipe, "     'data_file.txt' using 1:3 with lines lw 2 title 'AAL Exposed', \\\n");
        fprintf(pipe, "     'data_file.txt' using 1:4 with lines lw 2 title 'AAL Infected', \\\n");
        fprintf(pipe, "     'data_file.txt' using 1:5 with lines lw 2 title 'AAL Recovered', \\\n");
        fprintf(pipe, "     'data_file.txt' using 1:6 with lines lw 2 title 'KBH Susceptible', \\\n");
        fprintf(pipe, "     'data_file.txt' using 1:7 with lines lw 2 title 'AAL Infected', \\\n");
        fprintf(pipe, "     'data_file.txt' using 1:8 with lines lw 2 title 'KBH Exposed', \\\n");
        fprintf(pipe, "     'data_file.txt' using 1:9 with lines lw 2 title 'KBH Recovered'\n");
        fflush(pipe);
        _pclose(pipe);
    }
    return 0;
}