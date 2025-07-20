// include/macros.h

#ifndef MACROS_H
#define MACROS_H

#define ARISSERT(args, cond, fmt, ...)                                         \
  if (!(cond)) {                                                               \
    ari *err = ari_err(fmt, ##__VA_ARGS__);                                    \
    ari_del(args);                                                             \
    return err;                                                       \
  }

#endif
