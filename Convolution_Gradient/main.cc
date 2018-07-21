#include <stdlib.h>
#include <iostream>
#include <string>
#include <algorithm>
#include <ctime>
#include "image.h"
using namespace std;

#define UNDAMAGED 0
#define PARTIAL 2
#define DAMAGED 1 //0 represents undamaged pixels, while 1 represents damaged
//2 represents a damaged pixel that is already restored in the current iteration
#define LOW_EXTREME 0
#define HIGH_EXTREME 255 //These are the two cases for damaged pixels
#define G_THRESHOLD 255.0 //This is the threshold value for pixel gradient
#define ITERATION 100 //The total number of iterations

int sx=0;
int sy=0; //The coordinates of the starting searching point
int range=0; //The searching range

//Todo: Get the weights for a certain pixel
//Input: alpha -- the threshold value for pixel gradient
//       input -- the difference of pixel values between a damaged pixel and a good
//or restored pixel
//Return: the weigth of this pixel
double big_F(double alpha, double input) {
  double result=0.0;
  if(abs(input)<=alpha/2.0) {
    result = 1.0-input*input/alpha/alpha;
  }
  else if(abs(input)<=alpha) {
    result = (1.0-abs(input)/alpha)*(1.0-abs(input)/alpha);
  }
  return result;
}

//Todo: Get the weights for each pixel in the neighbourhood of the damaged pixel
//Input: a -- a reference to the image we are trying to restore
//       damaged -- the pointer to the 2D array that records the damagedness of every pixel
//       grad -- a 2D array that is used to record weightings from neighboring pixels
//       x -- the horizontal coordiante of the damaged pixel
//       y -- the vertical coordiante of the damaged pixel
//       alpha -- the threshold value for pixel gradient
//Return: the sum of weights in the neighbourhood of a certian damaged pixel
double weight_find(Image& a, int** damaged, double grad[3][3], int x, int y, double alpha) {
  double result=0.0;
  int count=0;
  int i,j;

//Go through the neighbourhood and get the total number of good or restored pixels
  for(j=-1;j<2;j++) {
    for(i=-1;i<2;i++) {
      if(damaged[y+j][x+i]==UNDAMAGED||damaged[y+j][x+i]==PARTIAL) {
        grad[j+1][i+1]= ((double) (a.Pixel(x+i,y+j)-a.Pixel(x,y)));
        count++;
      }
    }
  }

//Go through the neighbourhood and get the sum of weights in the neighbourhood
  for(j=-1;j<2;j++) {
    for(i=-1;i<2;i++) {
      if(damaged[y+j][x+i]==UNDAMAGED||damaged[y+j][x+i]==PARTIAL) {
        grad[j+1][i+1]=big_F(alpha, grad[j+1][i+1])/((double) count);
        result += grad[j+1][i+1];
      }
    }
  }

  return result;
}

//Todo: Acquire the sum of weighted pixel values in the neighbourhood of a certian
//damaged pixel
//Input: a -- a reference to the image we are trying to restore
//       damaged -- the pointer to the 2D array that records the damagedness of every pixel
//       grad -- a 2D array that is used to record weightings from neighboring pixels
//       x -- the horizontal coordiante of the damaged pixel
//       y -- the vertical coordiante of the damaged pixel
//Return: the sum of weighted pixel values in the neighbourhood of a certian damaged pixel
double accumulate(Image& a, int** damaged, double grad[3][3], int x, int y) {
  double result=0.0;
  int i,j;
//Go through each pixel in the neighbourhood
  for(j=-1;j<2;j++) {
    for(i=-1;i<2;i++) {
//Only include the weights from good or restored pixels
      if(damaged[y+j][x+i]==UNDAMAGED||damaged[y+j][x+i]==PARTIAL) {
        result += ((double) a.Pixel(x+i,y+j))*grad[j+1][i+1];
      }
    }
  }
  return result;
}

