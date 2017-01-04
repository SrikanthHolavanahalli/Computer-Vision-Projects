all : segment stereo render

segment: CImg.h segment.cpp
	g++ -Dcimg_display=0 segment.cpp -o segment -I. -O3 -lpthread -g

stereo: CImg.h stereo.cpp
	g++ -Dcimg_display=0 stereo.cpp -o stereo -I. -O3 -lpthread -g

render: CImg.h render.cpp
	g++ -Dcimg_display=0 render.cpp -o render -I. -O3 -lpthread -g

clean:
	rm segment stereo render
