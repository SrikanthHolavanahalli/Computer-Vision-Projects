all: CImg.h Siftmatching.h Siftmatching.cpp ransac.cpp ransac.h a2.cpp
	g++ a2.cpp Siftmatching.cpp ransac.cpp -o a2 -lX11 -lpthread -I. -Isiftpp -O3 siftpp/sift.cpp

clean:
	rm a2
