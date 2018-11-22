#pragma once
#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include "Windows.h"


typedef struct Message_t {
	int command;
	int procRank;
} Message;

class Visitor {
	int procRank; 
	int queueRank; 
	int barberRank;
public:
	int numInQueue;
	Visitor(int procRank = -1, int queueRank = 1, int barberRank = 0) :
		procRank(procRank),
		queueRank(queueRank),
		barberRank(barberRank) {}

	void Start() {
		MPI_Status status;
		Message message;
		message.command = 1;
		message.procRank = procRank;
		printf("Visitor %i \n", procRank);
		while (-2 != message.command)
		{
			message.command = 1;
			MPI_Send(&message, sizeof(Message), MPI_CHAR, queueRank, 123, MPI_COMM_WORLD);
			MPI_Recv(&message, sizeof(Message), MPI_CHAR, queueRank, 123, MPI_COMM_WORLD, &status);
			switch (message.command) {
			case -2: {
				break;
			}
			case -1: {
				Sleep(1000);
				break;
			}
			default: {
				MPI_Send(this, sizeof(Visitor), MPI_CHAR, queueRank, 123, MPI_COMM_WORLD);
				MPI_Recv(this, sizeof(Visitor), MPI_CHAR, barberRank, 10, MPI_COMM_WORLD, &status);
				printf("Visitor %i was served\n", procRank);
				MPI_Send(&message, sizeof(Message), MPI_CHAR, barberRank, 10, MPI_COMM_WORLD);
			}
			}
		}

		printf("Visitor %i leaves\n", procRank);
	}
	void cutHair() {
		//cut hair
	}

	int getProcRank()
	{
		return procRank;
	}
};

class Queue {
	Visitor *visitorQueue;
	int len; 
	int currVisitors; 
	int maxVisitors; 
	int procRank; 
	int numOfWorkingProc; 
	int barberRank;
	bool barberSleep;

public:
	Queue(int procRank = 1, int len = 100, int barberRank = 0, int numOfWorkingProc = 2) {
			this->procRank = procRank;
			this->len = len;
			this->barberRank = barberRank;
			this->numOfWorkingProc = numOfWorkingProc - 2;
			visitorQueue = new Visitor[len];
			currVisitors = 0;
			maxVisitors = len / 2;
			barberSleep = false;
	}

	int Check() {
		if (0 == len)
			return -2; //end
		if (0 == currVisitors)
			return 1;//check barber
		if (currVisitors < maxVisitors)
			return 0;//get a sit
		else
			return -1;//go away
	}

	void start() {

		printf("started with %i processes and maxVisitors %i \n", numOfWorkingProc, maxVisitors);
		MPI_Status status;
		Message message;
		MPI_Recv(&message.command, 1, MPI_INT, barberRank, 1, MPI_COMM_WORLD, &status);
		MPI_Send(&message.command, 1, MPI_INT, barberRank, 1, MPI_COMM_WORLD);

		while ((numOfWorkingProc > 0) || (!barberSleep)) {
			MPI_Recv(&message, sizeof(Message), MPI_CHAR, MPI_ANY_SOURCE, 123, MPI_COMM_WORLD, &status);
			printf("get request from %i process  - command %i\n", message.procRank, message.command);
			switch (message.command) {
			case 1:
			{
				message.command = Check();
				if (-2 == message.command) {
					numOfWorkingProc--;
				}
				MPI_Send(&message, sizeof(Message), MPI_CHAR, message.procRank, 123, MPI_COMM_WORLD);
				if (message.command >= 0) {
					len--;
					currVisitors++;
					MPI_Recv(&visitorQueue[len], sizeof(Visitor), MPI_CHAR, message.procRank, 123, MPI_COMM_WORLD, &status);
					visitorQueue[len].numInQueue = len;
					printf("Placed the Visitor %i on the %i place\n", visitorQueue[len].getProcRank(), len);
					if ((1 == message.command) && (barberSleep)) {
						printf("going to wake up the barber\n");
						MPI_Send(&message.command, 1, MPI_INT, barberRank, 100, MPI_COMM_WORLD);
						MPI_Recv(&message.command, 1, MPI_INT, barberRank, 100, MPI_COMM_WORLD, &status);
						barberSleep = false;
					}
				}
				break;
			}

			case 2:
			{
				message.command = currVisitors;
				MPI_Send(&message, sizeof(Message), MPI_CHAR, barberRank, 1, MPI_COMM_WORLD);
				if (message.command > 0) {
					MPI_Send(&visitorQueue[len + currVisitors - 1], sizeof(Visitor), MPI_CHAR, barberRank, 2, MPI_COMM_WORLD);
					MPI_Recv(&message, sizeof(Message), MPI_CHAR, barberRank, 1, MPI_COMM_WORLD, &status);
					currVisitors--;
				}
				else
					barberSleep = true;
				break;
			}
			}
		}
		if (barberSleep) {
			printf("going to wake up the barber\n");
			message.command = -2;
			MPI_Send(&message.command, 1, MPI_INT, barberRank, 100, MPI_COMM_WORLD);
			MPI_Recv(&message.command, 1, MPI_INT, barberRank, 100, MPI_COMM_WORLD, &status);
			printf("the barber wokes up (%i)\n", message.command);
		}
		free(visitorQueue);
		printf("barber ended his work\n");
	}
};

class Barber {
	int queueRank, procRank;
public:
	Barber(int procRank = 0, int queueRank = 1) :
		procRank(procRank),
		queueRank(queueRank) {}

	void start() {
		Visitor currentVisitor;
		MPI_Status status;
		Message message;
		message.command = 2;
		message.procRank = procRank;

		MPI_Send(&message.command, 1, MPI_INT, queueRank, 1, MPI_COMM_WORLD);
		MPI_Recv(&message.command, 1, MPI_INT, queueRank, 1, MPI_COMM_WORLD, &status);
		while (message.command != -2) {
			message.command = 2;
			MPI_Send(&message, sizeof(Message), MPI_CHAR, queueRank, 123, MPI_COMM_WORLD);
			MPI_Recv(&message, sizeof(Message), MPI_CHAR, queueRank, 1, MPI_COMM_WORLD, &status);
			if (message.command > 0) {
				MPI_Recv(&currentVisitor, sizeof(Visitor), MPI_CHAR, queueRank, 2, MPI_COMM_WORLD, &status);

				MPI_Send(&message, sizeof(Message), MPI_CHAR, queueRank, 1, MPI_COMM_WORLD);

				printf("barber started with Visitor %i and with the place = *%i*\n", currentVisitor.getProcRank(), currentVisitor.numInQueue);
				currentVisitor.cutHair();
				MPI_Send(&currentVisitor, sizeof(Visitor), MPI_CHAR, currentVisitor.getProcRank(), 10, MPI_COMM_WORLD);
				MPI_Recv(&message, sizeof(Message), MPI_CHAR, currentVisitor.getProcRank(), 10, MPI_COMM_WORLD, &status);
				message.procRank = procRank;
			}
			else if (message.command != -2) {

				printf("barber went to sleep\n");
				MPI_Recv(&message.command, 1, MPI_INT, queueRank, 100, MPI_COMM_WORLD, &status);
				printf("barber woke up (%i)\n", message.command);
				MPI_Send(&message.command, 1, MPI_INT, queueRank, 100, MPI_COMM_WORLD);
			}
		}
		printf("barber ended his work\n");
	}
};