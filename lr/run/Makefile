UNAME:=$(shell uname)

ifeq (${UNAME}, Linux)
TIME_CMD=/usr/bin/time -f '%e seconds -  %M KB RAM'
endif
ifeq (${UNAME}, Darwin)
TIME_CMD=/usr/bin/time -l
endif

all: bin/keygen params.bin keys.bin encrypted_data.bin encrypted_prediction.bin result.csv

clean:
	rm -rf bin params.bin keys.bin encrypted_data.bin encrypted_prediction.bin result.csv result_bypos.csv || true

build:
	[ -d bin ] || mkdir bin
	cd bin; cmake ../.. -DCMAKE_BUILD_TYPE=optim -DCMAKE_CXX_COMPILER=g++ -DCMAKE_C_COMPILER=gcc -DENABLE_SPQLIOS_FMA=on -DENABLE_SPQLIOS_AVX=off -DENABLE_NAYUKI_PORTABLE=on -DENABLE_TESTS=off
	cd bin; make -j 4 VERBOSE=1

bin/keygen:
	make build

keys.bin:
	${TIME_CMD} ./bin/keygen

params.bin: keys.bin
	@[ -f $@ ] # keygen should already have created this file

encrypted_data.bin: keys.bin
	${TIME_CMD} ./bin/encrypt
	@[ -f $@ ] # verify that the target is present

encrypted_prediction.bin: params.bin encrypted_data.bin
	${TIME_CMD} ./bin/cloud
	@[ -f $@ ] # verify that the target is present

result.csv: keys.bin encrypted_prediction.bin
	${TIME_CMD} ./bin/decrypt
	@[ -f $@ ] # verify that the target is present

result_bypos.csv: result.csv
	@[ -f $@ ] # verify that the target is present

