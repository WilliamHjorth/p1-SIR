#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define ALDERS_GRUPPER 4
#define MAX_DAYS 150
// Antal sub-steps pr. dag for den stokastiske simulering
#define STEPS_PER_DAY 10

typedef struct
{
    float *S;
    float *I;
    float *R;
    float beta;
    float gamma;
    int length;
} SIR_model;

typedef struct
{
    float S;
    float E;
    float I;
    float R;
    float sigma;
    float beta;
    float gamma;
} SEIR_model;

typedef struct
{
    float S;
    float E;
    float I;
    float H;
    float R;
    float sigma;
    float h; // Hospitalisering rate
    float beta;
    float gamma;
} SEIHR_model;

// Startværdier Aalborg
float S0_AAL = 121878;
float E0_AAL = 0;
float I0_AAL = 10;
float R0_AAL = 0;

// Startværdier Koebenhavn
float S0_KBH = 667099;
float E0_KBH = 0;
float I0_KBH = 50;
float R0_KBH = 0;

// Parametre
float beta_AAL = 0.3247;
float gamma_AAL = 0.143;
float sigma_AAL = 1.0 / 6.0; // inkubationstid 6 dage
float Modtagelig_AAL = 1;

float beta_KBH = 0.4;
float gamma_KBH = 0.143;
float sigma_KBH = 1.0 / 6.0;
float Modtagelig_kbh = 1;
// faktore for de givne aldre og hospitaliseret
float h_factor[ALDERS_GRUPPER] = {0.2, 1.0, 2.0, 5.0};
float age_beta_factor[ALDERS_GRUPPER] = {0.8, 1.0, 1.2, 1.5};
float age_sigma_factor[ALDERS_GRUPPER] = {1.0, 1.0, 0.9, 0.8};
float age_gamma_factor[ALDERS_GRUPPER] = {1.2, 1.0, 0.9, 0.7};
float age_Modtagelighed[ALDERS_GRUPPER] = {0.6, 0.5, 0.7, 0.4};

float N_AAL = 121878;
float N_KBH = 667099;
float t = 0.001; // Overførsels rate

// Funktionsprototyper
void bruger_input();
void tilpas_funktion();
void faerdige_covid_simuleringer();
void udvid_med_smitte_stop_og_vaccine(int *use_app, int *use_vaccine);
// city_choice: 0=Aalborg, 1=Koebenhavn, 2=Begge
// is_stochastic: 0=deterministisk, 1=stokastisk
void sirModelToByer(int model_type, int use_app, int use_vaccine, int city_choice, FILE *file, int replicate_num, int is_stochastic, int print_to_terminal);
void runMultipleReplicates(int model_type, int use_app, int use_vaccine, int numReplicates, int city_choice);
long poisson(double lambda);

// mangler at vi kan printe det ud. Tænker vi kan bruge modeltype som en int den tager ligesom vi gør i tilpas funktionen();
void Gnuplot(int modeltype);
void createGnuplotScript(const char *scriptFile, const char *dataFile, int numReplicates, int model_type, int city_choice);
void createGnuplotScriptSingle(const char *scriptFile, const char *dataFile, int model_type, int city_choice);
// model_type: 1=SIR, 2=SEIR, 3=SEIHR

int main(void)
{
    srand((unsigned int)time(NULL));
    bruger_input();
    return 0;
}

// Brugermenu
void bruger_input()
{
    printf("\nDette er et program, der simulerer smittespredning!\n\n");
    printf("Vælg:\n 1) Covid-19 smittesimulering (Enter)\n 2) Tilpas model (T)\n 3) Koer Koebenhavn eller Aalborg (M)\n");

    int ch = getchar();
    if (ch == '\n')
    {
        faerdige_covid_simuleringer();
    }
    else if (ch == 'T' || ch == 't')
    {
        tilpas_funktion();
    }
    else if (ch == 'M' || ch == 'm')
    {
        // Vælg model type
        char valg[6];
        printf("\nVælg model (SIR, SEIR, SEIHR): ");
        scanf("%5s", valg);

        int model_type = 0;
        if (strcmp(valg, "SIR") == 0 || strcmp(valg, "sir") == 0)
        {
            model_type = 1;
        }
        else if (strcmp(valg, "SEIR") == 0 || strcmp(valg, "seir") == 0)
        {
            model_type = 2;
        }
        else if (strcmp(valg, "SEIHR") == 0 || strcmp(valg, "seihr") == 0)
        {
            model_type = 3;
        }
        else
        {
            printf("Ugyldigt valg.\n");
            return;
        }

        // Vælg simulering type
        char sim_type;
        printf("Vælg simulering type:\n");
        printf(" N) Normal (deterministisk)\n");
        printf(" S) Stokastisk (multiple replicates)\n");
        printf("Valg: ");
        scanf(" %c", &sim_type);

        if (sim_type == 'N' || sim_type == 'n')
        {
            // Spørg om by valg for dette run
            char city_ch;
            int city_choice = 0; // 0=Aalborg,1=Kbh,2=Begge
            printf("Vælg by: A=Aalborg, K=Koebenhavn, S=Begge: ");
            scanf(" %c", &city_ch);
            if (city_ch == 'K' || city_ch == 'k')
                city_choice = 1;
            else if (city_ch == 'S' || city_ch == 's')
                city_choice = 2;
            // Normal deterministisk simulering
            int use_app = 0, use_vaccine = 0;
            udvid_med_smitte_stop_og_vaccine(&use_app, &use_vaccine);

            FILE *file = fopen("data_file.txt", "w");
            if (!file)
            {
                printf("Kan ikke åbne fil\n");
                return;
            }

            sirModelToByer(model_type, use_app, use_vaccine, city_choice, file, 0, 0, 1);
            fclose(file);

            printf("Simulering færdig. Data gemt i data_file.txt\n");

            // Lav gnuplot script for en enkelt simulering
            createGnuplotScriptSingle("plot_single.gnu", "data_file.txt", model_type, city_choice);
            printf("Gnuplot script gemt i plot_single.gnu\n");
            printf("Kør: gnuplot plot_single.gnu\n");
        }
        else if (sim_type == 'S' || sim_type == 's')
        {
            // Stokastisk simulering med flere kopier
            int numReplicates;
            printf("Antal replicates: ");
            scanf("%d", &numReplicates);

            int use_app = 0, use_vaccine = 0;
            udvid_med_smitte_stop_og_vaccine(&use_app, &use_vaccine);

            // Spørg om by valg for kopier
            char city_ch;
            int city_choice = 0;
            printf("Vælg by: A=Aalborg, K=Koebenhavn, S=Begge: ");
            scanf(" %c", &city_ch);
            if (city_ch == 'K' || city_ch == 'k')
                city_choice = 1;
            else if (city_ch == 'S' || city_ch == 's')
                city_choice = 2;

            runMultipleReplicates(model_type, use_app, use_vaccine, numReplicates, city_choice);
        }
        else
        {
            printf("Ugyldigt valg.\n");
        }
    }
    else
    {
        printf("Ugyldigt valg.\n");
    }
}

