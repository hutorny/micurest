/*
 * 004.h
 *
 *  Created on: Aug 19, 2015
 *      Author: eugene
 */

#ifndef SUITES_BASIC_004_H_
#define SUITES_BASIC_004_H_

#ifdef __cplusplus
extern "C" {
#endif

extern unsigned * uitem(unsigned n);
extern const char* strv();

struct CPod {
	char c;
	int  i;
	long l;
	unsigned long long u;
	char s[12];
};
extern struct CPod * cpod();
extern struct CPod dpod;

#ifdef __cplusplus
}
#endif

#endif /* SUITES_BASIC_004_H_ */
