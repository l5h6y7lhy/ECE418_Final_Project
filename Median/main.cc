#include <stdlib.h>
#include <ctime>
#include <iostream>
#include <algorithm>
#include <cmath>
#include "image.h"
using namespace std;

#define LOW_EXTREME 0
#define HIGH_EXTREME 255 //These are the two cases for damaged pixels
#define MASK_SIZE 40 //This is the size of the mask on which we are going to take
//median for each damaged pixel

//Todo: Put all pixel values in the mask into a vector
//Input: a -- a reference to the image we are trying to restore
//       b -- a refernece to the vector that stores all pixel values
//       x1 -- the horizontal coordinate of the damaged pixel
//       y1 -- the vertical coordinate of the damaged pixel
//Return: None
void put_data(Image& a, vector<int> & b, int x1, int y1) {
//Go through all pixels in the mask area
	for(int y=-MASK_SIZE/2; y<MASK_SIZE/2+1; y++) {
		for(int x=-MASK_SIZE/2; x<MASK_SIZE/2+1; x++) {
			b.push_back(a.Pixel(x1+x,y1+y));
		}
	}
	return;
}

//The main function of the median filtering method. It will apply median
//filtering method to restore images and save the result in another PNG file.
//There are six inputs: The first is the file of the input image
//The second is the file of the output image
//The third and fourth inputs are the horizontal and vertical coordinates of the
//top left corner of the operating area.
//The fifth and sixth inputs are the horizontal and vertical coordinates of the
//bottom right corner of the operating area.
int main (int argc, char* argv[])
{
	int x,y;  //Some indexing variables
	int sx,sy;
	int ex,ey; //The coordinates of feature points
	vector<int> b; //A vector that stores all pixel values in the mask
	double MSE=0.0;

	// verify arguments' correctness
	if (argc != 7)
	{
		cerr << "Useage: " << argv[0]
		     << " input.png output.png"
				 << " coordinates of the top left corner"
				 << " coordinates of the bottom right corner" << endl;
		return 1;
	}

	// load the input image
	Image image;
	Image original;
	image.LoadPng (argv[1]);

	if(image.Height()==512) {
		original.LoadPng ("lena.png");
	}
	else {
		original.LoadPng ("cameraman_original.png");
	}

//Read in the coordinates of some feature points in the area that we are going to
//search for damaged pixles
	sx = atoi(argv[3]);
	sy = atoi(argv[4]);
	ex = atoi(argv[5]);
	ey = atoi(argv[6]);

	clock_t begin = clock(); //The place where the real algorithm starts

//Go through each pixel in the searching area
	for(y = sy; y < ey; y++) {
		for(x = sx; x < ex; x++) {
			if(image.Pixel(x,y)==LOW_EXTREME || image.Pixel(x,y)==HIGH_EXTREME) {
//Put all pixel values in the mask into the vector b
				put_data(image, b, x, y);
//Order the pixel values
				sort(b.begin(),b.end());
//Set the damaged pixel to the median pixel value in the mask
				image.Pixel(x,y) = b[(b.size()-1)/2];
//Empty the vector for the next round
				b.resize(0);
			}
		}
	}

//The place where the real algorithm ends
	clock_t end = clock();
	double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
	cout<<"Run-time in seconds: "<<elapsed_secs<<endl;

	// save the image to PNG format
	image.SavePng (argv[2]);

//Calculate and print out MSE and PSNR values
	for(y=0; y<image.Height(); y++) {
		for(x=0; x<image.Width(); x++) {
			MSE += ((double) (image.Pixel(x,y)-original.Pixel(x,y)))*((double) (image.Pixel(x,y)-original.Pixel(x,y)));
		}
	}
	MSE = MSE/((double) image.Height())/((double) image.Width());
	cout<<"MSE: "<<MSE<<endl;
	cout<<"PSNR: "<<20*log10(255.0/sqrt(MSE))<<endl;

	return 0;
}
