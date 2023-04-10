.SILENT: clean cov test

test: clean 
	gcc test_ringbuffer.c -o test_ringbuffer
	./test_ringbuffer
	rm -f test_ringbuffer

clean:
	rm -f ringbuffer
	rm -f *.gcno
	rm -f *.gcda

cov:
	gcc -fprofile-arcs -ftest-coverage test_ringbuffer.c -o ringbuffer
	./ringbuffer
	gcovr -e 'cutest\.h'
	make clean
