all: CImg.h haar.h CNN.h Baseline.h Eigen.h bow.h a3.cpp Classifier.h NearestNeighbor.h
	g++ -std=c++11 a3.cpp -o a3 -lX11 -lpthread -I. -Isiftpp -O3 siftpp/sift.cpp

clean:
	rm a3
