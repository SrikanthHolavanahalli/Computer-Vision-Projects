#include<SImage.h>
#include<SImageIO.h>
#include<cmath>
#include<algorithm>
#include<iostream>
#include<fstream>
#include<vector>
#include<DrawText.h>



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

// Below is a helper functions that overlays rectangles
// on an image plane for visualization purpose. 

// Draws a rectangle on an image plane, using the specified gray level value and line width.
//
void overlay_rectangle(SDoublePlane &input, int _top, int _left, int _bottom, int _right, double graylevel, int width)
{
  for(int w=-width/2; w<=width/2; w++) {
    int top = _top+w, left = _left+w, right=_right+w, bottom=_bottom+w;

    // if any of the coordinates are out-of-bounds, truncate them 
    top = min( max( top, 0 ), input.rows()-1);
    bottom = min( max( bottom, 0 ), input.rows()-1);
    left = min( max( left, 0 ), input.cols()-1);
    right = min( max( right, 0 ), input.cols()-1);
      
    // draw top and bottom lines
    for(int j=left; j<=right; j++)
	  input[top][j] = input[bottom][j] = graylevel;
    // draw left and right lines
    for(int i=top; i<=bottom; i++)
	  input[i][left] = input[i][right] = graylevel;
  }
}

// DetectedSymbol class may be helpful!
//  Feel free to modify.
//
typedef enum {NOTEHEAD=0, QUARTERREST=1, EIGHTHREST=2} Type;
class DetectedSymbol {
public:
  int row, col, width, height;
  Type type;
  char pitch;
  double confidence;
};

// Function that outputs the ascii detection output file
void  write_detection_txt(const string &filename, const vector<struct DetectedSymbol> &symbols)
{
  ofstream ofs(filename.c_str());

  for(int i=0; i<symbols.size(); i++)
    {
      const DetectedSymbol &s = symbols[i];
      ofs << s.row << " " << s.col << " " << s.width << " " << s.height << " ";
      if(s.type == NOTEHEAD)
	ofs << "filled_note " << s.pitch;
      else if(s.type == EIGHTHREST)
	ofs << "eighth_rest _";
      else 
	ofs << "quarter_rest _";
      ofs << " " << s.confidence << endl;
    }
}

// Function that outputs a visualization of detected symbols
void  write_detection_image(const string &filename, const vector<DetectedSymbol> &symbols, const SDoublePlane &input)
{
  SDoublePlane output_planes[3];
  for(int i=0; i<3; i++)
    output_planes[i] = input;

  for(int i=0; i<symbols.size(); i++)
    {
      const DetectedSymbol &s = symbols[i];

      overlay_rectangle(output_planes[s.type], s.row, s.col, s.row+s.height-1, s.col+s.width-1, 255, 2);
      overlay_rectangle(output_planes[(s.type+1) % 3], s.row, s.col, s.row+s.height-1, s.col+s.width-1, 0, 2);
      overlay_rectangle(output_planes[(s.type+2) % 3], s.row, s.col, s.row+s.height-1, s.col+s.width-1, 0, 2);

      if(s.type == NOTEHEAD)
	{
	  char str[] = {s.pitch, 0};
	  draw_text(output_planes[0], str, s.row, s.col+s.width+1, 0, 2);
	  draw_text(output_planes[1], str, s.row, s.col+s.width+1, 0, 2);
	  draw_text(output_planes[2], str, s.row, s.col+s.width+1, 0, 2);
	}
    }

  SImageIO::write_png_file(filename.c_str(), output_planes[0], output_planes[1], output_planes[2]);
}



// The rest of these functions are incomplete. These are just suggestions to 
// get you started -- feel free to add extra functions, change function
// parameters, etc.

// Convolve an image with a separable convolution kernel
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
  cout << "after convo sep" << endl ;
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
     // cout << pixel;
      output[i][j] = pixel;
    }
  }
  return output;
}

// Aniket : Hamming distance
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
  return output;
}

