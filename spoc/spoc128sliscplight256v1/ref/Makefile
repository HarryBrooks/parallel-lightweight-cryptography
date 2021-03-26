all:
	make spoc
	make spocNoOpt
	make spocIntel
	make spocIntelNoOpt
	make spocLarge
	make spocLargeNoOpt

large:
	make spocLarge
	make spocLargeNoOpt

intel:
	make spocIntel
	make spocIntelNoOpt

noPrint:
	make smallNoPrintMP
	make smallNoPrintNoOpt
	make largeNoPrintMP
	make largeNoPrintNoOpt

clean:
	rm spoc
	rm spocNoOpt
	rm spocIntel
	rm spocIntelNoOpt
	rm spocLarge
	rm spocLargeNoOpt
	rm -rf *.dSYM

cleanIntel:
	rm spocIntel
	rm spocIntelNoOpt
	rm -rf *.dSYM

cleanLarge:
	rm spocLarge
	rm spocLargeNoOpt
	rm -rf *.dSYM

cleanNoPrint:
	rm smallNoPrint*
	rm largeNoPrint*
	rm -rf *.dSYM
	rm -rf *.optrpt

spoc: encrypt.c genkat_aead.c sliscp_light256.c
	gcc -o spoc encrypt.c genkat_aead.c sliscp_light256.c -std=c99 -Wall -Wextra -Wshadow -fsanitize=address,undefined -O2

spocNoOpt: encrypt.c genkat_aead.c sliscp_light256.c
	gcc -o spocNoOpt encrypt.c genkat_aead.c sliscp_light256.c -std=c99 -Wall -Wextra -Wshadow -fsanitize=address,undefined -O0

spocIntel: encrypt.c genkat_aead.c sliscp_light256.c
	icc -o spocIntel encrypt.c genkat_aead.c sliscp_light256.c -std=c99 -Wall -Wextra -Wshadow -O2 -g -qopt-report=5

spocIntelNoOpt: encrypt.c genkat_aead.c sliscp_light256.c
	icc -o spocIntelNoOpt encrypt.c genkat_aead.c sliscp_light256.c -std=c99 -Wall -Wextra -Wshadow -O0 -g

spocLarge: encrypt.c largeMessageTest.c sliscp_light256.c
	icc -o spocLarge encrypt.c largeMessageTest.c sliscp_light256.c -std=c99 -Wall -Wextra -Wshadow -O2 -g

spocLargeNoOpt: encrypt.c largeMessageTest.c sliscp_light256.c
	icc -o spocLargeNoOpt encrypt.c largeMessageTest.c sliscp_light256.c -std=c99 -Wall -Wextra -Wshadow -O0 -g

smallNoPrintMP: encrypt.c genkat_aead_no_print.c sliscp_light256.c
	icc -o smallNoPrintMP encrypt.c genkat_aead_no_print.c sliscp_light256.c -std=c99 -Wall -Wextra -Wshadow -O2 -qopenmp

smallNoPrintNoOpt: encrypt.c genkat_aead_no_print.c sliscp_light256.c
	icc -o smallNoPrintNoOpt encrypt.c genkat_aead_no_print.c sliscp_light256.c -std=c99 -Wall -Wextra -Wshadow -O0 

largeNoPrintMP: encrypt.c largeMessageTest_no_print.c sliscp_light256.c
	icc -o largeNoPrintMP encrypt.c largeMessageTest_no_print.c sliscp_light256.c -std=c99 -Wall -Wextra -Wshadow -O2 -qopenmp

largeNoPrintNoOpt: encrypt.c largeMessageTest_no_print.c sliscp_light256.c
	icc -o largeNoPrintNoOpt encrypt.c largeMessageTest_no_print.c sliscp_light256.c -std=c99 -Wall -Wextra -Wshadow -O0