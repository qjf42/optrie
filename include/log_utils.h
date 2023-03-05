#ifndef __OP_TRIE_LOG_UTILS_H__
#define __OP_TRIE_LOG_UTILS_H__

#include <cstdarg>
#include <cstdio>

#ifndef LOG_INFO
inline void logging(const char *level_str, const char *format, ...) {
  va_list val;
  va_start(val, format);
  printf("[%s] [OpTrie] ", level_str);
  vprintf(format, val);
  printf("\n");
  fflush(stderr);
  va_end(val);
}

#define LOG_INFO(fmt, vals...) logging("INFO", fmt, ##vals);
#define LOG_DEBUG(fmt, vals...) logging("DEBUG", fmt, ##vals);
#define LOG_ERR(fmt, vals...) logging("ERROR", fmt, ##vals);
#define LOG_WARN(fmt, vals...) logging("WARNING", fmt, ##vals);
#endif

#endif  // __OP_TRIE_LOG_UTILS_H__