std::pair<int**,int> question_4(const SDoublePlane &input, const SDoublePlane &filter,double threshold_perc)
{
  SDoublePlane hamming_score=hamming_distance(input,filter);
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
 
  SDoublePlane H_filter(1,3);
  SDoublePlane V_filter(3,1);
    
  //V_filter = {{-1/8,-2/8,-1/8}, {0/8,0/8,0/8} , { 1/8,2/8,1/8}};
    
  H_filter[0][0] =  1.0/8;
  H_filter[0][1] =  0;
  H_filter[0][2] =  -1.0/8;
 
  V_filter[0][0] =  1.0/8;
  V_filter[0][1] =  2.0/8;
  V_filter[0][2] =  1.0/8;

  Sobel_x = convolve_separable(input , H_filter,V_filter);

  H_filter[0][0] = 1.0/8;
  H_filter[0][1] = 2.0/8;
  H_filter[0][2] = 1.0/8;
 
  V_filter[0][0] = 1.0/8;
  V_filter[0][1] = 0;
  V_filter[0][2] = -1.0/8;

  cout << "after x  gradient" << endl;

  Sobel_y = convolve_separable(input , H_filter,V_filter);
  
  int tempResult =0;

  for(int i = 0 ; i< input.rows(); i++) {
    for(int j = 0; j < input.cols();j++) {
      tempResult = sqrt(Sobel_x[i][j] * Sobel_x[i][j] + Sobel_y[i][j] * Sobel_y[i][j]) ;
      if(tempResult > 110.0)
        tempResult = 0;
      else
        tempResult = 255;
      output[i][j]  = tempResult;
    }
  }

  return output;
}

SDoublePlane edgePixelDistance( const SDoublePlane &input )
{
	SDoublePlane D(input.rows(),input.cols());
	for(int i = 0 ; i < input.rows(); i++)
	{
		for(int j =0 ; j < input.cols() ; j++)
		{
			double min = 10000;
			double gamma = 0 ;
			double closestEdgeDistance = 0;
			for(int a = 0 ; a< input.rows(); a++)
			{
				for(int b= 0 ; b < input.cols() ; b++)
				{
					if(input[a][b] > 0)
					gamma = 0;
					else
					gamma = 994499;
					closestEdgeDistance = gamma + sqrt((i-a)*(i-a) + (j-b)*(j-b));
					if (min > closestEdgeDistance)
					 min = closestEdgeDistance;
				}
			}
			D[i][j] = min;
		}
	}
	
	return D;
}
  
std::pair<int**,int> question_5(const SDoublePlane &input, const SDoublePlane &templt, double threshold )
{
	//SDoublePlane closestEdgeMatrix = edgePixelDistane(input);
	SDoublePlane scoringFunctionMatrix(input.rows(), input.cols());
	int count = 0 ; 
	int** matched = new int*[input.rows()*input.cols()];

	
	for(int i = 0 ; i < input.rows(); i++)
	{
		for(int j = 0; j < input.cols(); j++)
		{	double sum = 0;
			for(int k = 0; k < templt.rows() ; k++)
			{
				for(int l = 0; l < templt.cols() ; l++)
				{
					sum = sum + templt[k][l] * input[i+k][j+l];
				}
			}
			
			scoringFunctionMatrix[i][j]  = sum ;
		}
	}


  
  
for(int i = 0 ; i < scoringFunctionMatrix.rows() ; i++ )
{
	for(int j = 0 ; j < scoringFunctionMatrix.rows() ; j++)
	{
		if(scoringFunctionMatrix[i][j] > threshold )
		matched[count]=new int[2];
        matched[count][0]= i ;
        matched[count][1]= j ;
        count+=1;
        //cout << "\n row : " << row_index << " col : " << col_index << " score : " << hamming_score[row_index][col_index];
    }
}


  
  
return std::make_pair(matched, count);	
}

// Apply an edge detector to an image, returns the binary edge map
// 
SDoublePlane find_edges(const SDoublePlane &input, double thresh=0)
{
  SDoublePlane output(input.rows(), input.cols());

  // Implement an edge detector of your choice, e.g.
  // use your sobel gradient operator to compute the gradient magnitude and threshold
  
  return output;
}



SDoublePlane image_filter(const SDoublePlane &input, const SDoublePlane &blurred) {
  SDoublePlane output(input.rows(), input.cols());
  for(int i=0;i<input.rows();i++) {
    for(int j=0;j<input.cols();j++) {
      output[i][j] = input[i][j] - blurred[i][j];
      if(output[i][j] < 10.0) {
        output[i][j] = 255.0;
      } else {
        output[i][j] = 0.0;
      }
    }
  }
  return output;
}

