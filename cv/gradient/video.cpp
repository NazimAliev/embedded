//
// Copyright (c) by Nazim Aliev
// All rights reserved.
//
// nazim.ru@gmail.com
//

/*

video.cpp

find concentric circles on video

Mat[row,column]
0/0---column--->
 |
row
 |
 v

Point(x,y)
0/0---X--->
 |
 Y
 |
 v
Point(x,y) <--> at(row,col)
*/
#include <stdio.h>
#include <strings.h>
#include <assert.h>
#include "opencv2/opencv.hpp"

//#define TEST

// accumulator is smaller than frame to put crossed lines in one pixel
// cross points not accure focused in one point!
#define ACC_SCALE (2)
#define RMAX (256)

using namespace cv;

void uav_accumCount(const Mat& img, const Mat& grad_x, const Mat& grad_y, Mat& acc, Mat& tmp_acc, Mat& tmp);
void uav_circleFilter(const Mat& img, const Mat& grad_x, const Mat& grad_y, Point max_loc, vector<Point>& outPoints, Mat& tmp);
void uav_radiusFilter(vector<Point>& inPoints, Point max_loc, int* hist);
void uav_drawHist(Mat& img, int* hist);
void uav_msize(const char*str, Mat m);

int main(int, char**)
{

#ifdef TEST
	VideoCapture cap("/data/bublik.png"); //test: open image 
#else
	VideoCapture cap(0); // open the default camera
#endif
	if(!cap.isOpened())  // check if we succeeded
		return -1;
	int w = cap.get(CV_CAP_PROP_FRAME_WIDTH);
	int h = cap.get(CV_CAP_PROP_FRAME_HEIGHT);
	//w = 200;
	//h = 100;
	int xroi;
	int yroi;
	xroi = 0;
	yroi = 0;
	Rect roi(xroi, yroi, w, h);
	printf("Video size %dx%d\n", w, h);
	printf("Roi size %dx%d\n", xroi, yroi);

	// tmp and acc must be same type, or runtime mat error will erase
	// after acc + tmp
	Mat acc = Mat::zeros(Size(w/ACC_SCALE,h/ACC_SCALE), CV_8UC1);
	Mat tmp_acc = Mat::zeros(Size(w/ACC_SCALE,h/ACC_SCALE), CV_8UC1);
	Mat tmp = Mat::zeros(Size(w,h), CV_8UC1);
	vector<Point> cPoints;
	int hist[RMAX];

	Mat roi_bw;
	Mat roi_gray;
	Mat histWindow(200,400, CV_8UC3);
	Mat grad_x, grad_y;
	Mat grad2_x, grad2_y;
	Mat subFrame;
	// min and max value of frame
	double min, max;
	// coordinates of min and max
	Point min_loc, max_loc;

	namedWindow("Image",1);
	namedWindow("Accum",1);
	namedWindow("Hist",1);
#ifndef TEST
	for(;;)
#endif
	{
		// get frame
		Mat frame;
		cap >> frame; // get a new frame from camera
		//msize("Frame", frame);
		//subFrame = frame(Range(0, h), Range(0, w));
		subFrame = frame(roi);
		cvtColor(subFrame, roi_gray, COLOR_BGR2GRAY);
		threshold(roi_gray,roi_bw, 150, 255, THRESH_BINARY);
		//GaussianBlur(edges, edges, Size(3,3), 1.5, 1.5);
		// 1st sobel for bw image - find edges and then find gradient only for edges
		// to reduce CPU overload when gradients will be calculated
		Sobel(roi_bw, grad_x, CV_64F, 1, 0, 3);
		Sobel(roi_bw, grad_y, CV_64F, 0, 1, 3);
		// accuracy gradiend based on grayscale image 
		Sobel(roi_gray, grad2_x, CV_64F, 1, 0, 3);
		Sobel(roi_gray, grad2_y, CV_64F, 0, 1, 3);
		for(int y = 0; y < roi_bw.rows; y++)
		{
			for(int x = 0; x < roi_bw.cols; x++)
			{
				double gx = grad_x.at<double>(y,x);
				double gy = grad_y.at<double>(y,x);
				if(hypot(gx,gy) < 100)
				{
					// clear non-edges gradients
					grad2_x.at<double>(y,x) = 0;
					grad2_y.at<double>(y,x) = 0;
				}
			}
		}
		// ================================================
		// find gradient
		uav_accumCount(roi_gray, grad2_x, grad2_y, acc, tmp_acc, tmp);
		minMaxLoc(acc, &min, &max, &min_loc, &max_loc);
		// circle filter
		uav_circleFilter(roi_gray, grad2_x, grad2_y, max_loc, cPoints, tmp);
		uav_radiusFilter(cPoints, max_loc, hist);
		// ================================================
		//printf("Max %f[row%d,col=%d]\n", max, max_loc.y, max_loc.x);
		cvtColor(roi_bw, roi_bw, COLOR_GRAY2BGR);
		uav_drawHist(histWindow, hist);
		circle(roi_bw, Point(max_loc.x*ACC_SCALE,max_loc.y*ACC_SCALE), 5, Scalar(100,200,100), 2); 
		imshow("Image", roi_bw);
		imshow("Accum", acc);
		imshow("Hist", histWindow);
#ifdef TEST
		waitKey(0);
#else
		if(waitKey(30) >= 0) break;
#endif
	}
	// the camera will be deinitialized automatically in VideoCapture destructor
	return 0;
}