// Tilpasning af modellen, // mangler det der hvor man kan vælge alle værdierene til modellerne
void tilpas_funktion()
{
    char valg[6];
    printf("Vælg model (SIR, SEIR, SEIHR): ");
    scanf("%5s", valg);

    int model_type = 0;
    if (strcmp(valg, "SIR") == 0 || strcmp(valg, "sir") == 0)
    {
        model_type = 1;
    }
    else if (strcmp(valg, "SEIR") == 0 || strcmp(valg, "seir") == 0)
    {
        model_type = 2;
    }
    else if (strcmp(valg, "SEIHR") == 0 || strcmp(valg, "seihr") == 0)
    {
        model_type = 3;
    }
    else
    {
        printf("Forkert input.\n");
        return;
    }

    // ask city
    char city_ch;
    int city_choice = 0;
    printf("Vælg by: A=Aalborg, K=Koebenhavn, S=Begge: ");
    scanf(" %c", &city_ch);
    if (city_ch == 'K' || city_ch == 'k')
        city_choice = 1;
    else if (city_ch == 'S' || city_ch == 's')
        city_choice = 2;

    FILE *file = fopen("data_file.txt", "w");
    if (!file)
    {
        printf("Kan ikke åbne fil\n");
        return;
    }

    sirModelToByer(model_type, 0, 0, city_choice, file, 0, 0, 2);
    fclose(file);

    printf("Simuleringen er færdig. Dataen er gemt i data_file.txt\n");
}

// Færdige Covid-simuleringer
void faerdige_covid_simuleringer()
{
    char city_choice_ch;
    printf("Vælg by: A=Aalborg, K=Koebenhavn, S=Begge: ");
    scanf(" %c", &city_choice_ch);
    int city_choice = 0;
    if (city_choice_ch == 'K' || city_choice_ch == 'k')
        city_choice = 1;
    else if (city_choice_ch == 'S' || city_choice_ch == 's')
        city_choice = 2;

    int use_app = 0, use_vaccine = 0;
    udvid_med_smitte_stop_og_vaccine(&use_app, &use_vaccine);

    // Modelvalg (for demo SIR, kan ændres)
    int model_type = 2; // SEIR som standard

    FILE *file = fopen("data_file.txt", "w");
    if (!file)
    {
        printf("Kan ikke åbne fil\n");
        return;
    }

    sirModelToByer(model_type, use_app, use_vaccine, city_choice, file, 0, 0, 1);
    fclose(file);

    printf("Simulering færdig. Data gemt i data_file.txt\n");
}

// Vaccine og app
void udvid_med_smitte_stop_og_vaccine(int *use_app, int *use_vaccine)
{
    char app, vaccine;
    printf("Er smittestop-app aktiv (j/n)? ");
    scanf(" %c", &app);
    printf("Er vaccine udrullet (j/n)? ");
    scanf(" %c", &vaccine);

    *use_app = (app == 'j' || app == 'J') ? 1 : 0;
    *use_vaccine = (vaccine == 'j' || vaccine == 'J') ? 1 : 0;
}

