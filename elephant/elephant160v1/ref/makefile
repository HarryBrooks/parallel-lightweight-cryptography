# Simple makefile for compiling elephant locally and on hamilton

all:
	make elephant
	make elephantNoOpt
	make elephantIntel
	make elephantIntelNoOpt

elephant: encrypt.c genkat_aead.c spongent.c
	gcc -o elephant encrypt.c genkat_aead.c spongent.c -std=c99 -Wall -Wextra -Wshadow -fsanitize=address,undefined -O2 -g
elephantNoOpt: encrypt.c genkat_aead.c spongent.c
	gcc -o elephantNoOpt encrypt.c genkat_aead.c spongent.c -std=c99 -Wall -Wextra -Wshadow -fsanitize=address,undefined -O0

intel:
	make elephantIntel
	make elephantIntelMP
	make elephantIntelNoOpt

noPrint:
	make smallNoPrintMP
	make smallNoPrintNoOpt
	make largeNoPrintMP
	make largeNoPrintNoOpt

elephantIntel: encrypt.c genkat_aead.c spongent.c
	icc -o elephantIntel encrypt.c genkat_aead.c spongent.c -std=c99 -Wall -Wextra -Wshadow -O2

elephantIntelMP: encrypt.c genkat_aead.c spongent.c
	icc -o elephantIntelMP encrypt.c genkat_aead.c spongent.c -std=c99 -Wall -Wextra -Wshadow -O2 -qopenmp

elephantIntelNoOpt: encrypt.c genkat_aead.c spongent.c
	icc -o elephantIntelNoOpt encrypt.c genkat_aead.c spongent.c -std=c99 -Wall -Wextra -Wshadow -O0 

largeIntelMP: encrypt.c genkat_aead_large.c spongent.c
	icc -o elephantIntelMPLarge encrypt.c genkat_aead_large.c spongent.c -std=c99 -Wall -Wextra -Wshadow -O2 -qopenmp

largeSingleIntelMP: encrypt.c largeMessageTest.c spongent.c
	icc -o elephantIntelMPLargeSingle encrypt.c largeMessageTest.c spongent.c -std=c99 -Wall -Wextra -Wshadow -O3 -qopenmp

largeSingleIntelMPNoOpt: encrypt.c largeMessageTest.c spongent.c
	icc -o elephantIntelMPLargeSingleNoOpt encrypt.c largeMessageTest.c spongent.c -std=c99 -Wall -Wextra -Wshadow -O0

smallNoPrintMP: encrypt.c genkat_aead_no_print.c spongent.c
	icc -o smallNoPrintMP encrypt.c genkat_aead_no_print.c spongent.c -std=c99 -Wall -Wextra -Wshadow -O2 -qopenmp

smallNoPrintNoOpt: encrypt.c genkat_aead_no_print.c spongent.c
	icc -o smallNoPrintNoOpt encrypt.c genkat_aead_no_print.c spongent.c -std=c99 -Wall -Wextra -Wshadow -O0 

largeNoPrintMP: encrypt.c largeMessageTest_no_print.c spongent.c
	icc -o largeNoPrintMP encrypt.c largeMessageTest_no_print.c spongent.c -std=c99 -Wall -Wextra -Wshadow -O2 -qopenmp

largeNoPrintNoOpt: encrypt.c largeMessageTest_no_print.c spongent.c
	icc -o largeNoPrintNoOpt encrypt.c largeMessageTest_no_print.c spongent.c -std=c99 -Wall -Wextra -Wshadow -O0
	
largeSingle:
	make largeSingleIntelMP
	make largeSingleIntelMPNoOpt

cleanLarge:
	rm elephantIntelMPLargeSingle
	rm elephantIntelMPLargeSingleNoOpt
	rm -rf *.dSYM
	rm -rf *.optrpt

cleanIntel:
	rm elephantIntel
	rm elephantIntelMP
	rm elephantIntelNoOpt
	rm -rf *.dSYM
	rm -rf *.optrpt

cleanNoPrint:
	rm smallNoPrint*
	rm largeNoPrint*
	rm -rf *.dSYM
	rm -rf *.optrpt

clean:
	rm elephant
	rm elephantNoOpt
	rm elephantIntel
	rm elephantIntelNoOpt
