#ifndef _tests_h_
#define _tests_h_

#if defined(__EMU__)
#define ROMS_BINARY_PATH "../../../roms/tests/"
#define ALL_TESTS_ROM "cpu_instrs.gb"

void testAll(void);
void testRom(char *fn);
void testMain(void);
#endif

void runTest(void);
#endif
