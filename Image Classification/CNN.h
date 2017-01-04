#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cstddef>
#include <iterator>
class CNN : public Classifier
{	
	public:
  		CNN(const vector<string> &_class_list) : Classifier(_class_list) {
  			prepareMap();
			
  		}
		static const int size=231;  // subsampled image resolution
		
		map<string, int > className_classValue_map;
		map<int, string > classValue_className_map;

		// extract features from an image, which in this case just involves resampling and 
		// rearranging into a vector of pixel data.
		CImg<double> extract_features(const string &filename)
		{
		  return (CImg<double>(filename.c_str())).resize(size,size,1,1);
		}

		
		// Maps the classes to a corresponding number which is used in train.dat
		void prepareMap(){
			
			int p=0;
			for(int c=0; c <class_list.size(); c++)
				className_classValue_map[class_list[c] ] = ++p;
			p=0;
			for(int j=0;j<class_list.size();j++)
				classValue_className_map[++p]=class_list[j];
			
		}
		
		// trains the SVM with the features retrived from the input images by preparing train.dat
		virtual void train(const Dataset &filenames) 
		{	
		
			for(Dataset::const_iterator c_iter=filenames.begin(); c_iter != filenames.end(); ++c_iter)
			{
				cout << "Processing " << c_iter->first << endl;
                                
				// convert each image to be a row of this "model" image
				for(int i=0; i<c_iter->second.size(); i++)
                {
					
					CImg<double> imageInAMatrix = extract_features(c_iter->second[i].c_str());
				
					imageInAMatrix.save("file.bmp");		                        
                                        
					ifstream myfile("file.bmp");
                  
					int val = className_classValue_map[c_iter->first]; 
					std::string s = "echo " + std::to_string(val) + " > overfeat_results1.txt"  ;
					system(s.c_str());				
					system("./overfeat/bin/linux_64/overfeat -L 18 ./file.bmp >> overfeat_results1.txt");
                    prepareFileForSVMTrain(); 
				}		
			 }	
			system("./svm_multiclass/svm_multiclass_learn -c 1.0  train_deep.dat model_file_deep");	
		}	
		
		
		
		//Converts the output of CNN to the format required by SVM for train images
		virtual void prepareFileForSVMTrain()
		{
			ofstream SVM_trainFile("train_deep.dat",ios::app);	
			ifstream results("overfeat_results1.txt");
			int x=1;
			int dim = 1;
				if(results.is_open())
				{
					int line_num = 1;
					int even = 1;
					std::string line;
                    int prevSize = 0;
	
					while(getline(results,line))
					{
						std::istringstream iss(line);
						
						if(prevSize != line.size())
						{
				
							if(line.size() <= 2)
						    { 
								SVM_trainFile << line << " ";
							}
							else if(line.size() < 20 && line.size() > 2)
							{
								int n , h, w;
								iss >> n >> h >> w;
								dim = h*w;
								cout << dim ;
							}
							else{
								int x = 1;
                                int y = 1;
								// referred from  www.cplusplus.com/forum/beginner/87238
								std::istream_iterator<std::string> beg(iss), end;
								std::vector<std::string> tokens(beg,end);
								
								for(auto s:tokens)
								{       
										
                                    if (s != std::to_string(0) && ( y%dim==1 || dim == 1 ) )
									{
									   SVM_trainFile <<" "<< x << ":" <<  s;	
									}
									x++;
									y++;
								}
									SVM_trainFile << '\n';	
							}                                   
							prevSize = line.size();
						}                   
					}
				}
		}
		
		
		//Converts the output of CNN to the format required by SVM for test images
	virtual void prepareFileForSVMTest()
		{
			ofstream SVM_testFile ("test_deep.dat");
			ifstream results("overfeat_results_test.txt");
			int x=1;
			int dim = 1;
				if(results.is_open())
				{
					int line_num = 1;
					int even = 1;
					std::string line;
                                        int prevSize = 0;
					while(getline(results,line))
					{
						std::istringstream iss(line);
						
						if(prevSize != line.size())
						{
							if(line.size() <= 2)
							{
						
								SVM_testFile << line << " ";
							}
							else if(line.size() < 20 && line.size() > 2)
							{
								
								int n , h, w;
								
								iss >> n >> h >> w;
								
								dim = h*w;
							 	
							}
							else{
								int x = 1;
								int y = 1;
								// referred from  www.cplusplus.com/forum/beginner/87238
								std::istream_iterator<std::string> beg(iss), end;
								std::vector<std::string> tokens(beg,end);
								
								for(auto s:tokens)
								{       
										
                                    if (s != std::to_string(0) &&( y % dim ==1 || dim == 1 ))
									{
											SVM_testFile <<" "<< x << ":" <<  s;							   
                                    }
									x++;
									y++;
								}
									SVM_testFile << '\n';	
							}       
							prevSize = line.size();
						}                   
					}
				}
		}
	
		
		//classifies the test images using the model file generated
		virtual string classify(const string &filename)
		  {
// 		  	
			CImg<double> test_image = extract_features(filename);
			ofstream SVM_testFile("test_deep.dat");
			
			ofstream results("overfeat_results_test.txt");
			
			
			test_image.save("file.bmp");		                                                  
		    ifstream myfile("file.bmp");
            
			int val = 2; 
			std::string s = "echo " + std::to_string(val) + " > overfeat_results_test.txt"  ;
			system(s.c_str());				
			system("./overfeat/bin/linux_64/overfeat -L 18 ./file.bmp >> overfeat_results_test.txt");
			
			prepareFileForSVMTest();
		
			system("./svm_multiclass/svm_multiclass_classify  test_deep.dat model_file_deep output_file_deep");
			string outputClass;
			string line;
			ifstream output ("output_file_deep");
			if (output.is_open())
			{
				if ( getline (output,line) ){
					outputClass=  line.substr(0,line.find(" "));
					return classValue_className_map[atoi(outputClass.c_str())];
					
				}
			}
			
			output.close();
			SVM_testFile.close();
			return "-1"; 
		}  
	
		virtual void load_model()
		{
		
		}
  
  };

