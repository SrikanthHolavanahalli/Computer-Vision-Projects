//Baseline.h
#include <iostream>
#include <fstream>
class Baseline : public Classifier
{	
	public:
  		Baseline(const vector<string> &_class_list) : Classifier(_class_list) {
  			prepareMap();
  		}
		static const int size=40;  // subsampled image resolution
		map<string, int > className_ClassValue_map;
		map<int, string > classValue_className_map;
		
		// extract features from an image, which in this case just involves resampling and rearranging into a vector of pixel data.
		CImg<double> extract_features(const string &filename)
		{
		  return (convertToGreyScale(CImg<double>(filename.c_str()))).resize(size,size,1,1).unroll('x'); // Grey Scale image
// 	  	  return (CImg<double>(filename.c_str())).resize(size,size,1,3).unroll('x'); // Colored image 
		  
		}
		
	
		//Prepare a mapping between class name (folder name) and numeric id (class label for training and testing SVM) and vice-versa
		void prepareMap(){
			
			int p=0;
			for(int c=0; c <class_list.size(); c++)
				className_ClassValue_map[class_list[c] ] = ++p;
			p=0;
			for(int j=0;j<class_list.size();j++)
				classValue_className_map[++p]=class_list[j];
			
		}
		
		// Convert the colored image to grey scale
		CImg<double> convertToGreyScale(CImg<double> image) {
			double R, G, B,mean;
			for (int c=0; c<image.width() ; c++)
				for (int r=0; r<image.height() ; r++) {
					R = image(c,r,0,0);                         
					G = image(c,r,0,1);                         
					B = image(c,r,2);                           
					mean = (R + G + B)/3;                  
					image(c,r,0) = image(c,r,1) = image(c,r,2) = mean; 
				}
			return image;
		}
		
		// This method performs the PCA algorithm , prepare the training file and runs SVM on it
		virtual void train(const Dataset &filenames) 
		{	
			ofstream SVM_trainFile ("train_baseline.dat");
			
			for(Dataset::const_iterator c_iter=filenames.begin(); c_iter != filenames.end(); ++c_iter)
			{
				cout << "Processing " << c_iter->first << endl;
				// convert each image to be a row of this "model" image
				for(int i=0; i<c_iter->second.size(); i++){
					CImg<double> imageInARow = extract_features(c_iter->second[i].c_str());
					SVM_trainFile << className_ClassValue_map[c_iter->first];
					int x=1;
					for(int k=0;k<imageInARow.width();k++){
						SVM_trainFile << " " << x <<":"<< imageInARow(k,0);
						x++;
					}
					SVM_trainFile<<endl;
				}
			}
			system("./svm_multiclass/svm_multiclass_learn -c 1.0  train_baseline.dat model_file_baseline");
		}
	
		// This method prepares the test file for the the test image and runs SVM on it to predict its class
		virtual string classify(const string &filename)
		  {

			CImg<double> test_image = extract_features(filename);
			ofstream SVM_testFile ("test_baseline.dat");
			SVM_testFile << 2;
			int x=1;
			for(int k=0;k<test_image.width();k++){
				SVM_testFile <<  " " << x <<":"<< test_image(k,0);
				x++;
			}
			SVM_testFile<<endl;
		
			system("./svm_multiclass/svm_multiclass_classify  test_baseline.dat model_file_baseline output_file_baseline");
			string outputClass;
			string line;
			ifstream output ("output_file_baseline");
			if (output.is_open())
			{
				if ( getline (output,line) ){
					outputClass=  line.substr(0,line.find(" "));
					cout << "Predicted class : "<<classValue_className_map[atoi(outputClass.c_str())]<<endl;;
					return classValue_className_map[atoi(outputClass.c_str())];
					
				}
			}
			output.close();
			return "-1"; 
		}  
	
		virtual void load_model()
		{
		
		}
  };