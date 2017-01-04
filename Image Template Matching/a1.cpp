#include<SImage.h>
#include<SImageIO.h>
#include<cmath>
#include<algorithm>
#include<iostream>
#include<fstream>
#include<vector>
#include<DrawText.h>
#include<essentials.h>
#include<extra_manipulations.h>
#include<hough.h>
#include<string>
#include<sstream>


using namespace std;

// The simple image class is called SDoublePlane, with each pixel represented as
// a double (floating point) type. This means that an SDoublePlane can represent
// values outside the range 0-255, and thus can represent squared gradient magnitudes,
// harris corner scores, etc. 
//
// The SImageIO class supports reading and writing PNG files. It will read in
// a color PNG file, convert it to grayscale, and then return it to you in 
// an SDoublePlane. The values in this SDoublePlane will be in the range [0,255].
//
// To write out an image, call write_png_file(). It takes three separate planes,
// one for each primary color (red, green, blue). To write a grayscale image,
// just pass the same SDoublePlane for all 3 planes. In order to get sensible
// results, the values in the SDoublePlane should be in the range [0,255].
//


SDoublePlane gaussian(const SDoublePlane &filter, double sigma) {
  double r;
  double s = 2.0*sigma*sigma;
  double sum = 0.0;
  for(int i=0;i<filter.cols();i++) {
    for(int j=0;j<filter.rows();j++) {
      r = i*i + j*j;
      filter[i][j] = (exp(-r/s))/(M_PI*s);
      sum += filter[i][j];
    }
  }
  for(int i=0;i<filter.cols();i++) {
    for(int j=0;j<filter.rows();j++) {
      filter[i][j] /= sum;
    }
  }
  return filter;
}

SDoublePlane convolve_separable(const SDoublePlane &input, const SDoublePlane &row_filter, const SDoublePlane &col_filter)
{
  SDoublePlane output(input.rows(), input.cols()); 
  float pixel;
  for(int i=1;i<input.rows()-1;i++) {
    for(int j=1;j<input.cols()-1;j++) { 
      pixel = 0.0;
      for(int k=-1;k<=1;k++) {
        pixel = pixel+(row_filter[0][k+1]*input[i][j-k]);

      }

      for(int l=-1;l<=1;l++) {
        pixel = pixel+(col_filter[l+1][0]*input[i-l][j]);
      }
      output[i][j] = pixel;
    }
  }
  return output;
}

// Convolve an image with a separable convolution kernel
//

SDoublePlane convolve_general(const SDoublePlane &input, const SDoublePlane &filter)
{
  double pixel;
  SDoublePlane output(input.rows(), input.cols());
  for(int i=1;i<input.rows()-1;i++) {
    for(int j=1;j<input.cols()-1;j++) { 
      pixel = 0.0;
      for(int k=-1;k<=1;k++) {
        for(int l=-1;l<=1;l++) {
     
          
          pixel = pixel+(input[i-l][j-k]*filter[l+1][k+1]);
        }
      }
      output[i][j] = pixel;
    }
  }
  return output;
}

// Convolve an image with a separable convolution kernel
//

// Aniket : Hamming distance
int template_no=0;
SDoublePlane hamming_distance(const SDoublePlane &input, const SDoublePlane &filter)
{
  double pixel;
  SDoublePlane output(input.rows(), input.cols());

  for(int i=0;i<=input.rows()-filter.rows();i++) {
    for(int j=0;j<=input.cols()-filter.cols();j++) { 
      pixel = 0.0;
      for(int k=0;k<=filter.rows()-1;k++) {
        for(int l=0;l<=filter.cols()-1;l++) {
          pixel = pixel+(input[i+k][j+l]*filter[k][l])+((256-input[i+k][j+l])*(256-filter[k][l]));
        }
      }
      output[i][j] = pixel;
    }
  }
  // Print Hamming-distance matrix
  //SImageIO::write_png_file("scores4.png", output, output, output);
  return output;
}

std::pair<int**,int> question_4(const SDoublePlane &input, const SDoublePlane &filter,double threshold_perc)
{
  SDoublePlane hamming_score=hamming_distance(input,filter);
   // Print Hamming-distance matrix
   template_no+=1;
   write_text_file(hamming_score,template_no);
  double threshold=(hamming_distance(filter,filter)[0][0])*threshold_perc;
  int cnt=0;
  int** matched=new int*[input.rows()*input.cols()];

  
  for(int row_index=0;row_index<input.rows();row_index++){
    for(int col_index=0;col_index<input.cols();col_index++){
      if(hamming_score[row_index][col_index] > threshold){
        matched[cnt]=new int[2];
        matched[cnt][0]=row_index;
        matched[cnt][1]=col_index;
        cnt+=1;
      }
    }
  }
  return std::make_pair(matched,cnt);
} 
//

