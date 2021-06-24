(LDFLAGS)=-ljsoncpp -lcurl -lcrypto
(INCLUDE)=-ispeech
Jarvis:jarvis.cc 
	g++ -o $@ $^ $(INCLUDE) -std=c++11 -ljsoncpp -lcurl -lcrypto 

.PHONY:clean
clean:
	rm -f Jarvis
