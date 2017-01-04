// reffered the blog http://www.keymolen.com/2013/05/hough-transformation-c-implementation.html
// for key concepts in implementing the hough transform. I have mentioned the parts which have
// been referred further below

void hough_transform(const SDoublePlane &input, std::vector<int>& hough_lines, const SDoublePlane actual_input) {
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
      hough_lines[i] = 0;
      output[i][j] = actual_input[i][j];
      if(input[i][j] > 200.0) {
        for(int k=0;k<180;k++) {
          // This formula for calculating rho was taken from the blog
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
  int hough_count = 0;
  for(int y=0;y<accumulator_height;y++) {
    for(int z=0;z<accumulator_width;z++) {
      if(accumulator[(int)(y*accumulator_width)+z] > 600) {
        int max_point = accumulator[(int)(y*accumulator_width)+z];
        // This concept of determining local maxima was used from 
        // the blog.
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

        // This part of setting the rho threshold values to 45 and 135 and the formula 
        // for obtaining the x1,y1 and x2,y2 co-ordinates have been obtained from the 
        // above mentioned reference

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

        // Start to plot all the points into lines.
        // The plotting is restricted to staff co-ordinates. This serves as
        // the next voting for the accumulator. Bresenham's line algorithm
        // is used to plot the line.

        if((abs(y_start - y_end) < 2) && (y_start-prev >= 5) && (abs(y_start-prev) <= 50 || hough_count%5 == 0)) {
          int dx = x_end-x_start;
          int dy = y_end-y_start;
          int diff = 2*dy - dx;
          int y_temp = y_start;
          prev = y_start;
          hough_count++;
          hough_lines[y_start] = hough_count;
          if(y_start < input.rows() && x_start < input.cols() && y_start > 0 and x_start > 0) {
            output[y_start][x_start] = 255.0;
            actual_input[y_start][x_start] = 0.0;
          }
          if(diff > 0) {
            y_temp = y_temp+1;
            diff = diff - (2*dx);
          }

          for(int x=x_start+1;x<x_end;x++) {
            if(y_temp < input.rows() && x < input.cols() && x >= 0 && y_temp > 0) {
              output[y_temp][x] = 255.0;
              actual_input[y_temp][x] = 0.0;
              diff = diff+(2*dy);
              if(diff > 0) {
                y_temp = y_temp+1;
                diff = diff - (2*dx);
              }
            }
          }
        }
      }
    }
  }
  SImageIO::write_png_file("staves.png", actual_input, actual_input, output);
}