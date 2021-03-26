# Simple makefile for compiling elephant locally and on hamilton

all:
	make pyjamask
	make pyjamaskNoOpt
	make pyjamaskIntel
	make pyjamaskIntelNoOpt

intel:
	make pyjamaskIntel
	make pyjamaskIntelMP
	make pyjamaskIntelNoOpt

largeSingle:
	make largeSingleIntelMP
	make largeSingleIntelMPNoOpt

noPrint:
	make smallNoPrintMP
	make smallNoPrintNoOpt
	make largeNoPrintMP
	make largeNoPrintNoOpt

cleanLarge:
	rm pyjamaskIntelMPLargeSingle
	rm pyjamaskIntelMPLargeSingleNoOpt
	rm -rf *.dSYM
	rm -rf *.optrpt

cleanIntel:
	rm pyjamaskIntel
	rm pyjamaskIntelMP
	rm pyjamaskIntelNoOpt
	rm -rf *.dSYM
	rm -rf *.optrpt

cleanNoPrint:
	rm smallNoPrint*
	rm largeNoPrint*
	rm -rf *.dSYM
	rm -rf *.optrpt

clean:
	rm pyjamask
	rm pyjamaskNoOpt
	rm pyjamaskIntel
	rm pyjamaskIntelNoOpt

pyjamask: encrypt.c genkat_aead.c pyjamask.c
	gcc -o pyjamask encrypt.c genkat_aead.c pyjamask.c -std=c99 -Wall -Wextra -Wshadow -fsanitize=address,undefined -O2 -g

pyjamaskNoOpt: encrypt.c genkat_aead.c pyjamask.c
	gcc -o pyjamaskNoOpt encrypt.c genkat_aead.c pyjamask.c -std=c99 -Wall -Wextra -Wshadow -fsanitize=address,undefined -O0

pyjamaskIntel: encrypt.c genkat_aead.c pyjamask.c
	icc -o pyjamaskIntel encrypt.c genkat_aead.c pyjamask.c -std=c99 -Wall -Wextra -Wshadow -O2 -qopt-report=5 -g

pyjamaskIntelMP: encrypt.c genkat_aead.c pyjamask.c
	icc -o pyjamaskIntelMP encrypt.c genkat_aead.c pyjamask.c -std=c99 -Wall -Wextra -Wshadow -O2 -qopenmp -g

pyjamaskIntelNoOpt: encrypt.c genkat_aead.c pyjamask.c
	icc -o pyjamaskIntelNoOpt encrypt.c genkat_aead.c pyjamask.c -std=c99 -Wall -Wextra -Wshadow -O0 

largeSingleIntelMP: encrypt.c largeMessageTest.c pyjamask.c
	icc -o pyjamaskIntelMPLargeSingle encrypt.c largeMessageTest.c pyjamask.c -std=c99 -Wall -Wextra -Wshadow -O2 -qopenmp -g

largeSingleIntelMPNoOpt: encrypt.c largeMessageTest.c pyjamask.c
	icc -o pyjamaskIntelMPLargeSingleNoOpt encrypt.c largeMessageTest.c pyjamask.c -std=c99 -Wall -Wextra -Wshadow -O0

smallNoPrintMP: encrypt.c genkat_aead_no_print.c pyjamask.c
	icc -o smallNoPrintMP encrypt.c genkat_aead_no_print.c pyjamask.c -std=c99 -Wall -Wextra -Wshadow -O2 -qopenmp

smallNoPrintNoOpt: encrypt.c genkat_aead_no_print.c pyjamask.c
	icc -o smallNoPrintNoOpt encrypt.c genkat_aead_no_print.c pyjamask.c -std=c99 -Wall -Wextra -Wshadow -O0 

largeNoPrintMP: encrypt.c largeMessageTest_no_print.c pyjamask.c
	icc -o largeNoPrintMP encrypt.c largeMessageTest_no_print.c pyjamask.c -std=c99 -Wall -Wextra -Wshadow -O2 -qopenmp

largeNoPrintNoOpt: encrypt.c largeMessageTest_no_print.c pyjamask.c
	icc -o largeNoPrintNoOpt encrypt.c largeMessageTest_no_print.c pyjamask.c -std=c99 -Wall -Wextra -Wshadow -O0