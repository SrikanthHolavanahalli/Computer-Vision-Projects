#include "ransac.h"
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
#include <cfloat>
#include <list>
#include <queue>
#include <set>
#include <time.h>

//Use the cimg namespace to access the functions easily
using namespace cimg_library;
using namespace std;



//section 2 part 1

void inverse_warping(CImg<double> input_image, CImg<double> &h, string warp_id) {
    double u = 0, v = 0, w = 0;

    cout << "==============================" << endl;

    printMatrix(h);

    cout << "==============================" << endl;

    CImg<double> h_inverse = h.invert();

    cout << "==============================" << endl;

    printMatrix(h_inverse);

    cout << "==============================" << endl;

    CImg<double> dest(input_image.width(), input_image.height(), 1, 3, 0);

    for (int i = 0; i < input_image.width(); i++) {
        for (int j = 0; j < input_image.height(); j++) {
            u = h_inverse(0, 0, 0, 0) * i + h_inverse(1, 0, 0, 0) * j + h_inverse(2, 0, 0, 0);
            v = h_inverse(0, 1, 0, 0) * i + h_inverse(1, 1, 0, 0) * j + h_inverse(2, 1, 0, 0);
            w = h_inverse(0, 2, 0, 0) * i + h_inverse(1, 2, 0, 0) * j + h_inverse(2, 2, 0, 0);

            u = rint(u / w);
            v = rint(v / w);

            if (u < input_image.width() && v < input_image.height() && u >= 0 && v >= 0) {
                dest(i, j, 0) = input_image(u, v, 0);
                dest(i, j, 1) = input_image(u, v, 1);
                dest(i, j, 2) = input_image(u, v, 2);
            }
        }
    }


    dest.get_normalize(0, 255).save(warp_id.c_str());

}


//perform ransac

void perform_ransac(CImg<double> input_image, CImg<double> new_image, string warp_id) {
    
    vector<pair<SiftDescriptor, SiftDescriptor> > matching_descriptors = getMatchingDescriptors(input_image, new_image);


    CImg<double> mat_a(8, 8, 1, 1, 0);
    CImg<double> mat_b(1, 8, 1, 1, 0);
    CImg<double> h(1, 8, 1, 1, 0);

    CImg<double> H(3, 3);
    CImg<double> H_MAX(3, 3);


    //randomly select 4 points;
    int index[4];
    std::set<int> index_set;
    std::set<int>::iterator it;
    int count = 0;
    int loop = 0;
    int inliersCount = 0;
    int tempInlierCount = 0;

    srand(time(NULL));

    while (loop < 1000) {
        vector<pair<SiftDescriptor, SiftDescriptor> > random_des;
        while (count < 4) {
            int ind = rand() % matching_descriptors.size();
            it = index_set.find(ind);
            if (it == index_set.end()) {
                index[count] = ind;
                count++;
            }

        }

        for (int i = 0; i < 4; i++) {
            random_des.push_back(matching_descriptors[index[i]]);
        }




        fillMatrixA(mat_a, random_des);

        fillMatrixB(mat_b, random_des);
  
        h = mat_b.solve(mat_a);

        int pos = 0;
        for (int j = 0; j < H.height(); j++) {
            for (int i = 0; i < H.width(); i++) {
                H(i, j) = h(0, pos);
                pos++;
            }
        }
        H(2, 2) = 1;

        tempInlierCount = getInliersCount(H, matching_descriptors);

        if (tempInlierCount > inliersCount) {
            inliersCount = tempInlierCount;
            H_MAX = H;
        }

        loop++;
    }

    cout << matching_descriptors.size() << "::" << inliersCount << endl;

    inverse_warping(input_image, H_MAX, warp_id);
}

int getInliersCount(CImg<double> &H, vector<pair<SiftDescriptor, SiftDescriptor> > desc_pairs) {
    CImg<double> temp(1, 3, 1, 1, 1);
    CImg<double> result;
    int count = 0;

    for (int i = 0; i < desc_pairs.size(); i++) {
        temp(0, 0) = desc_pairs[i].first.col;
        temp(0, 1) = desc_pairs[i].first.row;

        //cout << "Second col:" <<  desc_pairs[i].second.col << "second row" << desc_pairs[i].second.row << endl;

        result = H * temp;

        result /= result(0, 2);

        //printMatrix(result);

        if ((abs(desc_pairs[i].second.col - result(0, 0)) < 10) && abs((desc_pairs[i].second.row - result(0, 1)) < 10)) {
            count++;
        }



    }

    return count;

}

void printMatrix(CImg<double> img) {
    for (int j = 0; j < img.height(); j++) {
        for (int i = 0; i < img.width(); i++) {
            cout << img(i, j, 0) << " ";
        }

        cout << endl;
    }
}

void fillMatrixB(CImg<double> &mat_b, vector<pair<SiftDescriptor, SiftDescriptor> > random_des) {
    int i = 0, r = 0;
    int a = 0;
    while (i < 8) {
        for (int j = 0; j < 2; j++) {
            if (j % 2 == 0) {
                mat_b(0, r, 0, 0) = random_des[a].second.col;
                i++;
                r++;
            } else {
                mat_b(0, r, 0, 0) = random_des[a].second.row;
                i++;
                r++;
            }
        }
        a++;
    }
}

