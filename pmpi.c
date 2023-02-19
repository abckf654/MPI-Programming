#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int ierr, num_procs, my_id, temp;
int * mtx;

int MPI_Init(int *argc, char ***argv)
{
    int ret = PMPI_Init(argc, argv);
    temp = MPI_Comm_rank(MPI_COMM_WORLD, &my_id);
    temp = MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    mtx = (int *) malloc((num_procs + 1) * sizeof(int));
    int i;
    for (i = 0; i < num_procs + 1; i ++)
        mtx[i] = 0;
    mtx[0] = my_id;
    return ret;
}

int MPI_Send(const void *buf, int count, MPI_Datatype datatype, int dest, int tag,
              MPI_Comm comm)
{
    int ret = PMPI_Send(buf, count, datatype, dest, tag, comm);
    mtx[dest + 1] ++;
    return ret;
}

int MPI_Isend(const void *buf, int count, MPI_Datatype datatype, int dest, int tag,
              MPI_Comm comm, MPI_Request *request)
{
    int ret = PMPI_Isend(buf, count, datatype, dest, tag, comm, request);
    mtx[dest + 1] ++;
    return ret;
}

int MPI_Finalize( void )
{
    int * ptr;
    if (my_id == 0)
    {
        int i, j;
        ptr = (int *) malloc(((num_procs + 1) * (num_procs + 1)) * sizeof(int));
        for (i = 0; i < num_procs + 1; i ++)
        {
            ptr[my_id * (num_procs + 1) + i] = mtx[i];
        }
        
        for (i = 1; i < num_procs; i ++)
        {
            for (j = 0; j < num_procs + 1; j ++)
            {
                int temp1 = 0;
                PMPI_Recv(&temp1, 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                ptr[i * (num_procs + 1) + j] = temp1;
            }
        }
    }
    else
    {
        int i;
        for (i = 0; i < num_procs + 1; i ++)
        {
            int temp1 = mtx[i];
            PMPI_Send(&temp1, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        }
    }
    
    int ret = PMPI_Finalize();
    
    FILE *fptr;
    fptr = fopen("matrix.data", "w");
    if (my_id == 0)
    {
        int i, j;
        for (i = 0; i < num_procs; i ++)
        {
            for (j = 0; j < num_procs + 1; j ++)
            {
                fprintf(fptr, "%d", ptr[i * (num_procs + 1) + j]);
                if (j < num_procs)
                    fprintf(fptr, "%s", "\t");
                else
                    fprintf(fptr, "%s", "\n");

            }
        }
    }
    fclose(fptr);

    free(mtx);
    free(ptr);
    return ret;
}