// Apply a sobel operator to an image, returns the result
// 
SDoublePlane sobel_gradient_filter(const SDoublePlane &input, bool _gx) {
  
  SDoublePlane output(input.rows(), input.cols());
  
  SDoublePlane Sobel_x(input.rows(), input.cols());
  SDoublePlane Sobel_y(input.rows(), input.cols());
 
  SDoublePlane H_filter(3,3);
  SDoublePlane V_filter(3,3);
    
  //V_filter = {{-1/8,-2/8,-1/8}, {0/8,0/8,0/8} , { 1/8,2/8,1/8}};
    
//   H_filter[0][0] =  1.0/8;
//   H_filter[0][1] =  0;
//   H_filter[0][2] =  -1.0/8;
 
//   V_filter[0][0] =  1.0/8;
//   V_filter[1][0] =  2.0/8;
//   V_filter[2][0] =  1.0/8;

//   Sobel_x = convolve_separable(input , H_filter,V_filter);

//   H_filter[0][0] = 1.0/8;
//   H_filter[0][1] = 2.0/8;
//   H_filter[0][2] = 1.0/8;
 
//   V_filter[0][0] = 1.0/8;
//   V_filter[1][0] = 0;
//   V_filter[2][0] = -1.0/8;

//   cout << "after x  gradient" << endl;

  H_filter[0][0] =  1.0;
  H_filter[1][0] =  0.0;
  H_filter[2][0] =  -1.0;
  H_filter[1][0] = -2.0 ;
  H_filter[1][1] = 0.0 ;
  H_filter[1][2] = 2.0;
  H_filter[2][0] = -1.0;
  H_filter[2][1] = 0.0;
  H_filter[2][2] = 1.0;

  V_filter[0][0] = -1.0;
  V_filter[0][1] = -2.0;
  V_filter[0][2] = -1.0;
  V_filter[1][0] =  0.0;
  V_filter[1][1] =  0.0;
  V_filter[1][2] =  0.0;
  V_filter[2][0] =  1.0;
  V_filter[2][1] =  2.0;
  V_filter[2][2] =  1.0;

  Sobel_x = convolve_general(input , H_filter);
  Sobel_y = convolve_general(input , V_filter);
  
  int tempResult =0;

  for(int i = 0 ; i< input.rows(); i++) {
    for(int j = 0; j < input.cols();j++) {
     //cout << "x = " <<Sobel_x[i][j] << " y = " << Sobel_y[i][j] << endl ;
      tempResult = sqrt(Sobel_x[i][j] * Sobel_x[i][j]   + Sobel_y[i][j] * Sobel_y[i][j]) ;
      if(tempResult/8.0 > 32)
        tempResult = 255;
      else
        tempResult = 0;
     output[i][j]  = tempResult; 
    } 
  }
return output;
}

SDoublePlane edgePixelDistance( const SDoublePlane &input )
{
	SDoublePlane Dist(input.rows(),input.cols());
  double minPix;
  double gamma;
  double closestEdgeDistance;
  int row_size = input.rows();
  int col_size = input.cols();
	for(int p = 1; p  < row_size; p++)
	{
		for(int q  = 1; q < col_size ; q++)
		{          
			minPix  = 10000;
			gamma = 0 ;
			closestEdgeDistance = 0;
			for(int a = 1 ; a < row_size; a++) {
				for(int b = 1 ; b < col_size ; b++) {
          gamma = ((input[a][b] > 0.0) ? 0 : 994499);
					closestEdgeDistance = gamma + sqrt( pow((p-a),2) + pow((q-b),2));
					if(minPix > closestEdgeDistance)
				    minPix = closestEdgeDistance;
				}
			}
			Dist[p][q] = minPix;
		}
    cout<<"\rNumber of rows processed: "<<p+1<<std::flush;
	}
	cout<<"\nFinished !! \n";
	return Dist;
}
  
