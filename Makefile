test: clean ringbuffer.c test_ringbuffer.c ringbuffer.h 
	gcc test_ringbuffer.c ringbuffer.c -o ringbuffer
	./ringbuffer
	rm -f ringbuffer

clean:
	rm -f ringbuffer
	rm -f *.gcno
	rm -f *.gcda

cov:
	gcc -fprofile-arcs -ftest-coverage -o ringbuffer ringbuffer.c \
	test_ringbuffer.c 
	./ringbuffer
	clear
	gcovr -e 'cutest\.h'
