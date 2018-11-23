#include "mpi.h"
#include <time.h>
#include <iostream>
#include "windows.h"

static const int n = 4;
static const int root = 0;
void waiter(int procSize);
void philosopher(int procRank);
int message;
MPI_Status status;

// mpiexec -n 6 Philosophers
void main() {

	int procRank, procSize;

	MPI_Init(nullptr,nullptr);
	MPI_Comm_size(MPI_COMM_WORLD, &procSize);
	MPI_Comm_rank(MPI_COMM_WORLD, &procRank);
	
	if (procRank == root) {
		waiter(procSize);
	}
	else {
		philosopher(procRank);
	}
	std::cout << procRank << " finished" << std::endl;
	MPI_Finalize();
}

void waiter(int procSize) {
	int repeat = 0;
	int *fork = new int[procSize - 1];
	MPI_Status status;

	for (int i = 0; i < procSize - 1; i++) {
		fork[i] = 1;
	}

	for(int repeat = 0; repeat < n * (procSize - 1);) {
		MPI_Recv(&message, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

		if (status.MPI_TAG == 0) { // 0 Хочет есть 
			if (fork[status.MPI_SOURCE - 1] == 1 && fork[status.MPI_SOURCE % (procSize-1)] == 1) {
				fork[status.MPI_SOURCE - 1] = 0;
				fork[status.MPI_SOURCE % (procSize - 1)] = 0;

				MPI_Send(&message, 1, MPI_INT, status.MPI_SOURCE, 1, MPI_COMM_WORLD); // 1 Разрешил
				std::cout << "Philosoph " << status.MPI_SOURCE << " got forks" << std::endl;
			}
			else {
				MPI_Send(&message, 1, MPI_INT, status.MPI_SOURCE, 2, MPI_COMM_WORLD); // 2 Запретил
				std::cout << "Philosoph " << status.MPI_SOURCE << " didn't get the fork" << std::endl;
			}
		}
		else if (status.MPI_TAG == 1) { // 1 Закончил есть 
			std::cout << "Philosoph " << status.MPI_SOURCE << "  finished eating" << std::endl;
			fork[status.MPI_SOURCE - 1] = 1;
			fork[status.MPI_SOURCE % (procSize - 1)] = 1;
			MPI_Send(&message, 1, MPI_INT, status.MPI_SOURCE, 0, MPI_COMM_WORLD);	// 0 Забрал вилки
			repeat++;
			
		}
		std::cout << "FORKS: ";
		for (int i = 0; i < procSize - 1; i++) {
			std::cout << fork[i] << " ";
		}
		std::cout << std::endl;
		std::cout << std::endl;
	}
}

void philosopher(int procRank) {
	for (int repeat = 0; repeat < n; repeat++) {
		std::cout << "Philosoph " << procRank << " hungry" << std::endl;
		do {
			MPI_Send(&message, 1, MPI_INT, root, 0, MPI_COMM_WORLD);  // 0 Хочет есть 
			MPI_Recv(&message, 1, MPI_INT, root, MPI_ANY_TAG, MPI_COMM_WORLD, &status); // Ожидает 1 (Разрешение)
			for (int i = 0; i < 100000; i++);
		} while (status.MPI_TAG == 2);

		std::cout << "Philosoph " << procRank << " started eating" << std::endl;
		for (int i = 0; i < 1000000; i++);
		MPI_Send(&message, 1, MPI_INT, root, 1, MPI_COMM_WORLD); // 1 Закончил есть 

		MPI_Recv(&message, 1, MPI_INT, root, 0, MPI_COMM_WORLD, &status); // 0 отдал вилки
		std::cout << "Philosoph " << procRank << " is thinking" << std::endl;
		for (int i = 0; i < 1000000; i++);
		std::cout << std::endl;
	}
}