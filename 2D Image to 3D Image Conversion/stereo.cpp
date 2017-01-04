// Skeleton code for B657 A4 Part 3.
// D. Crandall
//
// Run like this, for example:
//   ./stereo part3/Aloe/view1.png part3/Aloe/view5.png part3/Aloe/gt.png
// and output files will appear in part3/Aloe
//
#include <vector>
#include <iostream>
#include <fstream>
#include <map>
#include <math.h>
#include <CImg.h>
#include <assert.h>
#define LABEL_CNT 50
#define WINDOW_SIZE 5
using namespace cimg_library;
using namespace std;

double sqr(double a) { return a*a; }

typedef struct NeighbrCost
{
  double NeighborsCost[5][50];
}NCost;

class Point
{
public:
	Point() {}
	Point(int _col, int _row) : row(_row), col(_col) {}
	int row, col;
};

// This code may or may not be helpful. :) It computes a 
//  disparity map by looking for best correspondences for each
//  window independently (no MRF).
//
CImg<double> naive_stereo(const CImg<double> &input1, const CImg<double> &input2, int window_size, int max_disp)
{  
  CImg<double> result(input1.width(), input1.height());

  for(int i=0; i<input1.height(); i++)
    for(int j=0; j<input1.width(); j++)
    {
      pair<int, double> best_disp(0, INFINITY);

      for (int d=0; d < max_disp; d++)
      {
        double cost = 0;
        for(int ii = max(i-window_size, 0); ii <= min(i+window_size, input1.height()-1); ii++)
          for(int jj = max(j-window_size, 0); jj <= min(j+window_size, input1.width()-1); jj++)
            cost += sqr(input1(min(jj+d, input1.width()-1), ii) - input2(jj, ii));

        if(cost < best_disp.second)
          best_disp = make_pair(d, cost);
      }
      
      result(j,i) = best_disp.first;
    }

  return result;
}

double calculateCost(const CImg<double> &input1, const CImg<double> &input2, int i, int j, int dispValue)
{
  double cost = 0;
  for(int ii = max(i-WINDOW_SIZE, 0); ii <= min(i+WINDOW_SIZE, input1.height()-1); ii++)
    for(int jj = max(j-WINDOW_SIZE, 0); jj <= min(j+WINDOW_SIZE, input1.width()-1); jj++)
      cost += sqr(input1(min(jj+dispValue, input1.width()-1), ii) - input2(jj, ii));
      
  return cost;
            
}

void intializeEnergyMessages(const CImg<double> &img1, const CImg<double> &img2, vector< vector <NCost> > &NeighborEnergyValues)
{
  int pixeltype = 0; // all background
  double NodeDCost = 0;
  
  for(int i=1; i<img1.height()-1; i++)
  {
    for(int j=1; j<img1.width()-1; j++)
    {
        Point currPixel(j,i);
        //cout << "test"<<endl;
        
       
        NodeDCost = 0;
        for(int dispIterator = 0; dispIterator < LABEL_CNT; dispIterator++)
        {
          NodeDCost = NodeDCost + calculateCost(img1, img2, i, j, dispIterator);   
        
          NeighborEnergyValues[i][j].NeighborsCost[4][dispIterator] = NodeDCost;
          
          for(int k=0; k<4; k++)
          {
            NeighborEnergyValues[i][j].NeighborsCost[k][dispIterator] = 0;
          }
        }
        //cout <<  NodeDCost << endl;
       
        
        
    }
  }

}

