#include "barber.h"

void main(int argc, char *argv[]) {

	int procNum;
	int procRank;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &procNum);
	MPI_Comm_rank(MPI_COMM_WORLD, &procRank);

	switch (procRank) {
	case 0:
	{
		Barber barber(0, 1);
		barber.start();
		break;
	}
	case 1:
	{
		Queue queue(1, 4, 0, procNum);
		queue.start();
		break;
	}
	default:
		Visitor visitor(procRank, 1, 0);
		visitor.Start();
	}

	MPI_Finalize();
}