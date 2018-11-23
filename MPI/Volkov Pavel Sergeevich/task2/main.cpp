#include <opencv2\opencv.hpp>
#include <mpi.h>
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#include "CImg-2.4.1\CImg.h"
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <string>




#define N 100

using namespace cv;
using namespace std;


int main(int argc, char **argv)
{

	int taskid;
	int numtasks;

	int wh[2];

	int *displacements;
	int *part_count;

	uchar *image_buffer;

	Mat image;
	Mat greyMat;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &taskid);
	MPI_Comm_size(MPI_COMM_WORLD, &numtasks);

	/*
		//cout << "Task_count = " << numtasks << "\n";
		int buf[N];

		
		
			int standart_part = N / numtasks;

			int *displacements;
			int *part_count;
			part_count = new int[numtasks];
			displacements = new int[numtasks];


			for (int i = 0; i < numtasks - 1; i++)
			{
				part_count[i] = standart_part;
				displacements[i] = standart_part * i;
			}
			part_count[numtasks - 1] = N - standart_part * (numtasks - 1);
			displacements[numtasks - 1] = (numtasks - 1) * standart_part;


		
			if (taskid == 0)
			{
				for (int i = 0; i < N; i++)
				{
					buf[i] = i;
				}

				for (int i = 0; i < numtasks; i++)
				{
					cout << "PT_COUNT " << part_count[i] << " DISP[i] " << displacements[i] << "\n";
				}

				
			}



			int *rec_buf = new int[part_count[taskid]];



		MPI_Barrier(MPI_COMM_WORLD);


	
		MPI_Scatterv(&buf[0], part_count, displacements, MPI_INT, rec_buf, part_count[taskid], MPI_INT, 0, MPI_COMM_WORLD);


		string out;

		for (int i = 0; i < part_count[taskid]; i++)
		{
			out += std::to_string(rec_buf[i]) + " ";
		}
		cout << out << "\n";



		MPI_Barrier(MPI_COMM_WORLD);



		*/




	//---------------------------------------- Get Image and bcast parametrs widht and heig --------------------/

	if (taskid == 0)
	{
		

		cout << "Enter the way or name of image: ";

		string name;

		cin >> name;

		image = imread(name, IMREAD_COLOR);

		if (image.empty())                      // Check for invalid input
		{

			while (image.empty())
			{
				cout << "Could not open or find the image\n Try again: ";

				cin >> name;

				image = imread(name, IMREAD_COLOR);
			}
		}

		
		cv::cvtColor(image, greyMat, CV_BGR2GRAY);
		imwrite("Gray_Image_BASE_NEW.jpg", greyMat);
		wh[0] = image.cols;
		wh[1] = image.rows;

		image_buffer = new uchar[wh[0] * wh[1]];

		int counter = 0;



		for (int i = 0; i < image.rows; i++)
			for (int j = 0; j < image.cols; j++)
			{
				//image_buffer[counter] = image.at<cv::Vec3b>(i, j)[0] * 0.11 + image.at<cv::Vec3b>(i, j)[1] * 0.59 + image.at<cv::Vec3b>(i, j)[2] * 0.30;
				image_buffer[counter] = greyMat.at<uchar>(i, j);
				counter++;
			}



	}




	// All proc getting widh and hei..
	MPI_Bcast(&wh, 2, MPI_INT, 0, MPI_COMM_WORLD);
	//cout << "wh[0] " << wh[0] << " ";
	

	

	//All process calculate data for scatterv

	int standart_part = wh[0] * wh[1] / numtasks;

	part_count = new int[numtasks];
	displacements = new int[numtasks];

	for (int i = 0; i < numtasks - 1; i++)
	{
		part_count[i] = standart_part;
		displacements[i] = standart_part * i;
	}
	part_count[numtasks - 1] = wh[0] * wh[1] - standart_part * (numtasks - 1);
	displacements[numtasks - 1] = (numtasks - 1) * standart_part;


	//All proc knows self parameters for scatterv




	uchar *rec_buf = new uchar[part_count[taskid]];


	MPI_Scatterv(&image_buffer[0], part_count, displacements, MPI_UNSIGNED_CHAR, rec_buf, part_count[taskid], MPI_INT, 0, MPI_COMM_WORLD);


	int min = 256;
	int max = 0;

	int _min;
	int _max;

	//cout << "id = " << taskid << " part_count = " << part_count[taskid] << "\n";

	// find min and max intens
	for (int i = 0; i < part_count[taskid]; i++)
	{
		if (rec_buf[i] > max) max = rec_buf[i];
		if (rec_buf[i] < min) min = rec_buf[i];
	}

	//cout << "Local min/max = " << min << "/" << max << "\n";

	// Mina and max reduce
	MPI_Allreduce(&min, &_min, 1, MPI_INT, MPI_MIN, MPI_COMM_WORLD);
	MPI_Allreduce(&max, &_max, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);
	


	//Minimal contrast correction is 5%
	if (_min < 12) _min = 12;
	if (_max > 244) _max = 244;

	//cout << "All min/max = " << _min << "/" << _max << "\n";


	// correction

	if (_max - _min == 0)
	{
		cout << "Image is uniform already!\n";
		MPI_Finalize();
	}

	double coef = 255.0 / double(_max - _min);

	for (int i = 0; i < part_count[taskid]; i++)
	{	
		int rez;
		rez = ((int)rec_buf[i] - (int)_min) * coef;
		
		rec_buf[i] = rez;

		if (rez > 255) rec_buf[i] = 255;
		if (rez < 0) rec_buf[i] = 0;
		
	}

	uchar *out_image_vector;

	if (taskid == 0)
	{
		out_image_vector = new uchar[wh[0] * wh[1]];
	}



	MPI_Gatherv(&rec_buf[0], part_count[taskid], MPI_UNSIGNED_CHAR, &out_image_vector[0], part_count, displacements, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);


	if (taskid == 0)
	{
		Mat out;
		out = greyMat.clone();

		int row;
		int col;

		for (int i = 0; i < wh[0] * wh[1]; i++)
		{
			row = i / wh[0];
			col = i % wh[0];
			out.at<uchar>(row, col) = out_image_vector[i];

		}
		
		imwrite("Contrast_image.jpg", out);
		//delete out_image_vector;
		//delete image_buffer;

	}

//	delete part_count;
//	delete displacements;


	MPI_Finalize();

}