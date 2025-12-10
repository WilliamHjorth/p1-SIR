#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Befolkningen er opdelt i 4 aldersgrupper
#define ALDERS_GRUPPER 4

// Antal sub-steps pr. dag for den stokastiske simulering
#define STEPS_PER_DAY 10

typedef struct
{
    int dage;
    float N, S, E, I, H, R;
    float beta, gamma, sigma, h, t;
} SEIHRS_model;

SEIHRS_model tekstfil[2];
SEIHRS_model tekstfil_orig[2];

// Funktionsprototyper
void brugerInput();
char *spoergOmFilnavn(void);
SEIHRS_model indlaasFil(FILE *f);
void appOgVaccine(int *use_app, int *use_vaccine);
void simulerEpidemi(int model_type, int use_app, int use_vaccine, int valg_input, FILE *output_fil, int replicate_num, int is_stochastic, int print_to_terminal);
void koerFlereKopier(int model_type, int use_app, int use_vaccine, int numReplicates, int valg_input);
long poisson(double lambda);
void lavGnuplotScript(const char *scriptFile, const char *dataFile, int numReplicates, int model_type, int valg_input);
void lavEnkeltGnuplotScript(const char *scriptFile, const char *dataFile, int model_type, int valg_input);

// Mainfunktion
int main(void)
{
    srand((unsigned int)time(NULL));

    brugerInput();

    return 0;
}

// Brugermenu
void brugerInput()
{
    printf("\nDette er et program, der simulerer smittespredning!");

    // Vælg model
    char valg[8];
    printf("\nVælg model (SIR, SEIR, SEIHRS): ");

    scanf(" %7s", valg);

    int model_type = 0;
    if (strcmp(valg, "SIR") == 0 || strcmp(valg, "sir") == 0)
    {
        model_type = 1;
    }
    else if (strcmp(valg, "SEIR") == 0 || strcmp(valg, "seir") == 0)
    {
        model_type = 2;
    }
    else if (strcmp(valg, "SEIHRS") == 0 || strcmp(valg, "seihrs") == 0)
    {
        model_type = 3;
    }
    else
    {
        printf("Ugyldigt valg.\n");
        return;
    }

    // Vælg antal filer (1 = én fil (én by) 2 = to filer (sammenligning))
    int valg_input = 0;

    printf("Tast 1 eller 2 for antal filer (2 = sammenligning): ");
    scanf(" %d", &valg_input);

    if (valg_input != 1 && valg_input != 2)
    {
        printf("Ugyldigt valg.\n");
        return;
    }

    // Hent filnavn i anden funktion
    char *filnavn = spoergOmFilnavn();

    // Åbn filen
    FILE *input_fil_1 = fopen(filnavn, "r");

    if (input_fil_1 == NULL)
    {
        printf("Kunne ikke åbne filen: %s\n", filnavn);
        printf("Tjek at filen findes i samme mappe som programmet.\n");
        exit(EXIT_FAILURE);
    }

    tekstfil[0] = indlaasFil(input_fil_1);
    tekstfil_orig[0] = tekstfil[0];
    fclose(input_fil_1);

    if (valg_input == 2)
    {
        char *filnavn2 = spoergOmFilnavn();
        FILE *input_fil_2 = fopen(filnavn2, "r");
        if (input_fil_2 == NULL)
        {
            printf("Kunne ikke åbne filen: %s\n", filnavn2);
            fclose(input_fil_1);
            return;
        }

        tekstfil[1] = indlaasFil(input_fil_2);
        tekstfil_orig[1] = tekstfil[1];

        fclose(input_fil_2);
    }
    // Vælg simuleringstype
    char sim_type;
    printf("Vælg simulerings type:\n");
    printf(" N) Normal (deterministisk)\n");
    printf(" S) Stokastisk (multiple replicates)\n");
    printf("Tast valg: ");
    scanf(" %c", &sim_type);

    if (sim_type == 'N' || sim_type == 'n')
    {
        // Normal deterministisk simulering
        int use_app = 0, use_vaccine = 0;
        appOgVaccine(&use_app, &use_vaccine);

        FILE *output_fil = fopen("data_file.txt", "w");
        if (!output_fil)
        {
            printf("Kan ikke åbne fil\n");
            return;
        }

        simulerEpidemi(model_type, use_app, use_vaccine, valg_input, output_fil, 0, 0, 1);

        fclose(output_fil);

        // Lav gnuplot script for en enkelt simulering
        lavEnkeltGnuplotScript("plot_single.gnu", "data_file.txt", model_type, valg_input);
        printf("Gnuplot script gemt i plot_single.gnu\n");
        printf("Kør: gnuplot plot_single.gnu\n");

        printf("Simuleringen er færdig. Data er gemt i data_file.txt\n");
    }
    else if (sim_type == 'S' || sim_type == 's')
    {
        // Stokastisk simulering med flere kopier
        int numReplicates = 0;
        printf("Indtast antal replicates: ");

        if (scanf("%d", &numReplicates) != 1 || numReplicates <= 0)
        {
            printf("Ugyldigt antal replicates.\n");
            return;
        }

        int use_app = 0, use_vaccine = 0;
        appOgVaccine(&use_app, &use_vaccine);

        koerFlereKopier(model_type, use_app, use_vaccine, numReplicates, valg_input);

        printf("Simuleringen er færdig. Data er gemt i stochastic_replicates.txt\n");
    }
    else
    {
        printf("Ugyldigt valg.\n");
    }
}

char *spoergOmFilnavn(void)
{
    static char filnavn[250];

    // Bed brugeren om filnavn
    printf("Upload en tekstfil (se evt skabelon: skabelon.seihr.txt)\n");
    printf("Indtast navnet på din fil (f.eks. SEIHR.txt): ");

    scanf("%249s", filnavn); // Læser ét ord, undgår buffer overflow
    return filnavn;
}

