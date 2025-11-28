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

void writeDataFile(SIRResults *results, int numReplicates, const char *filename)
{
    FILE *fp = fopen(filename, "w");
    if (fp == NULL)
    {
        fprintf(stderr, "Error opening file %s\n", filename);
        return;
    }

    // Iterate through each replicate
    for (int rep = 0; rep < numReplicates; rep++)
    {
        // Write header for this block (optional, but helpful for reading)
        fprintf(fp, "# Replicate %d\n", rep);
        
        // Write the time series for THIS specific replicate
        for (int i = 0; i < results[rep].length; i++)
        {
            fprintf(fp, "%.6f %d %d %d\n", 
                    results[rep].time[i], 
                    results[rep].S[i], 
                    results[rep].I[i], 
                    results[rep].R[i]);
        }
        
        // TWO newlines indicate the end of a dataset block to Gnuplot
        fprintf(fp, "\n\n");
    }

    fclose(fp);
}

void createGnuplotScript(const char *scriptFile, const char *dataFile, int numReplicates)
{
    FILE *fp = fopen(scriptFile, "w");
    if (fp == NULL)
    {
        fprintf(stderr, "Error opening file %s\n", scriptFile);
        return;
    }

    fprintf(fp, "set xlabel 'Time'\n");
    fprintf(fp, "set ylabel 'Number of individuals'\n");
    fprintf(fp, "set title 'Stochastic SIR Model - %d Replicates'\n", numReplicates);
    fprintf(fp, "set grid\n");
    fprintf(fp, "set key off\n");
    
    // Define semi-transparent colors (optional, requires a terminal that supports it, 
    // but looks better for many replicates). If this fails, remove the '80'.
    fprintf(fp, "set style line 1 lc rgb '#8000FF00' lt 1 lw 1\n"); // Green for S
    fprintf(fp, "set style line 2 lc rgb '#80FF0000' lt 1 lw 1\n"); // Red for I
    fprintf(fp, "set style line 3 lc rgb '#800000FF' lt 1 lw 1\n"); // Blue for R

    // Use Gnuplot iteration to plot all blocks
    // 'index i' tells gnuplot to pick the i-th block of data separated by \n\n
    fprintf(fp, "plot for [i=0:%d] '%s' index i using 1:2 with lines ls 1, \\\n", numReplicates-1, dataFile);
    fprintf(fp, "     for [i=0:%d] '%s' index i using 1:3 with lines ls 2, \\\n", numReplicates-1, dataFile);
    fprintf(fp, "     for [i=0:%d] '%s' index i using 1:4 with lines ls 3\n", numReplicates-1, dataFile);

    fprintf(fp, "pause -1 'Press enter to close'\n");

    fclose(fp);
}

void runReplicates(double beta, double gamma, int N, int S0, int I0, int R0, double tf, int numReplicates)
{
    SIRResults *allResults = (SIRResults *)malloc(numReplicates * sizeof(SIRResults));

    printf("\nRunning %d replicates...\n", numReplicates);

    // Run all replicates
    for (int rep = 0; rep < numReplicates; rep++)
    {
        sir(beta, gamma, N, S0, I0, R0, tf, &allResults[rep]);
        if ((rep + 1) % 50 == 0)
        {
            printf("Completed %d/%d replicates\n", rep + 1, numReplicates);
        }
    }

    // Write replicates data file
    writeDataFile(allResults, numReplicates, "sir_replicates.txt");

    // Create gnuplot script for replicates
    createGnuplotScript("plot_replicates.gp", "sir_replicates.txt", numReplicates);

    printf("\nReplicates data written to sir_replicates.txt\n");
    printf("Gnuplot script written to plot_replicates.gp\n");
    printf("\nOpening replicates plot in gnuplot...\n");

    // Open gnuplot directly to display the plot
    system("gnuplot -persist plot_replicates.gp");

    // Free all results
    for (int rep = 0; rep < numReplicates; rep++)
    {
        freeResults(&allResults[rep]);
    }
    free(allResults);
}

int main()
{
    srand(50); // Set random seed

    double beta = 0.1 / 1000.0;
    double gamma = 0.05;
    int N = 1000;
    int I0 = 10;
    int S0 = N - I0;
    int R0 = 0;
    double tf = 250.0;

    int runs;
    printf("Type number of simulations to run: \n");
    scanf("%d", &runs);

    runReplicates(beta, gamma, N, S0, I0, R0, tf, runs);
    
    return 0;
}