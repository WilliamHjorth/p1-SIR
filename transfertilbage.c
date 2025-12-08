#include <stdio.h>
#include <math.h>
#include <stdlib.h>

// Struktur til at holde styr på rejsende
typedef struct {
    float S;
    float E;
    float I;
    float R;
    int dage_i_byen; // Hvor mange dage de har været i byen
} Rejsende;

// Globale variable for rejsende
Rejsende rejsende_AAL_til_KBH = {0, 0, 0, 0, 0};
Rejsende rejsende_KBH_til_AAL = {0, 0, 0, 0, 0};

// Til at holde styr på de 90% inficerede der bliver længere
Rejsende lang_ophold_infected_AAL_til_KBH = {0, 0, 0, 0, 0};
Rejsende lang_ophold_infected_KBH_til_AAL = {0, 0, 0, 0, 0};

// funktionsprototyper
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
int sirModelToByer(float S_AAL_start, float E_AAL_start, float I_AAL_start, float R_AAL_start, 
                   float S_KBH_start, float E_KBH_start, float I_KBH_start, float R_KBH_start)
{
    int valid = 0; //betyder “vi har endnu ikke fået et gyldigt input”.
    char valg = 0; //holder brugerens input (“w” eller “m”).

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

    // Nulstil rejsende
    rejsende_AAL_til_KBH = (Rejsende){0, 0, 0, 0, 0};
    rejsende_KBH_til_AAL = (Rejsende){0, 0, 0, 0, 0};
    lang_ophold_infected_AAL_til_KBH = (Rejsende){0, 0, 0, 0, 0};
    lang_ophold_infected_KBH_til_AAL = (Rejsende){0, 0, 0, 0, 0};

    for (int n = 0; n < 150; n++)
    {
        float dt = 1.0f;
        
        // 1. Beregn hvor mange der rejser fra hver by (0,1% af hver kategori)
        float S_ud_AAL = t * S_AAL;
        float E_ud_AAL = t * E_AAL;
        float I_ud_AAL = t * I_AAL;
        float R_ud_AAL = t * R_AAL;
        
        float S_ud_KBH = t * S_KBH;
        float E_ud_KBH = t * E_KBH;
        float I_ud_KBH = t * I_KBH;
        float R_ud_KBH = t * R_KBH;
        
        // 2. Fordel de inficerede: 90% bliver i længere tid, 10% rejser med de andre
        float I_ud_AAL_long_stay = I_ud_AAL * 0.9f;  // 90% bliver længere
        float I_ud_AAL_normal = I_ud_AAL * 0.1f;     // 10% rejser normalt
        
        float I_ud_KBH_long_stay = I_ud_KBH * 0.9f;
        float I_ud_KBH_normal = I_ud_KBH * 0.1f;
        
        // 3. Træk ALLE udrejsende fra hjembyen
        S_AAL -= S_ud_AAL; //er præcis det samme som S_AAL = S_AAL - S_ud_AAL
        E_AAL -= E_ud_AAL;
        I_AAL -= I_ud_AAL;
        R_AAL -= R_ud_AAL;
        
        S_KBH -= S_ud_KBH;
        E_KBH -= E_ud_KBH;
        I_KBH -= I_ud_KBH;
        R_KBH -= R_ud_KBH;
        
        // 4. Tilføj rejsende til modtagerbyen med det samme
        // AAL -> KBH: Alle normale rejsende + kun 10% af inficerede
        S_KBH += S_ud_AAL;
        E_KBH += E_ud_AAL;
        I_KBH += I_ud_AAL_normal;  // Kun 10% af inficerede kommer med det samme
        R_KBH += R_ud_AAL;
        
        // KBH -> AAL: Alle normale rejsende + kun 10% af inficerede
        S_AAL += S_ud_KBH;
        E_AAL += E_ud_KBH;
        I_AAL += I_ud_KBH_normal;  // Kun 10% af inficerede kommer med det samme
        R_AAL += R_ud_KBH;
        
        // 5. Gem de rejsende der skal håndteres særskilt
        // AAL -> KBH rejsende (alle normale rejsende + 10% af I)
        rejsende_AAL_til_KBH.S += S_ud_AAL;
        rejsende_AAL_til_KBH.E += E_ud_AAL;
        rejsende_AAL_til_KBH.I += I_ud_AAL_normal;  // Kun de 10% der rejser normalt
        rejsende_AAL_til_KBH.R += R_ud_AAL;
        rejsende_AAL_til_KBH.dage_i_byen++;
        
        // KBH -> AAL rejsende
        rejsende_KBH_til_AAL.S += S_ud_KBH;
        rejsende_KBH_til_AAL.E += E_ud_KBH;
        rejsende_KBH_til_AAL.I += I_ud_KBH_normal;  // Kun de 10% der rejser normalt
        rejsende_KBH_til_AAL.R += R_ud_KBH;
        rejsende_KBH_til_AAL.dage_i_byen++;
        
        // Gem de 90% inficerede der bliver længere
        lang_ophold_infected_AAL_til_KBH.I += I_ud_AAL_long_stay;
        lang_ophold_infected_AAL_til_KBH.dage_i_byen++;
        
        lang_ophold_infected_KBH_til_AAL.I += I_ud_KBH_long_stay;
        lang_ophold_infected_KBH_til_AAL.dage_i_byen++;
        
        // 6. Håndter tilbagevendende rejsende efter 3 dage
        if (rejsende_AAL_til_KBH.dage_i_byen >= 3 && 
            (rejsende_AAL_til_KBH.S > 0 || rejsende_AAL_til_KBH.E > 0 || 
             rejsende_AAL_til_KBH.I > 0 || rejsende_AAL_til_KBH.R > 0)) {
            // AAL -> KBH rejsende vender tilbage til AAL
            S_AAL += rejsende_AAL_til_KBH.S; //S_AAL = S_AAL + rejsende_AAL_til_KBH.S;
            E_AAL += rejsende_AAL_til_KBH.E;
            I_AAL += rejsende_AAL_til_KBH.I;
            R_AAL += rejsende_AAL_til_KBH.R;
            
            // Træk fra KBH
            S_KBH -= rejsende_AAL_til_KBH.S;
            E_KBH -= rejsende_AAL_til_KBH.E;
            I_KBH -= rejsende_AAL_til_KBH.I;
            R_KBH -= rejsende_AAL_til_KBH.R;
            
            // Nulstil
            rejsende_AAL_til_KBH = (Rejsende){0, 0, 0, 0, 0};
        }
        
        if (rejsende_KBH_til_AAL.dage_i_byen >= 3 && 
            (rejsende_KBH_til_AAL.S > 0 || rejsende_KBH_til_AAL.E > 0 || 
             rejsende_KBH_til_AAL.I > 0 || rejsende_KBH_til_AAL.R > 0)) {
            // KBH -> AAL rejsende vender tilbage til KBH
            S_KBH += rejsende_KBH_til_AAL.S;
            E_KBH += rejsende_KBH_til_AAL.E;
            I_KBH += rejsende_KBH_til_AAL.I;
            R_KBH += rejsende_KBH_til_AAL.R;
            
            // Træk fra AAL
            S_AAL -= rejsende_KBH_til_AAL.S;
            E_AAL -= rejsende_KBH_til_AAL.E;
            I_AAL -= rejsende_KBH_til_AAL.I;
            R_AAL -= rejsende_KBH_til_AAL.R;
            
            // Nulstil
            rejsende_KBH_til_AAL = (Rejsende){0, 0, 0, 0, 0};
        }
        
        // 7. Håndter de 90% inficerede efter 7 dage (længere periode)
        if (lang_ophold_infected_AAL_til_KBH.dage_i_byen >= 7 && lang_ophold_infected_AAL_til_KBH.I > 0) {
            // De inficerede fra AAL i KBH vender tilbage til AAL (nu som recovered)
            R_AAL += lang_ophold_infected_AAL_til_KBH.I;
            I_KBH -= lang_ophold_infected_AAL_til_KBH.I;  // Træk fra KBH's inficerede
            
            // Nulstil
            lang_ophold_infected_AAL_til_KBH = (Rejsende){0, 0, 0, 0, 0};
        }
        
        if (lang_ophold_infected_KBH_til_AAL.dage_i_byen >= 7 && lang_ophold_infected_KBH_til_AAL.I > 0) {
            // De inficerede fra KBH i AAL vender tilbage til KBH (nu som recovered)
            R_KBH += lang_ophold_infected_KBH_til_AAL.I;
            I_AAL -= lang_ophold_infected_KBH_til_AAL.I;  // Træk fra AAL's inficerede
            
            // Nulstil
            lang_ophold_infected_KBH_til_AAL = (Rejsende){0, 0, 0, 0, 0};
        }
        
        // 8. Opdater N-værdierne (vigtigt for differentialligningerne)
        N_AAL = S_AAL + E_AAL + I_AAL + R_AAL;
        N_KBH = S_KBH + E_KBH + I_KBH + R_KBH;
        
        // 9. Differentialligninger for SEIR-modellen - AALBORG
        float dSdT_AAL = -beta_AAL * S_AAL * I_AAL / N_AAL;
        float dEdT_AAL = beta_AAL * S_AAL * I_AAL / N_AAL - sigma_AAL * E_AAL;
        float dIdT_AAL = sigma_AAL * E_AAL - gamma_AAL * I_AAL;
        float dRdt_AAL = gamma_AAL * I_AAL;
        
        // Differentialligninger for SEIR-modellen - KØBENHAVN
        float dSdT_KBH = -beta_KBH * S_KBH * I_KBH / N_KBH;
        float dEdT_KBH = beta_KBH * S_KBH * I_KBH / N_KBH - sigma_KBH * E_KBH;
        float dIdT_KBH = sigma_KBH * E_KBH - gamma_KBH * I_KBH;
        float dRdt_KBH = gamma_KBH * I_KBH;
        
        // Euler integration
        S_AAL += dSdT_AAL * dt;
        E_AAL += dEdT_AAL * dt;
        I_AAL += dIdT_AAL * dt;
        R_AAL += dRdt_AAL * dt;
        
        S_KBH += dSdT_KBH * dt;
        E_KBH += dEdT_KBH * dt;
        I_KBH += dIdT_KBH * dt;
        R_KBH += dRdt_KBH * dt;

        // Sikre at vi ikke får negative værdier
        if (S_AAL < 0) S_AAL = 0;
        if (E_AAL < 0) E_AAL = 0;
        if (I_AAL < 0) I_AAL = 0;
        if (R_AAL < 0) R_AAL = 0;
        if (S_KBH < 0) S_KBH = 0;
        if (E_KBH < 0) E_KBH = 0;
        if (I_KBH < 0) I_KBH = 0;
        if (R_KBH < 0) R_KBH = 0;

        // printer resultater
        printf("Day %d: AAL(S=%.0f, E=%.0f, I=%.0f, R=%.0f) KBH(S=%.0f, E=%.0f I=%.0f, R=%.0f)\n", 
               n, S_AAL, E_AAL, I_AAL, R_AAL, S_KBH, E_KBH, I_KBH, R_KBH);

        fprintf(file, "%d %f %f %f %f %f %f %f %f\n", 
                n, S_AAL, E_AAL, I_AAL, R_AAL, S_KBH, E_KBH, I_KBH, R_KBH);
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