//Todo: Replace all damaged pixel values
//Input: a -- a reference to the image we are trying to restore
//       damaged -- the pointer to the 2D array that records the damagedness of every pixel
//       alpha -- the threshold value for pixel gradient
//Return: None
void iter(Image& a, int** damaged, double alpha) {
  int x,y;
  double aaa;
  double valid; //Some indexing and temporary variables
//a 2D array that is used to record weightings from neighboring pixels
  double grad[3][3];
//Go through each pixel in the searching region
  for(y=sy;y<sy+range;y++) {
    for(x=sx;x<sx+range;x++) {
      if(damaged[y][x]==DAMAGED||damaged[y][x]==PARTIAL) {
//valid represents the sum of weights from neighboring pixels
        valid = weight_find(a, damaged, grad, x, y, alpha);
//aaa represents the sum of all weighted pixel values from the neighbors
        aaa=accumulate(a, damaged, grad, x, y);
//Replace the damaged pixel value
        a.Pixel(x,y) = (1.0-valid)*a.Pixel(x,y)+aaa;
//Mark the pixel as partially restored
        damaged[y][x]=2;
      }
    }
  }
  return;
}

//The main function of the gradient filtering method. It will apply gradient
//filtering method to restore images and save the result in another PNG file.
//There are five inputs: The first is the file of the input image
//The second is the file of the output image
//The third and fourth inputs are the horizontal and vertical coordinates of the
//start searching point.
//The fifth input is the value of the searching range.
int main (int argc, char* argv[])
{
  int x,y;
  int t=0; //Some indexing variables
  double MSE=0.0;

	// verify arguments' correctness
	if (argc != 6)
	{
		cerr << "Useage: " << argv[0]
		     << " input.png output.png"
         << "the horizontal and vertical coordinates of the start searching point "
         << "the searching range "<<endl;
		return 1;
	}

	// load the input image
	Image image;
  Image copy;
  Image original;
	image.LoadPng (argv[1]);
  copy = image;

  if(image.Height()==512) {
		original.LoadPng ("lena.png");
	}
	else {
		original.LoadPng ("cameraman_original.png");
	}

//Establish a 2D array that records whether each pixel is damaged or not
  int** damaged;
  damaged = new int*[image.Height()];
  for(y=0;y<image.Height();y++) {
    damaged[y] = new int[image.Width()];
  }

//Read in other parameters
  sx = atoi(argv[3]);
  sy = atoi(argv[4]);
  range = atoi(argv[5]);

//Go thorugh the whole image and check for damaged pixels
  for(y=0;y<image.Height();y++) {
    for(x=0;x<image.Width();x++) {
      if(image.Pixel(x,y)==LOW_EXTREME || image.Pixel(x,y)==HIGH_EXTREME) {
        damaged[y][x]=DAMAGED;
      }
      else {damaged[y][x]=UNDAMAGED;}
    }
  }

  clock_t begin = clock(); //The place where the real algorithm starts

//Keep processing the image for ITERATION+1 times to constantly reduce errors
  while(t<=ITERATION) {
//We set the threshold value for gradient as G_THRESHOLD
    if(t==0) {iter(image, damaged, G_THRESHOLD);}
    else {iter(image, damaged, G_THRESHOLD);}

//Recover the recording of damaged pixels
    for(y=sy;y<sy+range;y++) {
      for(x=sx;x<sx+range;x++) {
        if(copy.Pixel(x,y)==LOW_EXTREME || copy.Pixel(x,y)==HIGH_EXTREME) {
          damaged[y][x]=DAMAGED;
        }
        else {damaged[y][x]=UNDAMAGED;}
      }
    }
    t++;
  }

//The place where the real algorithm ends
	clock_t end = clock();
	double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
	cout<<"Run-time in seconds: "<<elapsed_secs<<endl;

//Save the output image
  image.SavePng (argv[2]);

//Clean the dynamic memory
  for(y=0;y<image.Height();y++) {
    delete[] damaged[y];
  }
  delete[] damaged;

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
