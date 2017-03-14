/*
 * dbg.h
 */

#ifndef TOOLS_DBG_H_
#define TOOLS_DBG_H_

void dbg(const char *fmt, ...) noexcept	__attribute__ ((format (printf, 1, 2)));

#endif /* TOOLS_DBG_H_ */
