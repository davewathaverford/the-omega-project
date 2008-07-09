#if ! defined MMAP_H
#define MMAP_H

#include <omega.h>

namespace omega {

struct PartialMMap {
  Relation mapping;
  Relation bounds;
  String var;
};

struct MMap {
public:
  Tuple<PartialMMap> partials;
};


struct PartialRead {
public:
  int from;
  Relation dataFlow;
};

struct Read {
public:
  Tuple<PartialRead> partials;
};

struct stm_info {
public:
  Relation IS;
  MMap map;
  Tuple<Read> read;
  String stm;
};

static inline int operator==(const PartialMMap &, const PartialMMap &)
{
  assert(false);
  return 0;
}
static inline int operator==(const MMap &, const MMap &)
{
  assert(false);
  return 0;
}
static inline int operator==(const PartialRead &, const PartialRead &)
{
  assert(false);
  return 0;
}
static inline int operator==(const Read &, const Read &)
{
  assert(false);
  return 0;
}
static inline int operator==(const stm_info &, const stm_info &)
{
  assert(false);
  return 0;
}

} // end namespace omega

#endif
