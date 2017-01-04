//Bag Of Words
#include <iostream>
#include <fstream>
#include <Sift.h>
#include <string>
class BOWSVMClassifier : public Classifier{	
	public:
  		static const int k=100; //k value for k means 
		vector<SiftDescriptor> finalKVal; //centroid of clusters
		map<string, int > className_ClassValue_map; //class to class number
		map<int, string > classValue_className_map; //class number to name
		BOWSVMClassifier(const vector<string> &_class_list) : Classifier(_class_list) {
			prepareMap();
  		}
		
		//creates two maps between class name and value in both directions
		void prepareMap(){		
			int p=0;
			for(int c=0; c <class_list.size(); c++)
				className_ClassValue_map[class_list[c] ] = ++p;
			p=0;
			for(int j=0;j<class_list.size();j++)
				classValue_className_map[++p]=class_list[j];
		}		
		
		//given an image, extracts the sift descriptors
		vector<SiftDescriptor> extract_sift_features(const string &filename){
			CImg<float> input_image(filename.c_str());
			CImg<float> grayInput = input_image.get_RGBtoHSI().get_channel(2);
			vector<SiftDescriptor> descriptors = Sift::compute_sift(grayInput);
			return descriptors;
		}
		
		//for all the images, creates a bank of sift descriptors
		map<string, vector<vector<SiftDescriptor> > > createBank(const Dataset &filenames){
			map<string, vector<vector<SiftDescriptor> > > dataMap;
			for(Dataset::const_iterator c_iter=filenames.begin(); c_iter != filenames.end(); ++c_iter){
				vector<vector<SiftDescriptor> > classSifts;
				for(int i=0; i<c_iter->second.size(); i++){
					vector<SiftDescriptor> imageSiftFeatures = extract_sift_features(c_iter->second[i].c_str());
					classSifts.push_back(imageSiftFeatures);
				}
				dataMap[c_iter->first] = classSifts;
			}
			return dataMap;
		}
		
		//performs k means on the training images
		vector<vector<vector<string> > > kmeans(map<string, vector<vector<SiftDescriptor> > > &dataMap){
			int cycleStop = 0; //convergence parameter
			vector<SiftDescriptor> kvals, prevKvals; //centroids
			std::map<string, vector<vector<SiftDescriptor> > >::iterator kt=dataMap.begin();
			int a = 0;
			int b = 0;
			
			//initial seeds generated
			for(int i = 0; i < k; i++){	
				if(kt!=dataMap.end()){
					kvals.push_back(dataMap[kt->first][a][b]);
					++kt;
				}
				else{
					b++;
					kt=dataMap.begin();
					if(b == dataMap[kt->first][a].size()){
						b = 0;
						a++;
					}
					kvals.push_back(dataMap[kt->first][a][b]);
					++kt;
				}
			}
			
			vector<vector<vector<string> > > groups;
			
			//k means algo
			while(cycleStop == 0){
				//initializing groups
				if(groups.size()!=0){
					groups.clear();
				}
				for(int i = 0; i < k; i++){
					vector<vector<string> > currentGroup;
					groups.push_back(currentGroup);
				}	
				
				//for each descriptor, finding corresponding cluster
				for(std::map<string, vector<vector<SiftDescriptor> > >::iterator it=dataMap.begin(); it!=dataMap.end(); ++it){
					for(int j = 0; j < it->second.size(); j++){
						for(int s = 0; s < it->second[j].size(); s++){
							int minK = 0;
							float minDist = std::numeric_limits<float>::max();
							for(int p = 0; p < kvals.size(); p++){
								float distance = 0;
								for(int l=0; l<128; l++){
									distance = distance + pow(it->second[j][s].descriptor[l] - kvals[p].descriptor[l], 2);
								}
								distance = sqrt(distance);
								if(distance < minDist){
									minDist = distance;
									minK = p;
								}
							}
							
							//putting the sift descriptors coordinates in the group
							vector<string> currentSift;
							currentSift.push_back(it->first);
							ostringstream ostr;
							ostr << j; 
							string Result = ostr.str();
							currentSift.push_back(Result);
							ostringstream ostr1;
							ostr1 << s; 
							string Result1 = ostr1.str();
							currentSift.push_back(Result1);
							groups[minK].push_back(currentSift);
						}
					}
				}
				if(prevKvals.size()!=0){
					prevKvals.clear();
				}
				
				//computing new k values
				for(int i = 0; i < k; i++){
					prevKvals.push_back(kvals[i]);
					for(int l = 0; l < 128; l++){
						kvals[i].descriptor[l] = 0;
					}
					for(int j = 0; j < groups[i].size(); j++){
						int arg1 = atoi(groups[i][j][1].c_str());
						int arg2 = atoi(groups[i][j][2].c_str()); 
						SiftDescriptor currentSift = dataMap[groups[i][j][0]][arg1][arg2];
						for(int l = 0; l < 128; l++){
							kvals[i].descriptor[l] = kvals[i].descriptor[l] + currentSift.descriptor[l];
						}
					}
					for(int l = 0; l < 128; l++){
						kvals[i].descriptor[l] = kvals[i].descriptor[l]/groups[i].size(); 
					}
				}
				
				//compares new and old k values to see if it has coverged
				cycleStop = 1;
				int q = 0;
				for(int i = 0; i < k; i++){
					float d = 0;
					for(int l = 0; l < 128; l++){
						d = d + pow(prevKvals[i].descriptor[l] - kvals[i].descriptor[l], 2);
					}
					d = sqrt(d);
					cout << d << endl;
					if(d > 15){
						q++;
						if(q > 0.10*k){
							cycleStop = 0;
							break;
						}
					}
				}
				finalKVal = prevKvals;
			}			
			return groups;
		}
		
