#include <stdio.h>
#include <sys/time.h>
#include <mpi.h>

#define SIZE_32KB (4u * 1024u) /* double */
#define SIZE_2MB  (256u * 1024u) /* double */
#define ITERATIONS 10

double diff_timeval(struct timeval t2, double t1_sec, double t1_usec);
double my_sqrt(double number);
double calculate_mean(double * data);
double calculate_std(double * data, double mean);

int main(int argc, char **argv)
{
    int ierr, num_procs, my_id;
    struct timeval t1, t2;
    ierr = MPI_Init(&argc, &argv);

    /* find out my process ID, and how many process were started. */
    ierr = MPI_Comm_rank(MPI_COMM_WORLD, &my_id);
    ierr = MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    
    for (int j = SIZE_32KB; j <= SIZE_2MB; j *= 2u)
    {
        for (int k = 1; k <= 4; k ++)
        {
            double msg[j];
            double avg_rtt[ITERATIONS];
            for (int i = 0; i < ITERATIONS + 1; i ++)
            {
                if (my_id < k * 2 && my_id % 2 == 0)
                {
                    MPI_Send(&msg, j, MPI_DOUBLE, my_id + 1, 0, MPI_COMM_WORLD);
                    MPI_Recv(&msg, j, MPI_DOUBLE, my_id + 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    if (my_id == 0)
                    {
                        gettimeofday(&t2, NULL);
                        if (i > 0)  avg_rtt[i - 1] = diff_timeval(t2, msg[0], msg[1]);
                        if (i == ITERATIONS)
                        {
                            double mean = calculate_mean(avg_rtt);
                            double std = calculate_std(avg_rtt, mean);
                            if (k == 1) printf("%d", j / 128);
                            printf(" %.2f %.2f", mean, std);
                            if (k == 4) printf("\n");
                        }
                    }
                }
                else if (my_id < k * 2 && my_id % 2 == 1)
                {
                    MPI_Recv(&msg, j, MPI_DOUBLE, my_id - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    if (my_id == 1)
                    {
                        gettimeofday(&t1, NULL);
                        msg[0] = t1.tv_sec * 1.0;
                        msg[1] = t1.tv_usec * 1.0;
                    }
                    MPI_Send(&msg, j, MPI_DOUBLE, my_id - 1, 0, MPI_COMM_WORLD);
                }
            }
        }
    }

    ierr = MPI_Finalize();
    return 0;
}

double diff_timeval(struct timeval t2, double t1_sec, double t1_usec)
{
    double end = (t2.tv_sec * 10e+6 + t2.tv_usec) * 1.0;
    double start = t1_sec * 10e+6 + t1_usec;
    return end - start;
}

double calculate_mean(double * data)
{
    double sum = 0.0, std = 0.0;
    for (int i = 0; i < ITERATIONS; i ++)
        sum += data[i];
    
    return sum / (ITERATIONS * 1.0);
}

double calculate_std(double * data, double mean)
{
    double std = 0.0;
    for (int i = 0; i < ITERATIONS; i ++)
        std += (data[i] - mean) * (data[i] - mean);
    
    std = my_sqrt(std / (ITERATIONS * 1.0));
    return std;
}

double my_sqrt(double number)
{
    double error = number * 10e-8; /* precision */ 
    double s = number;

    while ((s - number / s) > error)
    {
        s = (s + number / s) / 2;
    }
    return s;
}