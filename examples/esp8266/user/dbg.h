/*
 * dbg.h
 */

#ifndef TOOLS_DBG_H_
#define TOOLS_DBG_H_
#ifdef __cplusplus
	extern "C" {
	void dbg(const char *fmt, ...) noexcept	__attribute__ ((format (printf, 1, 2)));
	void dbg_addr(const char*, const unsigned char (&)[4], unsigned) noexcept;
#endif
#ifdef __cplusplus
	}
#else
	void dbg(const char *fmt, ...) __attribute__ ((format (printf, 1, 2)));
#endif

#endif /* TOOLS_DBG_H_ */