		//this function unwraps the groups and forms feature vectors for the training images
		map<string, vector<string> > dataSet(vector<vector<vector<string> > > groups, map<string, vector<vector<SiftDescriptor> > > dataMap){
			int k = groups.size();
			
			//mapping labels to the corresponding train files
			map<string, string> labels;
			for(int i = 0; i < k; i++){
				vector<vector<string> > currentGroup = groups[i];
				for(int j = 0; j < currentGroup.size(); j++){
					string filename = currentGroup[j][0] + currentGroup[j][1];
					labels[filename] = currentGroup[j][0]; 
				}
			}
			
			//mapping labels and the features to the corresponding train file
			map<string, vector<string> > data;
			
			//initializing feature counts
			for (std::map<string, string>::iterator it=labels.begin(); it!=labels.end(); ++it){
				vector<string> dataVector;
				ostringstream ostr;
				ostr << className_ClassValue_map[it->second]; 
				string Result = ostr.str();
				dataVector.push_back(Result);
				for(int i = 0; i < k; i++){
					dataVector.push_back("0");
				}
				data[it->first] = dataVector;
			}	
			
			//increment feature values based on the presence of sift vector of a image in a cluster
			for(int i = 0; i < k; i++){
				vector<vector<string> > currentGroup = groups[i];
				map<string, int> countOfCurrentK;
				for(int j = 0; j < currentGroup.size(); j++){
					string filename = currentGroup[j][0] + currentGroup[j][1];
					if(countOfCurrentK.find(filename) != countOfCurrentK.end()){
						countOfCurrentK[filename] = countOfCurrentK[filename] + 1; 
					}
					else{
						countOfCurrentK[filename] = 1; 
					}
				}
				
				//unwrap features in to the data datastructure
				for (std::map<string, int>::iterator it=countOfCurrentK.begin(); it!=countOfCurrentK.end(); ++it){
					ostringstream ostr;
					ostr << it->second; 
					string count = ostr.str();
					data[it->first][i+1] = count;
				}
			}
			return data;
		}
		
		
		vector<int> extract_features(const string &filename){
			//extracts features for the test image
			CImg<float> input_image(filename.c_str());
			CImg<float> grayInput = input_image.get_RGBtoHSI().get_channel(2);
			vector<SiftDescriptor> descriptors = Sift::compute_sift(grayInput);
			vector<int> groups;
			for(int i = 0; i < k; i++){
				groups.push_back(0);
			}
			
			//increment corresponding feature count based on the cluster the sift descriptor belongs to
			for(int s = 0; s < descriptors.size(); s++){
				int minK = 0;
				float minDist = std::numeric_limits<float>::max();
				for(int p = 0; p < finalKVal.size(); p++){
					float distance = 0;
					for(int l=0; l<128; l++){
						distance = distance + pow(descriptors[s].descriptor[l] - finalKVal[p].descriptor[l], 2);
					}
					distance = sqrt(distance);
					if(distance < minDist){
						minDist = distance;
						minK = p;
					}
				}
				groups[minK] = groups[minK] + 1;
			}
			return groups;
		}
		
		virtual void train(const Dataset &filenames) {	
			ofstream SVM_File ("train_bow.dat");
			map<string, vector<vector<SiftDescriptor> > > dataMap = createBank(filenames); //create bank
			vector<vector<vector<string> > > groups = kmeans(dataMap); //apply k mean
			map<string, vector<string> > data = dataSet(groups, dataMap); //form feature vectors for train images
			
			//writing train data to the train file
			for (std::map<string, vector<string> > ::iterator it=data.begin(); it!=data.end(); ++it){
				for(int i = 0; i < it->second.size(); i++){
					if(i==0){
						SVM_File << it->second[i];
					}
					else{
						SVM_File << " " << i <<":"<< it->second[i];
					}
				}
				SVM_File<<endl;
			}	
			
			//writing the centroids to a file
			ofstream kVectors("centroids_bow.dat");
			for(int i = 0; i < finalKVal.size(); i++){
				for(int j = 0; j < 128; j++){
					if(j==0){
						kVectors << finalKVal[i].descriptor[j];
					}
					else{
						kVectors << " " << finalKVal[i].descriptor[j];
					}
				}
				kVectors<<endl;
			}
			
			//system call to run svm and form a model
			system("./svm_multiclass/svm_multiclass_learn -c 1.0 train_bow.dat model_file_bow");	
		}
		
		virtual string classify(const string &filename){
			//get centroids if they aren't already fetched
			if(finalKVal.size() != k){
				ifstream kVectors ("centroids_bow.dat");
				int lineNum = 0;
				if (kVectors.is_open())
				{
					string line;
					while ( getline (kVectors,line) ){
						string buf;
						stringstream ss(line); 
						SiftDescriptor siftVals;
						while (ss >> buf){
							siftVals.descriptor.push_back(atof(buf.c_str()));
						}
						finalKVal.push_back(siftVals);
					}
				}
				kVectors.close();
			}
			
			//extract the feature for the test image
			vector<int> test_image = extract_features(filename);
			ofstream SVM_testFile ("test_bow.dat");//file to write the test data in
			SVM_testFile << 2;//dummy label
			int x=1;
			for(int k=0;k<test_image.size();k++){
				SVM_testFile <<  " " << x <<":"<< test_image[k];
				x++;
			}
			SVM_testFile<<endl;
			
			//classifying using SVM
			system("./svm_multiclass/svm_multiclass_classify test_bow.dat model_file_bow output_file_bow");
			string outputClass;
			string line;
			ifstream output ("output_file_bow");
			
			//reading output file and printing results
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
