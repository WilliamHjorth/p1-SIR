// IDE: kunne man lave en struct til alle de indtastede værdier (for at kunne sende dem videre med funktionerne?)

#include <stdio.h>
#include <string.h>

typedef struct
{
    float S;
    float I;
    float R;
    float beta;
    float gamma;
} SIR_model_værdier;

typedef struct
{
    float S;
    float E;
    float I;
    float R;
    float sigma;
    float beta;
    float gamma;
} SEIR_model_værdier;

typedef struct
{
    float S;
    float E;
    float I;
    float H;
    float R;
    float sigma;
    float h;
    float beta;
    float gamma;
} SEIHR_model_værdier;

void bruger_input();
void færdige_covid_simuleringer();
void tilpas_funktion();
void udvid_med_smitte_stop_og_vaccine();
void sammenligning();

// startværdier
int main(void)
{
    bruger_input();

    udvid_med_smitte_stop_og_vaccine();

    sammenligning();

    return 0;
}

// funktion hvor brugeren bliver givet nogle valg
void bruger_input()
{

    printf("\nDette er et program, der kan simulere smittespredning!\n");
    printf("Gennem hele programmet kan du taste ctrl + C, hvis du ønsker at afslutte.\n\n\n");

    printf("Du har nu følgende to muligheder:\n\n");
    printf("  1) Tast Enter for at gå direkte til Covid-19 smittesimuleringer.\n\n");
    printf("  2) Tast T for selv at tilpasse modellen til et bestemt område.\n\n");

    printf("Indtast valg:\n");

    int ch = getchar(); // læser et tastetryk

    if (ch == '\n')
    {
        printf("Du valgte: Gå direkte til smittesimulering.\n");
        færdige_covid_simuleringer();
    }
    else if (ch == 'T' || ch == 't')
    {
        printf("Du valgte: Tilpas modellen.\n");
        tilpas_funktion();
    }
    else
    {
        printf("Ugyldigt valg. Prøv igen\n");
    }
}

// funktion hvor du kan tilpasse modellen
void tilpas_funktion()
{
    float S1, E1, I1, H1, R1, sigma1, h1, beta1, gamma1;

    char valg[6];
    char app;
    char vaccine;

    printf("Vælg hvilken model ved at taste SIR, SEIR eller SEIHR\n");
    scanf("%5s", valg);

    if (strcmp(valg, "SIR") == 0)
    {
        printf("Indtast Susceptibles (S):\n");
        scanf("%f", &S1);
        printf("Indtast Infected (I) :\n");
        scanf("%f", &I1);
        printf("Indtast Recovered (R) :\n");
        scanf("%f", &R1);
        printf("Indtast smittespredning (Beta):\n");
        scanf("%f", &beta1);
        printf("Indtast recovery-rate (gamma):\n");
        scanf("%f", &gamma1);

        SIR_model_værdier input;
        input.S = S1;
        input.I = I1;
        input.R = R1;
        input.beta = beta1;
        input.gamma = gamma1;

        SIR_model(input.S, input.I, input.R, input.beta, input.gamma);
    }
    else if (strcmp(valg, "SEIR") == 0)
    {
        printf("Indtast Susceptibles (S):\n");
        scanf("%f", &S1);
        printf("Indtast Exposed (E):\n");
        scanf("%f", &E1);
        printf("Indtast Infected (I) :\n");
        scanf("%f", &I1);
        printf("Indtast Recovered (R) :\n");
        scanf("%f", &R1);
        printf("Indtast smittespredning (Beta):\n");
        scanf("%f", &beta1);
        printf("Indtast recovery-rate (gamma):\n");
        scanf("%f", &gamma1);
        printf("Indtast (sigma):\n");
        scanf("%f", &sigma1);

        SEIR_model_værdier input;
        input.S = S1;
        input.E = E1;
        input.I = I1;
        input.R = R1;
        input.sigma = sigma1;
        input.beta = beta1;
        input.gamma = gamma1;

        SEIR_model(input.S, input.E, input.I, input.R, input.sigma, input.beta, input.gamma);
    }
    else if (strcmp(valg, "SEIHR") == 0)
    {
        printf("Indtast Susceptibles (S):\n");
        scanf("%f", &S1);
        printf("Indtast Exposed (E):\n");
        scanf("%f", &E1);
        printf("Indtast Infected (I) :\n");
        scanf("%f", &I1);
        printf("Indtast Hospitalized (H):\n");
        scanf("%f", &H1);
        printf("Indtast Recovered (R) :\n");
        scanf("%f", &R1);
        printf("Indtast smittespredning (Beta):\n");
        scanf("%f", &beta1);
        printf("Indtast recovery-rate (gamma):\n");
        scanf("%f", &gamma1);
        printf("Indtast (sigma):\n");
        scanf("%f", &sigma1);
        printf("Indtast (parameter for H?):\n");
        scanf("%f", &h1);

        SEIHR_model_værdier input;
        input.S = S1;
        input.E = E1;
        input.I = I1;
        input.H = H1;
        input.R = R1;
        input.h = h1;
        input.sigma = sigma1;
        input.beta = beta1;
        input.gamma = gamma1;

        SEIHR_model(input.S, input.E, input.I, input.H, input.R, input.sigma, input.beta, input.gamma, input.h);
    }
    else
    {
        printf("Forkert input. Tast enten SIR, SEIR eller SEIHR");
    }
}

void færdige_covid_simuleringer()
{
    char valg1;

    printf("Du får nu følgende valgmuligheder:\n");
    printf(" 1) Ønsker du at se en simulering for Aalborg, tast A\n");
    printf(" 2) Ønsker du at se en simulering for København, tast K\n");
    printf(" 3) Vil du sammenligne de to byer, tast S\n");

    // kode
    if (valg1 == 'A' || valg1 == 'a')
    {
        AAU_model();
    }
    else if (valg1 == 'K' || valg1 == 'k')
    {
        KBH_model();
    }
    else if (valg1 == 'S' || valg1 == 's')
    {
        AAU_og_KBH_model();
    }
    else
    {
        printf("Ugyldigt input. Prøv igen");
    }

    char valg2;
    printf("Ønsker du at tilføje en transferrate mellem byerne?\n");
    scanf(" %c", &valg2);

    if (valg2 == 'j' || valg2 == 'J')
    {
        // funktion_der_kalder_transfer_mellem_byer();
    }
    else
    {
        return;
    }
}

void udvid_med_smitte_stop_og_vaccine()
{

    char app;
    char vaccine;

    printf("Har hele befolkningen smittestop|appen(svar j/n)?\n");
    scanf(" %c", &app);

    printf("Er der udrullet en vaccine mod sygdommen(svar j/n)?\n");
    scanf(" %c", &vaccine);

    if (app == 'j' || app == 'J' && vaccine == 'j' || vaccine == 'J')
    {
        // kald funktion med både vaccine og app
    }
    else if (app == 'j' || app == 'J')
    {
        // kald funktion med app
    }
    else if (vaccine == 'j' || vaccine == 'J')
    {
        // kald funktion med vaccine
    }

    // her skal færdig graf komme ud.
}

void sammenligning()
{

    char svar;
    printf("Ønsker du at sammenligne med et andet område?\n"); // hvis ja bliver man stillet de samme spørgsmål (men samme model)
    scanf(" %c", svar);

    if (svar == 'j' || svar == 'J')
    {
        tilpas_funktion();
        // mangler noget
    }
    else
    {
        return 0;
    }

    // her skal færdig graf komme ud

    printf("Ønsker du at tilføje en transferrate mellem de to?\n");
}
