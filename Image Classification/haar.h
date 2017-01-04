#include <iostream>
#include <fstream>
class Haar : public Classifier
{	
	public:
  		Haar(const vector<string> &_class_list) : Classifier(_class_list) {
  			prepareMap();
			prepareRectangleFeatures();
  		}
		static const int size=40;  // subsampled image resolution
		static const int numberOfFeatures = 4500;  // number of 2-rectangle features to be genarated
		map<string, int > className_ClassValue_map;
		map<int, string > classValue_className_map;
		
		int rectangleFeatures[numberOfFeatures][5];
		
		// extract features from an image, which in this case just involves resampling and 
		// rearranging into a vector of pixel data.
		CImg<double> extract_features(const string &filename)
		{
		  return (CImg<double>(filename.c_str())).get_RGBtoYCbCr().get_channel(0).resize(size,size,1,1);
		}
		
		// following method creates specified number of rectangles 
		void prepareRectangleFeatures()
		{
		
			for(int i =0; i < numberOfFeatures; i++)
			{		
				
			    rectangleFeatures[i][0] = rand()%40;   // yPixel
				rectangleFeatures[i][1] = rand()%40;   // xPixel
				rectangleFeatures[i][2] = rand()%8;    // width
				rectangleFeatures[i][3] = rand()%6;    // height
			}	
			   
		}
		
		// Maps the classes to a corresponding number which is used in train.dat
		void prepareMap(){
			
			int p=0;
			for(int c=0; c <class_list.size(); c++)
				className_ClassValue_map[class_list[c] ] = ++p;
			p=0;
			for(int j=0;j<class_list.size();j++)
				classValue_className_map[++p]=class_list[j];
			
		}
		
		
		// trains the SVM with the features retrived from the input images by preparin train.dat
		virtual void train(const Dataset &filenames) 
		{	
			ofstream SVM_trainFile ("train_haar.dat");			
			for(Dataset::const_iterator c_iter=filenames.begin(); c_iter != filenames.end(); ++c_iter)
			{
				cout << "Processing " << c_iter->first << endl;
				for(int i=0; i<c_iter->second.size(); i++)
                                {
					CImg<double> imageInAMatrix = extract_features(c_iter->second[i].c_str());
					SVM_trainFile << className_ClassValue_map[c_iter->first];
					int x=1;
					for(int k=0;k<numberOfFeatures;k++)
					{ 
					if ( calculateIntegralImageValues(k, imageInAMatrix) != 0)
					SVM_trainFile << " " << x <<":"<< calculateIntegralImageValues(k,imageInAMatrix);
					x++;
					}
					SVM_trainFile<<endl;
				}
			}
			system("./svm_multiclass/svm_multiclass_learn -c 1.0  train_haar.dat model_file_haar");
		}
	
		//following method calculates integral image values of the images with rectangular features
		int calculateIntegralImageValues(int k, CImg<double> imageInAMatrix )
		{
			float integralImageValueWhite = 0;
			float integralImageValueGrey  = 0;
			
					
			    int xPixel = rectangleFeatures[k][0];
			    int yPixel = rectangleFeatures[k][1];
                            int width  = rectangleFeatures[k][2];
                            int height = rectangleFeatures[k][3];
			
				for(int l=0; l < width  ; l++)
				{
					for(int m = 0; m < height ; m++)
					{
						if((xPixel+ l) <= size && (yPixel + m) <= size)
						{
                                                integralImageValueWhite += imageInAMatrix( xPixel + l, yPixel + m );
                                                integralImageValueGrey  += imageInAMatrix( xPixel + height + l, yPixel + m );
					    }
                    }
				}		
			return (integralImageValueWhite - integralImageValueGrey);		
		}
		
	
		//classifies the test images using the model file generated
		virtual string classify(const string &filename)
		  {

			CImg<double> test_image = extract_features(filename);
			ofstream SVM_testFile ("test_haar.dat");
			SVM_testFile << 2;
			int x=1;
			for(int k=0;k<numberOfFeatures;k++)
			{
		                if(calculateIntegralImageValues(k,test_image)!=0)
				SVM_testFile << " " << x <<":"<< calculateIntegralImageValues(k,test_image);
				x++;
			}
			SVM_testFile<<endl;
		
			system("./svm_multiclass/svm_multiclass_classify test_haar.dat model_file_haar output_file_haar");
			string outputClass;
			string line;
			ifstream output ("output_file_haar");
			if (output.is_open())
			{
				if ( getline (output,line) ){
					outputClass=  line.substr(0,line.find(" "));
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

