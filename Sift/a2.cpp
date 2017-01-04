// B657 assignment 2 skeleton code
//
// Compile with: "make"
//
// See assignment handout for command line and project specifications.

//References
//http://cs.brown.edu/courses/cs143/results/proj2/sz22/ 

//Link to the header file
#include "CImg.h"
#include "Siftmatching.h"
#include "ransac.h"
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

// =========================================================== MAIN FUNCTION ===========================================================

int main(int argc, char **argv) {
    try {
        int start_s = clock();
        // the code you wish to time goes here

        if (argc < 2) {
            cout << "Insufficent number of arguments; correct usage:" << endl;
            cout << "    a2-p1 part_id ..." << endl;
            return -1;
        }

        string part = argv[1];
        int argv_size = argc - 1;
        string inputFile[103];
        int value = 2;
        int part1_3 = 0; // flag indicating section 3 

        // reading line by line input from file part_1.txt (100 image_names)
        if ((argv[3] != NULL) && strcmp(argv[3], "part_1.txt") == 0) {
            std::ifstream infile("part_1.txt");
            std::string line;
            part1_3 = 3;
            inputFile[1] = argv[2];
            while (std::getline(infile, line)) {
                std::istringstream iss(line);
                string image;
                if (!(iss >> image)) {
                    break;
                }
                inputFile[value] = image;
                value++;
            }
            argv_size = value;
        }// taking multiple Input images as arguments for part 1 --(Section 1 & 2) && part 2 -- (Section 3 & 4)
        else {
            for (int i = 1; i < argv_size; i++) {
                inputFile[i] = argv[i + 1];
            }
        }

        // NOTE :: Different parts are recognized using argv_size values
        //      Part 1 : argv_size = 3 (section 1), argv_size > 3 (section 2), argv_size = 100 (section 3)
        //      Part 2 : argv_size = 2 (section 1), argv_size > 2 (section 2 & 3)

        // =========================================================== PART 1 ===========================================================

        if (part == "part1" || part == "part1fast") {
            // find the sift descriptor for input images
            vector<SiftDescriptor> descriptors[argv_size];
            for (int n = 1; n < argv_size; n++) {
                CImg<double> input_image(inputFile[n].c_str());
                CImg<double> gray = input_image.get_RGBtoHSI().get_channel(2); // convert image to grayscale
                descriptors[n] = Sift::compute_sift(gray);
                for (int i = 0; i < descriptors[n].size(); i++) {
                    // cout << "Descriptor #" << i << ": x=" << descriptors[n][i].col << " y=" << descriptors[n][i].row << " descriptor=(";
                    for (int l = 0; l < 128; l++)
                        //    cout << descriptors[n][i].descriptor[l] << ",";
                        //cout << ")" << endl;

                        for (int j = 0; j < 5; j++)
                            for (int k = 0; k < 5; k++)
                                if (j == 2 || k == 2)
                                    for (int p = 0; p < 3; p++)
                                        if (descriptors[n][i].col + k < input_image.width() && descriptors[n][i].row + j < input_image.height())
                                            input_image(descriptors[n][i].col + k, descriptors[n][i].row + j, 0, p) = 0;
                }


                //cout << "----------- IMAGE " << n << " DESCRIPTORS CREATED ------------ " << endl;


                // Sift images for part1 section 1 & part1fast section 1
                if (argv_size == 3 && part1_3 == 0) {
                    //concatenating int and string for sift image name for instance "sift1"
                    std::ostringstream ss;
                    ss << n;
                    string sift_image = "sift" + ss.str() + ".png";
                    input_image.get_normalize(0, 255).save(sift_image.c_str());
                }
            }

            // Sift matching function ---> PART 1 :: SECTION 1
            if (argv_size == 3 && part1_3 == 0 && part == "part1") {
                if (descriptors[1].size() != 0 && descriptors[2].size() != 0) {
                    // finding image with maximum number of descriptors 
                    int max_descriptors = descriptors[1].size() > descriptors[2].size() ? 1 : 2;
                    if (max_descriptors == 1) {
                        float a = sift_matching(descriptors[1], descriptors[2], inputFile[1], inputFile[2], 1);
                    } else {
                        float b = sift_matching(descriptors[2], descriptors[1], inputFile[2], inputFile[1], 1);
                    }
                }
            }

            // PART 1 :: SECTION 2 & SECTION 3 combined
            if (argv_size > 3 || part1_3 == 3 || (part == "part1fast" && argv_size == 3)) {

                vector<pair<int, int> > desc_index;
                vector<vector<vector<float> > > descriptor_new; //Acts like sift descriptor to store k-dimensional vector


                if (part == "part1fast") {

                    int k = 5; // k-dimensional vector
                    int w = 128; // w

                    // resizing 3D vector
                    descriptor_new.resize(argv_size);
                    for (int y = 1; y < argv_size; ++y) {
                        descriptor_new[y].resize(descriptors[y].size());
                        for (int p = 0; p < descriptors[y].size(); ++p) {
                            descriptor_new[y][p].resize(k);
                        }
                    }
                    vector<float> temp_X_i = gaussian();
                    // performing dot product and storing k-dimensional vector in each new descriptor
                    for (int s = 1; s < argv_size; s++) {
                        for (int d = 0; d < descriptors[s].size(); d++) {
                            for (int l = 0; l < k; l++) {
                                descriptor_new[s][d][l] = get_convolution(descriptors[s][d], w, temp_X_i); // storing values
                            }
                        }
                    }
                }

                float value = 0.0;
                vector<float> result;
                vector<float> orig;
                cout << "Please wait program execution in progress..." << endl;

                for (int i = 2; i < argv_size; i++) {
                    int max_descriptors = descriptors[1].size() > descriptors[i].size() ? 1 : 2; // image with max descriptors 


                    //==================== PART 1 --> FAST ====================

                    if (part == "part1fast") {
                        if (max_descriptors == 1) {
                            if (argv_size == 3 && part1_3 == 0) {

                                //cout << inputFile[i] << endl;
                                // select descriptor pairs for k-dimensional vector
                                desc_index = k_dimensional_sift(descriptor_new[1], descriptor_new[i]);
                                // select matching pairs from pairs generated by above function
                                value = k_dim_sift_matching(descriptors[1], descriptors[i], desc_index, inputFile[1], inputFile[2], 1);
                                result.push_back(value);
                            } else {
                                // cout << inputFile[i] << endl;
                                // select descriptor pairs for k-dimensional vector
                                desc_index = k_dimensional_sift(descriptor_new[1], descriptor_new[i]);
                                // select matching pairs from pairs generated by above function
                                value = k_dim_sift_matching(descriptors[1], descriptors[i], desc_index, inputFile[1], inputFile[2], 0);
                                result.push_back(value);
                            }

                        } else {
                            if (argv_size == 3 && part1_3 == 0) {

                                //cout << inputFile[i] << endl;
                                // select descriptor pairs for k-dimensional vector
                                desc_index = k_dimensional_sift(descriptor_new[i], descriptor_new[1]);
                                // select matching pairs from pairs generated by above function
                                value = k_dim_sift_matching(descriptors[i], descriptors[1], desc_index, inputFile[2], inputFile[1], 1);
                                result.push_back(value);
                            } else {
                                //cout << inputFile[i] << endl;
                                // select descriptor pairs for k-dimensional vector
                                desc_index = k_dimensional_sift(descriptor_new[i], descriptor_new[1]);
                                // select matching pairs from pairs generated by above function
                                value = k_dim_sift_matching(descriptors[i], descriptors[1], desc_index, inputFile[2], inputFile[1], 0);
                                result.push_back(value);
                            }

                        }
                    }// ==================== PART 1 ---> SLOW ====================
                    else {
                        if (max_descriptors == 1) {
                            value = sift_matching(descriptors[1], descriptors[i], inputFile[1], inputFile[i], 2);
                            result.push_back(value);
                        } else {
                            value = sift_matching(descriptors[i], descriptors[1], inputFile[i], inputFile[1], 2);
                            result.push_back(value);
                        }
                    }
                }

                // Ascending and descending orders 
                orig = result; // copying result into original
                std::sort(result.begin(), result.end()); // sorting vector result

                if (part1_3 == 3) {
                    get_precision(inputFile, result, orig);
                } else {
                    cout << "Images in the descending order are as follows: " << endl;
                    get_descending_rank(inputFile, result, orig);
                }
            }
        }// =========================================================== PART 2 ===========================================================
        else if (part == "part2") {

            // ==================== PART 2 - SECITION 2 & 3 ====================
            if (argv_size > 2) {

                for (int i = 2; i < argv_size; i++) {
                    CImg<double> input_image(inputFile[1].c_str());
                    CImg<double> new_image(inputFile[i].c_str());
                    std::ostringstream ss;
                    ss << i;
                    string warped_id = "img_" + ss.str() + "-warped.png";
                    cout << warped_id << " <--------- Warped image generated... " << endl;
                    perform_ransac(input_image, new_image, warped_id);
                }

            }// ==================== PART 2 - SECITION 1 =====================
            else {

                CImg<double> input_image(inputFile[1].c_str());
                cout << "Inverse Warping of input image :: " << endl;

                CImg<double> h(3, 3, 1, 1, 0);

                h(0, 0, 0, 0) = 0.907;
                h(0, 1, 0, 0) = -0.153;
                h(0, 2, 0, 0) = -0.000306;

                h(1, 0, 0, 0) = 0.258;
                h(1, 1, 0, 0) = 1.44;
                h(1, 2, 0, 0) = 0.00073;

                h(2, 0, 0, 0) = -182;
                h(2, 1, 0, 0) = 58;
                h(2, 2, 0, 0) = 1;

                inverse_warping(input_image, h, "img_1-warped.png");
            }
        } else
            throw std::string("unknown part!");
        int stop_s = clock();
        //cout << "time: " << (stop_s - start_s) / double(CLOCKS_PER_SEC)*1000 << endl;
    } catch (const string &err) {
        cerr << "Error: " << err << endl;
    }
}