std::vector<int> find_lines(const SDoublePlane &input, double threshold) {
  std::vector<int> ledgers(input.rows());
  int prev = 0;
  int line_count = 0;
  for(int i=0;i<input.rows();i++) {
    int count = 0;
    for(int j=0;j<input.cols();j++) {
      if(input[i][j] < threshold) {
        count++;
      }
    }
    if(((double)count)/((double)input.cols()) >= 0.6) {
      if(i-prev >= 4) {
        ledgers[i] = line_count;
        prev = i;
        line_count++;
      }
    }
  }
  return ledgers;
}

SDoublePlane hough_transform(const SDoublePlane &input) {
  double hough_size = (sqrt(2.0)*(input.rows()>input.cols() ? input.rows() : input.cols()))/2.0;
  double accumulator_height = hough_size*2.0;
  double accumulator_width = 180.0;
  SDoublePlane accu_image(accumulator_height, accumulator_width);
  SDoublePlane output(input.rows(), input.cols());
  int accumulator[(int)(accumulator_height*accumulator_width)];
  memset(accumulator, 0, (int)(accumulator_height*accumulator_width) * sizeof(int));
  double center_1 = input.rows()/2.0;
  double center_2 = input.cols()/2.0;
  for(int i=1;i<input.rows();i++) {
    for(int j=1;j<input.cols();j++) {
      if(input[i][j] > 200.0) {
        for(int k=0;k<180;k++) {
          double rho = (((double)i - center_1) * cos((double)k * M_PI/180.0)) + (((double)j - center_2) * sin((double)k * M_PI/180.0));
          accumulator[(int)((round(rho+hough_size)*180.0))+k] += 1;
        }
      }
    }
  }
  for(int u = 0;u<accumulator_height-1;u++) {
    for(int j = 0;j<accumulator_width-1;j++) {
      accu_image[u][j] = accumulator[(int)(u*accumulator_width)+j]*20;
    }
  }

  SImageIO::write_png_file("hough_space.png", accu_image, accu_image, accu_image);
  int prev = 0;
  for(int y=0;y<accumulator_height;y++) {
    for(int z=0;z<accumulator_width;z++) {
      if(accumulator[(int)(y*accumulator_width)+z] > 600) {
        int max_point = accumulator[(int)(y*accumulator_width)+z];
        for(int c=-4;c<=4;c++) {
          for (int d=-4;d<=4;d++) {
            if((c+y >= 0 && c+y < accumulator_height) && (d+z >= 0 && d+z < accumulator_width)) {
              if(accumulator[(int)((c+y)*accumulator_width) + (d+z)] > max_point) {
                max_point = accumulator[(int)((c+y)*accumulator_width) + (d+z)];
                d=c=5;
              }
            }
          }
        }
        if(max_point > accumulator[(int)(y*accumulator_width)+z])
          continue;

        int x_start,y_start,x_end,y_end;
        x_start=y_start=x_end=y_end=0;
        if(z>=45 && z<=135) {
          y_start = 0;
          y_end = input.rows();
          x_start = (int)((((double)y - accumulator_height/2.0) - (((double)x_start - accumulator_width/2.0) * cos((double)z * M_PI/180.0))) / sin((double)z * M_PI/180.0)) + (double)(input.cols()/2);
          x_end = (int)((((double)y - accumulator_height/2.0) - (((double)x_end - accumulator_width/2.0) * cos((double)z * M_PI/180.0))) / sin((double)z * M_PI/180.0)) + (double)(input.cols()/2);
        } else {
          x_start = 0;
          x_end = input.cols();
          y_start = (int)((((double)y - accumulator_height/2.0) - (((double)y_start - accumulator_width/2.0) * sin((double)z * M_PI/180.0))) / cos((double)z * M_PI/180.0)) + (double)(input.rows()/2);
          y_end = (int)((((double)y - accumulator_height/2.0) - (((double)y_end - accumulator_width/2.0) * sin((double)z * M_PI/180.0))) / cos((double)z * M_PI/180.0)) + (double)(input.rows()/2);
        }
        
        if((abs(y_start - y_end) < 2) && (y_start-prev >= 5)) {
          int dx = x_end-x_start;
          int dy = y_end-y_start;
          int diff = 2*dy - dx;
          int y_temp = y_start;
          if(y_start < input.rows() && x_start < input.cols() && y_start > 0 and x_start > 0) {
            input[y_start][x_start] = 255.0;
          }
          if(diff > 0) {
            y_temp = y_temp+1;
            diff = diff - (2*dx);
          }

          for(int x=x_start+1;x<x_end;x++) {
            if(y_temp < input.rows() && x < input.cols() && x >= 0 && y_temp > 0) {
              input[y_temp][x] = 255.0;
              diff = diff+(2*dy);
              if(diff > 0) {
                y_temp = y_temp+1;
                diff = diff - (2*dx);
              }
            }
          }
        prev = y_start;
        }
      }
    }
  }

  return input;
}