// Funktion der læser filens indhold
SEIHRS_model indlaasFil(FILE *input_fil)
{

    SEIHRS_model seihr;

    fscanf(input_fil, " %*[^0-9]%d", &seihr.dage);

    fscanf(input_fil, " %*[^0-9]%f", &seihr.N);
    fscanf(input_fil, " %*[^0-9]%f", &seihr.S);
    fscanf(input_fil, " %*[^0-9]%f", &seihr.E);
    fscanf(input_fil, " %*[^0-9]%f", &seihr.I);
    fscanf(input_fil, " %*[^0-9]%f", &seihr.H);
    fscanf(input_fil, " %*[^0-9]%f", &seihr.R);

    fscanf(input_fil, " %*[^0-9]%f", &seihr.beta);
    fscanf(input_fil, " %*[^0-9]%f", &seihr.gamma);
    fscanf(input_fil, " %*[^0-9]%f", &seihr.sigma);
    fscanf(input_fil, " %*[^0-9]%f", &seihr.h);
    fscanf(input_fil, " %*[^0-9]%f", &seihr.t);

    printf("Din fil indeholder følgende værdier:\n");
    printf(" Tid i dage = %d\n\n N = %.3f\n S = %.3f\n E = %.3f\n I = %.3f\n H = %.3f\n R = %.3f\n\n Beta = %.3f\n Gamma = %.3f\n Sigma = %.3f\n h = %.3f\n t = %.3f\n", seihr.dage, seihr.N, seihr.S, seihr.E, seihr.I, seihr.H, seihr.R, seihr.beta, seihr.gamma, seihr.sigma, seihr.h, seihr.t);

    return seihr;
}

// Vaccine og app
void appOgVaccine(int *use_app, int *use_vaccine)
{
    char app, vaccine;
    printf("Er smittestop-app aktiv (j/n)? ");
    scanf(" %c", &app);
    printf("Er vaccine udrullet (j/n)? ");
    scanf(" %c", &vaccine);

    *use_app = (app == 'j' || app == 'J') ? 1 : 0;
    *use_vaccine = (vaccine == 'j' || vaccine == 'J') ? 1 : 0;
}

