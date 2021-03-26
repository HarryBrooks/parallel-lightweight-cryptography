# Simple makefile for compiling photon locally and on hamilton

all:
	make photon
	make photonNoOpt
	make photonIntel
	make photonIntelNoOpt
	make photonLarge
	make photonLargeNoOpt

large:
	make photonLarge
	make photonLargeNoOpt

intel:
	make photonIntel
	make photonIntelNoOpt

noPrint:
	make smallNoPrintMP
	make smallNoPrintNoOpt
	make largeNoPrintMP
	make largeNoPrintNoOpt
	
clean:
	rm photon
	rm photonNoOpt
	rm photonIntel
	rm photonIntelNoOpt
	rm photonLarge
	rm photonLargeNoOpt

cleanIntel:
	rm photonIntel
	rm photonIntelNoOpt

cleanLarge:
	rm photonLarge
	rm photonLargeNoOpt

cleanNoPrint:
	rm smallNoPrint*
	rm largeNoPrint*
	rm -rf *.dSYM
	rm -rf *.optrpt

photon: encrypt.c genkat_aead.c photon.c
	gcc -o photon encrypt.c genkat_aead.c photon.c -std=c99 -Wall -Wextra -Wshadow -fsanitize=address,undefined -O2

photonNoOpt: encrypt.c genkat_aead.c photon.c
	gcc -o photonNoOpt encrypt.c genkat_aead.c photon.c -std=c99 -Wall -Wextra -Wshadow -fsanitize=address,undefined -O0

photonIntel: encrypt.c genkat_aead.c photon.c
	icc -o photonIntel encrypt.c genkat_aead.c photon.c -std=c99 -Wall -Wextra -Wshadow -O2 -g qopt-report=2

photonIntelNoOpt: encrypt.c genkat_aead.c photon.c
	icc -o photonIntelNoOpt encrypt.c genkat_aead.c photon.c -std=c99 -Wall -Wextra -Wshadow -O0 -g

photonLarge: encrypt.c largeMessageTest.c photon.c
	icc -o photonLarge encrypt.c largeMessageTest.c photon.c -std=c99 -Wall -Wextra -Wshadow -O2 -g

photonLargeNoOpt: encrypt.c largeMessageTest.c photon.c
	icc -o photonLargeNoOpt encrypt.c largeMessageTest.c photon.c -std=c99 -Wall -Wextra -Wshadow -O0 -g

smallNoPrintMP: encrypt.c genkat_aead_no_print.c photon.c
	icc -o smallNoPrintMP encrypt.c genkat_aead_no_print.c photon.c -std=c99 -Wall -Wextra -Wshadow -O2 -qopenmp

smallNoPrintNoOpt: encrypt.c genkat_aead_no_print.c photon.c
	icc -o smallNoPrintNoOpt encrypt.c genkat_aead_no_print.c photon.c -std=c99 -Wall -Wextra -Wshadow -O0 

largeNoPrintMP: encrypt.c largeMessageTest_no_print.c photon.c
	icc -o largeNoPrintMP encrypt.c largeMessageTest_no_print.c photon.c -std=c99 -Wall -Wextra -Wshadow -O2 -qopenmp

largeNoPrintNoOpt: encrypt.c largeMessageTest_no_print.c photon.c
	icc -o largeNoPrintNoOpt encrypt.c largeMessageTest_no_print.c photon.c -std=c99 -Wall -Wextra -Wshadow -O0
