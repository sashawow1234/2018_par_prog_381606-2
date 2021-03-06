#include "pch.h"
#include <stdio.h>
#include <mpi.h>
#include <windows.h>
#include <time.h>


int main(int argc, char* argv[])
{
	int n = 1000;
	char symbol = 'A';
	char *arr = nullptr;
	int lin_count = 0, count = 0, GlobalCount = 0;

	//srand(time(NULL));

	int ProcNum, ProcRank;
	int *send_counts, *displacements;
	char *RecieveBuffer;
	int localBuff, reminder;

	double time, linstarttime, linendtime;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &ProcNum);
	MPI_Comm_rank(MPI_COMM_WORLD, &ProcRank);

	if (ProcRank == 0)
	{
		arr = new char[n];
		for (int i = 0; i < n; i++)
		{
			arr[i] = rand() % 5 + 65;
			printf("%c", arr[i]);
		}
		printf("\n");
		/*linstarttime = MPI_Wtime();
		for (int i = 0; i < n; i++)
			if (arr[i] == symbol) lin_count++;
		linendtime = MPI_Wtime();
		printf("frequency of %c = %f\n", symbol, (float)lin_count / (float)n);
		printf("work time of linuar algoritm = %f\n", linendtime - linstarttime);*/
	}

	time = MPI_Wtime();
	send_counts = new int[ProcNum];
	displacements = new int[ProcNum];
	localBuff = n / ProcNum;
	reminder = n % ProcNum;

	send_counts[0] = localBuff + reminder;
	displacements[0] = 0;
	for (int i = 1; i < ProcNum; ++i)
	{
		send_counts[i] = localBuff;
		displacements[i] = reminder + i * localBuff;
	}

	RecieveBuffer = new char[send_counts[ProcRank]];

	MPI_Scatterv(arr, send_counts, displacements, MPI_CHAR, RecieveBuffer, send_counts[ProcRank], MPI_INT, 0, MPI_COMM_WORLD);

	count = RecieveBuffer[0];
	for (int i = 0; i < send_counts[ProcRank]; ++i)
		if (symbol == RecieveBuffer[i])
			count++;

	MPI_Reduce(&count, &GlobalCount, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);  

	if (ProcRank == 0)
	{
		time = MPI_Wtime() - time;
		printf("Frequency of %c = %f\n", symbol, (float)GlobalCount / (float)n);
		printf("Work time of parallel algoritm = %f\n", time);
		delete arr;
	}

	MPI_Finalize();

	delete[] send_counts;
	delete[] displacements;
	delete[] RecieveBuffer;

	return 0;
}