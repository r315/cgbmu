#ifndef _tests_h_
#define _tests_h_

#define ROMS_BINARY_PATH "../roms/tests/"
#define ALL_TESTS_ROM "cpu_instrs.gb"

void runTest(void);
void testAll(void);
void testRom(char *fn);
#endif