// Skeleton code for B657 A4 Part 2.
// D. Crandall
//
//
#define PI 3.14
#include <vector>
#include <iostream>
#include <fstream>
#include <map>
#include <math.h>
#include <CImg.h>
#include <assert.h>
#include <limits>

using namespace cimg_library;
using namespace std;

double beta = 36;
double tot[3], mean[3], var[3];

#define INF numeric_limits<float>::infinity()


class Point
{
public:
	Point() {}
	Point(int _col, int _row) : row(_row), col(_col) {}
	int row, col;
};

double get_gaussian_probability(const double val, const double mean, const double variance)
{
	return ((1/(sqrt(variance*2*PI)))*exp(-1*pow((val-mean),2)/(2*variance)));
}

double get_cost(const double label, const double val, const double mean, const double variance)
{

}

double getMessageFromNeighbors(const CImg<double> &img, int j, int i , int k,int iLabel);

CImg<double> naive_segment(const CImg<double> &img, const vector<Point> &fg, const vector<Point> &bg)
{
	// implement this in step 2...
	//  this placeholder just returns a random disparity map
	CImg<double> result(img.width(), img.height(),1,3);

	//double tot[3], mean[3], var[3];
	//double beta = 24;
  
  double cost = 0;
	

	//Calculating mean for each of the Red, Green and Blue channels of the Foreground Pixels
	int fg_length = fg.size();

	tot[0] = tot[1] = tot[2] = 0;

	for(int i=0; i<fg_length;i++)
	{
		tot[0] = tot[0] + img(fg[i].col,fg[i].row,0,0);
		tot[1] = tot[1] + img(fg[i].col,fg[i].row,0,1);
		tot[2] = tot[2] + img(fg[i].col,fg[i].row,0,2);
	}

	mean[0] = tot[0]/fg_length;
	mean[1] = tot[1]/fg_length;
	mean[2] = tot[2]/fg_length;

	tot[0] = tot[1] = tot[2] = 0;

	//Calculating Variance for the Foreground Pixels
	for(int i=0;i<fg_length;i++)
	{
		tot[0] = tot[0] + pow((img(fg[i].col,fg[i].row,0,0) - mean[0]),2);
		tot[1] = tot[1] + pow((img(fg[i].col,fg[i].row,0,1) - mean[1]),2);
		tot[2] = tot[2] + pow((img(fg[i].col,fg[i].row,0,2) - mean[2]),2);
	}
	
	var[0] = tot[0]/fg_length;
	var[1] = tot[1]/fg_length;
	var[2] = tot[2]/fg_length;

	for(int i=0; i < fg_length; i++)
	{
		result(fg[i].col,fg[i].row,0,0) = 1;
		result(fg[i].col,fg[i].row,0,1) = 1;
		result(fg[i].col,fg[i].row,0,2) = 1;
	}

	for(int i=0;i<bg.size();i++)
	{
		result(bg[i].col,bg[i].row,0,0) = 0;
		result(bg[i].col,bg[i].row,0,1) = 0;
		result(bg[i].col,bg[i].row,0,2) = 0;
	}

	for(int i=0; i<img.height(); i++)
	{
		for(int j=0; j<img.width(); j++)
		{
			if((result(j,i,0,0) != 0) || (result(j,i,0,0) != 1))
			{
				cost= 0;
				for(int k=0;k<3;k++)
				{
					cost = cost + (-1.0)*log(get_gaussian_probability(img(j,i,0,k),mean[k],var[k]));
				}

				if(cost < beta)
				{
					result(j,i,0,0) = 1;
					result(j,i,0,1) = 1;
					result(j,i,0,2) = 1;
				}
				else
				{
					result(j,i,0,0) = 0;
					result(j,i,0,1) = 0;
					result(j,i,0,2) = 0;
				}

			}
		}
	}
 
  //result.save("result_naive.png"); 
	return result;
}

typedef struct NeighbrCost
{
  double NeighborsCost[5][2];
    
}NCost;

int checkIfBGorFG(int w ,int h,const vector<Point> &fg, const vector<Point> &bg )
{
    int type = 0;
    for(int i = 0; i < fg.size() ; i++)
    {
        if(fg[i].row == h && fg[i].col == w)
        {type = 1;
        break;
        }
    }
    
    if(type == 0)
    {
      for(int j = 0; j < bg.size() ; j++)
      {
        if(bg[j].row == h && bg[j].col == w)
        {type = 2;
        break;}
      }  
    }
    
    
    return type;

}

