
SDoublePlane image_filter(const SDoublePlane &input, const SDoublePlane &blurred) {
  SDoublePlane output(input.rows(), input.cols());
  for(int i=0;i<input.rows();i++) {
    for(int j=0;j<input.cols();j++) {
      output[i][j] = (input[i][j] - blurred[i][j] < 10.0) ? 0.0 : 255.0;
    }
  }
  return output;
}

std::vector<int> find_lines(const SDoublePlane &input, double threshold, double fill=0.6) {
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
    if(((double)count)/((double)input.cols()) >= fill) {
      if(i-prev >= 4) {
        ledgers[i] = line_count;
        prev = i;
        line_count++;
      }
    }
  }
  return ledgers;
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
  cout<<"Rescale by ratio"<<"..... \n";
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
  char result = 'A';
  int staff_diff = 100;
  for(int i=row-(diff);i<row+(diff);i++) {
    if(lines[i] > 0) {
      if(abs(i-row) < 3) {
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
      if(abs(i-row) < diff) {
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
      if((lines[i]%5 == 1 || lines[i]%5 == 0) and abs(i-row) <= diff+5) {
        if(staff_diff>abs(i-row)) {
          result = 'C';
          staff_diff = abs(i-row);
        }
      }
    }
  }
  return result;
}

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

  return new_image;

}

void detect_and_print(std::pair<int**,int>* results, const SDoublePlane* template_image, const SDoublePlane hough, std::vector<int> lines, int diff, string filename, double conf = 0.60) {
  vector<DetectedSymbol> symbols;
  for(int j=0;j<3;j++) {
    int** matched=results[j].first;
    for(int i=0; i<results[j].second; i++)
      {
        DetectedSymbol s;
        s.row = matched[i][0];
        s.col = matched[i][1];
        s.width = template_image[j].rows();
        s.height = template_image[j].cols();
        s.type = (Type) (j);  
        s.confidence = conf;
        s.pitch = detect_pitch(hough, matched[i][0], matched[i][1], lines, diff);
        symbols.push_back(s);
      }
  }
  write_detection_image(filename, symbols, hough);
  write_detection_txt(filename, symbols);
}


void write_text_file(SDoublePlane input,int template_no){
	string template_name;	
	if(template_no==1){
		template_name="After convolution with Template_1";		
	}
	else if(template_no==2){
		template_name="After convolution with Template_2";		
	}
	else {
		template_name="After convolution with Template_3";		
	}
	
	std::ofstream outfile;
	outfile.open("scores4.txt", std::ios_base::app);
	outfile<< template_name<<"\n";
	// Scan matrix and add to file
	for(int i=0;i<input.rows();i++){
		//std::stringstream os=NULL;
		for(int j=0;j<input.cols();j++){
			//cout << "here" <<endl;
			//os<<"i : "<<i<<" j : "<<j<<" Hamming distance score : "<<input[i][j]<<"\n";
			//outfile<<os.str();
			outfile<<"i : "<<i<<" j : "<<j<<" Hamming distance score : "<<input[i][j]<<"\n";			
		}	
	}
	outfile.close();
}


