#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <stdlib.h> // Needed for rand() and srand()

#define ALDERS_GRUPPER 4
#define MAX_DAYS 500
#define STEPS_PER_DAY 10 // 10 steps per day = dt of 0.1

// Math constants
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// --- GLOBALS AND PARAMETERS ---
float S0_AAL = 121878, E0_AAL = 0, I0_AAL = 10, R0_AAL = 0;
float S0_KBH = 667099, E0_KBH = 0, I0_KBH = 50, R0_KBH = 0;

float beta_AAL = 0.3247;
float gamma_AAL = 0.143;
float sigma_AAL = 1.0 / 6.0;

float beta_KBH = 0.4;
float gamma_KBH = 0.143;
float sigma_KBH = 1.0 / 6.0;

float h_factor[ALDERS_GRUPPER] = {0.2, 1.0, 2.0, 5.0};
float age_beta_factor[ALDERS_GRUPPER] = {0.8, 1.0, 1.2, 1.5};
float age_sigma_factor[ALDERS_GRUPPER] = {1.0, 1.0, 0.9, 0.8};
float age_gamma_factor[ALDERS_GRUPPER] = {1.2, 1.0, 0.9, 0.7};

float N_AAL = 121878;
float N_KBH = 667099;

// --- PROTOTYPES ---
void bruger_input();
void udvid_med_smitte_stop_og_vaccine(int *use_app, int *use_vaccine);
long poisson(double lambda);

// The Core Solver Function
void solve_model(int model_type, int use_app, int use_vaccine, FILE *file, int replicate_id, int is_stochastic);

// Wrappers
void run_deterministic(int model_type);
void run_stochastic(int model_type, int replicates);

// Plotting
void create_gnuplot_script(const char *filename, const char *datafile, int replicates, int model_type, int is_stochastic);

// --- MAIN ---
int main(void) {
    // SEED RANDOMNESS ONCE
    srand(time(NULL)); 
    bruger_input();
    return 0;
}

// --- HELPER: POISSON RANDOM NUMBER GENERATOR ---
// This turns a "Rate" into a discrete random "Number of Events"
long poisson(double lambda) {
    if (lambda <= 0) return 0;
    
    // For large rates, use Normal Approximation (faster)
    if (lambda > 30) {
        double u1 = (double)rand() / RAND_MAX;
        double u2 = (double)rand() / RAND_MAX;
        if (u1 < 1e-9) u1 = 1e-9;
        double z = sqrt(-2.0 * log(u1)) * cos(2.0 * M_PI * u2);
        long val = (long)(lambda + sqrt(lambda) * z + 0.5);
        return (val < 0) ? 0 : val;
    }

    // For small rates, use Knuth's algorithm
    double L = exp(-lambda);
    double p = 1.0;
    long k = 0;
    do {
        k++;
        p *= (double)rand() / RAND_MAX;
    } while (p > L && k < 10000);
    return k - 1;
}

// --- MENU ---
void bruger_input() {
    printf("\nCOVID-19 Simulator\n");
    printf("1. Kør Normal (Deterministisk)\n");
    printf("2. Kør Stokastisk (Flere Replicates)\n");
    printf("Valg: ");
    
    int valg;
    if (scanf("%d", &valg) != 1) return;

    char model_str[10];
    printf("Model (SIR, SEIR, SEIHR): ");
    scanf("%9s", model_str);

    int model_type = 1;
    if (strcasecmp(model_str, "SEIR") == 0) model_type = 2;
    if (strcasecmp(model_str, "SEIHR") == 0) model_type = 3;

    if (valg == 1) {
        run_deterministic(model_type);
    } else {
        int reps;
        printf("Antal replicates (f.eks. 50): ");
        scanf("%d", &reps);
        run_stochastic(model_type, reps);
    }
}

// --- WRAPPERS ---
void run_deterministic(int model_type) {
    int app = 0, vac = 0;
    udvid_med_smitte_stop_og_vaccine(&app, &vac);

    FILE *f = fopen("data_det.txt", "w");
    if(!f) return;

    // Call solver with is_stochastic = 0
    solve_model(model_type, app, vac, f, 0, 0); 
    fclose(f);

    create_gnuplot_script("plot_det.gnu", "data_det.txt", 1, model_type, 0);
    printf("Kør: gnuplot -p plot_det.gnu\n");
}

void run_stochastic(int model_type, int replicates) {
    int app = 0, vac = 0;
    udvid_med_smitte_stop_og_vaccine(&app, &vac);

    FILE *f = fopen("data_stoch.txt", "w");
    if(!f) return;

    printf("Simulerer %d replicates...\n", replicates);
    for(int i=0; i<replicates; i++) {
        // Call solver with is_stochastic = 1
        solve_model(model_type, app, vac, f, i, 1);
    }
    fclose(f);

    create_gnuplot_script("plot_stoch.gnu", "data_stoch.txt", replicates, model_type, 1);
    printf("Kør: gnuplot -p plot_stoch.gnu\n");
}

