// include/macros.h

#ifndef MACROS_H
#define MACROS_H

#define ARISSERT(args, cond, err)          \
  if (!(cond)) {                           \
    ari_del(args);                         \
    return ari_err(err);                   \
  }

#endif