void intializeEnergyMessages(const CImg<double> &img, const vector<Point> &fg, const vector<Point> &bg, vector< vector <NCost> > &NeighborEnergyValues)
{
  int pixeltype = 0; // all background
  double NodeDCost = 0;
  for(int i=1; i<img.height()-1; i++)
  {
    for(int j=1; j<img.width()-1; j++)
    {
        Point currPixel(j,i);
        //cout << "test"<<endl;
        pixeltype = checkIfBGorFG(j,i, fg, bg);  
        if(pixeltype == 0)
        {
              NodeDCost = 0;
              for(int k=0;k<3;k++)
      			  {
      				  NodeDCost = NodeDCost + (-1.0)*log(get_gaussian_probability(img(j,i,0,k),mean[k],var[k]));   
      			  }
              NeighborEnergyValues[i][j].NeighborsCost[4][0] = beta;
              NeighborEnergyValues[i][j].NeighborsCost[4][1] = NodeDCost;
              
              //cout <<  NodeDCost << endl;
        }
        else if(pixeltype == 1)
        {
          NeighborEnergyValues[i][j].NeighborsCost[4][0] = 1000;
          NeighborEnergyValues[i][j].NeighborsCost[4][1] = 0;
        }
        else
        {
          NeighborEnergyValues[i][j].NeighborsCost[4][0] = 0;
          NeighborEnergyValues[i][j].NeighborsCost[4][1] = 1000;
        }
        for(int k=0; k<4; k++)
        {
          NeighborEnergyValues[i][j].NeighborsCost[k][0] = 0;
          NeighborEnergyValues[i][j].NeighborsCost[k][1] = 0;
        }
        
    }
  }

}


void testIntitilaization(const CImg<double> &img,  vector< vector <NCost> > &NeighborEnergyValues)
{

  for(int i=1; i<img.height()-1; i++)
  {
    for(int j=1; j<img.width()-1; j++)
    { 
       cout << "0  " <<  NeighborEnergyValues[j][i].NeighborsCost[4][0] << "  1  " << NeighborEnergyValues[i][j].NeighborsCost[4][1] << endl;
    }
    
  }
  

}


