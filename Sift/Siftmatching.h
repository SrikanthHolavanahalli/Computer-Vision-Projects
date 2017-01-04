/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   siftmatching.h
 * Author: dipika
 *
 * Created on February 24, 2016, 10:48 PM
 */

#ifndef SIFTMATCHING_H
#define SIFTMATCHING_H

#include "CImg.h"
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

// ------------------------- FUNCTION DECLARATION ------------------------------

float euclidean_sift(SiftDescriptor, SiftDescriptor); // To find Euclidean distance between 2 SiftDescriptors

float euclidean_vec(vector<float>, vector<float>); // k-dimensional Euclidean between vectors

float sift_matching(vector<SiftDescriptor>&, vector<SiftDescriptor>&, string, string, int); // Part 1 Section 1, 2, 3

vector<float> gaussian(); // Part1 Section 4 -- to find gaussian vector of 128D
 
vector<pair<int, int> > k_dimensional_sift(const vector<vector<float> >&, const vector<vector<float> >&); // to perform sift on k-dimensional vectors

float k_dim_sift_matching(vector<SiftDescriptor>&, vector<SiftDescriptor>&, vector<pair<int, int> >, string, string, int); // to perform sift on specific descriptors

bool check_repetition(vector<int>, int);

float get_convolution(SiftDescriptor, int, vector<float>);

void get_precision(string input[], vector<float>, vector<float>);

void get_descending_rank(string input[], vector<float>, vector<float>);

#endif /* SIFTMATCHING_H */

