// Skeleton code for B657 A4 Part 1.
// D. Crandall
//
#include <vector>
#include <iostream>
#include <fstream>
#include <map>
#include <math.h>
#include <CImg.h>
#include <assert.h>

using namespace cimg_library;
using namespace std;

int main(int argc, char *argv[])
{
  if(argc != 3)
    {
      cerr << "usage: " << argv[0] << " image_file disp_file" << endl;
      return 1;
    }

  string input_filename1 = argv[1], input_filename2 = argv[2];

  // read in images and gt
  CImg<double> image_rgb(input_filename1.c_str());
  CImg<double> image_disp(input_filename2.c_str());

  CImg<double> image_result =image_rgb;
  CImg<double> image_right_stereo = image_rgb ;
	
  //CImg<double> image_result       = image_rgb;
  //CImg<double> image_red_channel  = image_rgb;
  //CImg<double> image_cyan_channel = image_rgb;
  //CImg<double> image_left_stereo  = image_rgb;
  //CImg<double> image_right_stereo = image_rgb;

  double constant_add_factor = 0;
  double constant_multiply_factor = 20;

//creating left and right stereo images

  
 for(int i = 0; i < image_rgb.width() ; i++)
 {
 	for(int j = 0 ; j < image_rgb.height() ; j++)
        {
		 
		  //image_left_stereo(i + horizontal_shift_size ,j,0,0)  = image_rgb(i,j,0,0) ; 
		  //image_left_stereo(i + horizontal_shift_size ,j,0,1)  = image_rgb(i,j,0,1) ;
		  //image_left_stereo(i + horizontal_shift_size ,j,0,2)  = image_rgb(i,j,0,2) ;
		  
		  //constant_shift_factor = float(image_disp(i,j) * constant_multiply_factor) ;
		  
		  
		  //cout << float(image_disp(i,j))<< " ";
		  
		  constant_add_factor = int(image_disp(i,j)/255 * constant_multiply_factor);
		  //constant_add_factor = 10;
		  
		  //image_right_stereo(i,j) =    image_rgb(i + constant_add_factor , j)  ;
		    
		  image_right_stereo(i,j,0,0) = image_rgb(i + constant_add_factor , j,0,0)   ; 	
		  image_right_stereo(i,j,0,1) = image_rgb(i + constant_add_factor , j,0,1)   ; 
		  image_right_stereo(i,j,0,2) = image_rgb(i + constant_add_factor , j,0,2)   ;   
		        
        }

 }
 
 //image_left_stereo.save("left_stereo.png");
 image_right_stereo.save("right_stereo.png");


  for(double i = 0; i < image_rgb.width() ; i++)
{
	for(double  j = 0 ; j < image_rgb.height() ; j++)
	{	
		//image_red_channel(i,j,0,0) = image_right_stereo(i,j,0,0);
        //image_red_channel(i,j,0,1) = 0;
        //image_red_channel(i,j,0,2) = 0;
		
        //image_cyan_channel(i,j,0,2) = image_left_stereo(i,j,0,2);
		//image_cyan_channel(i,j,0,1) = image_left_stereo(i,j,0,1);
        //image_cyan_channel(i,j,0,0) = 0;
        
        
        image_result(i,j,0,0) = image_rgb(i,j,0,0);
        image_result(i,j,0,1) = image_right_stereo(i,j,0,1);
        image_result(i,j,0,2) = image_right_stereo(i,j,0,2);
               // cout << image_rgb(i,j);
               // int dummy;
        	//cin >> dummy;
	}
       //	image_result[i][j] = image_rgb(i,j,0,0);  
}


  //image_rgb.draw_image(0,0,0,image_disp,100);
  image_result.get_normalize(0,255).save((input_filename1 + "-stereogram.png").c_str());
  
  //image_red_channel.save("red.png");
  //image_cyan_channel.save("cyan.png");
  
  //image_red_channel.draw_image(0,0,0,image_cyan_channel,100);
  //image_rgb.draw_image(0,0,0,image_red_channel,100);
  
  //image_rgb.get_normalize(0,255).save("image_result.png"); 
  //image_red_channel.get_normalize(0,255).save("red_result.png"); 
  
  
  image_result.save("result.png");
  //image_result.get_normalize(0,255).save("result.png");
  return 0;
}
