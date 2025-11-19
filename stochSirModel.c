#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

typedef struct
{
    double *time;
    int *S;
    int *I;
    int *R;
    int length;
    int capacity;
} SIRResults;

void initResults(SIRResults *results, int initialCapacity)
{
    results->time = (double *)malloc(initialCapacity * sizeof(double));
    results->S = (int *)malloc(initialCapacity * sizeof(int));
    results->I = (int *)malloc(initialCapacity * sizeof(int));
    results->R = (int *)malloc(initialCapacity * sizeof(int));
    results->length = 0;
    results->capacity = initialCapacity;
}

void appendResults(SIRResults *results, double t, int S, int I, int R)
{
    if (results->length >= results->capacity)
    {
        results->capacity *= 2;
        results->time = (double *)realloc(results->time, results->capacity * sizeof(double));
        results->S = (int *)realloc(results->S, results->capacity * sizeof(int));
        results->I = (int *)realloc(results->I, results->capacity * sizeof(int));
        results->R = (int *)realloc(results->R, results->capacity * sizeof(int));
    }
    results->time[results->length] = t;
    results->S[results->length] = S;
    results->I[results->length] = I;
    results->R[results->length] = R;
    results->length++;
}

void freeResults(SIRResults *results)
{
    free(results->time);
    free(results->S);
    free(results->I);
    free(results->R);
}

double rexp(double rate)
{
    return -log(1.0 - (double)rand() / RAND_MAX) / rate; // Time between events
}

void sir(double beta, double gamma, int N, int S0, int I0, int R0, double tf, SIRResults *results)
{
    double time = 0.0;
    int S = S0;
    int I = I0;
    int R = R0;

    initResults(results, 1000);

    while (time < tf)
    {
        appendResults(results, time, S, I, R);

        double pf1 = beta * S * I;
        double pf2 = gamma * I;
        double pf = pf1 + pf2; // Probability rate

        double dt = rexp(pf);
        time += dt;

        if (time > tf)
        {
            break;
        }

        double ru = (double)rand() / RAND_MAX;

        if (ru < (pf1 / pf))
        {
            S--;
            I++;
        }
        else
        {
            I--;
            R++;
        }

        if (I == 0)
        {
            break;
        }
    }
}

void writeDataFile(SIRResults *results, const char *filename)
{
    FILE *fp = fopen(filename, "w");
    if (fp == NULL)
    {
        fprintf(stderr, "Error opening file %s\n", filename);
        return;
    }

    fprintf(fp, "# time S I R\n");
    for (int i = 0; i < results->length; i++)
    {
        fprintf(fp, "%.6f %d %d %d\n", results->time[i], results->S[i], results->I[i], results->R[i]);
    }

    fclose(fp);
}

void createGnuplotScript(const char *scriptFile, const char *data_file)
{
    FILE *fp = fopen(scriptFile, "w");
    if (fp == NULL)
    {
        fprintf(stderr, "Error opening file %s\n", scriptFile);
        return;
    }

    fprintf(fp, "set xlabel 'Time'\n");
    fprintf(fp, "set ylabel 'Population'\n");
    fprintf(fp, "set title 'Stochastic SIR Model'\n");
    fprintf(fp, "set grid\n");
    fprintf(fp, "set key right top\n");
    fprintf(fp, "plot '%s' using 1:2 with lines lw 2 linecolor rgb 'green' title 'S', \\\n", data_file);
    fprintf(fp, "     '%s' using 1:3 with lines lw 2 linecolor rgb 'red' title 'I', \\\n", data_file);
    fprintf(fp, "     '%s' using 1:4 with lines lw 2 linecolor rgb 'blue' title 'R'\n", data_file);
    fprintf(fp, "pause -1 'Press enter to close'\n");

    fclose(fp);
}

int main()
{
    srand(50); // Set random seed

    SIRResults results;
    double beta = 0.1 / 1000.0;
    double gamma = 0.05;
    int N = 1000;
    int S0 = 1000;
    int I0 = 1;
    int R0 = 0;
    double tf = 200.0;

    // Run simulation
    sir(beta, gamma, N, S0, I0, R0, tf, &results);

    // Write data file
    writeDataFile(&results, "sir_data.txt");

    // Create gnuplot script
    createGnuplotScript("plot_sir.gp", "sir_data.txt");

    printf("\nData written to sir_data.txt\n");
    printf("Gnuplot script written to plot_sir.gp\n");
    printf("\nOpening plot in gnuplot...\n");

    // Open gnuplot directly to display the plot
    system("gnuplot -persist plot_sir.gp");

    freeResults(&results);

    return 0;
}