void udvid_med_smitte_stop_og_vaccine(int *use_app, int *use_vaccine) {
    char a, v;
    printf("App (j/n)? "); scanf(" %c", &a);
    printf("Vaccine (j/n)? "); scanf(" %c", &v);
    *use_app = (a == 'j' || a == 'J');
    *use_vaccine = (v == 'j' || v == 'J');
}

// --- CORE SOLVER FUNCTION ---
// Handles both Deterministic and Stochastic based on the flag
void solve_model(int model_type, int use_app, int use_vaccine, FILE *file, int replicate_id, int is_stochastic) 
{
    // Define State Variables
    float S_AAL[ALDERS_GRUPPER], E_AAL[ALDERS_GRUPPER], I_AAL[ALDERS_GRUPPER], R_AAL[ALDERS_GRUPPER], H_AAL[ALDERS_GRUPPER];
    
    // Initialize (Using Aalborg only for clarity in the plot, Copenhagen logic is identical)
    for (int i = 0; i < ALDERS_GRUPPER; i++) {
        S_AAL[i] = S0_AAL / ALDERS_GRUPPER;
        E_AAL[i] = E0_AAL / ALDERS_GRUPPER;
        I_AAL[i] = I0_AAL / ALDERS_GRUPPER;
        R_AAL[i] = R0_AAL / ALDERS_GRUPPER;
        H_AAL[i] = 0;
    }

    // Write header for Gnuplot Indexing
    if (file) fprintf(file, "# Replicate %d\n", replicate_id);

    double dt = 1.0 / STEPS_PER_DAY; // Time step size (e.g., 0.1 day)

    for (int day = 0; day < MAX_DAYS; day++) 
    {
        // Inner loop for time stepping
        for (int step = 0; step < STEPS_PER_DAY; step++) 
        {
            float total_I = 0;
            for(int k=0; k<ALDERS_GRUPPER; k++) total_I += I_AAL[k];

            for (int i = 0; i < ALDERS_GRUPPER; i++) 
            {
                // Calculate Parameters
                float beta = beta_AAL * age_beta_factor[i];
                float sigma = sigma_AAL * age_sigma_factor[i];
                float gamma = gamma_AAL * age_gamma_factor[i];
                float h_rate = h_factor[i] * 0.01;

                if (use_app) { beta *= 0.75; gamma *= 1.1; }
                if (use_vaccine) { beta *= 0.5; }

                // --- CALCULATE TRANSITIONS ---
                
                // 1. Infection (S -> E or I)
                double rate_inf = beta * S_AAL[i] * total_I / N_AAL;
                double n_inf = 0;

                // 2. Progression (E -> I)
                double rate_prog = sigma * E_AAL[i];
                double n_prog = 0;

                // 3. Recovery/Hospital (I -> R or H)
                double rate_rec = gamma * I_AAL[i];
                double n_rec = 0;
                double rate_hosp = (model_type == 3) ? h_rate * I_AAL[i] : 0;
                double n_hosp = 0;

                // 4. Hospital Recovery (H -> R)
                double rate_h_rec = (model_type == 3) ? (gamma/2.0) * H_AAL[i] : 0;
                double n_h_rec = 0;

                if (is_stochastic) {
                    // STOCHASTIC: Use Poisson to determine integer events
                    n_inf = poisson(rate_inf * dt);
                    n_prog = poisson(rate_prog * dt);
                    n_rec = poisson(rate_rec * dt);
                    n_hosp = poisson(rate_hosp * dt);
                    n_h_rec = poisson(rate_h_rec * dt);

                    // Safety Checks: Cannot move more people than exist
                    if (n_inf > S_AAL[i]) n_inf = S_AAL[i];
                    if (n_prog > E_AAL[i]) n_prog = E_AAL[i];
                    if ((n_rec + n_hosp) > I_AAL[i]) {
                        // If sum of leaving I > I, scale them down proportionally
                        double factor = I_AAL[i] / (n_rec + n_hosp);
                        n_rec *= factor;
                        n_hosp *= factor;
                    }
                    if (n_h_rec > H_AAL[i]) n_h_rec = H_AAL[i];

                } else {
                    // DETERMINISTIC: Just math
                    n_inf = rate_inf * dt;
                    n_prog = rate_prog * dt;
                    n_rec = rate_rec * dt;
                    n_hosp = rate_hosp * dt;
                    n_h_rec = rate_h_rec * dt;
                }

                // --- APPLY UPDATES ---
                if (model_type == 1) { // SIR
                    S_AAL[i] -= n_inf;
                    I_AAL[i] += n_inf - n_rec;
                    R_AAL[i] += n_rec;
                } 
                else if (model_type == 2) { // SEIR
                    S_AAL[i] -= n_inf;
                    E_AAL[i] += n_inf - n_prog;
                    I_AAL[i] += n_prog - n_rec;
                    R_AAL[i] += n_rec;
                }
                else if (model_type == 3) { // SEIHR
                    S_AAL[i] -= n_inf;
                    E_AAL[i] += n_inf - n_prog;
                    I_AAL[i] += n_prog - n_rec - n_hosp;
                    H_AAL[i] += n_hosp - n_h_rec;
                    R_AAL[i] += n_rec + n_h_rec;
                }
            }
        } // End Step

        // Write data to file once per day
        float sumS=0, sumE=0, sumI=0, sumH=0, sumR=0;
        for(int k=0; k<ALDERS_GRUPPER; k++) {
            sumS+=S_AAL[k]; sumE+=E_AAL[k]; sumI+=I_AAL[k]; sumH+=H_AAL[k]; sumR+=R_AAL[k];
        }
        
        if (file) fprintf(file, "%d %.2f %.2f %.2f %.2f %.2f\n", day, sumS, sumE, sumI, sumH, sumR);

    } // End Day

    // Double newline required for Gnuplot 'index' to work
    if (file) fprintf(file, "\n\n");
}

