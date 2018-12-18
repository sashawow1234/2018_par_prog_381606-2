#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Task # 27: Count the number of mismatched characters in two strings

void InitializeStrings(char *string1, char *string2, const int size)
{
    int i;
    srand(time(NULL));

    for (i = 0; i < size; i++)
    {
        string1[i] = 'A' + rand() % ('Z' - 'A' + 1) + (rand() % 2) * ('a' - 'A');
        string2[i] = 'A' + rand() % ('Z' - 'A' + 1) + (rand() % 2) * ('a' - 'A');
    }
    string1[size] = '\0';
    string2[size] = '\0';
}

int CountMismatchesInTwoStrings(char *string1, char *string2, const int endIndex)
{
    int result = 0;
    int i;

    for (i = 0; i < endIndex; i++)
    {
        if (string1[i] != string2[i])
        {
            result++;
        }
    }

    return result;
}

int main(int argc, char **argv)
{
    int MAX_SIZE;
    int procNum, procRank = 0;
    int partSize, partRemainder;
    int i, partRemainderFlag = 0;
    int linearResult = 0, parallelResult = 0, mismatchCount = 0;
    double startTime = 0, endTime = 0;
    double linearTime = 0, parallelTime = 0;
    char *str1 = NULL, *str2 = NULL;
    char *temp1 = NULL, *temp2 = NULL;
    MPI_Status status;

    MPI_Init(&argc, &argv);
    // Get the number of processors
    MPI_Comm_size(MPI_COMM_WORLD, &procNum);
    // Get rank of current process
    MPI_Comm_rank(MPI_COMM_WORLD, &procRank);

    /************Linear realization************/
    if (procRank == 0)
    {
        if (argc == 2)
        {
            MAX_SIZE = atoi(argv[1]);
        }
        else
        {
            MAX_SIZE = 10000000;
        }
        partSize = MAX_SIZE / procNum;
        partRemainder = MAX_SIZE % procNum;
    }

    MPI_Bcast(&MAX_SIZE, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&partSize, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&partRemainder, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (procRank == 0)
    {
        str1 = (char *)malloc(sizeof(char) * (MAX_SIZE + 1));
        str2 = (char *)malloc(sizeof(char) * (MAX_SIZE + 1));
        InitializeStrings(str1, str2, MAX_SIZE);
        startTime = MPI_Wtime();
        linearResult = CountMismatchesInTwoStrings(str1, str2, MAX_SIZE);
        endTime = MPI_Wtime();
        linearTime = endTime - startTime;

        printf("Linear time = %.3f\n", endTime - startTime);
        printf("Linear result = %d\n", linearResult);

        /************Parallel realization********/

        startTime = MPI_Wtime();

        partRemainderFlag = 0;
        for (i = 1; i < procNum - 1; i++)
        {
            MPI_Send(&str1[partSize * i], partSize, MPI_CHAR, i, 1, MPI_COMM_WORLD);
            MPI_Send(&str2[partSize * i], partSize, MPI_CHAR, i, 2, MPI_COMM_WORLD);
        }
        if (procNum - 1 == i)
        {
            // Add remainder to last processor's partSize
            MPI_Send(&str1[partSize * (procNum - 1)], partSize + partRemainder, MPI_CHAR, (procNum - 1), 1, MPI_COMM_WORLD);
            MPI_Send(&str2[partSize * (procNum - 1)], partSize + partRemainder, MPI_CHAR, (procNum - 1), 2, MPI_COMM_WORLD);
        }

        mismatchCount = CountMismatchesInTwoStrings(str1, str2, partSize);
    }
    else if (procRank == procNum - 1)
    {
        temp2 = (char *)malloc(sizeof(char) * (partSize + partRemainder));
        temp1 = (char *)malloc(sizeof(char) * (partSize + partRemainder));
        MPI_Recv(temp1, partSize + partRemainder, MPI_CHAR, 0, 1, MPI_COMM_WORLD, &status);
        MPI_Recv(temp2, partSize + partRemainder, MPI_CHAR, 0, 2, MPI_COMM_WORLD, &status);
        mismatchCount = CountMismatchesInTwoStrings(temp1, temp2, partSize + partRemainder);
        
        free(temp1);
        free(temp2);
    }
    else
    {
        temp1 = (char *)malloc(sizeof(char) * partSize);
        temp2 = (char *)malloc(sizeof(char) * partSize);
        MPI_Recv(temp1, partSize, MPI_CHAR, 0, 1, MPI_COMM_WORLD, &status);
        MPI_Recv(temp2, partSize, MPI_CHAR, 0, 2, MPI_COMM_WORLD, &status);
        mismatchCount = CountMismatchesInTwoStrings(temp1, temp2, partSize);
        free(temp1);
        free(temp2);
    }

    // Collect the result into main processor
    MPI_Reduce(&mismatchCount, &parallelResult, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    if (procRank == 0 && procNum > 1)
    {
        if (linearResult != parallelResult)
        {
            printf("Error! Results are not equal\n");
        }

        endTime = MPI_Wtime();
        parallelTime = endTime - startTime;
        printf("MPI time = %.3f\n", endTime - startTime);
        printf("MPI result = %d\n", parallelResult);
        free(str1);
        free(str2);
    }

    MPI_Finalize();

    return 0;
}
