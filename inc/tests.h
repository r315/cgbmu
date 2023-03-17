#ifndef _tests_h_
#define _tests_h_

#if defined(__EMU__)
void testAll(void);
void testRom(char *fn);
void TEST_main(uint32_t flags);
#endif

void testButtons(void);
void runTest(void);
#endif
