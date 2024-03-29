PREFIX=/usr

libMPU9250.so: MPU9250.cpp MPU9250.h
	g++ -shared MPU9250.cpp -lRTIMULib -olibMPU9250.so

install: libMPU9250.so
	cp libMPU9250.so $(PREFIX)/lib/.
	cp MPU9250.h $(PREFIX)/include/.
	
clean:
	rm -f libMPU9250.so		