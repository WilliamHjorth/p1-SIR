#include <stdio.h>
#include <string.h>

#define ALDERS_GRUPPER 4
#define MAX_DAYS 150

typedef struct
{
    float S;
    float I;
    float R;
    float beta;
    float gamma;
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
    float h; // hospitalisering rate
    float beta;
    float gamma;
} SEIHR_model;

// Startværdier Aalborg
float S0_AAL = 121878;
float E0_AAL = 0;
float I0_AAL = 10;
float R0_AAL = 0;

// Startværdier København
float S0_KBH = 667099;
float E0_KBH = 0;
float I0_KBH = 50;
float R0_KBH = 0;

// Parametre
float beta_AAL = 0.3247;
float gamma_AAL = 0.143;
float sigma_AAL = 1.0 / 6.0; // inkubationstid 6 dage

float beta_KBH = 0.4;
float gamma_KBH = 0.143;
float sigma_KBH = 1.0 / 6.0;

float h_factor[ALDERS_GRUPPER] = {0.2, 1.0, 2.0, 5.0};
float age_beta_factor[ALDERS_GRUPPER] = {0.8, 1.0, 1.2, 1.5};
float age_sigma_factor[ALDERS_GRUPPER] = {1.0, 1.0, 0.9, 0.8};
float age_gamma_factor[ALDERS_GRUPPER] = {1.2, 1.0, 0.9, 0.7};

float N_AAL = 121878;
float N_KBH = 667099;

// Funktionsprototyper
void bruger_input();
void tilpas_funktion();
void færdige_covid_simuleringer();
void udvid_med_smitte_stop_og_vaccine(int *use_app, int *use_vaccine);
void sirModelToByer(int model_type, int use_app, int use_vaccine);
// model_type: 1=SIR, 2=SEIR, 3=SEIHR

int main(void)
{
    bruger_input();
    return 0;
}

// Brugermenu
void bruger_input()
{
    printf("\nDette er et program, der simulerer smittespredning!\n\n");
    printf("Vælg:\n 1) Covid-19 smittesimulering (Enter)\n 2) Tilpas model (T)\n");

    int ch = getchar();
    if (ch == '\n')
    {
        færdige_covid_simuleringer();
    }
    else if (ch == 'T' || ch == 't')
    {
        tilpas_funktion();
    }
    else
    {
        printf("Ugyldigt valg.\n");
    }
}

// Tilpasning af modellen
void tilpas_funktion()
{
    char valg[6];
    printf("Vælg model (SIR, SEIR, SEIHR): ");
    scanf("%5s", valg);

    if (strcmp(valg, "SIR") == 0 || strcmp(valg, "sir") == 0)
    {
        sirModelToByer(1, 0, 0);
    }
    else if (strcmp(valg, "SEIR") == 0 || strcmp(valg, "seir") == 0)
    {
        sirModelToByer(2, 0, 0);
    }
    else if (strcmp(valg, "SEIHR") == 0 || strcmp(valg, "seihr") == 0)
    {
        sirModelToByer(3, 0, 0);
    }
    else
    {
        printf("Forkert input.\n");
    }
}