// Hovedsimulering for én eller begge filer
void simulerEpidemi(int model_type, int use_app, int use_vaccine, int valg_input, FILE *file, int replicate_num, int is_stochastic, int print_to_terminal)
{
    tekstfil[0] = tekstfil_orig[0];
    if (valg_input == 2)
        tekstfil[1] = tekstfil_orig[1];

    /* Vaccine-effekt og app-effekt anvendes kun én gang og
      efterfølgende simuleringer starter altid fra samme udgangspunkt. */
    if (use_vaccine)
    {
        // Reducérer modtagelige med 20%
        tekstfil[0].S *= 0.8;
        if (valg_input == 2)
        {
            tekstfil[1].S *= 0.8;
        }

        // Reducérer smitteevne (beta) med 50%
        tekstfil[0].beta *= 0.5;
        if (valg_input == 2)
        {
            tekstfil[1].beta *= 0.5;
        }
    }
    // App effekt bliver brugt én gang
    if (use_app)
    {
        tekstfil[0].beta *= 0.75; // Reducerer kontakter
        if (valg_input == 2)
        {
            tekstfil[1].beta *= 0.75;
        }
    }

    // Fordeler på aldersgrupper
    float S_input_1[ALDERS_GRUPPER], E_input_1[ALDERS_GRUPPER], I_input_1[ALDERS_GRUPPER], R_input_1[ALDERS_GRUPPER], H_input_1[ALDERS_GRUPPER];
    float S_input_2[ALDERS_GRUPPER], E_input_2[ALDERS_GRUPPER], I_input_2[ALDERS_GRUPPER], R_input_2[ALDERS_GRUPPER], H_input_2[ALDERS_GRUPPER];

    for (int i = 0; i < ALDERS_GRUPPER; i++)
    {
        S_input_1[i] = tekstfil[0].S / ALDERS_GRUPPER;
        E_input_1[i] = tekstfil[0].E / ALDERS_GRUPPER;
        I_input_1[i] = tekstfil[0].I / ALDERS_GRUPPER;
        R_input_1[i] = tekstfil[0].R / ALDERS_GRUPPER;
        H_input_1[i] = tekstfil[0].H / ALDERS_GRUPPER;

        // Input 2 kun hvis den er valgt (ellers sat til 0)
        if (valg_input == 2)
        {
            S_input_2[i] = tekstfil[1].S / ALDERS_GRUPPER;
            E_input_2[i] = tekstfil[1].E / ALDERS_GRUPPER;
            I_input_2[i] = tekstfil[1].I / ALDERS_GRUPPER;
            R_input_2[i] = tekstfil[1].R / ALDERS_GRUPPER;
            H_input_2[i] = tekstfil[1].H / ALDERS_GRUPPER;
        }
        else
        {
            S_input_2[i] = 0;
            E_input_2[i] = 0;
            I_input_2[i] = 0;
            R_input_2[i] = 0;
            H_input_2[i] = 0;
        }
    }

    // Aldersspecifikke parametre
    float age_h_1[ALDERS_GRUPPER] = {0.3, 0.3, 0.3, 0.7};    // Sandsynlighed for at ende på hospitalet
    float age_beta_1[ALDERS_GRUPPER] = {0.5, 0.8, 0.6, 0.4}; // Antal kontakter

    float age_h_2[ALDERS_GRUPPER] = {0.3, 0.3, 0.3, 0.7};    // Sandsynlighed for at ende på hospitalet
    float age_beta_2[ALDERS_GRUPPER] = {0.5, 0.8, 0.6, 0.4}; // Antal kontakter

    float age_modtagelig_igen[ALDERS_GRUPPER] = {1.0f / 365, 1.0f / 300, 1.0f / 250, 1.0f / 180}; // jo yngre, jo længere er man immun

    // Header til datafil
    if (file != NULL)
    {
        if (valg_input == 1)
            fprintf(file, "# Replicate %d - Input 1\n", replicate_num);
        else if (valg_input == 2)
            fprintf(file, "# Replicate %d - Both\n", replicate_num);
    }

    // Simulering (tids-loop)
    for (int n = 0; n < tekstfil[0].dage; n++)
    {
        // Transfer rate pr dag
        float t1 = tekstfil[0].t;

        // Stokastisk del
        if (is_stochastic)
        {
            double dt = 1.0 / (double)STEPS_PER_DAY;

            // Udfører STEPS_PER_DAY sub-steps per dag
            for (int step = 0; step < STEPS_PER_DAY; step++)
            {
                // Udregn infectious totals hver sub-step
                float total_I_input_1 = 0, total_I_input_2 = 0;

                for (int ii = 0; ii < ALDERS_GRUPPER; ii++)
                {
                    total_I_input_1 += I_input_1[ii];
                    if (valg_input == 2)
                    {
                        total_I_input_2 += I_input_2[ii];
                    }
                }

                for (int i = 0; i < ALDERS_GRUPPER; i++)
                {

                    float S_out_1 = 0, E_out_1 = 0, I_out_1 = 0, R_out_1 = 0;
                    float S_out_2 = 0, E_out_2 = 0, I_out_2 = 0, R_out_2 = 0;

                    if (valg_input == 2) // Migration mellem byer sker kun hvis der er to filer (ellers sat til 0)
                    {
                        // Migration (skaleret med dt) - hospitaliserede kan ikke flytte sig
                        S_out_1 = t1 * dt * S_input_1[i];
                        E_out_1 = t1 * dt * E_input_1[i];
                        I_out_1 = t1 * dt * I_input_1[i];
                        R_out_1 = t1 * dt * R_input_1[i];

                        S_out_2 = t1 * dt * S_input_2[i];
                        E_out_2 = t1 * dt * E_input_2[i];
                        I_out_2 = t1 * dt * I_input_2[i];
                        R_out_2 = t1 * dt * R_input_2[i];
                    }

                    //  Input 1 - rate skaleret med dt
                    double rate_inf_1 = tekstfil[0].beta * S_input_1[i] * total_I_input_1 / tekstfil[0].N;
                    long n_inf_1 = poisson(rate_inf_1 * dt);
                    long n_prog_1 = 0;
                    long n_rec_1 = poisson(tekstfil[0].gamma * I_input_1[i] * dt);
                    long n_hosp_1 = 0;
                    long n_h_rec_1 = 0;
                    long n_R_to_S_1 = 0;

                    if (model_type == 2)
                    {
                        n_prog_1 = poisson(tekstfil[0].sigma * E_input_1[i] * dt);
                    }
                    else if (model_type == 3)
                    {
                        n_prog_1 = poisson(tekstfil[0].sigma * E_input_1[i] * dt);
                        n_hosp_1 = poisson(tekstfil[0].h * I_input_1[i] * dt);
                        n_h_rec_1 = poisson((tekstfil[0].gamma / 2.0) * H_input_1[i] * dt);
                        n_R_to_S_1 = poisson(age_modtagelig_igen[i] * R_input_1[i] * dt);
                    }
                    if (n_inf_1 > (long)S_input_1[i])
                        n_inf_1 = (long)S_input_1[i];
                    if (n_prog_1 > (long)E_input_1[i])
                        n_prog_1 = (long)E_input_1[i];
                    if (n_rec_1 > (long)I_input_1[i])
                        n_rec_1 = (long)I_input_1[i];
                    if (n_hosp_1 > (long)I_input_1[i])
                        n_hosp_1 = (long)I_input_1[i];
                    if (n_h_rec_1 > (long)H_input_1[i])
                        n_h_rec_1 = (long)H_input_1[i];
                    if (n_R_to_S_1 > (long)R_input_1[i])
                        n_R_to_S_1 = (long)R_input_1[i];

                    if (model_type == 1)
                    {
                        S_input_1[i] -= n_inf_1;
                        I_input_1[i] += n_inf_1 - n_rec_1;
                        R_input_1[i] += n_rec_1;
                    }
                    else if (model_type == 2)
                    {
                        S_input_1[i] -= n_inf_1;
                        E_input_1[i] += n_inf_1 - n_prog_1;
                        I_input_1[i] += n_prog_1 - n_rec_1;
                        R_input_1[i] += n_rec_1;
                    }
                    else if (model_type == 3)
                    {
                        S_input_1[i] += n_R_to_S_1 - n_inf_1;
                        E_input_1[i] += n_inf_1 - n_prog_1;
                        I_input_1[i] += n_prog_1 - n_rec_1 - n_hosp_1;
                        H_input_1[i] += n_hosp_1 - n_h_rec_1;
                        R_input_1[i] += n_rec_1 + n_h_rec_1 - n_R_to_S_1;
                    }

                    // Input 2 - rates skaleret med dt (kun hvis valg_input == 2)
                    if (valg_input == 2)
                    {
                        double rate_inf_2 = tekstfil[1].beta * S_input_2[i] * total_I_input_2 / tekstfil[1].N;
                        long n_inf_2 = poisson(rate_inf_2 * dt);
                        long n_prog_2 = 0;
                        long n_rec_2 = poisson(tekstfil[1].gamma * I_input_2[i] * dt);
                        long n_hosp_2 = 0;
                        long n_h_rec_2 = 0;
                        long n_R_to_S_2 = 0;

                        if (model_type == 2)
                        {
                            n_prog_2 = poisson(tekstfil[1].sigma * E_input_2[i] * dt);
                        }
                        else if (model_type == 3)
                        {
                            n_prog_2 = poisson(tekstfil[1].sigma * E_input_2[i] * dt);
                            n_hosp_2 = poisson(tekstfil[1].h * I_input_2[i] * dt);
                            n_h_rec_2 = poisson((tekstfil[1].gamma / 2.0) * H_input_2[i] * dt);
                            n_R_to_S_2 = poisson(age_modtagelig_igen[i] * R_input_2[i] * dt); // tab af immunitet (R til S)
                        }
                        if (n_inf_2 > (long)S_input_2[i])
                            n_inf_2 = (long)S_input_2[i];
                        if (n_prog_2 > (long)E_input_2[i])
                            n_prog_2 = (long)E_input_2[i];
                        if (n_rec_2 > (long)I_input_2[i])
                            n_rec_2 = (long)I_input_2[i];
                        if (n_hosp_2 > (long)I_input_2[i])
                            n_hosp_2 = (long)I_input_2[i];
                        if (n_h_rec_2 > (long)H_input_2[i])
                            n_h_rec_2 = (long)H_input_2[i];
                        if (n_R_to_S_2 > (long)R_input_2[i])
                            n_R_to_S_2 = (long)R_input_2[i];

                        if (model_type == 1)
                        {
                            S_input_2[i] -= n_inf_2;
                            I_input_2[i] += n_inf_2 - n_rec_2;
                            R_input_2[i] += n_rec_2;
                        }
                        else if (model_type == 2)
                        {
                            S_input_2[i] -= n_inf_2;
                            E_input_2[i] += n_inf_2 - n_prog_2;
                            I_input_2[i] += n_prog_2 - n_rec_2;
                            R_input_2[i] += n_rec_2;
                        }
                        else if (model_type == 3)
                        {
                            S_input_2[i] += n_R_to_S_2 - n_inf_2;
                            E_input_2[i] += n_inf_2 - n_prog_2;
                            I_input_2[i] += n_prog_2 - n_rec_2 - n_hosp_2;
                            H_input_2[i] += n_hosp_2 - n_h_rec_2;
                            R_input_2[i] += n_rec_2 + n_h_rec_2 - n_R_to_S_2;
                        }
                    }

                    if (valg_input == 2)
                    {
                        // Migration mellem byerne (kun hvis valg_input == 2)
                        S_input_1[i] += -S_out_1 + S_out_2;
                        E_input_1[i] += -E_out_1 + E_out_2;
                        I_input_1[i] += -I_out_1 + I_out_2;
                        R_input_1[i] += -R_out_1 + R_out_2;

                        S_input_2[i] += -S_out_2 + S_out_1;
                        E_input_2[i] += -E_out_2 + E_out_1;
                        I_input_2[i] += -I_out_2 + I_out_1;
                        R_input_2[i] += -R_out_2 + R_out_1;
                    }

                    // Beskyt mod negative værdier
                    S_input_1[i] = fmaxf(S_input_1[i], 0.0f);
                    E_input_1[i] = fmaxf(E_input_1[i], 0.0f);
                    I_input_1[i] = fmaxf(I_input_1[i], 0.0f);
                    R_input_1[i] = fmaxf(R_input_1[i], 0.0f);
                    H_input_1[i] = fmaxf(H_input_1[i], 0.0f);

                    S_input_2[i] = fmaxf(S_input_2[i], 0.0f);
                    E_input_2[i] = fmaxf(E_input_2[i], 0.0f);
                    I_input_2[i] = fmaxf(I_input_2[i], 0.0f);
                    R_input_2[i] = fmaxf(R_input_2[i], 0.0f);
                    H_input_2[i] = fmaxf(H_input_2[i], 0.0f);
                }
            }
        }
        else
        {
            // Deterministisk del

            float total_input_1 = 0;
            float total_input_2 = 0;

            for (int i = 0; i < ALDERS_GRUPPER; i++)
            {
                total_input_1 += I_input_1[i];
                if (valg_input == 2)
                {
                    total_input_2 += I_input_2[i];
                }
            }

            float dS_input_1[ALDERS_GRUPPER], dE_input_1[ALDERS_GRUPPER], dI_input_1[ALDERS_GRUPPER], dR_input_1[ALDERS_GRUPPER], dH_input_1[ALDERS_GRUPPER];
            float dS_input_2[ALDERS_GRUPPER], dE_input_2[ALDERS_GRUPPER], dI_input_2[ALDERS_GRUPPER], dR_input_2[ALDERS_GRUPPER], dH_input_2[ALDERS_GRUPPER];

            float sum_S_input_1 = 0, sum_E_input_1 = 0, sum_I_input_1 = 0, sum_R_input_1 = 0, sum_H_input_1 = 0;
            float sum_S_input_2 = 0, sum_E_input_2 = 0, sum_I_input_2 = 0, sum_R_input_2 = 0, sum_H_input_2 = 0;

            for (int i = 0; i < ALDERS_GRUPPER; i++)
            {
                sum_S_input_1 += S_input_1[i];
                sum_E_input_1 += E_input_1[i];
                sum_I_input_1 += I_input_1[i];
                sum_R_input_1 += R_input_1[i];
                sum_H_input_1 += H_input_1[i];

                sum_S_input_2 += S_input_2[i];
                sum_E_input_2 += E_input_2[i];
                sum_I_input_2 += I_input_2[i];
                sum_R_input_2 += R_input_2[i];
                sum_H_input_2 += H_input_2[i];

                // Default =  (ingen migration if valg_input == 1)
                float S_ud_1 = 0, E_ud_1 = 0, I_ud_1 = 0, R_ud_1 = 0;
                float S_ud_2 = 0, E_ud_2 = 0, I_ud_2 = 0, R_ud_2 = 0;

                if (valg_input == 2)
                {
                    // Transfer rate per aldersgruppe (H rejser ikke)
                    float t1 = tekstfil[0].t;

                    S_ud_1 = t1 * S_input_1[i];
                    E_ud_1 = t1 * E_input_1[i];
                    I_ud_1 = t1 * I_input_1[i];
                    R_ud_1 = t1 * R_input_1[i];

                    S_ud_2 = t1 * S_input_2[i];
                    E_ud_2 = t1 * E_input_2[i];
                    I_ud_2 = t1 * I_input_2[i];
                    R_ud_2 = t1 * R_input_2[i];
                }

                // Aldersspecifikke parametre
                float beta_i_1 = tekstfil[0].beta * age_beta_1[i];
                float gamma_i_1 = tekstfil[0].gamma;
                float sigma_i_1 = tekstfil[0].sigma;
                float h_i_1 = age_h_1[i];

                if (valg_input == 2)
                {
                    float beta_i_2 = tekstfil[1].beta * age_beta_2[i];
                    float gamma_i_2 = tekstfil[1].gamma;
                    float sigma_i_2 = tekstfil[1].sigma;
                    float h_i_2 = age_h_2[i];
                }

                if (model_type == 1)
                { // SIR
                    dS_input_1[i] = -beta_i_1 * S_input_1[i] * total_input_1 / tekstfil[0].N - S_ud_1 + S_ud_2;
                    dI_input_1[i] = beta_i_1 * S_input_1[i] * total_input_1 / tekstfil[0].N - gamma_i_1 * I_input_1[i] - I_ud_1 + I_ud_2;
                    dR_input_1[i] = gamma_i_1 * I_input_1[i] - R_ud_1 + R_ud_2;
                    dE_input_1[i] = 0;
                    dH_input_1[i] = 0;
                }
                else if (model_type == 2)
                { // SEIR
                    dS_input_1[i] = -beta_i_1 * S_input_1[i] * total_input_1 / tekstfil[0].N - S_ud_1 + S_ud_2;
                    dE_input_1[i] = beta_i_1 * S_input_1[i] * total_input_1 / tekstfil[0].N - sigma_i_1 * E_input_1[i] - E_ud_1 + E_ud_2;
                    dI_input_1[i] = sigma_i_1 * E_input_1[i] - gamma_i_1 * I_input_1[i] - I_ud_1 + I_ud_2;
                    dR_input_1[i] = gamma_i_1 * I_input_1[i] - R_ud_1 + R_ud_2;
                    dH_input_1[i] = 0;
                }
                else if (model_type == 3)
                { // SEIHRS
                    dS_input_1[i] = -beta_i_1 * S_input_1[i] * total_input_1 / tekstfil[0].N + age_modtagelig_igen[i] * R_input_1[i] - S_ud_1 + S_ud_2;
                    dE_input_1[i] = beta_i_1 * S_input_1[i] * total_input_1 / tekstfil[0].N - sigma_i_1 * E_input_1[i] - E_ud_1 + E_ud_2;
                    dI_input_1[i] = sigma_i_1 * E_input_1[i] - gamma_i_1 * I_input_1[i] - h_i_1 * I_input_1[i] - I_ud_1 + I_ud_2;
                    dH_input_1[i] = h_i_1 * I_input_1[i] - (gamma_i_1 / 2) * H_input_1[i];
                    dR_input_1[i] = gamma_i_1 * I_input_1[i] + (gamma_i_1 / 2) * H_input_1[i] - age_modtagelig_igen[i] * R_input_1[i] - R_ud_1 + R_ud_2;
                }
                // By 2 – kun hvis valg_input == 2
                if (valg_input == 2)
                {

                    if (model_type == 1)
                    {
                        dS_input_2[i] = -tekstfil[1].beta * S_input_2[i] * total_input_2 / tekstfil[1].N - S_ud_2 + S_ud_1;
                        dI_input_2[i] = tekstfil[1].beta * S_input_2[i] * total_input_2 / tekstfil[1].N - tekstfil[1].gamma * I_input_2[i] - I_ud_2 + I_ud_1;
                        dR_input_2[i] = tekstfil[1].gamma * I_input_2[i] - R_ud_2 + R_ud_1;
                        dE_input_2[i] = 0;
                        dH_input_2[i] = 0;
                    }
                    else if (model_type == 2)
                    {
                        dS_input_2[i] = -tekstfil[1].beta * S_input_2[i] * total_input_2 / tekstfil[1].N - S_ud_2 + S_ud_1;
                        dE_input_2[i] = tekstfil[1].beta * S_input_2[i] * total_input_2 / tekstfil[1].N - tekstfil[1].gamma * E_input_2[i] - E_ud_2 + E_ud_1;
                        dI_input_2[i] = tekstfil[1].sigma * E_input_2[i] - tekstfil[1].gamma * I_input_2[i] - I_ud_2 + I_ud_1;
                        dR_input_2[i] = tekstfil[1].gamma * I_input_2[i] - R_ud_2 + R_ud_1;
                        dH_input_2[i] = 0;
                    }
                    else if (model_type == 3)
                    {
                        dS_input_2[i] = -tekstfil[1].beta * S_input_2[i] * total_input_2 / tekstfil[1].N + age_modtagelig_igen[i] * R_input_2[i] - S_ud_2 + S_ud_1;
                        dE_input_2[i] = tekstfil[1].beta * S_input_2[i] * total_input_2 / tekstfil[1].N - tekstfil[1].sigma * E_input_2[i] - E_ud_2 + E_ud_1;
                        dI_input_2[i] = tekstfil[1].sigma * E_input_2[i] - tekstfil[1].gamma * I_input_2[i] - tekstfil[1].h * I_input_2[i] - I_ud_2 + I_ud_1;
                        dH_input_2[i] = tekstfil[1].h * I_input_2[i] - (tekstfil[1].gamma / 2) * H_input_2[i];
                        dR_input_2[i] = tekstfil[1].gamma * I_input_2[i] + (tekstfil[1].gamma / 2) * H_input_2[i] - age_modtagelig_igen[i] * R_input_2[i] - R_ud_2 + R_ud_1;
                    }
                }
                else
                {
                    // Hvis kun én by, sæt ændringer for by 2 til 0
                    dS_input_2[i] = dE_input_2[i] = dI_input_2[i] = dR_input_2[i] = dH_input_2[i] = 0;
                }

                // Opdater værdier (deterministisk)
                S_input_1[i] += dS_input_1[i];
                E_input_1[i] += dE_input_1[i];
                I_input_1[i] += dI_input_1[i];
                R_input_1[i] += dR_input_1[i];
                H_input_1[i] += dH_input_1[i];

                S_input_2[i] += dS_input_2[i];
                E_input_2[i] += dE_input_2[i];
                I_input_2[i] += dI_input_2[i];
                R_input_2[i] += dR_input_2[i];
                H_input_2[i] += dH_input_2[i];

                // Negativ populationsbeskyttelse
                S_input_1[i] = fmaxf(S_input_1[i], 0);
                E_input_1[i] = fmaxf(E_input_1[i], 0);
                I_input_1[i] = fmaxf(I_input_1[i], 0);
                R_input_1[i] = fmaxf(R_input_1[i], 0);
                H_input_1[i] = fmaxf(H_input_1[i], 0);

                S_input_2[i] = fmaxf(S_input_2[i], 0);
                E_input_2[i] = fmaxf(E_input_2[i], 0);
                I_input_2[i] = fmaxf(I_input_2[i], 0);
                R_input_2[i] = fmaxf(R_input_2[i], 0);
                H_input_2[i] = fmaxf(H_input_2[i], 0);
            }
        }

        // Summér alle aldersgrupper til output
        float sum_S_input_1 = 0, sum_E_input_1 = 0, sum_I_input_1 = 0, sum_R_input_1 = 0, sum_H_input_1 = 0;
        float sum_S_input_2 = 0, sum_E_input_2 = 0, sum_I_input_2 = 0, sum_R_input_2 = 0, sum_H_input_2 = 0;

        for (int i = 0; i < ALDERS_GRUPPER; i++)
        {
            sum_S_input_1 += S_input_1[i];
            sum_E_input_1 += E_input_1[i];
            sum_I_input_1 += I_input_1[i];
            sum_R_input_1 += R_input_1[i];
            sum_H_input_1 += H_input_1[i];

            sum_S_input_2 += S_input_2[i];
            sum_E_input_2 += E_input_2[i];
            sum_I_input_2 += I_input_2[i];
            sum_R_input_2 += R_input_2[i];
            sum_H_input_2 += H_input_2[i];
        }

        // Udskriv værdier (dag for dag)
        if (model_type == 1) // sir
        {
            if (print_to_terminal)
            {
                if (valg_input == 1)
                    printf("Day %d | Input 1:(S=%.0f,I=%.0f,R=%.0f)\n", n, sum_S_input_1, sum_I_input_1, sum_R_input_1);
                else if (valg_input == 2)
                    printf("Day %d | Input 1:(S=%.0f, I=%.0f, R=%.0f) Input 2: (S=%.0f, I=%.0f, R=%.0f)\n", n, sum_S_input_1, sum_I_input_1, sum_R_input_1, sum_S_input_2, sum_I_input_2, sum_R_input_2);
            }

            if (file != NULL)
            {
                fprintf(file, "%d %.6f %.6f %.6f %.6f %.6f %.6f\n",
                        n, sum_S_input_1, sum_I_input_1, sum_R_input_1,
                        sum_S_input_2, sum_I_input_2, sum_R_input_2);
            }
        }
        else if (model_type == 2) // SEIR
        {
            if (print_to_terminal)
            {
                if (valg_input == 1)
                    printf("Day %d | Input 1:(S=%.0f, E=%.0f, I=%.0f,R=%.0f)\n", n, sum_S_input_1, sum_E_input_1, sum_I_input_1, sum_R_input_1);
                else if (valg_input == 2)
                    printf("Day %d | Input 1:(S=%.0f, E=%.0f, I=%.0f, R=%.0f) Input 2: (S=%.0f, E=%.0f, I=%.0f, R=%.0f)\n", n, sum_S_input_1, sum_E_input_1, sum_I_input_1, sum_R_input_1, sum_S_input_2, sum_E_input_2, sum_I_input_2, sum_R_input_2);
            }

            if (file != NULL)
            {
                fprintf(file, "%d %.6f %.6f %.6f %.6f %.6f %.6f %.6f %.6f\n",
                        n, sum_S_input_1, sum_E_input_1, sum_I_input_1, sum_R_input_1,
                        sum_S_input_2, sum_E_input_2, sum_I_input_2, sum_R_input_2);
            }
        }
        else if (model_type == 3) // SEIHRS
        {
            if (print_to_terminal)
            {
                if (valg_input == 1)
                    printf("Day %d | Input 1:(S=%.0f, E=%.0f, I=%.0f, H=%.0f, R=%.0f)\n", n, sum_S_input_1, sum_E_input_1, sum_I_input_1, sum_H_input_1, sum_R_input_1);
                else if (valg_input == 2)
                    printf("Day %d | Input 1:(S=%.0f, E=%.0f, I=%.0f, H=%.0f, R=%.0f) Input 2: (S=%.0f, E=%.0f, I=%.0f, H=%.0f, R=%.0f)\n", n, sum_S_input_1, sum_E_input_1, sum_I_input_1, sum_H_input_1, sum_R_input_1, sum_S_input_2, sum_E_input_2, sum_I_input_2, sum_H_input_2, sum_R_input_2);
            }

            if (file != NULL)
            {
                fprintf(file, "%d %.6f %.6f %.6f %.6f %.6f %.6f %.6f %.6f %.6f %.6f\n",
                        n, sum_S_input_1, sum_E_input_1, sum_I_input_1, sum_H_input_1, sum_R_input_1,
                        sum_S_input_2, sum_E_input_2, sum_I_input_2, sum_H_input_2, sum_R_input_2);
            }
        }
    }

    if (file != NULL)
    {
        fprintf(file, "\n\n");
    }
}

