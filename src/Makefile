libgstdualfisheye.so: gstdualfisheye.o
	$(CXX) -shared $^ -o $@ `pkg-config --libs opencv4` `pkg-config --libs protobuf` `pkg-config --libs gstreamer-video-1.0`

gstdualfisheye.o: CXXFLAGS+=-fPIC `pkg-config --cflags gstreamer-video-1.0` `pkg-config --cflags opencv4`