// implement this!
//  this placeholder just returns a random disparity map
//
CImg<double> mrf_stereo(const CImg<double> &img1, const CImg<double> &img2)
{
  
  int neighbor_Count = 4;
  double alpha = 0.40;
  double entropy_j =1000;  
  double min_entropy_label = 0;
  double energy_value_from_neighbors = 0;
  double entropy_m_i_j = 100000;
  double D_i = 1.0;
  double V = 0;
  double D_j = 0;  //D = findDistance(); for j

  int no_of_neighbors = 4;
  int no_of_labels = 2;


  min_entropy_label = 0;
  Point TempNghbr; 


  vector< vector <NCost> > NeighborEnergyValues(img1.height(), vector<NCost>(img1.width()));

  intializeEnergyMessages(img1, img2, NeighborEnergyValues);
  CImg<double> result(img1.width(), img1.height());

  cout << "Loopy BP started" << endl;
  double cost = 0;
  int N = 100;
  double Energy_I = 0;
  double Energy_J = 0;
  int sentNode =0; 

  while(N > 0)
  {
   for(int i=1; i<img1.height()-1; i++)
    {
      for(int j=1; j<img1.width()-1; j++)
      {     
        for (int k = 0; k < no_of_neighbors; k++) 
        {
          min_entropy_label = 0; 
          entropy_m_i_j = 0;
          for(int l = 0; l < LABEL_CNT ; l++)  // all the labels node j can take ( i->j )
          { 

            for(int mi = 0; mi < LABEL_CNT ; mi++)  // all the labels node i take
            {  
              Energy_I =0;
              D_i =  NeighborEnergyValues[i][j].NeighborsCost[4][mi];
              for (int tempK = 0 ; tempK < 4 ; tempK++ ) // receive message from all neighbors except from the one i am sending
              {  
                if(k != tempK)
                  Energy_I += NeighborEnergyValues[i][j].NeighborsCost[tempK][mi];
              }         
              //cout << D_i << endl;
                                     
              if(l != mi) V = 1;
                Energy_I += D_i + alpha * V;         
             //cout << "E_i  " << Energy_I << endl;              
                             
             if( Energy_I < entropy_m_i_j ) // get minimum of energry values that node i thinks j should have
                entropy_m_i_j =  Energy_I;   
                    
            }
            sentNode =0; 
     
            if(k == 0)       // left neighbor     
            {  
               D_j = NeighborEnergyValues[i][j-1].NeighborsCost[4][l];
               TempNghbr.col = j-1;
               TempNghbr.row = i;
               sentNode =2; 
            }
            else if(k == 1) // up neighbor
            {
               D_j = NeighborEnergyValues[i-1][j].NeighborsCost[4][l];
               TempNghbr.col = j;
               TempNghbr.row = i-1;
               sentNode =3;
            }
            else if(k == 2) // right neighbor
            {
               D_j = NeighborEnergyValues[i][j+1].NeighborsCost[4][l];
               TempNghbr.col = j+1;
               TempNghbr.row = i;
               sentNode =0;
            }
            else           //  down neighbor
            {
               D_j = NeighborEnergyValues[i+1][j].NeighborsCost[4][l];
               TempNghbr.col = j;
               TempNghbr.row = i+1;
               sentNode = 1;
            }

            // NeighborEnergyValues[j][i].NeighborsCost[k][l] = entropy_m_i_j + D_j;  // updating neighbors messages what i think of them 

            Energy_J = entropy_m_i_j + D_j;


            //cout << "E_J: " << Energy_J << endl;      
            NeighborEnergyValues[TempNghbr.row][TempNghbr.col].NeighborsCost[sentNode][l] = Energy_J ;



          }
    
        } // end of k loop
      } // end of j loop
    }  // end of i loop

    cout << N <<endl;  
    N--;
  } // end of while loop

  double final_cost = 0;
  double compare_cost = 10000;
  int minClass = 0;
  cout << "checkpoint 1" << endl;
  result.fill(img1);
  result.fill(0);

  cout << "h: " << img1.height() << " w: " << img1.width() << endl;

  for(int a=1; a<img1.height()-1; a++)
  {
    for(int b=1; b<img1.width()-1; b++)
    { 
      compare_cost = 100000;
      for(int nc=0; nc < LABEL_CNT ;nc++)
      {
        final_cost = 0;
        for(int dk =0 ; dk < 5; dk++)
        {
          final_cost += NeighborEnergyValues[a][b].NeighborsCost[dk][nc];
          // cout << final_cost << endl;
        } 
        if (final_cost < compare_cost)
        {
          compare_cost = final_cost;
          minClass = nc;
        }
      }  

    result(b,a) = minClass;
    }
  }
  result.save("result_mrf_part3.png");
  return result;
}

int main(int argc, char *argv[])
{
  if(argc != 4 && argc != 3)
    {
      cerr << "usage: " << argv[0] << " image_file1 image_file2 [gt_file]" << endl;
      return 1;
    }

  string input_filename1 = argv[1], input_filename2 = argv[2];
  string gt_filename;
  if(argc == 4)
    gt_filename = argv[3];

  // read in images and gt
  CImg<double> image1(input_filename1.c_str());
  CImg<double> image2(input_filename2.c_str());
  CImg<double> gt;

  if(gt_filename != "")
  {
    gt = CImg<double>(gt_filename.c_str());

    // gt maps are scaled by a factor of 3, undo this...
    for(int i=0; i<gt.height(); i++)
      for(int j=0; j<gt.width(); j++)
        gt(j,i) = gt(j,i) / 3.0;
  }
  
  // do naive stereo (matching only, no MRF)
  CImg<double> naive_disp = naive_stereo(image1, image2, 5, 50);
  naive_disp.get_normalize(0,255).save((input_filename1 + "-disp_naive.png").c_str());

  // do stereo using mrf
  CImg<double> mrf_disp = mrf_stereo(image1, image2);
  mrf_disp.get_normalize(0,255).save((input_filename1 + "-disp_mrf.png").c_str());

  // Measure error with respect to ground truth, if we have it...
  if(gt_filename != "")
    {
      cout << "Naive stereo technique mean error = " << (naive_disp-gt).sqr().sum()/gt.height()/gt.width() << endl;
      cout << "MRF stereo technique mean error = " << (mrf_disp-gt).sqr().sum()/gt.height()/gt.width() << endl;

    }

  return 0;
}