// Hovedsimulering for én eller begge byer
void sirModelToByer(int model_type, int use_app, int use_vaccine, int city_choice, FILE *file, int replicate_num, int is_stochastic, int print_to_terminal)
{
    float S_out_A = 0;
    float I_out_A = 0;
    float R_out_A = 0;
    float S_out_K = 0;
    float I_out_K = 0;
    float R_out_K = 0;
    float E_out_A = 0;
    float E_out_K = 0;
    // Fordeling til aldersgrupper
    float S_AAL[ALDERS_GRUPPER], E_AAL[ALDERS_GRUPPER], I_AAL[ALDERS_GRUPPER], R_AAL[ALDERS_GRUPPER], H_AAL[ALDERS_GRUPPER];
    float S_KBH[ALDERS_GRUPPER], E_KBH[ALDERS_GRUPPER], I_KBH[ALDERS_GRUPPER], R_KBH[ALDERS_GRUPPER], H_KBH[ALDERS_GRUPPER];

    for (int i = 0; i < ALDERS_GRUPPER; i++)
    {
        S_AAL[i] = S0_AAL / ALDERS_GRUPPER;
        E_AAL[i] = E0_AAL / ALDERS_GRUPPER;
        I_AAL[i] = I0_AAL / ALDERS_GRUPPER;
        R_AAL[i] = R0_AAL / ALDERS_GRUPPER;
        H_AAL[i] = 0;

        S_KBH[i] = S0_KBH / ALDERS_GRUPPER;
        E_KBH[i] = E0_KBH / ALDERS_GRUPPER;
        I_KBH[i] = I0_KBH / ALDERS_GRUPPER;
        R_KBH[i] = R0_KBH / ALDERS_GRUPPER;
        H_KBH[i] = 0;
    }

    // Skriv replicate header
    if (file != NULL)
    {
        if (city_choice == 0)
            fprintf(file, "# Replicate %d - Aalborg\n", replicate_num);
        else if (city_choice == 1)
            fprintf(file, "# Replicate %d - København\n", replicate_num);
        else
            fprintf(file, "# Replicate %d - Both\n", replicate_num);
    }

    // Simulering
    for (int n = 0; n < MAX_DAYS; n++)
    {
        // Transfer rate (daglig basis)
        float t = 0.001;

        if (is_stochastic)
        {
            double dt = 1.0 / (double)STEPS_PER_DAY;
            // Udføre STEPS_PER_DAY sub-steps per dag
            for (int step = 0; step < STEPS_PER_DAY; step++)
            {
                // Udregn infectious totals hver sub-step
                float total_I_AAL = 0, total_I_KBH = 0;
                for (int ii = 0; ii < ALDERS_GRUPPER; ii++)
                {
                    total_I_AAL += I_AAL[ii];
                    total_I_KBH += I_KBH[ii];
                }

                for (int i = 0; i < ALDERS_GRUPPER; i++)
                {
                    // migration skaleret med dt for hver sub-step
                    S_out_A = t * dt * S_AAL[i];
                    E_out_A = t * dt * E_AAL[i];
                    I_out_A = t * dt * I_AAL[i];
                    R_out_A = t * dt * R_AAL[i];

                    S_out_K = t * dt * S_KBH[i];
                    E_out_K = t * dt * E_KBH[i];
                    I_out_K = t * dt * I_KBH[i];
                    R_out_K = t * dt * R_KBH[i];

                    float beta_i_A = beta_AAL * age_beta_factor[i];
                    float sigma_i_A = sigma_AAL * age_sigma_factor[i];
                    float gamma_i_A = gamma_AAL * age_gamma_factor[i];
                    float h_i_A = h_factor[i] * 0.01;

                    float beta_i_K = beta_KBH * age_beta_factor[i];
                    float sigma_i_K = sigma_KBH * age_sigma_factor[i];
                    float gamma_i_K = gamma_KBH * age_gamma_factor[i];
                    float h_i_K = h_factor[i] * 0.01;

                    if (use_app)
                    {
                        beta_i_A *= 0.75;
                        gamma_i_A *= 1.1;
                        beta_i_K *= 0.75;
                        gamma_i_K *= 1.1;
                    }
                    if (use_vaccine)
                    {
                        // Anvend vaccine effekt per sub-step
                        beta_i_A *= 0.75;
                        gamma_i_A *= 1.1;
                        beta_i_K *= 0.75;
                        gamma_i_K *= 1.1;
                    }

                    // Aalborg - rates skaleret med dt
                    double rate_inf_A = beta_i_A * S_AAL[i] * total_I_AAL / N_AAL;
                    long n_inf_A = poisson(rate_inf_A * dt);
                    long n_prog_A = 0;
                    long n_rec_A = poisson(gamma_i_A * I_AAL[i] * dt);
                    long n_hosp_A = 0;
                    long n_h_rec_A = 0;
                    if (model_type == 2)
                    {
                        n_prog_A = poisson(sigma_i_A * E_AAL[i] * dt);
                    }
                    else if (model_type == 3)
                    {
                        n_prog_A = poisson(sigma_i_A * E_AAL[i] * dt);
                        n_hosp_A = poisson(h_i_A * I_AAL[i] * dt);
                        n_h_rec_A = poisson((gamma_i_A / 2.0) * H_AAL[i] * dt);
                    }
                    if (n_inf_A > (long)S_AAL[i])
                        n_inf_A = (long)S_AAL[i];
                    if (n_prog_A > (long)E_AAL[i])
                        n_prog_A = (long)E_AAL[i];
                    if (n_rec_A > (long)I_AAL[i])
                        n_rec_A = (long)I_AAL[i];
                    if (n_hosp_A > (long)I_AAL[i])
                        n_hosp_A = (long)I_AAL[i];
                    if (n_h_rec_A > (long)H_AAL[i])
                        n_h_rec_A = (long)H_AAL[i];

                    if (model_type == 1)
                    {
                        S_AAL[i] -= n_inf_A;
                        I_AAL[i] += n_inf_A - n_rec_A;
                        R_AAL[i] += n_rec_A;
                    }
                    else if (model_type == 2)
                    {
                        S_AAL[i] -= n_inf_A;
                        E_AAL[i] += n_inf_A - n_prog_A;
                        I_AAL[i] += n_prog_A - n_rec_A;
                        R_AAL[i] += n_rec_A;
                    }
                    else if (model_type == 3)
                    {
                        S_AAL[i] -= n_inf_A;
                        E_AAL[i] += n_inf_A - n_prog_A;
                        I_AAL[i] += n_prog_A - n_rec_A - n_hosp_A;
                        H_AAL[i] += n_hosp_A - n_h_rec_A;
                        R_AAL[i] += n_rec_A + n_h_rec_A;
                    }

                    // Koebenhavn - rates skaleret med dt
                    double rate_inf_K = beta_i_K * S_KBH[i] * total_I_KBH / N_KBH;
                    long n_inf_K = poisson(rate_inf_K * dt);
                    long n_prog_K = 0;
                    long n_rec_K = poisson(gamma_i_K * I_KBH[i] * dt);
                    long n_hosp_K = 0;
                    long n_h_rec_K = 0;
                    if (model_type == 2)
                    {
                        n_prog_K = poisson(sigma_i_K * E_KBH[i] * dt);
                    }
                    else if (model_type == 3)
                    {
                        n_prog_K = poisson(sigma_i_K * E_KBH[i] * dt);
                        n_hosp_K = poisson(h_i_K * I_KBH[i] * dt);
                        n_h_rec_K = poisson((gamma_i_K / 2.0) * H_KBH[i] * dt);
                    }
                    if (n_inf_K > (long)S_KBH[i])
                        n_inf_K = (long)S_KBH[i];
                    if (n_prog_K > (long)E_KBH[i])
                        n_prog_K = (long)E_KBH[i];
                    if (n_rec_K > (long)I_KBH[i])
                        n_rec_K = (long)I_KBH[i];
                    if (n_hosp_K > (long)I_KBH[i])
                        n_hosp_K = (long)I_KBH[i];
                    if (n_h_rec_K > (long)H_KBH[i])
                        n_h_rec_K = (long)H_KBH[i];

                    if (model_type == 1)
                    {
                        S_KBH[i] -= n_inf_K;
                        I_KBH[i] += n_inf_K - n_rec_K;
                        R_KBH[i] += n_rec_K;
                    }
                    else if (model_type == 2)
                    {
                        S_KBH[i] -= n_inf_K;
                        E_KBH[i] += n_inf_K - n_prog_K;
                        I_KBH[i] += n_prog_K - n_rec_K;
                        R_KBH[i] += n_rec_K;
                    }
                    else if (model_type == 3)
                    {
                        S_KBH[i] -= n_inf_K;
                        E_KBH[i] += n_inf_K - n_prog_K;
                        I_KBH[i] += n_prog_K - n_rec_K - n_hosp_K;
                        H_KBH[i] += n_hosp_K - n_h_rec_K;
                        R_KBH[i] += n_rec_K + n_h_rec_K;
                    }

                    // Anvend migrations justeringer for dette sub-step
                    S_AAL[i] += -S_out_A + S_out_K;
                    I_AAL[i] += -I_out_A + I_out_K;
                    R_AAL[i] += -R_out_A + R_out_K;

                    S_KBH[i] += -S_out_K + S_out_A;
                    I_KBH[i] += -I_out_K + I_out_A;
                    R_KBH[i] += -R_out_K + R_out_A;
                }
            }
        }
        else
        {
            // Deterministiske flows (oprindelig adfærd)
            // Vi har stadig brug for total for at beregne infektionspresset
            float total_I_AAL = 0, total_I_KBH = 0;
            for (int i = 0; i < ALDERS_GRUPPER; i++)
            {
                total_I_AAL += I_AAL[i];
                total_I_KBH += I_KBH[i];
            }

            float dS_AAL[ALDERS_GRUPPER], dE_AAL[ALDERS_GRUPPER], dI_AAL[ALDERS_GRUPPER], dR_AAL[ALDERS_GRUPPER], dH_AAL[ALDERS_GRUPPER];
            float dS_KBH[ALDERS_GRUPPER], dE_KBH[ALDERS_GRUPPER], dI_KBH[ALDERS_GRUPPER], dR_KBH[ALDERS_GRUPPER], dH_KBH[ALDERS_GRUPPER];

            float sum_S_AAL = 0, sum_E_AAL = 0, sum_I_AAL = 0, sum_R_AAL = 0, sum_H_AAL = 0;
            float sum_S_KBH = 0, sum_E_KBH = 0, sum_I_KBH = 0, sum_R_KBH = 0, sum_H_KBH = 0;
            for (int i = 0; i < ALDERS_GRUPPER; i++)
            {
                sum_S_AAL += S_AAL[i];
                sum_E_AAL += E_AAL[i];
                sum_I_AAL += I_AAL[i];
                sum_R_AAL += R_AAL[i];
                sum_H_AAL += H_AAL[i];
                sum_S_KBH += S_KBH[i];
                sum_E_KBH += E_KBH[i];
                sum_I_KBH += I_KBH[i];
                sum_R_KBH += R_KBH[i];
                sum_H_KBH += H_KBH[i];
            }
            // transfer rate?
            float t = 0.001;
            float S_ud_AAL = t * sum_S_AAL;
            float E_ud_AAL = t * sum_E_AAL;
            float I_ud_AAL = t * sum_I_AAL;
            float R_ud_AAL = t * sum_R_AAL;

            float S_ud_KBH = t * sum_S_KBH;
            float E_ud_KBH = t * sum_E_KBH;
            float I_ud_KBH = t * sum_I_KBH;
            float R_ud_KBH = t * sum_R_KBH;

            for (int i = 0; i < ALDERS_GRUPPER; i++)
            {
                float beta_i_A = beta_AAL * age_beta_factor[i];
                float sigma_i_A = sigma_AAL * age_sigma_factor[i];
                float gamma_i_A = gamma_AAL * age_gamma_factor[i];
                float Modtagelighed_i_A = Modtagelig_AAL * age_Modtagelighed[i];
                float h_i_A = h_factor[i] * 0.01;

                float beta_i_K = beta_KBH * age_beta_factor[i];
                float sigma_i_K = sigma_KBH * age_sigma_factor[i];
                float gamma_i_K = gamma_KBH * age_gamma_factor[i];
                float Modtagelighed_i_K = Modtagelig_kbh * age_Modtagelighed[i];
                float h_i_K = h_factor[i] * 0.01;

                if (use_app)
                {
                    beta_i_A *= 0.75;
                    gamma_i_A *= 1.1;
                    beta_i_K *= 0.75;
                    gamma_i_K *= 1.1;
                }
                if (use_vaccine)
                {
                    beta_i_A *= 0.75;
                    gamma_i_A *= 1.1;
                    beta_i_K *= 0.75;
                    gamma_i_K *= 1.1;
                }

                if (model_type == 1)
                { // SIR
                    dS_AAL[i] = -beta_i_A * S_AAL[i] * total_I_AAL / N_AAL - S_out_A + S_out_K;
                    dI_AAL[i] = beta_i_A * S_AAL[i] * total_I_AAL / N_AAL - gamma_i_A * I_AAL[i] - I_out_A + I_out_K;
                    dR_AAL[i] = gamma_i_A * I_AAL[i] - R_out_A + R_out_K;
                    dE_AAL[i] = 0;
                    dH_AAL[i] = 0;

                    dS_KBH[i] = -beta_i_K * S_KBH[i] * total_I_KBH / N_KBH - S_ud_KBH + S_ud_AAL;
                    dI_KBH[i] = beta_i_K * S_KBH[i] * total_I_KBH / N_KBH - gamma_i_K * I_KBH[i] - I_ud_KBH + I_ud_AAL;
                    dR_KBH[i] = gamma_i_K * I_KBH[i] - R_ud_KBH + R_ud_AAL;
                    dE_KBH[i] = 0;
                    dH_KBH[i] = 0;
                }
                else if (model_type == 2)
                { // SEIR
                    dS_AAL[i] = -beta_i_A * S_AAL[i] * total_I_AAL / N_AAL - S_ud_AAL + S_ud_KBH;
                    dE_AAL[i] = beta_i_A * S_AAL[i] * total_I_AAL / N_AAL - sigma_i_A * E_AAL[i] - E_ud_AAL + E_ud_KBH;
                    dI_AAL[i] = sigma_i_A * E_AAL[i] - gamma_i_A * I_AAL[i] - I_ud_AAL + I_ud_KBH;
                    dR_AAL[i] = gamma_i_A * I_AAL[i] - R_ud_AAL + R_ud_KBH;
                    dH_AAL[i] = 0;

                    dS_KBH[i] = -beta_i_K * S_KBH[i] * total_I_KBH / N_KBH - S_ud_KBH + S_ud_AAL;
                    dE_KBH[i] = beta_i_K * S_KBH[i] * total_I_KBH / N_KBH - sigma_i_K * E_KBH[i] - E_ud_KBH + E_ud_AAL;
                    dI_KBH[i] = sigma_i_K * E_KBH[i] - gamma_i_K * I_KBH[i] - I_ud_KBH + I_ud_AAL;
                    dR_KBH[i] = gamma_i_K * I_KBH[i] - R_ud_KBH + R_ud_AAL;
                    dH_KBH[i] = 0;
                }
                else if (model_type == 3)
                { // SEIHR
                    dS_AAL[i] = -beta_i_A * S_AAL[i] * total_I_AAL / N_AAL + Modtagelighed_i_A * R_AAL[i] - S_ud_AAL + S_ud_KBH;
                    dE_AAL[i] = beta_i_A * S_AAL[i] * total_I_AAL / N_AAL - sigma_i_A * E_AAL[i] - E_ud_AAL + E_ud_KBH;
                    dI_AAL[i] = sigma_i_A * E_AAL[i] - gamma_i_A * I_AAL[i] - h_i_A * I_AAL[i] - I_ud_AAL + I_ud_KBH;
                    dH_AAL[i] = h_i_A * I_AAL[i] - gamma_i_A / 2 * H_AAL[i];
                    dR_AAL[i] = gamma_i_A * I_AAL[i] + gamma_i_A / 2 * H_AAL[i] - Modtagelighed_i_A * R_AAL[i] - R_ud_AAL + R_ud_KBH;

                    dS_KBH[i] = -beta_i_K * S_KBH[i] * total_I_KBH / N_KBH + Modtagelighed_i_A * R_KBH[i] - S_ud_KBH + S_ud_AAL;
                    dE_KBH[i] = beta_i_K * S_KBH[i] * total_I_KBH / N_KBH - sigma_i_K * E_KBH[i] - E_ud_KBH + E_ud_AAL;
                    dI_KBH[i] = sigma_i_K * E_KBH[i] - gamma_i_K * I_KBH[i] - h_i_K * I_KBH[i] - I_ud_KBH + I_ud_AAL;
                    dH_KBH[i] = h_i_K * I_KBH[i] - gamma_i_K / 2 * H_KBH[i];
                    dR_KBH[i] = gamma_i_K * I_KBH[i] + gamma_i_K / 2 * H_KBH[i] - Modtagelighed_i_A * R_KBH[i] - R_ud_KBH + R_ud_AAL;
                }

                // Opdater (deterministisk)
                S_AAL[i] += dS_AAL[i];
                E_AAL[i] += dE_AAL[i];
                I_AAL[i] += dI_AAL[i];
                R_AAL[i] += dR_AAL[i];
                H_AAL[i] += dH_AAL[i];
                S_KBH[i] += dS_KBH[i];
                E_KBH[i] += dE_KBH[i];
                I_KBH[i] += dI_KBH[i];
                R_KBH[i] += dR_KBH[i];
                H_KBH[i] += dH_KBH[i];
            }
        }

        // Summér alle aldersgrupper for print/save
        float sum_S_AAL = 0, sum_E_AAL = 0, sum_I_AAL = 0, sum_R_AAL = 0, sum_H_AAL = 0;
        float sum_S_KBH = 0, sum_E_KBH = 0, sum_I_KBH = 0, sum_R_KBH = 0, sum_H_KBH = 0;
        for (int i = 0; i < ALDERS_GRUPPER; i++)
        {
            sum_S_AAL += S_AAL[i];
            sum_E_AAL += E_AAL[i];
            sum_I_AAL += I_AAL[i];
            sum_R_AAL += R_AAL[i];
            sum_H_AAL += H_AAL[i];
            sum_S_KBH += S_KBH[i];
            sum_E_KBH += E_KBH[i];
            sum_I_KBH += I_KBH[i];
            sum_R_KBH += R_KBH[i];
            sum_H_KBH += H_KBH[i];
        }

        if (print_to_terminal)
        {
            if (city_choice == 0)
                printf("Day %d | AAL(S=%.0f,I=%.0f,R=%.0f)\n", n, sum_S_AAL, sum_I_AAL, sum_R_AAL);
            else if (city_choice == 1)
                printf("Day %d | KBH(S=%.0f,I=%.0f,R=%.0f)\n", n, sum_S_KBH, sum_I_KBH, sum_R_KBH);
            else
                printf("Day %d | AAL(S=%.0f,I=%.0f,R=%.0f) KBH(S=%.0f,I=%.0f,R=%.0f)\n", n, sum_S_AAL, sum_I_AAL, sum_R_AAL, sum_S_KBH, sum_I_KBH, sum_R_KBH);
        }

        if (file != NULL)
        {
            fprintf(file, "%d %.6f %.6f %.6f %.6f %.6f %.6f %.6f %.6f %.6f %.6f\n",
                    n, sum_S_AAL, sum_E_AAL, sum_I_AAL, sum_H_AAL, sum_R_AAL,
                    sum_S_KBH, sum_E_KBH, sum_I_KBH, sum_H_KBH, sum_R_KBH);
        }
    }

    if (file != NULL)
    {
        fprintf(file, "\n\n");
    }
}

