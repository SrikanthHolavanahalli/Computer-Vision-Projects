/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

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

// ----------------------------- FUNCTION DEFINITION ----------------------------

// Euclidean distance function for 2 descriptors

float euclidean_sift(SiftDescriptor desc_1, SiftDescriptor desc_2) {
    //length = 128 for each descriptor
    float distance = 0.0;
    for (int i = 0; i < 128; i++) {
        // type casting from SiftDescriptor to float
        distance += ((static_cast<float> (desc_1.descriptor[i]) - static_cast<float> (desc_2.descriptor[i]))*(static_cast<float> (desc_1.descriptor[i]) - static_cast<float> (desc_2.descriptor[i])));
    }
    distance = sqrt(distance);
    return distance;
}

float euclidean_vec(vector<float> desc_1, vector<float> desc_2) {
    //length = 128 for each descriptor
    //    cout<<"in the euclidean new..."<<endl;
    float distance = 0.0;
    for (int i = 0; i < desc_1.size(); i++) {
        // type casting from SiftDescriptor to float
        distance += (((desc_1[i]) - (desc_2[i]))*((desc_1[i]) - (desc_2[i])));
    }
    distance = sqrt(distance);
    return distance;
}


//check if there is a repetition in inner loop

bool check_repetition(vector<int> repeat, int value) {
    for (int i = 0; i < repeat.size(); i++) {
        if (repeat[i] == value) {
            return true;
            break;
        }
    }
    return false;
}

// To draw sift matching lines between 2 input images

float sift_matching(vector<SiftDescriptor>& max_desc, vector<SiftDescriptor>& min_desc, string first, string second, int part) {

    CImg<double> input_image_1(first.c_str());
    CImg<double> input_image_2(second.c_str());
    CImg<double> merged_image(input_image_1);
    merged_image.append(input_image_2); // merging 2 images 

    // initializing variables for calculating distance and count of descriptors --> SECTION 2, 3
    float dist = 0.0, result = 0.0;
    int count = 0;
    vector<int> repeated;

    // outer for loop for 1st Descriptors
    for (int i = 0; i < max_desc.size(); i++) {
        float first_best = 3000;
        float second_best = 4000;
        int min_post;
        //vector<float> dist_array;
        // inner loop for 2nd Descriptors
        for (int j = 0; j < min_desc.size(); j++) {
            float value = euclidean_sift(max_desc[i], min_desc[j]);
            if (value < first_best) {
                second_best = first_best;
                first_best = value;
                min_post = j;
            } else if (value < second_best) {
                second_best = value;
            }
        }

        float ratio = first_best / second_best;
        ratio = roundf(ratio * 100) / 100;


        if (ratio < 0.75 && (check_repetition(repeated, min_post) == false)) {
            repeated.push_back(min_post);
            count++;
            //part 1 section 2 calculating distance and count of descriptors
            if (part == 2) {
                dist += first_best;
                count++;
            }
            //part 1 section 1 plotting lines for sift matching
            if (part == 1) {
                const unsigned char color[] = {255, 135, 64};
                // type casting from sift descriptor to float
                float x = static_cast<float> (max_desc[i].col);
                float y = static_cast<float> (max_desc[i].row);
                float x1 = static_cast<float> ((min_desc[min_post].col)+(input_image_1.width()));
                float y1 = static_cast<float> (min_desc[min_post].row);

                // draw lines and convert coordinates to type int
                merged_image.draw_line((int) x, (int) y, (int) (x1), (int) (y1), color);
            }
        }
    }

    // saving the merged image --> PART 1 :: SECTION 1
    if (part == 1) {

        merged_image.get_normalize(0, 255).save("sift_matching_slow.png");
        cout << "\n------- PART 1 :: Section 1 Executed -> Sift_Matching File generated -------" << endl;
    }

    // calculating distance/count --> PART 1 :: SECTION 2 & 3
    if (part == 2) {
        result = static_cast<float> (dist / count);
        //cout << repeated.size() << " part 2 -------repeated size" << endl;
        //cout << "Distance :" << dist << " Count :" << count << " Result: " << result << endl;
    }
    return result;
}


//Guassian distrubution function to find 128D vector X(superscript)i

vector<float> gaussian() {

    vector<float> X_i;
    int k = 0;
    float min = 0.0;
    float max = 128.0;
    for (int i = 0; i < 128; i++) {
        //int rand_num = rand() % 100;
        float gaus_no = static_cast<float> ((i - min + 1) / max);
        //float gaus_no = static_cast<float> (rand_num * 0.01);
        if ((gaus_no >= 0.0)&&(gaus_no <= 1.0)) {
            X_i.push_back(gaus_no);
        }
    }
    return X_i;
}


// return matching pair of k dimensional vector