// Kører flere kopier
void koerFlereKopier(int model_type, int use_app, int use_vaccine, int numReplicates, int valg_input)
{
    FILE *file = fopen("stochastic_replicates.txt", "w");
    if (!file)
    {
        printf("Kan ikke åbne fil\n");
        return;
    }

    printf("Kører %d replicates...\n", numReplicates);

    for (int rep = 0; rep < numReplicates; rep++)
    {
        // reset sker allerede i simulerEpidemi
        printf("Replicate %d/%d\n", rep + 1, numReplicates);
        simulerEpidemi(model_type, use_app, use_vaccine, valg_input, file, rep, 1, 1);
    }

    fclose(file);
    printf("Alle replicates færdige. Data gemt i stochastic_replicates.txt\n");

    // Lav gnuplot script
    lavGnuplotScript("plot_replicates.gnu", "stochastic_replicates.txt", numReplicates, model_type, valg_input);
    printf("Gnuplot script gemt i plot_replicates.gnu\n");
    printf("Kør: gnuplot plot_replicates.gnu\n");
}

void lavGnuplotScript(const char *scriptFile, const char *dataFile, int numReplicates, int model_type, int valg_input)
{
    FILE *fp = fopen(scriptFile, "w");
    if (fp == NULL)
    {
        fprintf(stderr, "Error opening file %s\n", scriptFile);
        return;
    }

    fprintf(fp, "set xlabel 'Dage'\n");
    fprintf(fp, "set ylabel 'Antal individer'\n");

    if (model_type == 1)
    {
        fprintf(fp, "set title 'Stochastic SIR Model - %d Replicates (Input 1)'\n", numReplicates);
    }
    else if (model_type == 2)
    {
        fprintf(fp, "set title 'Stochastic SEIR Model - %d Replicates (Input 1)'\n", numReplicates);
    }
    else if (model_type == 3)
    {
        fprintf(fp, "set title 'Stochastic SEIHRS Model - %d Replicates (Input 1)'\n", numReplicates);
    }

    fprintf(fp, "set grid\n");
    fprintf(fp, "set key outside top center\n");
    fprintf(fp, "set key maxcols 5\n");

    // Definer semi-transparent farver
    fprintf(fp, "set style line 1 lc rgb '#8000FF00' lt 1 lw 1\n"); // Grøn for S
    fprintf(fp, "set style line 2 lc rgb '#80FFA500' lt 1 lw 1\n"); // Orange for E
    fprintf(fp, "set style line 3 lc rgb '#80FF0000' lt 1 lw 1\n"); // Rød for I
    fprintf(fp, "set style line 4 lc rgb '#80800080' lt 1 lw 1\n"); // Lilla for H
    fprintf(fp, "set style line 5 lc rgb '#800000FF' lt 1 lw 1\n"); // Blå for R

    // Plot baseret på valgte model

    int baseA = 1; // Input 1 kolonner start forskydning baseA -> S = baseA+1 -> kolonne 2
    int baseK = 4; // Input 2 kolonner start forskydning baseK -> S = baseK+1 -> kolonne 7

    // Byg en eksplicit plot-liste per replicate for at undgå gnuplot 'for' kompatibilitetsproblemer
    int first = 1;
    fprintf(fp, "plot ");
    for (int rep = 0; rep < numReplicates; rep++)
    {
        if (model_type == 1)
        {
            // SIR
            if (valg_input == 1 || valg_input == 2)
            {
                int colS = baseA + 1;
                int colI = baseA + 2;
                int colR = baseA + 3;
                if (!first)
                    fprintf(fp, ", \\\n");
                if (rep == 0)
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 1 title 'S (Input 1)'", dataFile, rep, colS);
                else
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 1 notitle", dataFile, rep, colS);
                fprintf(fp, ", \\\n");
                if (rep == 0)
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 3 title 'I (Input 1)'", dataFile, rep, colI);
                else
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 3 notitle", dataFile, rep, colI);
                fprintf(fp, ", \\\n");
                if (rep == 0)
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 5 title 'R (Input 1)'", dataFile, rep, colR);
                else
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 5 notitle", dataFile, rep, colR);
                first = 0;
            }
            if (valg_input == 2)
            {
                int colS = baseK + 1;
                int colI = baseK + 2;
                int colR = baseK + 3;
                if (!first)
                    fprintf(fp, ", \\\n");
                if (rep == 0)
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 1 title 'S (Input 2)'", dataFile, rep, colS);
                else
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 1 notitle", dataFile, rep, colS);
                fprintf(fp, ", \\\n");
                if (rep == 0)
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 3 title 'I (Input 2)'", dataFile, rep, colI);
                else
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 3 notitle", dataFile, rep, colI);
                fprintf(fp, ", \\\n");
                if (rep == 0)
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 5 title 'R (Input 2)'", dataFile, rep, colR);
                else
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 5 notitle", dataFile, rep, colR);
                first = 0;
            }
        }
        else if (model_type == 2)
        {
            // SEIR
            if (valg_input == 1 || valg_input == 2)
            {
                int colS = baseA + 1;
                int colE = baseA + 2;
                int colI = baseA + 3;
                int colR = baseA + 4;
                if (!first)
                    fprintf(fp, ", \\\n");
                if (rep == 0)
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 1 title 'S (Input 1)'", dataFile, rep, colS);
                else
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 1 notitle", dataFile, rep, colS);
                fprintf(fp, ", \\\n");
                if (rep == 0)
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 2 title 'E (Input 1)'", dataFile, rep, colE);
                else
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 2 notitle", dataFile, rep, colE);
                fprintf(fp, ", \\\n");
                if (rep == 0)
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 3 title 'I (Input 1)'", dataFile, rep, colI);
                else
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 3 notitle", dataFile, rep, colI);
                fprintf(fp, ", \\\n");
                if (rep == 0)
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 5 title 'R (Input 1)'", dataFile, rep, colR);
                else
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 5 notitle", dataFile, rep, colR);
                first = 0;
            }
            if (valg_input == 2)
            {
                int baseK = 5;
                int colS = baseK + 1;
                int colE = baseK + 2;
                int colI = baseK + 3;
                int colR = baseK + 4;
                if (!first)
                    fprintf(fp, ", \\\n");
                if (rep == 0)
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 1 title 'S (Input 2)'", dataFile, rep, colS);
                else
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 1 notitle", dataFile, rep, colS);
                fprintf(fp, ", \\\n");
                if (rep == 0)
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 2 title 'E (Input 2)'", dataFile, rep, colE);
                else
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 2 notitle", dataFile, rep, colE);
                fprintf(fp, ", \\\n");
                if (rep == 0)
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 3 title 'I (Input 2)'", dataFile, rep, colI);
                else
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 3 notitle", dataFile, rep, colI);
                fprintf(fp, ", \\\n");
                if (rep == 0)
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 5 title 'R (Input 2)'", dataFile, rep, colR);
                else
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 5 notitle", dataFile, rep, colR);
                first = 0;
            }
        }
        else if (model_type == 3)
        {
            // SEIHRS
            if (valg_input == 1 || valg_input == 2)
            {
                int colS = baseA + 1;
                int colE = baseA + 2;
                int colI = baseA + 3;
                int colH = baseA + 4;
                int colR = baseA + 5;
                if (!first)
                    fprintf(fp, ", \\\n");
                if (rep == 0)
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 1 title 'S (Input 1)'", dataFile, rep, colS);
                else
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 1 notitle", dataFile, rep, colS);
                fprintf(fp, ", \\\n");
                if (rep == 0)
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 2 title 'E (Input 1)'", dataFile, rep, colE);
                else
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 2 notitle", dataFile, rep, colE);
                fprintf(fp, ", \\\n");
                if (rep == 0)
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 3 title 'I (Input 1)'", dataFile, rep, colI);
                else
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 3 notitle", dataFile, rep, colI);
                fprintf(fp, ", \\\n");
                if (rep == 0)
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 4 title 'H (Input 1)'", dataFile, rep, colH);
                else
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 4 notitle", dataFile, rep, colH);
                fprintf(fp, ", \\\n");
                if (rep == 0)
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 5 title 'R (Input 1)'", dataFile, rep, colR);
                else
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 5 notitle", dataFile, rep, colR);
                first = 0;
            }
            if (valg_input == 2)
            {
                int baseK = 6;
                int colS = baseK + 1;
                int colE = baseK + 2;
                int colI = baseK + 3;
                int colH = baseK + 4;
                int colR = baseK + 5;
                if (!first)
                    fprintf(fp, ", \\\n");
                if (rep == 0)
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 1 title 'S (Input 2)'", dataFile, rep, colS);
                else
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 1 notitle", dataFile, rep, colS);
                fprintf(fp, ", \\\n");
                if (rep == 0)
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 2 title 'E (Input 2)'", dataFile, rep, colE);
                else
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 2 notitle", dataFile, rep, colE);
                fprintf(fp, ", \\\n");
                if (rep == 0)
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 3 title 'I (Input 2)'", dataFile, rep, colI);
                else
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 3 notitle", dataFile, rep, colI);
                fprintf(fp, ", \\\n");
                if (rep == 0)
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 4 title 'H (Input 2)'", dataFile, rep, colH);
                else
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 4 notitle", dataFile, rep, colH);
                fprintf(fp, ", \\\n");
                if (rep == 0)
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 5 title 'R (Input 2)'", dataFile, rep, colR);
                else
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 5 notitle", dataFile, rep, colR);
                first = 0;
            }
        }
    }
    fprintf(fp, "\n");

    fprintf(fp, "pause -1 'Tast enter for at lukke'\n");

    fclose(fp);
}