int find_ledger_diff (const SDoublePlane &image, int threshold) {
  std::vector<int> lines= find_lines(image, threshold);
  int count = 0;
  int diff = 0;
  int prev = 0;
  for(int i=0;i<image.rows();i++) {
    if(lines[i] > 0) {
      if(prev != 0 && i-prev < 50 && i-prev > 2) {
        diff += i-prev;
        count++;
      } 
      prev = i;
      if(count == 4) {
        break;
      }
    }
  }
  diff = diff/4;
  return diff;
}

SDoublePlane rescale_template (const SDoublePlane &input, double ratio, const char* name) {
  double image_ratio = (double)input.cols()/(double)input.rows();
  cout<<ratio<<"\n";
  int height = (int)((double)input.rows()*ratio);
  int width = (int)((double)input.cols()*ratio*image_ratio);
  SDoublePlane output(input.rows()+height, input.cols()+width);
  int k = 0;
  int l = 0;

  for(int i=0;i<input.rows();i++) {
    l = 0;
    if((i+k)%(input.rows()/height) == 0 && i != 0 && k<height) {
      for(int j=0;j<input.cols();j++) {
        output[i+k][j+l] = input[i][j];
        if((l+j)%(input.cols()/width) == 0 && j != 0 && l<width) {
          (ratio > 0.0) ? l++ : l--;
        }
        output[i+k][j+l] = input[i][j];
      }
      (ratio > 0.0) ? k++ : k--;
      l=0;
    }
    for(int j=0;j<input.cols();j++) {
      output[i+k][j+l] = input[i][j];
      if((l+j)%(input.cols()/width) == 0 && j != 0 && l<width) {
        (ratio > 0.0) ? l++ : l--;
      }
      output[i+k][j+l] = input[i][j];
    }
  }

  SImageIO::write_png_file(name, output, output, output);
  return output;
}

char detect_pitch (const SDoublePlane &input, int row, int col, std::vector<int> lines, int diff) {
  char result;
  int staff_diff = 100;
  for(int i=row-(diff*2);i<row+(diff*2);i++) {
    if(lines[i] > 0) {
      if(abs(i-row) < 2) {
        switch(lines[i]%5) {
          case 1:
            if(staff_diff>abs(i-row)) {
              result = 'F';
              staff_diff = abs(i-row);
            }
          case 2:
            if(staff_diff>abs(i-row)) {
              result = 'D';
              staff_diff = abs(i-row);
            }
          case 3:
            if(staff_diff>abs(i-row)) {
              result = 'B';
              staff_diff = abs(i-row);
            }
          case 4:
            if(staff_diff>abs(i-row)) {
              result = 'G';
              staff_diff = abs(i-row);
            }
          case 0:
            if(staff_diff>abs(i-row)) {
              result = 'E';
              staff_diff = abs(i-row);
            }
        } 
      }
      if(abs(i-row) < diff/2) {
        switch(lines[i]%5) {
          case 1:
            if(staff_diff>abs(i-row)) {
              result = 'E';
              staff_diff = abs(i-row);
            }
          case 2:
            if(staff_diff>abs(i-row)) {
              result = 'C';
              staff_diff = abs(i-row);
            }
          case 3:
            if(staff_diff>abs(i-row)) {
              result = 'A';
              staff_diff = abs(i-row);
            }
          case 4:
            if(staff_diff>abs(i-row)) {
              result = 'F';
              staff_diff = abs(i-row);
            }
          case 0:
            if(staff_diff>abs(i-row)) {
              result = 'D';
              staff_diff = abs(i-row);
            }
        }
      }
    }
  }
  return result;
}

