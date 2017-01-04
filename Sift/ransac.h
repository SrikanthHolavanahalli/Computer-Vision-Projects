#include "CImg.h"
#include "Siftmatching.h"
#include <ctime>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <vector>
#include <Sift.h>
#include <sstream>
#include <algorithm>
#include <fstream>
#include <list>
#include <queue>

//Use the cimg namespace to access the functions easily
using namespace cimg_library;
using namespace std;


void inverse_warping(CImg<double> input_image, CImg<double> &h, string warp_id);

void perform_ransac(CImg<double> input_image, CImg<double> new_image, string warp_id);

vector<pair<SiftDescriptor,SiftDescriptor> > getMatchingDescriptors(CImg<double>, CImg<double>);

void fillMatrixA(CImg<double> &mat_a, vector<pair<SiftDescriptor,SiftDescriptor> > random_des);

void fillMatrixB(CImg<double> &mat_b, vector<pair<SiftDescriptor,SiftDescriptor> > random_des);

int getInliersCount(CImg<double> &H, vector<pair<SiftDescriptor,SiftDescriptor> > desc_pairs);

void printMatrix(CImg<double> img);