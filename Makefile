.PHONY: test
test: test_iter test_streamreader
	./test_streamreader
	./test_iter

.INTERMEDIATE: test_iter.o genericiter.o
test_iter: test_iter.o genericiter.o
genericiter.o: genericiter.c genericiter.h

.INTERMEDIATE: test_streamreader.o genericstreamreader.o
test_streamreader: test_streamreader.o genericstreamreader.o
genericstreamreader.o: genericstreamreader.c genericstreamreader.h