// Opret Gnuplot-script for enkelt simulering (normal/deterministisk)
void lavEnkeltGnuplotScript(const char *scriptFile, const char *dataFile, int model_type, int valg_input)
{
    FILE *fp = fopen(scriptFile, "w");
    if (fp == NULL)
    {
        fprintf(stderr, "Error opening file %s\n", scriptFile);
        return;
    }

    fprintf(fp, "set xlabel 'Dage'\n");
    fprintf(fp, "set ylabel 'Antal individer'\n");

    if (model_type == 1)
    {
        fprintf(fp, "set title 'SIR Model - Deterministisk Simulering'\n");
    }
    else if (model_type == 2)
    {
        fprintf(fp, "set title 'SEIR Model - Deterministisk Simulering'\n");
    }
    else if (model_type == 3)
    {
        fprintf(fp, "set title 'SEIHRS Model - Deterministisk Simulering'\n");
    }

    fprintf(fp, "set grid\n");
    fprintf(fp, "set key outside top center\n");
    fprintf(fp, "set key maxcols 5\n");

    // Definer farver (solid for enkelt simulering)
    fprintf(fp, "set style line 1 lc rgb '#00FF00' lt 1 lw 2\n"); // Grøn for S
    fprintf(fp, "set style line 2 lc rgb '#FFA500' lt 1 lw 2\n"); // Orange for E
    fprintf(fp, "set style line 3 lc rgb '#FF0000' lt 1 lw 2\n"); // Rød for I
    fprintf(fp, "set style line 4 lc rgb '#800080' lt 1 lw 2\n"); // Lilla for H
    fprintf(fp, "set style line 5 lc rgb '#0000FF' lt 1 lw 2\n"); // Blå for R

    int baseA = 1; // Input 1 offset
    int baseK = 4;
    // Input 2 offset, afhængig af model_type

    fprintf(fp, "plot \\\n");

    // INPUT 1
    if (model_type == 1) // SIR
    {
        fprintf(fp, "    '%s' using 1:%d with lines ls 1 title 'S (Input 1)', \\\n", dataFile, baseA + 1);
        fprintf(fp, "    '%s' using 1:%d with lines ls 3 title 'I (Input 1)', \\\n", dataFile, baseA + 2);
        fprintf(fp, "    '%s' using 1:%d with lines ls 5 title 'R (Input 1)'", dataFile, baseA + 3);
    }
    else if (model_type == 2) // SEIR
    {
        fprintf(fp, "    '%s' using 1:%d with lines ls 1 title 'S (Input 1)', \\\n", dataFile, baseA + 1);
        fprintf(fp, "    '%s' using 1:%d with lines ls 2 title 'E (Input 1)', \\\n", dataFile, baseA + 2);
        fprintf(fp, "    '%s' using 1:%d with lines ls 3 title 'I (Input 1)', \\\n", dataFile, baseA + 3);
        fprintf(fp, "    '%s' using 1:%d with lines ls 5 title 'R (Input 1)'", dataFile, baseA + 4);
    }
    else if (model_type == 3) // SEIHRS
    {
        fprintf(fp, "    '%s' using 1:%d with lines ls 1 title 'S (Input 1)', \\\n", dataFile, baseA + 1);
        fprintf(fp, "    '%s' using 1:%d with lines ls 2 title 'E (Input 1)', \\\n", dataFile, baseA + 2);
        fprintf(fp, "    '%s' using 1:%d with lines ls 3 title 'I (Input 1)', \\\n", dataFile, baseA + 3);
        fprintf(fp, "    '%s' using 1:%d with lines ls 4 title 'H (Input 1)', \\\n", dataFile, baseA + 4);
        fprintf(fp, "    '%s' using 1:%d with lines ls 5 title 'R (Input 1)'", dataFile, baseA + 5);
    }

    // INPUT 2
    if (valg_input == 2)
    {
        fprintf(fp, ", \\\n");

        if (model_type == 1) // SIR
        {
            fprintf(fp, "    '%s' using 1:%d with lines ls 1 title 'S (Input 2)', \\\n", dataFile, baseK + 1);
            fprintf(fp, "    '%s' using 1:%d with lines ls 3 title 'I (Input 2)', \\\n", dataFile, baseK + 3);
            fprintf(fp, "    '%s' using 1:%d with lines ls 5 title 'R (Input 2)'", dataFile, baseK + 5);
        }
        else if (model_type == 2) // SEIR
        {
            int baseK = 5;
            fprintf(fp, "    '%s' using 1:%d with lines ls 1 title 'S (Input 2)', \\\n", dataFile, baseK + 1);
            fprintf(fp, "    '%s' using 1:%d with lines ls 2 title 'E (Input 2)', \\\n", dataFile, baseK + 2);
            fprintf(fp, "    '%s' using 1:%d with lines ls 3 title 'I (Input 2)', \\\n", dataFile, baseK + 3);
            fprintf(fp, "    '%s' using 1:%d with lines ls 5 title 'R (Input 2)'", dataFile, baseK + 4);
        }
        else if (model_type == 3) // SEIHRS
        {
            int baseK = 6;
            fprintf(fp, "    '%s' using 1:%d with lines ls 1 title 'S (Input 2)', \\\n", dataFile, baseK + 1);
            fprintf(fp, "    '%s' using 1:%d with lines ls 2 title 'E (Input 2)', \\\n", dataFile, baseK + 2);
            fprintf(fp, "    '%s' using 1:%d with lines ls 3 title 'I (Input 2)', \\\n", dataFile, baseK + 3);
            fprintf(fp, "    '%s' using 1:%d with lines ls 4 title 'H (Input 2)', \\\n", dataFile, baseK + 4);
            fprintf(fp, "    '%s' using 1:%d with lines ls 5 title 'R (Input 2)'", dataFile, baseK + 5);
        }
    }

    fprintf(fp, "\n");
    fprintf(fp, "pause -1 'Tast enter for at lukke'\n");

    fclose(fp);
}
// Stokastiske hjælpefunktioner
// Poisson RNG baseret på Knuth for små lambda og normalapproximation for store lambda
long poisson(double lambda)
{
    if (lambda <= 0)
        return 0;
    if (lambda > 30.0)
    {
        // Normalapproksimation
        double u1 = (double)rand() / RAND_MAX;
        double u2 = (double)rand() / RAND_MAX;
        if (u1 < 1e-12)
            u1 = 1e-12;
        double z = sqrt(-2.0 * log(u1)) * cos(2.0 * M_PI * u2);
        long val = (long)(lambda + sqrt(lambda) * z + 0.5);
        return val < 0 ? 0 : val;
    }
    double L = exp(-lambda);
    double p = 1.0;
    long k = 0;
    do
    {
        k++;
        p *= (double)rand() / RAND_MAX;
    } while (p > L && k < 100000);
    return k - 1;
}
