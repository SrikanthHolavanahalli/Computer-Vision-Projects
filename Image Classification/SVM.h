//SVM.h
#include <iostream>
#include <fstream>
class SVMClassifier : public Classifier
{	
	public:
  		SVMClassifier(const vector<string> &_class_list) : Classifier(_class_list) {
  			prepareMap();
  		}
		static const int size=40;  // subsampled image resolution
		map<string, int > className_ClassValue_map;
		map<int, string > classValue_className_map;
		
		// extract features from an image, which in this case just involves resampling and 
		// rearranging into a vector of pixel data.
		CImg<double> extract_features(const string &filename)
		{
// 		  cout<<"Image size : "<<imageInARow.width()<<endl;
		  return (CImg<double>(filename.c_str())).get_RGBtoYCbCr().get_channel(0).resize(size,size,1,1).unroll('x');
		}
	
		void prepareMap(){
			
			int p=0;
			for(int c=0; c <class_list.size(); c++)
				className_ClassValue_map[class_list[c] ] = ++p;
			p=0;
			for(int j=0;j<class_list.size();j++)
				classValue_className_map[++p]=class_list[j];
			
		}
		virtual void train(const Dataset &filenames) 
		{	
// 			prepareMap();
			// cout<<classValue_className_map[1]<<endl;
			ofstream SVM_trainFile ("/u/bde/jsureshk-bde-sriksrin-a3/svm_multiclass/train.dat");
// 			CImg<double> imageInARow(size*size*3,1,1);
			for(Dataset::const_iterator c_iter=filenames.begin(); c_iter != filenames.end(); ++c_iter)
			{
				cout << "Processing " << c_iter->first << endl;

				// convert each image to be a row of this "model" image
				for(int i=0; i<c_iter->second.size(); i++){
					CImg<double> imageInARow = extract_features(c_iter->second[i].c_str());
					// cout<<"Image size : "<<imageInARow.width()<<endl;
					SVM_trainFile << className_ClassValue_map[c_iter->first];
					int x=1;
					for(int k=0;k<imageInARow.width();k++){
						SVM_trainFile << " " << x <<":"<< imageInARow(k,0);
						x++;
					}
					SVM_trainFile<<endl;
				}
	
			
			}
			system("./svm_multiclass/svm_multiclass_learn -c 1.0 -v 3 /u/bde/jsureshk-bde-sriksrin-a3/svm_multiclass/train.dat /u/bde/jsureshk-bde-sriksrin-a3/svm_multiclass/model_file");
		}
	
	
		virtual string classify(const string &filename)
		  {
// 		  	cout<<"Bipra"<<classValue_className_map[atoi("1")]<<endl;
			CImg<double> test_image = extract_features(filename);
			ofstream SVM_testFile ("/u/bde/jsureshk-bde-sriksrin-a3/svm_multiclass/test.dat");
			SVM_testFile << 2;
			int x=1;
			for(int k=0;k<test_image.width();k++){
				SVM_testFile <<  " " << x <<":"<< test_image(k,0);
				x++;
			}
			SVM_testFile<<endl;
		
			system("./svm_multiclass/svm_multiclass_classify -v 3 /u/bde/jsureshk-bde-sriksrin-a3/svm_multiclass/test.dat /u/bde/jsureshk-bde-sriksrin-a3/svm_multiclass/model_file /u/bde/jsureshk-bde-sriksrin-a3/svm_multiclass/output_file");
			string outputClass;
			string line;
			ifstream output ("/u/bde/jsureshk-bde-sriksrin-a3/svm_multiclass/output_file");
			if (output.is_open())
			{
				if ( getline (output,line) ){
					outputClass=  line.substr(0,line.find(" "));
					cout << "Predicted class : "<<classValue_className_map[atoi(outputClass.c_str())]<<endl;;
					return classValue_className_map[atoi(outputClass.c_str())];
					
				}
			}
			// else
// 				cout<<"Output file not found"<<endl;
			output.close();
			return "-1"; 
		}  
	
		virtual void load_model()
		{
		
		}
  
  };