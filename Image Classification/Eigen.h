//Eigen.h
#include <iostream>
#include <fstream>
#include <sstream>
class Eigen : public Classifier
{	
	public:
  		Eigen(const vector<string> &_class_list) : Classifier(_class_list) {
  			prepareMap();
  		}
		static const int size=40;  // subsampled image resolution
		map<int, string > classValue_className_map;
		map<string,int> className_classValue_map;
		
		// k is the number of top eigen vectors to be used 
		static const int k=80;
		
		// extract features from an image, which in this case just involves resampling and 
		// rearranging into a vector of pixel data.
		CImg<double> extract_features(const string &filename)
		{
		  return (convertToGreyScale(CImg<double>(filename.c_str()))).resize(size,size,1,1).unroll('x');
		  
		}
		
		//Prepare a mapping between class name (folder name) and numeric id (class label for training and testing SVM) and vice-versa
		void prepareMap(){
			
			int p=1;
			for(int j=0;j<class_list.size();j++){
				classValue_className_map[p]=class_list[j];
				className_classValue_map[class_list[j]]=p;
				p++;
			}			
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
		
		// Generate and save all the eigen faces
		void generateEigenFace(CImg<double> eigenVector, int colIndex) {
			CImg<double> eigenFace(40,40,1,1);
			eigenFace.fill(0);
			std::ostringstream o;
			int i = 0;
			for (int m=0; m < eigenFace.height(); m++)
				for (int n=0; n < eigenFace.width(); n++)
					eigenFace(n,m) = eigenVector(colIndex,i++);

			eigenFace.normalize(0,255);
			o << colIndex;
			string fileName = "EigenFace_" + o.str() + ".png";
			eigenFace.save_png(fileName.c_str());
    	}
		
		// This method performs the PCA algorithm , prepare the training file and runs SVM on it
		virtual void train(const Dataset &filenames) 
		{
			CImg<double> imageMatrix(1250,size * size);
			int n=0;
			vector<int> classNumber(1250);
			int index=0;
			for(Dataset::const_iterator c_iter=filenames.begin(); c_iter != filenames.end(); ++c_iter)
			{
				cout << "Processing " << c_iter->first << endl;
				
				// Prepare a matrix of  1250 * (size*size) dimensions (width * height)
				
				for(int i=0; i<c_iter->second.size(); i++){
					CImg<double> imageInARow = extract_features(c_iter->second[i].c_str());
					classNumber[index]=className_classValue_map[c_iter->first]; // this vector stores the class id of each image
					index++;
					for(int m=0;m<(size*size);m++){
						imageMatrix(n,m)=imageInARow(m,0);
					}
					
					n++;	
				}
			}
			
			//Computing the mean matrix of dimension 1 * (size*size) (width * height) 
			CImg<double> meanMatrix (1,size*size);
			for(int i=0;i<(size*size);i++){
				meanMatrix(0,i)=imageMatrix.get_row(i).mean();
			}
			
			//Subtracting mean matrix from the image matrix
			CImg<double> meanSubtractedImageMatrix=imageMatrix - meanMatrix ;
			
			// computing covariance matrix of dimension (size*size) * (size*size)
			CImg<double> covarianceMatrix= meanSubtractedImageMatrix.operator*(meanSubtractedImageMatrix.get_transpose());
			
			//Computing the eigenvectors and eigenvalues from the covariance matrix
			CImg<double> eigenVal;
			CImg<double> eigenVec;
			covarianceMatrix.symmetric_eigen(eigenVal,eigenVec);
			
			eigenVal.save_ascii("EigenValues");
			
			// Generate Eigen Faces
			for(int i=0;i<k;i++){
				generateEigenFace(eigenVec,i);
			}
			
			CImg<double> dimensionAfterPCA=eigenVec.get_columns(0,k-1).get_transpose();
			
			// Saving eigenvector to a file to be used later in classify method
			ofstream PCA("PCA.txt");
			for(int a=0;a<dimensionAfterPCA.height();a++){
				for(int b=0;b<dimensionAfterPCA.width();b++){
					PCA << dimensionAfterPCA(b,a)<<" ";
				}
				PCA << endl;
			}

			// Original Image Matrix * EigenVector corresponding to Top K Eigen Values
			CImg<double> imageMatrixAfterPCA= dimensionAfterPCA.operator*(imageMatrix).transpose();
			
			// prepare train file of SVM and train it
			ofstream SVM_trainFile ("train_eigen.dat");
	
			for(int i=0;i<imageMatrixAfterPCA.height();i++){
				SVM_trainFile << classNumber[i]  ;
				int x=1;
				for(int j=0;j<imageMatrixAfterPCA.width();j++){
					SVM_trainFile << " " << x <<":"<< imageMatrixAfterPCA(j,i);
					x++;
				}
				SVM_trainFile<<endl;
				
			}
			system("./svm_multiclass/svm_multiclass_learn -c 1.0  train_eigen.dat model_file_eigen");
			
		}
		
	
		// Extract top K features from Test Image using PCA,run SVM classify and return the predicted class
		virtual string classify(const string &filename)
		  {

			CImg<double> test_image = extract_features(filename).transpose();
		
			// Read the eigen vector saved in the test() method
			string line;
			ifstream PCA("PCA.txt");
			CImg<double> dimensionAfterPCA(size*size,k);
			int colIndex=0;
			int rowIndex=0;
			while(getline( PCA, line )){
				stringstream lineStream(line);
				double value;
				while(lineStream >> value)
        		{
           			 dimensionAfterPCA(colIndex,rowIndex)=value;
           			 colIndex++;
        		}
        		colIndex=0;
        		rowIndex++;        		
			}
		
			// Test Image Matrix * EigenVector corresponding to Top K Eigen Values
			CImg<double> imageMatrixAfterPCA=dimensionAfterPCA.operator*(test_image).transpose();
			
			// Prepare Test file for SVM
			ofstream SVM_testFile ("test_eigen.dat");
 			SVM_testFile << 2;
			int x=1;
			for(int k=0;k<imageMatrixAfterPCA.width();k++){
				SVM_testFile <<  " " << x <<":"<< imageMatrixAfterPCA(k,0);
				x++;
			}
			SVM_testFile<<endl;
		
			system("./svm_multiclass/svm_multiclass_classify test_eigen.dat model_file_eigen output_file_eigen");
			
			//Parse the output file and return the predicted class
			string outputClass;
			ifstream output ("output_file_eigen");
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