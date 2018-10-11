#include <iostream>
#include <mpi.h>
int main (int argc, char* argv[]){

	double tStart = 0, tEnd = 0;
	int errCode;
	int vecSize = 20;
	int procNum, procRank;
	double sum = 0, res = 0;
	MPI_Status status;

	errCode = MPI_Init(&argc,&argv);

	if(errCode!= 0){
		return errCode;
	}

	MPI_Comm_size(MPI_COMM_WORLD,&procNum);
    MPI_Comm_rank(MPI_COMM_WORLD,&procRank);
	
	if (procRank == 0){

		double *vec = new double[vecSize];
		double currSum;
		int step = vecSize / procNum;
		for (int i = 0; i < vecSize; i++)
		{
			vec[i] = rand()%100 - 50;
			if(vecSize<=20){
				std::cout<<vec[i]<<" ";
			}
		}

		tStart = MPI_Wtime();

		for(int i = 1; i < procNum; i++){
			MPI_Send(&step, 1, MPI_INT, i, 1, MPI_COMM_WORLD);
			MPI_Send(&vec[(i - 1) * step], step, MPI_DOUBLE, i, 2, MPI_COMM_WORLD);
		}

		//досчитываем хвост вектора
		for (int i = (procNum-1)*step; i < vecSize;  i++){
			sum += vec[i];  
		}

		for(int i = 1; i < procNum; i++){
			MPI_Recv(&currSum, 1, MPI_DOUBLE, i, 3, MPI_COMM_WORLD, &status);
			sum += currSum;
		}

		res = sum / vecSize;

		tEnd = MPI_Wtime();

		std::cout<<'\n';
		//std::cout<<"summ of elements = "<<sum;
		std::cout<<"arithmetic mean of the vector = "<<res;
		std::cout<<'\n';
		std::cout<<"spent time = "<<tEnd-tStart;
	}
	else{

		int step;
		double currSum = 0;

		MPI_Recv(&step, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);
        double* currVec = new double[step];

        MPI_Recv(currVec, step, MPI_DOUBLE, 0, 2, MPI_COMM_WORLD, &status);
		for (int i = 0; i < step; i++){
			currSum += currVec[i];
		}

		MPI_Send(&currSum, 1, MPI_DOUBLE, 0, 3, MPI_COMM_WORLD);

		delete[]currVec;
	}

	MPI_Finalize();
	return 0;
}


//cd/d
//mpiexec -n 5 lab1MPI.exe