// Changed - Aniket 02-8-16
SDoublePlane handle_edge_case(SDoublePlane &input_image,SDoublePlane &template_image){
	int image_row,image_col,template_row,template_col;
	template_row=template_image.rows();
	template_col=template_image.cols();
	image_row=input_image.rows();
	image_col=input_image.cols();
	
	for(int i=1;i<template_row;i++){
		if(image_row%template_row==0) break;
		image_row+=1; 	
	}

	for(int i=1;i<template_col;i++){
		if(image_col%template_col==0) break;
		image_col+=1; 	
	}	
	//cout<<"\n Template : "<<template_image.rows()<<" : "<<template_image.cols();
	//cout<<"\n Old Image : "<<input_image.rows()<<" : "<<input_image.cols();
	//cout<<"\n New Image : "<<image_row<<" : "<<image_col<<endl;

	// Allocate size for new image
	SDoublePlane new_image=SDoublePlane(image_row,image_col);
	for(int i=0;i<input_image.rows();i++){
		for(int j=0;j<input_image.cols();j++){
			new_image[i][j]=input_image[i][j];		
		}		
	}
	
	
        // Fill empty rows 
	for(int i=input_image.rows();i<new_image.rows();i++){
		for(int j=0;j<input_image.cols();j++){
			
			new_image[i][j]=input_image[input_image.rows()-1][j];		
		}
	}
	
	

	// Fill empty cols
	for(int j=input_image.cols();j<new_image.cols();j++){
		//cout<<"\n j : "<<j<<endl;
        	for(int i=0;i<new_image.rows();i++){
			//cout<<"\n i : "<<i<<endl;
			new_image[i][j]=new_image[i][input_image.cols()-1];
		}
        }
	
	//cout<<"\n here"<<endl;
	
	//cout<<"\n Template : "<<template_image.rows()<<" : "<<template_image.cols();
	//cout<<"\n Old Image : "<<input_image.rows()<<" : "<<input_image.cols();
	//cout<<"\n New Image : "<<image_row<<" : "<<image_col;


	return new_image;

}


char detect_note(int j){
	char result;
	if(j==0) result='A';
	else if(j==1) result='B';
	else if(j==2||j==7) result='C';
	else if(j==3||j==8) result='D';
	else if(j==4||j==9) result='E';
	else if(j==5||j==10) result='F';
	else if(j==6) result='G';
	else result='X';
	return result;
}

