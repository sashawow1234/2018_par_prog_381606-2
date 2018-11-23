
#include <iostream>
#include <mpi.h>
int main (int argc, char* argv[]){

	int errCode;
	
	errCode = MPI_Init(&argc,&argv);

	if(errCode!= 0){
		return errCode;
	}
	
	double *vec = NULL;
	int vecSize = 20;
	int procNum, procRank;
	double localSum = 0, sum = 0, res = 0;
	int step = 0;
	MPI_Status status;
	

	MPI_Comm_size(MPI_COMM_WORLD,&procNum);
    MPI_Comm_rank(MPI_COMM_WORLD,&procRank);
	
	if (procRank == 0){

		vec = new double[vecSize];
		step = vecSize / procNum;

		for (int i = 0; i < vecSize; i++){
			vec[i] = rand()%100 - 50;
			if(vecSize<=20){
				std::cout<<vec[i]<<" ";
			}
		}
	}


		MPI_Bcast(&step, 1, MPI_INT, 0, MPI_COMM_WORLD);
	
		int *sendCounts = new int[procNum];
		int *displs = new int[procNum];
		int balance = vecSize % procNum;
		int tmp = 0;

		for(int i = 0; i < procNum; i++){
			displs[i] = tmp;
			sendCounts[i] = step;
			if(balance != 0){
				sendCounts[i]++;
				balance--;
			}
			tmp += sendCounts[i];
		} 

		double *buf = new double[sendCounts[procRank]];
		MPI_Scatterv(vec, sendCounts, displs, MPI_DOUBLE, buf, sendCounts[procRank], MPI_DOUBLE, 0, MPI_COMM_WORLD);

		for(int i = 0; i < sendCounts[procRank]; i++){
			localSum += buf[i];
		}
		
		MPI_Reduce(&localSum, &sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
		
		res = sum / vecSize;

		if (procRank == 0){
		std::cout<<'\n';
		//std::cout<<"summ of elements = "<<sum<<'\n';
		std::cout<<"arithmetic mean of the vector = "<<res;
		}

		delete sendCounts;
		delete displs;
		delete buf;
		delete vec;

	MPI_Finalize();
	return 0;
}


//cd/d
//mpiexec -n 5 lab1MPI.exe