std::pair<int**,int> question_5(const SDoublePlane &input, const SDoublePlane &templt, double threshold )
{
	
	cout << "inside question 5" << endl;
  SDoublePlane scoringFunctionMatrix(input.rows(), input.cols());
	int count = 0 ; 
	int** matched = new int*[input.rows()*input.cols()];
  int i, j , k , l ;
  try {
  	for(i = 0 ; i < input.rows() - 1; i++) {
  		for(j = 0 ; j < input.cols() - 1; j++) {	
        double sum = 0;
  			for(k = 0; k < templt.rows() ; k++) {
  				for(l = 0; l < templt.cols() ; l++) {	
  					if((i+k) < input.rows() && (j + l) < input.cols() )
  					sum = sum + templt[k][l] * input[i+k][j+l];
  				}
  			}
  			scoringFunctionMatrix[i][j]  = sum ;
  	  	cout<<"\r"<<((double)i*100.0)/(double)input.rows()<<"% "<<std::flush;
  		}
  	}
  } catch(std::exception& e) {
		std::cerr << "Exception caught: " << e.what() << std::endl;
	}

  cout << "\nDone!! \n" <<endl;  
  
  for(int i = 0 ; i < scoringFunctionMatrix.rows() ; i++ )
  {
  	for(int j = 0 ; j < scoringFunctionMatrix.cols() ; j++)
  	{
      if(scoringFunctionMatrix[i][j] < threshold ) {
        matched[count]=new int[2];
        matched[count][0]= i ;
        matched[count][1]= j ;
        count+=1;
      }    
    }
  }
  return std::make_pair(matched, count);	
}

int main(int argc, char *argv[])
{
  if(!(argc >= 2)) {
      cerr << "usage: " << argv[0] << " input_image" << endl;
      return 1;
  }

  string input_filename(argv[1]);
  
  string template_file("template1.png");
  string template_2_file("template2.png");
  string template_3_file("template3.png");
  
  SDoublePlane input_image= SImageIO::read_png_file(input_filename.c_str());
  SDoublePlane templates[3] = {SImageIO::read_png_file(template_file.c_str()),
                                SImageIO::read_png_file(template_2_file.c_str()),
                                SImageIO::read_png_file(template_3_file.c_str())};
   /*** Remove Scores4.txt *****/
 remove("scores4.txt");								
  // Changes by : Aniket
  // Handling of edge case
  /**************************************************************/
  SDoublePlane input_image_modified[3];
  for(int i=0;i<3;i++){
	  input_image_modified[i]=handle_edge_case(input_image,templates[i]);
  }
  /**************************************************************/
  std::vector<int> lines = find_lines(input_image, 150.0);
  int input_ledger_diff = find_ledger_diff(input_image, 150.0);
  cout<<input_ledger_diff<<" - this is the ledger line difference. \n";
  cout<<"Rescaling templates and matching with hamming score.... \n";
  SDoublePlane scaled_template;
  if(abs(input_ledger_diff-11) >= 2) {
    for(int i=0;i<3;i++) {
      templates[i] = rescale_template(templates[i], (double)(input_ledger_diff-11.0)/11.0, "scaled_template.png");
    }
  }

  double q4_threshold=0.93;
  int cnt;
  std::pair<int**,int> results[3];

  for(int i=0;i<3;i++) {
    results[i]=question_4(input_image_modified[i],templates[i],q4_threshold);
  }
  
  detect_and_print(results, templates, input_image, lines, input_ledger_diff, "detected4", q4_threshold);


  SDoublePlane mean_filter(3,3);
  cout<<"Applying sobel filter.... \n";
  SDoublePlane temp_output_image  = sobel_gradient_filter(input_image,true);
  SImageIO::write_png_file("edges.png", temp_output_image, temp_output_image, temp_output_image);
  cout<<"Computing image edge pixel distances.... \n";
  SDoublePlane image_edge_map     = edgePixelDistance(temp_output_image);
  SDoublePlane template_edges[3];
  for(int i=0;i<3;i++) {
    template_edges[i] = sobel_gradient_filter(templates[i], true);
  }
  double edge_threshold = 800;
  for(int i=0;i<3;i++) {
    cout<<"Edge processing and matching for template: "<<i+1<<"\n";
    results[i]=question_5(image_edge_map,template_edges[i],edge_threshold);
  }


  detect_and_print(results, templates, input_image, lines, input_ledger_diff, "detected5");


  // SDoublePlane output_image = image_filter(input_image, convolve_general(input_image, gaussian(mean_filter, 20.0)));
  SDoublePlane output_image = sobel_gradient_filter(convolve_general(input_image, gaussian(mean_filter, 20.0)),true);
  std::vector<int> hough_lines(input_image.rows());
  cout<<"Transforming to hough space to detect staff lines.... \n";
  hough_transform(output_image, hough_lines, input_image);


  detect_and_print(results, templates, input_image, hough_lines, input_ledger_diff, "detected7", 0.784);


  cout<<"Complete!! The output files have been generated \n";
}