// --- GNUPLOT SCRIPT GENERATOR ---
void create_gnuplot_script(const char *filename, const char *datafile, int replicates, int model_type, int is_stochastic) {
    FILE *fp = fopen(filename, "w");
    if (!fp) return;

    fprintf(fp, "set terminal wxt size 1200,800 enhanced persist\n");
    fprintf(fp, "set xlabel 'Days'\n");
    fprintf(fp, "set ylabel 'Individuals'\n");
    fprintf(fp, "set grid\n");
    fprintf(fp, "set key top right\n");

    // Colors: Green(S), Orange(E), Red(I), Purple(H), Blue(R)
    // If stochastic, we use Alpha channel (last 2 hex digits) for transparency.
    // e.g., #4000FF00 -> 40 is alpha (transparency), 00FF00 is green.
    if (is_stochastic) {
        fprintf(fp, "set title 'Stochastic Model (%d Replicates)'\n", replicates);
        // Semi-transparent lines
        fprintf(fp, "set style line 1 lc rgb '#40008000' lt 1 lw 1\n"); // S
        fprintf(fp, "set style line 2 lc rgb '#40FFA500' lt 1 lw 1\n"); // E
        fprintf(fp, "set style line 3 lc rgb '#40FF0000' lt 1 lw 1\n"); // I
        fprintf(fp, "set style line 4 lc rgb '#40800080' lt 1 lw 1\n"); // H
        fprintf(fp, "set style line 5 lc rgb '#400000FF' lt 1 lw 1\n"); // R
    } else {
        fprintf(fp, "set title 'Deterministic Model'\n");
        // Solid lines
        fprintf(fp, "set style line 1 lc rgb '#008000' lt 1 lw 2\n"); 
        fprintf(fp, "set style line 2 lc rgb '#FFA500' lt 1 lw 2\n");
        fprintf(fp, "set style line 3 lc rgb '#FF0000' lt 1 lw 2\n");
        fprintf(fp, "set style line 4 lc rgb '#800080' lt 1 lw 2\n");
        fprintf(fp, "set style line 5 lc rgb '#0000FF' lt 1 lw 2\n");
    }

    fprintf(fp, "plot ");

    // Loop to plot all replicates
    // We only give a Title to the very first replicate (index 0) so the legend isn't huge.
    int max_i = (is_stochastic) ? replicates : 1;
    
    // We construct the plot command string iteratively
    // Columns: 1=Day, 2=S, 3=E, 4=I, 5=H, 6=R
    
    // S
    fprintf(fp, "'%s' index 0 using 1:2 w l ls 1 title 'S', ", datafile);
    // E (if applicable)
    if (model_type >= 2) fprintf(fp, "'' index 0 using 1:3 w l ls 2 title 'E', ");
    // I
    fprintf(fp, "'' index 0 using 1:4 w l ls 3 title 'I', ");
    // H (if applicable)
    if (model_type == 3) fprintf(fp, "'' index 0 using 1:5 w l ls 4 title 'H', ");
    // R
    fprintf(fp, "'' index 0 using 1:6 w l ls 5 title 'R' ");

    // If stochastic, add the rest of the lines without titles
    if (is_stochastic && replicates > 1) {
        fprintf(fp, ", \\\n");
        fprintf(fp, "for [i=1:%d] '%s' index i using 1:2 w l ls 1 notitle, ", replicates-1, datafile);
        if (model_type >= 2) fprintf(fp, "for [i=1:%d] '' index i using 1:3 w l ls 2 notitle, ", replicates-1);
        fprintf(fp, "for [i=1:%d] '' index i using 1:4 w l ls 3 notitle, ", replicates-1);
        if (model_type == 3) fprintf(fp, "for [i=1:%d] '' index i using 1:5 w l ls 4 notitle, ", replicates-1);
        fprintf(fp, "for [i=1:%d] '' index i using 1:6 w l ls 5 notitle", replicates-1);
    }

    fprintf(fp, "\n");
    fclose(fp);
}