void fillMatrixA(CImg<double> &mat_a, vector<pair<SiftDescriptor, SiftDescriptor> > random_des) {

    int r = 0;
    int i = 0;
    int a = 0;

    for (int i = 0; i < 4; i++) {
        //cout << random_des[i].first.row << "::::" << random_des[i].second.row << endl;
    }

    while (i < 8) {
        //cout << "cout filling first column - inside i" << endl;
        for (int j = 0; j < 2; j++) {


            if (j % 2 == 0) {
                //cout <<  random_des[a].first.col << endl;
                mat_a(0, r, 0, 0) = random_des[a].first.col;
                a++;
            } else {
                mat_a(0, r, 0, 0) = 0;

            }
            r++;
            i++;
        }
    }

    //filling second column
    //	cout << "cout filling 2nd column" << endl;
    i = 0;
    r = 0;
    a = 0;
    while (i < 8) {
        for (int j = 0; j < 2; j++) {
            if (j % 2 == 0) {
                mat_a(1, r, 0, 0) = random_des[a].first.row;
                r++;
                i++;
                a++;
            } else {
                mat_a(1, r, 0, 0) = 0;
                r++;
                i++;
            }
        }
    }

    //filling third column
    //	cout << "cout filling 3rd column" << endl;
    i = 0;
    r = 0;
    a = 0;
    while (i < 8) {
        for (int j = 0; j < 2; j++) {
            if (j % 2 == 0) {
                mat_a(2, r, 0, 0) = 1;
                r++;
                i++;
            } else {
                mat_a(2, r, 0, 0) = 0;
                r++;
                i++;
            }
        }
    }


    //filling 4th column
    //cout << "cout filling 4th column" << endl;
    i = 0;
    r = 0;
    a = 0;
    while (i < 8) {
        for (int j = 0; j < 2; j++) {
            if (j % 2 != 0) {
                mat_a(3, r, 0, 0) = random_des[a].first.col;
                r++;
                i++;
                a++;
            } else {
                mat_a(3, r, 0, 0) = 0;
                r++;
                i++;
            }
        }
    }


    //filling 5th column
    //cout << "cout filling 5th column" << endl;
    i = 0;
    r = 0;
    a = 0;
    while (i < 8) {
        for (int j = 0; j < 2; j++) {
            if (j % 2 != 0) {
                mat_a(4, r, 0, 0) = random_des[a].first.row;
                r++;
                i++;
                a++;
            } else {
                mat_a(4, r, 0, 0) = 0;
                r++;
                i++;
            }
        }
    }


    //filling 6th column
    //cout << "cout filling 6th column" << endl;
    i = 0;
    r = 0;
    a = 0;
    while (i < 8) {
        for (int j = 0; j < 2; j++) {
            if (j % 2 != 0) {
                mat_a(5, r, 0, 0) = 1;
                r++;
                i++;
                a++;
            } else {
                mat_a(5, r, 0, 0) = 0;
                r++;
                i++;
            }
        }
    }

    //filling 7th column
    //cout << "cout filling 7th column" << endl;
    i = 0;
    r = 0;
    a = 0;

    while (i < 8) {
        for (int j = 0; j < 2; j++) {
            if (j % 2 == 0) {
                mat_a(6, r, 0, 0) = -1 * random_des[a].first.col * random_des[a].second.col;
                r++;
                i++;
                //a++;
            } else {
                mat_a(6, r, 0, 0) = -1 * random_des[a].first.col * random_des[a].second.row;
                r++;
                i++;
                //b++;
            }
        }
        a++;
    }


    //filling 8th column
    //cout << "cout filling 8th column" << endl;
    i = 0;
    r = 0;
    a = 0;

    while (i < 8) {
        for (int j = 0; j < 2; j++) {
            if (j % 2 == 0) {
                mat_a(7, r, 0, 0) = -1 * random_des[a].first.row * random_des[a].second.col;
                r++;
                i++;
                //a++;
            } else {
                mat_a(7, r, 0, 0) = -1 * random_des[a].first.row * random_des[a].second.row;
                r++;
                i++;
                //b++;
            }
        }
        a++;

    }


}

vector<pair<SiftDescriptor, SiftDescriptor> > getMatchingDescriptors(CImg<double> input_image, CImg<double> new_image) {

    //CImg<double> input_image(inputFile[n].c_str());
    CImg<double> gray = input_image.get_RGBtoHSI().get_channel(2); // convert image to grayscale
    vector<SiftDescriptor> input_descriptor = Sift::compute_sift(gray);
    gray = new_image.get_RGBtoHSI().get_channel(2);
    vector<SiftDescriptor> new_descriptor = Sift::compute_sift(gray);

    vector<pair<SiftDescriptor, SiftDescriptor> > matching_descriptors;

    float first_min = 30000, second_min = 30000;
    SiftDescriptor first_min_des;
    SiftDescriptor second_min_des;
    vector<int> repeated;

    for (int i = 0; i < input_descriptor.size(); i++) {
        // vector<float> dist_array;
        float first_min = 30000, second_min = 30000;
        int min_post;
        // inner loop for 2nd Descriptors
        for (int j = 0; j < new_descriptor.size(); j++) {
            float value = euclidean_sift(input_descriptor[i], new_descriptor[j]);

            if (value < first_min) {
                second_min_des = first_min_des;
                first_min_des = new_descriptor[j];
                second_min = first_min;
                first_min = value;
                min_post = j;
            } else if (value < second_min) {
                second_min_des = new_descriptor[j];
                second_min = value;
            }
        }

        //   cout << first_min  << ":" << second_min << endl;
        if ((first_min / second_min) < 0.75 && (check_repetition(repeated, min_post) == false)) {
            repeated.push_back(min_post);
            pair<SiftDescriptor, SiftDescriptor> p = make_pair(input_descriptor[i], first_min_des);
            matching_descriptors.push_back(p);
            //cout << "pushing matching decs" << endl;
        }

    }

    //cout << matching_descriptors.size() << endl;
    return matching_descriptors;
}

