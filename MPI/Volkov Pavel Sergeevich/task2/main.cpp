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

// USING OPENCVV!!!!!!!!!!!!!


using namespace cv;
using namespace std;


void to_buf(int *buf, Mat mat)
{

	int length = mat.cols * mat.rows;
	
	buf = new int[length];

	int counter = 0;

	for (int i = 0; i < mat.rows; i++)
		for (int j = 0; j < mat.cols; j++)
		{
			buf[counter] = mat.at<cv::Vec3b>(i, j)[0];
			counter++;
		}

}
string name;

Mat image_base;


int *buffer;
int *wh;

int main(int argc, char **argv)
{

	int * buffer_2;
	Mat image;
	int worksize;
	int taskid;
	int numtasks;
	MPI_Status Stat;
	int st_raw_cout;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &taskid);
	MPI_Comm_size(MPI_COMM_WORLD, &numtasks);

	if (numtasks == 1)
	{
		cout << "Please, get more process" << "\n";
		return 0;

	}


	wh = new int[2];
	if (taskid == 0)
	{

	

		cout << "Enter the way or name of image: ";
	
		cin >> name;

		image = imread(name, IMREAD_COLOR);

		if (image.empty())                      // Check for invalid input
		{

			while (image.empty())
			cout << "Could not open or find the image\n Try again: " << std::endl;

			cin >> name;

			image = imread(name, IMREAD_COLOR);
		}


		buffer = new int[image.rows * image.cols];

		image_base = image.clone();



		int counter = 0;

		for (int i = 0; i < image.rows; i++)
			for (int j = 0; j < image.cols; j++)
			{
				buffer[counter] = image.at<cv::Vec3b>(i, j)[0] * 0.11 + image.at<cv::Vec3b>(i, j)[1] * 0.59 + image.at<cv::Vec3b>(i, j)[2] * 0.30;
				image_base.at<cv::Vec3b>(i, j)[0] = image.at<cv::Vec3b>(i, j)[0] * 0.11 + image.at<cv::Vec3b>(i, j)[1] * 0.59 + image.at<cv::Vec3b>(i, j)[2] * 0.30;
				image_base.at<cv::Vec3b>(i, j)[1] = image.at<cv::Vec3b>(i, j)[0] * 0.11 + image.at<cv::Vec3b>(i, j)[1] * 0.59 + image.at<cv::Vec3b>(i, j)[2] * 0.30;
				image_base.at<cv::Vec3b>(i, j)[2] = image.at<cv::Vec3b>(i, j)[0] * 0.11 + image.at<cv::Vec3b>(i, j)[1] * 0.59 + image.at<cv::Vec3b>(i, j)[2] * 0.30;
				counter++;
			}






		wh[0] = image.cols;
		wh[1] = image.rows;


		//cout << wh[0] <<  "\n";
	
		for (int i = 1; i < numtasks; i++)
			MPI_Send(&wh[0], 2, MPI_INT, i, 0, MPI_COMM_WORLD);
		
		st_raw_cout = wh[1] / (numtasks - 1);


		//cout << " st_raw_cout * wh[0] " << st_raw_cout * wh[0] << "\n";

	//	cout << "sta = " << st_raw_cout << "\n";
		for (int i = 1; i < numtasks - 1; i++)
		{
			MPI_Send(&buffer[st_raw_cout * (i - 1)* wh[0]], st_raw_cout * wh[0], MPI_INT, i, 1, MPI_COMM_WORLD);
		}

	//	cout << "Last " << wh[1] - st_raw_cout * (numtasks - 2) << "\n";
	//	cout << "last count " << (wh[1] - st_raw_cout * (numtasks - 2)) * wh[0] << "\n";

		MPI_Send(&buffer[image.cols * image.rows - (wh[1] - st_raw_cout * (numtasks - 2)) * wh[0]], (wh[1] - st_raw_cout * (numtasks - 2)) * wh[0], MPI_INT, numtasks - 1, 1, MPI_COMM_WORLD);

	}

	else
	{
		int min = 255, max = 0;
		int gist[256];
		for (int i = 0; i < 256; i++)
		{
			gist[i] = 0;
		}
	

		//All
//		cout  << taskid << "\n";
		int  *wh_2 = new int[2];
		MPI_Recv(wh_2, 2, MPI_INT, 0, 0, MPI_COMM_WORLD, &Stat);
	//	cout << "WH[0] = " << wh_2[0] << "\n";
	//	cout << "WH[1] = " << wh_2[1] << "\n";
		
		int st_raw_cout_2 = wh_2[1] / (numtasks - 1);

	//	cout << "st_raw_cout_2 " << st_raw_cout_2 << '\n';

		buffer_2 = new int[st_raw_cout_2 * wh_2[0]];



		if (taskid != numtasks - 1)
		{
			MPI_Recv(buffer_2, st_raw_cout_2 * wh_2[0], MPI_INT, 0, 1, MPI_COMM_WORLD, &Stat);

			worksize = st_raw_cout_2 * wh_2[0];

			for (int i = 0; i < st_raw_cout_2 * wh_2[0]; i++)
			{
				if (min > buffer_2[i]) min = buffer_2[i];
				if (max < buffer_2[i]) max = buffer_2[i];
			}


		}


		if (taskid == numtasks - 1)
		{
			buffer_2 = new int[(wh_2[1] - st_raw_cout_2 * (numtasks - 2)) * wh_2[0]];
			MPI_Recv(buffer_2, (wh_2[1] - st_raw_cout_2 * (numtasks - 2)) * wh_2[0], MPI_INT, 0, 1, MPI_COMM_WORLD, &Stat);


			worksize = (wh_2[1] - st_raw_cout_2 * (numtasks - 2)) * wh_2[0];

			for (int i = 0; i <worksize; i++)
			{
				if (min > buffer_2[i]) min = buffer_2[i];
				if (max < buffer_2[i]) max = buffer_2[i];
			}

		
		}

	//	cout << "min = " << min << "\n" << "max = " << max << "\n";;
		
		int minmax[2];

		minmax[0] = min;
		minmax[1] = max;

		MPI_Send(&minmax[0], 2, MPI_INT, 0, 2, MPI_COMM_WORLD);
	}

	if (taskid == 0)
	{
		int true_min = 255, true_max = 0;

		int rec_buf[2];

		for (int i = 1; i < numtasks; i++)
		{
			MPI_Recv(rec_buf, 2, MPI_INT, i, 2, MPI_COMM_WORLD, &Stat);
			if (true_min > rec_buf[0]) true_min = rec_buf[0];
			if (true_max < rec_buf[1]) true_max = rec_buf[1];
		}
		cout << "true min/max " << true_min << "//" << true_max << "\n";
	
		int tminmax[2];
		tminmax[0] = true_min;
		tminmax[1] = true_max;


		for (int i = 1; i < numtasks; i++)
		{
			MPI_Send(&tminmax[0], 2, MPI_INT, i, 3, MPI_COMM_WORLD);
		}

	}
	else
	{


		int minmax[2];
		MPI_Recv(minmax, 2, MPI_INT, 0, 3, MPI_COMM_WORLD, &Stat);
	//	cout << "myrank " << taskid << " tr" << minmax[0] << " " << minmax[1] << "\n";

		double coef = 255.0 / double(minmax[1] - minmax[0]);

		for (int i = 0; i < worksize; i++)
		{
			buffer_2[i] = (buffer_2[i] - minmax[0]) * coef;
			if (buffer_2[i] > 255) buffer_2[i] = 255;
			if (buffer_2[i] < 0) buffer_2[i] = 0;
			//cout << buffer_2[i] << " ";
		}



		
			MPI_Send(&buffer_2[0], worksize, MPI_INT, 0, 4, MPI_COMM_WORLD);
		


	}



	if (taskid == 0)
	{
		
		int *last_buf = new int[st_raw_cout * wh[0]];

		Mat out = image.clone();
		for (int i = 1; i < numtasks - 1; i++)
		{
			MPI_Recv(last_buf, st_raw_cout * wh[0], MPI_INT, i, 4, MPI_COMM_WORLD, &Stat);

			for (int j = 0; j < st_raw_cout * wh[0]; j++)
			{
				int raw = st_raw_cout * (i - 1) + j / wh[0];
				int col = j % wh[0];
				out.at<cv::Vec3b>(raw, col)[0] = last_buf[j];
				out.at<cv::Vec3b>(raw, col)[1] = last_buf[j];
				out.at<cv::Vec3b>(raw, col)[2] = last_buf[j];
				
			}


		}

		
		
		//cout << "Last " << (wh[1] - st_raw_cout * (numtasks - 2)) * wh[0];

		last_buf = new int[(wh[1] - st_raw_cout * (numtasks - 2)) * wh[0]];
		MPI_Recv(last_buf, (wh[1] - st_raw_cout * (numtasks - 2)) * wh[0], MPI_INT, numtasks - 1, 4, MPI_COMM_WORLD, &Stat);
		
		
		for (int j = 0; j < (wh[1] - st_raw_cout * (numtasks - 2)) * wh[0]; j++)
		{


			int raw = st_raw_cout * (numtasks - 2) + j / wh[0];
			int col = j % wh[0];
		//	cout << "r " << raw << " l " << col << "\n";
			out.at<cv::Vec3b>(raw, col)[0] = last_buf[j];
			out.at<cv::Vec3b>(raw, col)[1] = last_buf[j];
			out.at<cv::Vec3b>(raw, col)[2] = last_buf[j];

		}


		imwrite("Gray_Image_contarst_" + name, out);
		imwrite("Gray_Image_Base_" + name, image_base);

	}

	// -- ALL


	MPI_Finalize();

	return 0;
}