CImg<double> mrf_segment(const CImg<double> &img, const vector<Point> &fg, const vector<Point> &bg)
{
	// implement this in step 3...
	//  this placeholder just returns a random disparity map by calling naive_segment
  // cout << "debug 1" << endl;
   
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
	
   
   vector< vector <NCost> > NeighborEnergyValues(img.height(), vector<NCost>(img.width()));
 
   intializeEnergyMessages(img, fg, bg, NeighborEnergyValues);
   
  // testIntitilaization(img,NeighborEnergyValues);

   cout << "Loopy BP started" << endl;
   double cost = 0;
   int N = 100;
   double Energy_I = 0;
   double Energy_J = 0;
   int sentNode =0; 
   

//while(N>0)
//result =  naive_segment(img, fg, bg); 



  while(N > 0)
  {
   for(int i=1; i<img.height()-1; i++)
  {
    for(int j=1; j<img.width()-1; j++)
    {     
      for (int k = 0; k < no_of_neighbors; k++) 
      {
       min_entropy_label = 0; 
       entropy_m_i_j = 0;
        for(int l = 0; l < no_of_labels ; l++)  // all the labels node j can take ( i->j )
        { 
          
          for(int mi = 0; mi < no_of_labels ; mi++)  // all the labels node i take
          {  Energy_I =0;
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
            int dummy;
            //cin >> dummy;
     } // end of k loop
    } // end of j loop
  }  // end of i loop
  
  cout << N <<endl;  
    N--;
 } // end of while loop
 


 double final_cost = 0;
 double compare_cost = 10000;
 int minClass = 0;
 CImg<double> result(img.width(), img.height());
 cout << "checkpoint 1" << endl;
 result.fill(img);
 result.fill(0);

 /*
 for(int i = 0;i<img.height();i++)
 {
    cout << i << endl;
    for(int j = 0; j < img.width(); j++)
    {
        result(j,i) = 0;
    }
 }
 */

 cout << "h: " << img.height() << " w: " << img.width() << endl;
 
 for(int a=1; a<img.height()-1; a++)
  {
    for(int b=1; b<img.width()-1; b++)
    { 
      compare_cost = 100000;
      for(int nc=0; nc < 2 ;nc++)
      {
        final_cost = 0;
        final_cost += NeighborEnergyValues[a][b].NeighborsCost[4][nc];
        //cout << final_cost << endl;
        for(int dk =0 ; dk < 4; dk++)
        {
          final_cost += NeighborEnergyValues[a][b].NeighborsCost[dk][nc];
         // cout << final_cost << endl;
        }  
           
         // cout << final_cost << endl;
          
          int dummy;
          //cin >> dummy;
           
          if (final_cost < compare_cost)
          {
            compare_cost = final_cost;
            minClass = nc;
          }
      }  
      
      //cout << "a: "  << a << " b: "<< b << endl;
      
      
      
        if(minClass == 0)
        {
         //   cout << "a: "  << a << " b: "<< b << " in if then" << endl;
            result(b,a) = 0;
            //result(b,a,0,1) = 0;
            //result(b,a,0,2) = 0;

        }
     	  else
        {
           // cout << "a: "  << a << " b: "<< b << " in if else" << endl;
            result(b,a) = 1;
            //result(b,a,0,1) = 1;
            //result(b,a,0,2) = 1;

        }
      }
    }
    
  result.save("result_mrf.png");
  return result;

}//end of function





// Take in an input image and a binary segmentation map. Use the segmentation map to split the 
//  input image into foreground and background portions, and then save each one as a separate image.
//
// You'll just need to modify this to additionally output a disparity map.
//
void output_segmentation(const CImg<double> &img, const CImg<double> &labels, const string &fname)
{
	// sanity checks. If one of these asserts fails, you've given this function invalid arguments!
	assert(img.height() == labels.height());
	assert(img.width() == labels.width());
  
  
  
	CImg<double> img_fg = img, img_bg = img;
    
    CImg<double> img_disp = img;
    
	for(int i=0; i<labels.height(); i++)
	{
		for(int j=0; j<labels.width(); j++)
		{
			if(labels(j,i) == 0)
            {
                img_fg(j,i,0,0) = img_fg(j,i,0,1) = img_fg(j,i,0,2) = 0;
                img_disp(j,i,0,0) = img_disp(j,i,0,1) = img_disp(j,i,0,2) = 0;
            }
			else if(labels(j,i) == 1)
            {
				img_bg(j,i,0,0) = img_bg(j,i,0,1) = img_bg(j,i,0,2) = 0;
                img_disp(j,i,0,0) = img_disp(j,i,0,1) = img_disp(j,i,0,2) = 255;
            }
            else
            {
			    cout << " Row= " <<i << " " << "Col = " << j << " Label = " << labels(j,i) << endl;
				assert(0);
			}
		}
	}
    
    img_disp.get_normalize(0,255).save((fname + "_disp.png").c_str());
	img_fg.get_normalize(0,255).save((fname + "_fg.png").c_str());
	img_bg.get_normalize(0,255).save((fname + "_bg.png").c_str());
}

int main(int argc, char *argv[])
{
	string input_filename1, input_filename2;
	if(argc != 3)
	{
		cerr << "usage: " << argv[0] << " image_file seeds_file" << endl;
		return 1;
	}
	else
	{
		input_filename1 = argv[1];
		input_filename2 = argv[2];
	}
	// read in images and gt
	CImg<double> image_rgb(input_filename1.c_str());
	CImg<double> seeds_rgb(input_filename2.c_str());

	// figure out seed points
	vector<Point> fg_pixels, bg_pixels;

	for(int i=0; i<seeds_rgb.height(); i++)
	{
		for(int j=0; j<seeds_rgb.width(); j++)
		{
			// blue --> foreground
			if(max(seeds_rgb(j, i, 0, 0), seeds_rgb(j, i, 0, 1)) < 100 && seeds_rgb(j, i, 0, 2) > 100)
				fg_pixels.push_back(Point(j, i));

			// red --> background
			if(max(seeds_rgb(j, i, 0, 2), seeds_rgb(j, i, 0, 1)) < 100 && seeds_rgb(j, i, 0, 0) > 100)
				bg_pixels.push_back(Point(j, i));
		}
	}

	// do naive segmentation
	CImg<double> labels_naive = naive_segment(image_rgb, fg_pixels, bg_pixels);
	output_segmentation(image_rgb, labels_naive, input_filename1 + "-naive_segment_result");
    cout << "height = " << image_rgb.height() << " width= " << image_rgb.width() << endl;
	// do mrf segmentation
	CImg<double>  labels_mrf = mrf_segment(image_rgb, fg_pixels, bg_pixels);
    cout << "final test" <<endl;
    cout << "Input filename = " << input_filename1 << endl;
	output_segmentation(image_rgb, labels_mrf, input_filename1 + "-mrf_segment_result");

	return 0;
}
