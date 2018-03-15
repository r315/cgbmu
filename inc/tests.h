#ifndef _tests_h_
#define _tests_h_

#if defined(__EMU__)
#ifdef _WIN32
#define ROMS_BINARY_PATH ROM_PATH
#else
#define ROMS_BINARY_PATH "../../../roms/tests/"
#endif

#define ALL_TESTS_ROM "cpu_instrs.gb"

void testAll(void);
void testRom(char *fn);
void testMain(void);
#endif

void runTest(void);
#endif