// Færdige Covid-simuleringer
void færdige_covid_simuleringer()
{
    char city_choice;
    printf("Vælg by: A=Aalborg, K=København, S=Begge: ");
    scanf(" %c", &city_choice);

    int use_app = 0, use_vaccine = 0;
    udvid_med_smitte_stop_og_vaccine(&use_app, &use_vaccine);

    // Modelvalg (for demo SIR, kan ændres)
    int model_type = 2; // SEIR som standard
    sirModelToByer(model_type, use_app, use_vaccine);
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
void sirModelToByer(int model_type, int use_app, int use_vaccine)
{
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

    // Gnuplot fil
    FILE *file = fopen("data_file.txt", "w");
    if (!file)
    {
        printf("Kan ikke åbne fil\n");
        return;
    }

    // Simulation
    for (int n = 0; n < MAX_DAYS; n++)
    {
        float total_I_AAL = 0, total_I_KBH = 0;
        for (int i = 0; i < ALDERS_GRUPPER; i++)
        {
            total_I_AAL += I_AAL[i];
            total_I_KBH += I_KBH[i];
        }

        float dS_AAL[ALDERS_GRUPPER], dE_AAL[ALDERS_GRUPPER], dI_AAL[ALDERS_GRUPPER], dR_AAL[ALDERS_GRUPPER], dH_AAL[ALDERS_GRUPPER];
        float dS_KBH[ALDERS_GRUPPER], dE_KBH[ALDERS_GRUPPER], dI_KBH[ALDERS_GRUPPER], dR_KBH[ALDERS_GRUPPER], dH_KBH[ALDERS_GRUPPER];

        for (int i = 0; i < ALDERS_GRUPPER; i++)
        {
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
                S_AAL[i] *= 0.8;
                beta_i_A *= 0.5;
                S_KBH[i] *= 0.8;
                beta_i_K *= 0.5;
            }

            if (model_type == 1)
            { // SIR
                dS_AAL[i] = -beta_i_A * S_AAL[i] * total_I_AAL / N_AAL;
                dI_AAL[i] = beta_i_A * S_AAL[i] * total_I_AAL / N_AAL - gamma_i_A * I_AAL[i];
                dR_AAL[i] = gamma_i_A * I_AAL[i];
                dE_AAL[i] = 0;
                dH_AAL[i] = 0;

                dS_KBH[i] = -beta_i_K * S_KBH[i] * total_I_KBH / N_KBH;
                dI_KBH[i] = beta_i_K * S_KBH[i] * total_I_KBH / N_KBH - gamma_i_K * I_KBH[i];
                dR_KBH[i] = gamma_i_K * I_KBH[i];
                dE_KBH[i] = 0;
                dH_KBH[i] = 0;
            }
            else if (model_type == 2)
            { // SEIR
                dS_AAL[i] = -beta_i_A * S_AAL[i] * total_I_AAL / N_AAL;
                dE_AAL[i] = beta_i_A * S_AAL[i] * total_I_AAL / N_AAL - sigma_i_A * E_AAL[i];
                dI_AAL[i] = sigma_i_A * E_AAL[i] - gamma_i_A * I_AAL[i];
                dR_AAL[i] = gamma_i_A * I_AAL[i];
                dH_AAL[i] = 0;

                dS_KBH[i] = -beta_i_K * S_KBH[i] * total_I_KBH / N_KBH;
                dE_KBH[i] = beta_i_K * S_KBH[i] * total_I_KBH / N_KBH - sigma_i_K * E_KBH[i];
                dI_KBH[i] = sigma_i_K * E_KBH[i] - gamma_i_K * I_KBH[i];
                dR_KBH[i] = gamma_i_K * I_KBH[i];
                dH_KBH[i] = 0;
            }
            else if (model_type == 3)
            { // SEIHR
                dS_AAL[i] = -beta_i_A * S_AAL[i] * total_I_AAL / N_AAL;
                dE_AAL[i] = beta_i_A * S_AAL[i] * total_I_AAL / N_AAL - sigma_i_A * E_AAL[i];
                dI_AAL[i] = sigma_i_A * E_AAL[i] - gamma_i_A * I_AAL[i] - h_i_A * I_AAL[i];
                dH_AAL[i] = h_i_A * I_AAL[i] - gamma_i_A / 2 * H_AAL[i];
                dR_AAL[i] = gamma_i_A * I_AAL[i] + gamma_i_A / 2 * H_AAL[i];

                dS_KBH[i] = -beta_i_K * S_KBH[i] * total_I_KBH / N_KBH;
                dE_KBH[i] = beta_i_K * S_KBH[i] * total_I_KBH / N_KBH - sigma_i_K * E_KBH[i];
                dI_KBH[i] = sigma_i_K * E_KBH[i] - gamma_i_K * I_KBH[i] - h_i_K * I_KBH[i];
                dH_KBH[i] = h_i_K * I_KBH[i] - gamma_i_K / 2 * H_KBH[i];
                dR_KBH[i] = gamma_i_K * I_KBH[i] + gamma_i_K / 2 * H_KBH[i];
            }

            // Opdatering
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

        // Summér alle aldersgrupper
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

        // Print til terminal
        printf("Day %d | AAL(S=%.0f,E=%.0f,I=%.0f,H=%.0f,R=%.0f) | KBH(S=%.0f,E=%.0f,I=%.0f,H=%.0f,R=%.0f)\n",
               n, sum_S_AAL, sum_E_AAL, sum_I_AAL, sum_H_AAL, sum_R_AAL,
               sum_S_KBH, sum_E_KBH, sum_I_KBH, sum_H_KBH, sum_R_KBH);

        // Skriv til fil
        fprintf(file, "%d %f %f %f %f %f %f %f %f %f %f\n",
                n, sum_S_AAL, sum_E_AAL, sum_I_AAL, sum_H_AAL, sum_R_AAL,
                sum_S_KBH, sum_E_KBH, sum_I_KBH, sum_H_KBH, sum_R_KBH);
    }

    fclose(file);
    printf("Simulering færdig. Data gemt i data_file.txt\n");
}