vector<pair<int, int> > k_dimensional_sift(const vector<vector<float> >& desc1, const vector<vector<float> >& desc2) {
    vector<pair<int, int> > desc_index;
    // outer for loop for 1st Descriptors
    vector<int> repeated;

    for (int i = 0; i < desc1.size(); i++) {
        float first_best = 3000;
        float second_best = 4000;
        int min_post;

        // inner loop for 2nd Descriptors
        for (int j = 0; j < desc2.size(); j++) {
            float value = euclidean_vec(desc1[i], desc2[j]);
            if (value < first_best) {
                second_best = first_best;
                first_best = value;
                min_post = j;
            } else if (value < second_best) {
                second_best = value;
            }
        }

        float ratio = first_best / second_best;
        ratio = roundf(ratio * 100) / 100;

        //part 1 section 2 calculating distance and count of descriptors
        if (ratio < 0.75 && (check_repetition(repeated, min_post) == false)) {
            repeated.push_back(min_post);
            // cout << "Orig i values   -->" << i << endl;
            pair<int, int> indices = make_pair(i, min_post);
            desc_index.push_back(indices);
            // cout << "pairs formed from k dimensional vectors::--------------------> " << indices.first << " :: " << indices.second << " Size of Repetition " << repeated.size() << endl;
            // cout << "Orig j values   -->" << min_dist_pos << endl;
        }
    }
    return desc_index;
}

// perform sift on specific matching descriptors produced by k-dimensional vectors

float k_dim_sift_matching(vector<SiftDescriptor>& max_desc, vector<SiftDescriptor>& min_desc, vector<pair<int, int> > desc_index, string first, string second, int part) {

    CImg<double> input_image_1(first.c_str());
    CImg<double> input_image_2(second.c_str());
    CImg<double> merged_image(input_image_1);
    merged_image.append(input_image_2); // merging 2 images 

    vector<int> repeated;
    // initializing variables for calculating distance and count of descriptors --> SECTION 4
    float dist = 0.0, result = 0.0;
    int count = 0;
    vector<int> desc_match;
    desc_match.resize(desc_index.size());
    // outer for loop for 1st Descriptors
    for (int i = 0; i < desc_match.size(); i++) {
        float first_best = 3000;
        float second_best = 4000;
        int min_post;

        // inner loop for 2nd Descriptor
        for (int j = 0; j < desc_match.size(); j++) {
            float value = euclidean_sift(max_desc[desc_index[i].first], min_desc[desc_index[j].second]);
            if (value < first_best) {
                second_best = first_best;
                first_best = value;
                min_post = j;
            } else if (value < second_best) {
                second_best = value;
            }
        }

        float ratio = first_best / second_best;
        ratio = roundf(ratio * 100) / 100;

        //part 1 section 2 calculating distance and count of descriptors
        if (ratio < 0.75 && (check_repetition(repeated, min_post) == false)) {
            repeated.push_back(min_post);
            dist += first_best;
            //cout << "pairs formed after operation --> " << desc_index[i].first << " Minimum position--> " << desc_index[min_post].second << endl;
            count++;

            //part 1 section 1 plotting lines for sift matching
            if (part == 1) {
                const unsigned char color[] = {255, 135, 64};
                // type casting from sift descriptor to float
                float x = static_cast<float> (max_desc[i].col);
                float y = static_cast<float> (max_desc[i].row);
                float x1 = static_cast<float> ((min_desc[min_post].col)+(input_image_1.width()));
                float y1 = static_cast<float> (min_desc[min_post].row);

                // draw lines and convert coordinates to type int
                merged_image.draw_line((int) x, (int) y, (int) (x1), (int) (y1), color);
            }

        }
    }

    if (part == 1) {

        merged_image.get_normalize(0, 255).save("sift_matching_fast.png");
        cout << "\n------- PART 1 :: Section 1 fast Executed -> Sift_Matching FAST File generated -------" << endl;
    } else {
        if (count != 0) {
            result = static_cast<float> (dist / count);
        } else if (dist != 0) {
            result = dist;
        } else {
            result = 99999;
        }
        //cout << "Distance k -dimensionanal sift matching final:" << dist << " Count :" << count << " Result: " << result << endl;
        
    }
    return result;
}

// performs convolution

float get_convolution(SiftDescriptor desc, int w, vector<float> temp_X_i) {

    //descriptor  dot product with X_i
    float temp_summary = 0;
    for (int m = 0; m < temp_X_i.size(); m++) {
        temp_summary = temp_summary + (temp_X_i[m] * desc.descriptor[m]);
    }
    float tempSum = floor(temp_summary / w);

    return tempSum;
}

// to find precision

void get_precision(string input[], vector<float> result, vector<float> orig) {
    int correct_matches = 0;
    int first = 0;
    string query_img = input[1].substr(0, 3);
    cout << "10 top ranked images are as follows: " << endl;
    while (first != 10) {
        for (int i = 0; i < orig.size(); i++) {
            if (orig[i] == result[first]) {
                int f = first;
                orig[i] = 9999;
                cout << (f + 1) << " : " << input[i + 2] << endl;

                // Precision calculation
                string dectected_img = input[i + 2].substr(0, 3);

                if (query_img == dectected_img) {
                    correct_matches++;
                }
                first = first + 1;
                break;
            }
        }
    }
    int precision = (correct_matches * 10);
    cout << "Precision for Query image :: " << input[1] << " --> " << precision << "%" << endl;
}

// descending order rank

void get_descending_rank(string input[], vector<float> result, vector<float> orig) {

    int last = result.size() - 1; // finding maximum element position

    while (last != -1) {
        for (int i = 0; i < orig.size(); i++) {
            if (orig[i] == result[last]) {
                int l = last;
                orig[i] = 9999;
                cout << (l + 1) << " : " << input[i + 2] << endl;
                last = last - 1;
                break;
            }
        }
    }
}