// Køre flere kopier
void runMultipleReplicates(int model_type, int use_app, int use_vaccine, int numReplicates, int city_choice)
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
        printf("Replicate %d/%d\n", rep + 1, numReplicates);
        sirModelToByer(model_type, use_app, use_vaccine, city_choice, file, rep, 1, 0);
    }

    fclose(file);
    printf("Alle replicates færdige. Data gemt i stochastic_replicates.txt\n");

    // Lav gnuplot script
    createGnuplotScript("plot_replicates.gnu", "stochastic_replicates.txt", numReplicates, model_type, city_choice);
    printf("Gnuplot script gemt i plot_replicates.gnu\n");
    printf("Kør: gnuplot plot_replicates.gnu\n");
}

void createGnuplotScript(const char *scriptFile, const char *dataFile, int numReplicates, int model_type, int city_choice)
{
    FILE *fp = fopen(scriptFile, "w");
    if (fp == NULL)
    {
        fprintf(stderr, "Error opening file %s\n", scriptFile);
        return;
    }

    fprintf(fp, "set xlabel 'Days'\n");
    fprintf(fp, "set ylabel 'Number of individuals'\n");
    
    if (model_type == 1 && city_choice == 0) {
        fprintf(fp, "set title 'Stochastic SIR Model - %d Replicates (Aalborg)'\n", numReplicates);
    } else if (model_type == 2 && city_choice == 0) {
        fprintf(fp, "set title 'Stochastic SEIR Model - %d Replicates (Aalborg)'\n", numReplicates);
    } else if (model_type == 3 && city_choice == 0) {
        fprintf(fp, "set title 'Stochastic SEIHR Model - %d Replicates (Aalborg)'\n", numReplicates);
    }
    
    if (model_type == 1 && city_choice == 1) {
        fprintf(fp, "set title 'Stochastic SIR Model - %d Replicates (Koebenhavn)'\n", numReplicates);
    } else if (model_type == 2 && city_choice == 1) {
        fprintf(fp, "set title 'Stochastic SEIR Model - %d Replicates (Koebenhavn)'\n", numReplicates);
    } else if (model_type == 3 && city_choice == 1) {
        fprintf(fp, "set title 'Stochastic SEIHR Model - %d Replicates (Koebenhavn)'\n", numReplicates);
    }

    if (model_type == 1 && city_choice == 2) {
        fprintf(fp, "set title 'Stochastic SIR Model - %d Replicates (Begge byer)'\n", numReplicates);
    } else if (model_type == 2 && city_choice == 2) {
        fprintf(fp, "set title 'Stochastic SEIR Model - %d Replicates (Begge byer)'\n", numReplicates);
    } else if (model_type == 3 && city_choice == 2) {
        fprintf(fp, "set title 'Stochastic SEIHR Model - %d Replicates (Begge byer)'\n", numReplicates);
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
    int baseA = 1; // Aalborg kolonner start forskydning baseA -> S = baseA+1 -> kolonne 2
    int baseK = 6; // Kbh kolonner start forskydning baseK -> S = baseK+1 -> kolonne 7

    // Byg en eksplicit plot-liste per replicate for at undgå gnuplot 'for' kompatibilitetsproblemer
    int first = 1;
    fprintf(fp, "plot ");
    for (int rep = 0; rep < numReplicates; rep++)
    {
        if (model_type == 1)
        {
            // SIR: S, I, R
            if (city_choice == 0 || city_choice == 2)
            {
                int colS = baseA + 1;
                int colI = baseA + 3;
                int colR = baseA + 5;
                if (!first)
                    fprintf(fp, ", \\\n++");
                if (rep == 0)
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 1 title 'S (Aalborg)'", dataFile, rep, colS);
                else
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 1 notitle", dataFile, rep, colS);
                fprintf(fp, ", \\\n++");
                if (rep == 0)
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 3 title 'I (Aalborg)'", dataFile, rep, colI);
                else
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 3 notitle", dataFile, rep, colI);
                fprintf(fp, ", \\\n++");
                if (rep == 0)
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 5 title 'R (Aalborg)'", dataFile, rep, colR);
                else
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 5 notitle", dataFile, rep, colR);
                first = 0;
            }
            if (city_choice == 1 || city_choice == 2)
            {
                int colS = baseK + 1;
                int colI = baseK + 3;
                int colR = baseK + 5;
                if (!first)
                    fprintf(fp, ", \\\n++");
                if (rep == 0)
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 1 title 'S (København)'", dataFile, rep, colS);
                else
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 1 notitle", dataFile, rep, colS);
                fprintf(fp, ", \\\n++");
                if (rep == 0)
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 3 title 'I (København)'", dataFile, rep, colI);
                else
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 3 notitle", dataFile, rep, colI);
                fprintf(fp, ", \\\n++");
                if (rep == 0)
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 5 title 'R (København)'", dataFile, rep, colR);
                else
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 5 notitle", dataFile, rep, colR);
                first = 0;
            }
        }
        else if (model_type == 2)
        {
            // SEIR: S, E, I, R
            if (city_choice == 0 || city_choice == 2)
            {
                int colS = baseA + 1;
                int colE = baseA + 2;
                int colI = baseA + 3;
                int colR = baseA + 5;
                if (!first)
                    fprintf(fp, ", \\\n++");
                if (rep == 0)
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 1 title 'S (Aalborg)'", dataFile, rep, colS);
                else
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 1 notitle", dataFile, rep, colS);
                fprintf(fp, ", \\\n++");
                if (rep == 0)
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 2 title 'E (Aalborg)'", dataFile, rep, colE);
                else
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 2 notitle", dataFile, rep, colE);
                fprintf(fp, ", \\\n++");
                if (rep == 0)
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 3 title 'I (Aalborg)'", dataFile, rep, colI);
                else
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 3 notitle", dataFile, rep, colI);
                fprintf(fp, ", \\\n++");
                if (rep == 0)
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 5 title 'R (Aalborg)'", dataFile, rep, colR);
                else
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 5 notitle", dataFile, rep, colR);
                first = 0;
            }
            if (city_choice == 1 || city_choice == 2)
            {
                int colS = baseK + 1;
                int colE = baseK + 2;
                int colI = baseK + 3;
                int colR = baseK + 5;
                if (!first)
                    fprintf(fp, ", \\\n++");
                if (rep == 0)
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 1 title 'S (København)'", dataFile, rep, colS);
                else
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 1 notitle", dataFile, rep, colS);
                fprintf(fp, ", \\\n++");
                if (rep == 0)
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 2 title 'E (København)'", dataFile, rep, colE);
                else
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 2 notitle", dataFile, rep, colE);
                fprintf(fp, ", \\\n++");
                if (rep == 0)
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 3 title 'I (København)'", dataFile, rep, colI);
                else
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 3 notitle", dataFile, rep, colI);
                fprintf(fp, ", \\\n++");
                if (rep == 0)
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 5 title 'R (København)'", dataFile, rep, colR);
                else
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 5 notitle", dataFile, rep, colR);
                first = 0;
            }
        }
        else if (model_type == 3)
        {
            // SEIHR: S, E, I, H, R
            if (city_choice == 0 || city_choice == 2)
            {
                int colS = baseA + 1;
                int colE = baseA + 2;
                int colI = baseA + 3;
                int colH = baseA + 4;
                int colR = baseA + 5;
                if (!first)
                    fprintf(fp, ", \\\n+++");
                if (rep == 0)
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 1 title 'S (Aalborg)'", dataFile, rep, colS);
                else
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 1 notitle", dataFile, rep, colS);
                fprintf(fp, ", \\\n+++");
                if (rep == 0)
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 2 title 'E (Aalborg)'", dataFile, rep, colE);
                else
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 2 notitle", dataFile, rep, colE);
                fprintf(fp, ", \\\n+++");
                if (rep == 0)
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 3 title 'I (Aalborg)'", dataFile, rep, colI);
                else
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 3 notitle", dataFile, rep, colI);
                fprintf(fp, ", \\\n+++");
                if (rep == 0)
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 4 title 'H (Aalborg)'", dataFile, rep, colH);
                else
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 4 notitle", dataFile, rep, colH);
                fprintf(fp, ", \\\n+++");
                if (rep == 0)
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 5 title 'R (Aalborg)'", dataFile, rep, colR);
                else
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 5 notitle", dataFile, rep, colR);
                first = 0;
            }
            if (city_choice == 1 || city_choice == 2)
            {
                int colS = baseK + 1;
                int colE = baseK + 2;
                int colI = baseK + 3;
                int colH = baseK + 4;
                int colR = baseK + 5;
                if (!first)
                    fprintf(fp, ", \\\n+++");
                if (rep == 0)
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 1 title 'S (København)'", dataFile, rep, colS);
                else
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 1 notitle", dataFile, rep, colS);
                fprintf(fp, ", \\\n+++");
                if (rep == 0)
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 2 title 'E (København)'", dataFile, rep, colE);
                else
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 2 notitle", dataFile, rep, colE);
                fprintf(fp, ", \\\n+++");
                if (rep == 0)
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 3 title 'I (København)'", dataFile, rep, colI);
                else
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 3 notitle", dataFile, rep, colI);
                fprintf(fp, ", \\\n+++");
                if (rep == 0)
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 4 title 'H (København)'", dataFile, rep, colH);
                else
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 4 notitle", dataFile, rep, colH);
                fprintf(fp, ", \\\n+++");
                if (rep == 0)
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 5 title 'R (København)'", dataFile, rep, colR);
                else
                    fprintf(fp, "'%s' index %d using 1:%d with lines ls 5 notitle", dataFile, rep, colR);
                first = 0;
            }
        }
    }
    fprintf(fp, "\n");

    fprintf(fp, "pause -1 'Press enter to close'\n");

    fclose(fp);
}

// Opret Gnuplot-script for enkelt simulering (normal/deterministisk)
void createGnuplotScriptSingle(const char *scriptFile, const char *dataFile, int model_type, int city_choice)
{
    FILE *fp = fopen(scriptFile, "w");
    if (fp == NULL)
    {
        fprintf(stderr, "Error opening file %s\n", scriptFile);
        return;
    }

    fprintf(fp, "set xlabel 'Days'\n");
    fprintf(fp, "set ylabel 'Number of individuals'\n");

    if (model_type == 1)
    {
        fprintf(fp, "set title 'SIR Model - Deterministic Simulation'\n");
    }
    else if (model_type == 2)
    {
        fprintf(fp, "set title 'SEIR Model - Deterministic Simulation'\n");
    }
    else if (model_type == 3)
    {
        fprintf(fp, "set title 'SEIHR Model - Deterministic Simulation'\n");
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

    int baseA = 1; // Kolonne start forskydning for Aalborg
    int baseK = 6; // Kolonne start forskydning for Koebenhavn

    if (model_type == 1)
    {
        // SIR: S, I, R
        if (city_choice == 0 || city_choice == 2) {
            fprintf(fp, "plot '%s' using 1:%d with lines ls 1 title 'S (Aalborg)', \\\n+", dataFile, baseA+1);
            fprintf(fp, "'%s' using 1:%d with lines ls 3 title 'I (Aalborg)', \\\n+", dataFile, baseA+3);
            fprintf(fp, "'%s' using 1:%d with lines ls 5 title 'R (Aalborg)'\n", dataFile, baseA+5);
        }
        if (city_choice == 1 || city_choice == 2) {
            if (city_choice == 2) fprintf(fp, "replot \\\n");
            fprintf(fp, "'%s' using 1:%d with lines ls 1 title 'S (Koebenhavn)', \\\n+", dataFile, baseK+1);
            fprintf(fp, "'%s' using 1:%d with lines ls 3 title 'I (Koebenhavn)', \\\n+", dataFile, baseK+3);
            fprintf(fp, "'%s' using 1:%d with lines ls 5 title 'R (Koebenhavn)'\n", dataFile, baseK+5);
        }
    }
    else if (model_type == 2)
    {
        // SEIR
        if (city_choice == 0 || city_choice == 2) {
            fprintf(fp, "plot'%s' using 1:%d with lines ls 1 title 'S (Aalborg)', \\\n+", dataFile, baseA+1);
            fprintf(fp, "'%s' using 1:%d with lines ls 2 title 'E (Aalborg)', \\\n+", dataFile, baseA+2);
            fprintf(fp, "'%s' using 1:%d with lines ls 3 title 'I (Aalborg)', \\\n+", dataFile, baseA+3);
            fprintf(fp, "'%s' using 1:%d with lines ls 5 title 'R (Aalborg)'\n", dataFile, baseA+5);
        }
        if (city_choice == 1 || city_choice == 2) {
            if (city_choice == 2) fprintf(fp, "replot \\\n");
            fprintf(fp, "'%s' using 1:%d with lines ls 1 title 'S (Koebenhavn)', \\\n+", dataFile, baseK+1);
            fprintf(fp, "'%s' using 1:%d with lines ls 2 title 'E (Koebenhavn)', \\\n+", dataFile, baseK+2);
            fprintf(fp, "'%s' using 1:%d with lines ls 3 title 'I (Koebenhavn)', \\\n+", dataFile, baseK+3);
            fprintf(fp, "'%s' using 1:%d with lines ls 5 title 'R (Koebenhavn)'\n", dataFile, baseK+5);
        }
    }
    else if (model_type == 3)
    {
        // SEIHR
        if (city_choice == 0 || city_choice == 2) {
            fprintf(fp, "plot '%s' using 1:%d with lines ls 1 title 'S (Aalborg)', \\\n+", dataFile, baseA+1);
            fprintf(fp, "'%s' using 1:%d with lines ls 2 title 'E (Aalborg)', \\\n+", dataFile, baseA+2);
            fprintf(fp, "'%s' using 1:%d with lines ls 3 title 'I (Aalborg)', \\\n+", dataFile, baseA+3);
            fprintf(fp, "'%s' using 1:%d with lines ls 4 title 'H (Aalborg)', \\\n+", dataFile, baseA+4);
            fprintf(fp, "'%s' using 1:%d with lines ls 5 title 'R (Aalborg)'\n", dataFile, baseA+5);
        }
        if (city_choice == 1 || city_choice == 2) {
            if (city_choice == 2) fprintf(fp, "replot \\\n");
            fprintf(fp, "'%s' using 1:%d with lines ls 1 title 'S (Koebenhavn)', \\\n+", dataFile, baseK+1);
            fprintf(fp, "'%s' using 1:%d with lines ls 2 title 'E (Koebenhavn)', \\\n+", dataFile, baseK+2);
            fprintf(fp, "'%s' using 1:%d with lines ls 3 title 'I (Koebenhavn)', \\\n+", dataFile, baseK+3);
            fprintf(fp, "'%s' using 1:%d with lines ls 4 title 'H (Koebenhavn)', \\\n+", dataFile, baseK+4);
            fprintf(fp, "'%s' using 1:%d with lines ls 5 title 'R (Koebenhavn)'\n", dataFile, baseK+5);
        }
    }

    fprintf(fp, "pause -1 'Press enter to close'\n");

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