//
// This main file just outputs a few test images. You'll want to change it to do 
//  something more interesting!
//
int main(int argc, char *argv[])
{
  if(!(argc >= 2))
    {
      cerr << "usage: " << argv[0] << " input_image" << endl;
      return 1;
    }

  string input_filename(argv[1]);
  
  // Changes made : Aniket

string template_A("A.PNG");
string template_B("B.PNG");
string template_C("C.PNG");
string template_D("D.PNG");
string template_E("E.PNG");
string template_F("F.PNG");
string template_G("G.PNG");
string template_C1("C_1.PNG");
string template_D1("D_1.PNG");
string template_E1("E_1.PNG");
string template_F1("F_1.PNG");

SDoublePlane template_A_image= SImageIO::read_png_file(template_A.c_str());
SDoublePlane template_B_image= SImageIO::read_png_file(template_B.c_str());
SDoublePlane template_C_image= SImageIO::read_png_file(template_C.c_str());
SDoublePlane template_D_image= SImageIO::read_png_file(template_D.c_str());
SDoublePlane template_E_image= SImageIO::read_png_file(template_E.c_str());
SDoublePlane template_F_image= SImageIO::read_png_file(template_F.c_str());
SDoublePlane template_G_image= SImageIO::read_png_file(template_G.c_str());
SDoublePlane template_C1_image= SImageIO::read_png_file(template_C1.c_str());
SDoublePlane template_D1_image= SImageIO::read_png_file(template_D1.c_str());
SDoublePlane template_E1_image= SImageIO::read_png_file(template_E1.c_str());
SDoublePlane template_F1_image= SImageIO::read_png_file(template_F1.c_str());
  
SDoublePlane input_image= SImageIO::read_png_file(input_filename.c_str());
/*
SDoublePlane new_image1=handle_edge_case(input_image,template_A_image);
SDoublePlane new_image1=handle_edge_case(input_image,template_A_image);
SDoublePlane new_image1=handle_edge_case(input_image,template_A_image);
SDoublePlane new_image1=handle_edge_case(input_image,template_A_image);
SDoublePlane new_image1=handle_edge_case(input_image,template_A_image);
SDoublePlane new_image1=handle_edge_case(input_image,template_A_image);
SDoublePlane new_image1=handle_edge_case(input_image,template_A_image);
SDoublePlane new_image1=handle_edge_case(input_image,template_A_image);
SDoublePlane new_image1=handle_edge_case(input_image,template_A_image);
SDoublePlane new_image1=handle_edge_case(input_image,template_A_image);
SDoublePlane new_image1=handle_edge_case(input_image,template_A_image);
*/
std::vector<int> lines = find_lines(input_image, 150.0);
int input_ledger_diff = find_ledger_diff(input_image, 150.0);
cout<<input_ledger_diff<<" this is the diff \n";
SDoublePlane scaled_template;
  if(abs(input_ledger_diff-11) >= 2) {
    template_A_image = rescale_template(template_A_image, (double)(input_ledger_diff-11.0)/11.0, "scaled_template_A.png");
    template_B_image = rescale_template(template_B_image, (double)(input_ledger_diff-11.0)/11.0, "scaled_template_B.png");
    template_C_image = rescale_template(template_C_image, (double)(input_ledger_diff-11.0)/11.0, "scaled_template_C.png");
    template_D_image = rescale_template(template_D_image, (double)(input_ledger_diff-11.0)/11.0, "scaled_template_D.png");
    template_E_image = rescale_template(template_E_image, (double)(input_ledger_diff-11.0)/11.0, "scaled_template_E.png");
    template_F_image = rescale_template(template_F_image, (double)(input_ledger_diff-11.0)/11.0, "scaled_template_F.png");
    template_G_image = rescale_template(template_G_image, (double)(input_ledger_diff-11.0)/11.0, "scaled_template_G.png");
    template_C1_image = rescale_template(template_C1_image, (double)(input_ledger_diff-11.0)/11.0, "scaled_template_C1.png");
    template_D1_image = rescale_template(template_D1_image, (double)(input_ledger_diff-11.0)/11.0, "scaled_template_D1.png");
    template_E1_image = rescale_template(template_E1_image, (double)(input_ledger_diff-11.0)/11.0, "scaled_template_E1.png");
    template_F1_image = rescale_template(template_F1_image, (double)(input_ledger_diff-11.0)/11.0, "scaled_template_F1.png");
  }
  
// Changes made : Aniket
  double threshold=0.83;
  int cnt;
  std::pair<int**,int> results[11] = {question_4(input_image, template_G_image,threshold)};/*,
				     question_4(input_image, template_B_image,threshold),
					question_4(input_image, template_C_image,threshold),
					question_4(input_image, template_D_image,threshold),
					question_4(input_image, template_E_image,threshold),
					question_4(input_image, template_F_image,threshold),
					question_4(input_image, template_G_image,threshold),
					question_4(input_image, template_C1_image,threshold),
					question_4(input_image, template_D1_image,threshold),
					question_4(input_image, template_E1_image,threshold),
					question_4(input_image, template_F1_image,threshold)};*/
 // test step 2 by applying mean filters to the input image
  
  SDoublePlane mean_filter(3,3);

  //SDoublePlane output_image = sobel_gradient_filter(convolve_general(input_image, gaussian(mean_filter, 20.0)),true);
 SDoublePlane output_image = image_filter(input_image, convolve_general(input_image, gaussian(mean_filter, 20.0)));
  cout << "before symbols" << endl;
vector<DetectedSymbol> symbols;
  for(int j=0;j<1;j++) {
     cout<<"\n J : "<<j<<endl;
    int** matched=results[j].first;
    for(int i=0; i<results[j].second; i++)
      {
        DetectedSymbol s;
        s.row = matched[i][0];
        s.col = matched[i][1];
        s.width = 10;
        s.height = 10;
        s.type = (Type) 2;
        s.confidence = rand();
        s.pitch = detect_note(j);
        symbols.push_back(s);
      }
  }

  write_detection_txt("detected.txt", symbols);
  write_detection_image("detected.png", symbols, output_image);
}
