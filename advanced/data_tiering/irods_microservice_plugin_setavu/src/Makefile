INC=-I/usr/include/irods -I/usr/include/irods/boost
OPTS=-fPIC -shared -Wno-deprecated

all: libmsiSetAVU.so

libmsiSetAVU.so: msiSetAVU.cpp
	g++ ${OPTS} ${INC} -o $@ $^ /usr/lib/libirods_client.a
        
clean:
	@rm -f ./*.so
