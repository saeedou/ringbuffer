test: ringbuffer.c test_ringbuffer.c ringbuffer.h 
	gcc test_ringbuffer.c ringbuffer.c -o ringbuffer
	./ringbuffer

clear:
	rm ringbuffer
