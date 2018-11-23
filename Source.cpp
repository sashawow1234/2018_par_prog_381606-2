#include "mpi.h"
#include <iostream>
#include <ctime>
#include <Windows.h>
#include <algorithm>

const int SERVER = 0;
const int READ_REQUEST = 1;
const int FINISH_READ = 2;
const int WRITE_REQUEST = 3;
const int REQUEST = 4;
const int QUEUE_REQUEST = 5;
using namespace std;


void main(int argc, char **argv)
{
	int rank, size, data = 1, Request = -2, WritersCount = 3, index = 0, ReadersCount = 0, ReadyToRecieve = 1, OnResponse = 0, inQueue = 0;;
	time_t t;
	MPI_Status mpiStatus;
	MPI_Request mpiRequest;

	MPI_Init(NULL, NULL);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	//The first process is a server that will serve requests from writers and readers.
	if (rank == 0) {

		cout << "\n" << "The server has been started" << endl;
		cout << "Writers count = " << WritersCount << endl;

		while (true) { 
			if (ReadyToRecieve)
			{
				MPI_Irecv(&Request, 1, MPI_INT, MPI_ANY_SOURCE, REQUEST, MPI_COMM_WORLD, &mpiRequest);
				ReadyToRecieve = 0;
			}
			if (!ReadyToRecieve)
			{
				MPI_Test(&mpiRequest, &index, &mpiStatus);

				if ((index) && (Request == WRITE_REQUEST))
				{
					if (!ReadersCount)
					{
						OnResponse = 1;
						MPI_Send(&OnResponse, 1, MPI_INT, mpiStatus.MPI_SOURCE, SERVER, MPI_COMM_WORLD);
						cout << "\n" << "************WORKS WRITER************" << endl;
						cout << "Process " << mpiStatus.MPI_SOURCE << " is writing" << endl;
						MPI_Recv(&data, 1, MPI_INT, MPI_ANY_SOURCE, mpiStatus.MPI_SOURCE, MPI_COMM_WORLD, &mpiStatus);
						cout << "data = " << data << endl;
						ReadyToRecieve = 1;
					}
					else
					{
						OnResponse = 0;
						MPI_Send(&OnResponse, 1, MPI_INT, mpiStatus.MPI_SOURCE, SERVER, MPI_COMM_WORLD);
						cout << "\n" << "************WORKS WRITER************" << endl;
						cout << "Process " << mpiStatus.MPI_SOURCE << " : ACCESS DENIED" << endl;
						ReadyToRecieve = 1;
					}
				}
				if ((index) && (Request == READ_REQUEST)) 
				{
					cout << "\n" << "************************************" << endl;
					cout << "Process " << mpiStatus.MPI_SOURCE << " reading..." << endl;
					ReadersCount++;
					MPI_Isend(&data, 1, MPI_INT, mpiStatus.MPI_SOURCE, READ_REQUEST, MPI_COMM_WORLD, &Request);
					cout << "Server sent data to " << mpiStatus.MPI_SOURCE << ", data = " << data << endl;
					cout << "Current readers count: " << ReadersCount << endl;
					ReadyToRecieve = 1;
				}
				if ((index) && (Request == FINISH_READ))
				{
					ReadersCount--;
					cout << "\n" << "************************************" << endl;
					cout << "Process " << mpiStatus.MPI_SOURCE << " finishes reading " << endl;
					cout << "Current readers count: " << ReadersCount << endl;
					ReadyToRecieve = 1;
				}
			}
		}
	}

	//first process are writers
	if (rank > 0 && rank <= WritersCount)
	{
		Request = WRITE_REQUEST;
		srand((unsigned)time(&t));
		data = rank;
		while (true)
		{
			if (rand() % 100 < 8)
			{
				MPI_Send(&Request, 1, MPI_INT, SERVER, REQUEST, MPI_COMM_WORLD);
				MPI_Recv(&OnResponse, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &mpiStatus);
				if (OnResponse)
				{
					MPI_Send(&data, 1, MPI_INT, SERVER, rank, MPI_COMM_WORLD);
				}
				Sleep(15000 + 1000 * rank);
			}
		}
	}

	//readers remaining process
	if (rank > WritersCount)
	{
		while (true)
		{
			if (rand() % 100 < 10)
			{
				Request = READ_REQUEST;
				MPI_Send(&Request, 1, MPI_INT, SERVER, REQUEST, MPI_COMM_WORLD);
				MPI_Recv(&data, 1, MPI_INT, SERVER, READ_REQUEST, MPI_COMM_WORLD, &mpiStatus);
				Sleep(500 * rank);
				Request = FINISH_READ;
				MPI_Send(&Request, 1, MPI_INT, SERVER, REQUEST, MPI_COMM_WORLD);
				Sleep(10000 + 800 * rank);
			}
		}
	}

	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Finalize();
}