void uav_accumCount(const Mat& img, const Mat& grad_x, const Mat& grad_y, Mat& acc, Mat& tmp_acc, Mat& tmp)
{
	// current use in cycles
	int row, col;
	// current gradient
	int gx, gy;
	// to draw lines
	Point pt1, pt2;
	double min, max;
	Point  minloc, maxloc;
	// empty accumulator to calc cross points
	acc = Scalar(0,0,0);
	tmp_acc = Scalar(0,0,0);
	for(int y = 0; y < img.rows; y+=5)
	{
		row = y;
		for(int x = 0; x < img.cols; x+=5)
		{
			col = x;
			// gradient for point Point(x,y) or Mat(col,row) 
			gx = (int)grad_x.at<double>(row,col)/10;
			gy = (int)grad_y.at<double>(row,col)/10;
			//printf("[%d,%d]\n", gx, gy);
			// don't show small gradients
			if(hypot(gx,gy) > 10)
			{
				pt1.x = x/ACC_SCALE;
				pt1.y = y/ACC_SCALE;
				pt2 = pt1 + Point(gx,gy);
				//printf("[%d,%d] + [%f,%f] = [%d,%d]\n", x, y,gx,gy,pt2.x,pt2.y);
				//arrowedLine(tmp, Point(x,y), Point(x,y) + Point(gx,gy), Scalar(100, 100, 100), 1, 8, 0, 0.1);
				line(tmp_acc, pt1, pt2, Scalar(1, 1, 1), 5);
				// accumulate cross points
				acc = acc + tmp_acc;
				minMaxLoc(acc, &min, &max, &minloc, &maxloc);
				//FIXME: should be better method to avoid overflow
				if(max > 127)
					acc /= 2;
				tmp_acc = Scalar(0,0,0);
			}
		}
	}
}

void uav_circleFilter(const Mat& img, const Mat& grad_x, const Mat& grad_y, Point max_loc, vector<Point>& outPoints, Mat& tmp)
{
	// current use in cycles
	int row, col;
	// current gradient
	int gx, gy;
	// point diff for atan2 calc
	int dx, dy;
	float angle_grad, angle_center, angle_diff;
	float R;
	tmp = Scalar(0,0,0);
	for(int y = 0; y < img.rows; y++)
	{
		row = y;
		for(int x = 0; x < img.cols; x++)
		{
			col = x;
			// gradient for point Point(x,y) or Mat(col,row) 
			gx = (int)grad_x.at<double>(row,col)/10;
			gy = (int)grad_y.at<double>(row,col)/10;
			R = hypot(gx,gy);
			// filter points low gradient magnitude
			if(R > 10)
			{
				// TODO: convert R to hist index and +1
				dx = x - max_loc.x*ACC_SCALE;
				dy = y - max_loc.y*ACC_SCALE;
				//printf("[%d %d]\n", dx, dy);
				angle_center = atan2f(dy,dx);
				angle_grad = atan2f(gy,gx);
				angle_diff = 180.0*abs(angle_center - angle_grad)/M_PI;
				// filter points if direction to circle center and gradient angle is differ
				if(angle_diff < 5 || angle_diff > 175)
				{
					tmp.at<uchar>(row, col) = 255;
					// filtered points: in ideal case, only cicrles
					outPoints.push_back(Point(x,y));
				}
				//else
				//	printf("[%.0f %.0f] %f ", 180.0*angle_center/M_PI,180.0*angle_grad/M_PI, R);
			}
		}
	}
}

//TODO: need hough alg to accuracy center define
void uav_radiusFilter(vector<Point>& inPoints, Point max_loc, int* hist)
{
	Point p, p2, dp;
	float r;
	float rmax = 0;
	unsigned int i;
	memset(hist, 0, RMAX*sizeof(int));
	vector<float> tmp;
	// find max distance from points to center
	for(i=0; i<inPoints.size(); i++)
	{
    	p = inPoints[i];
		p2.x = max_loc.x*ACC_SCALE;
		p2.y = max_loc.y*ACC_SCALE;
		dp = p - p2;
		r = hypot(dp.x,dp.y);
		if(r > rmax)
			rmax = r;
		tmp.push_back(r);
	}
	// normalize
	int hist_idx;
	for(i=0; i<tmp.size(); i++)
	{
    	r = tmp[i];
		hist_idx = (int)(r * (RMAX-1))/rmax;
		// build hist 
		assert(hist_idx < RMAX);
		hist[hist_idx]++;
		inPoints.pop_back();
	}
	// plot hist
	/*
	for(i=0; i<RMAX; i++)
	{
		printf("%d ", hist[i]);
	}
	printf("\n");
	*/
}

// draw hist on img
// RMAX: number of bars (hist size)
void uav_drawHist(Mat& img, int* hist)
{
	const int w = 4;
	Point pt1, pt2;
	pt1.y = 200;
	img = Scalar(0,0,0);
	for(int i=0; i<RMAX; i++)
	{
		pt1.x = i*w;
		pt2.x = i*w;
		pt2.y = 200 - hist[i]*RMAX/256;
		line(img, pt1, pt2, Scalar(1, 80, 1), w);
	}
}

// helper func: return Mat size
void msize(const char*str, Mat m)
{
	Size s = m.size();
	int rows = s.height;
	int cols = s.width;
	printf("%s rows=%d cols=%d\n", str, rows, cols);
}

