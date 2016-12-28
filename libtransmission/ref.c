/*
 * This file Copyright (C) 2016 Mnemosyne LLC
 *
 * It may be used under the GNU GPL versions 2 or 3
 * or any future license endorsed by Mnemosyne LLC.
 *
 */

#include <assert.h>

#ifdef _WIN32
 #include <windows.h>
 #define TR_ATOMIC_INC(x) InterlockedIncrement (&(x))
 #define TR_ATOMIC_DEC(x) InterlockedDecrement (&(x))
#else
 #define TR_ATOMIC_INC(x) __sync_add_and_fetch (&(x), 1)
 #define TR_ATOMIC_DEC(x) __sync_sub_and_fetch (&(x), 1)
#endif

#include "transmission.h"
#include "log.h"
#include "ref.h"

#define op_dbgmsg(op) \
  do \
    { \
      if (tr_logGetDeepEnabled ()) \
        tr_logAddDeep (__FILE__, __LINE__, ref_obj_to_string (ref), \
                       "%s:%d is " op " refcount of %p to %d", \
                       file, line, ref, new_count); \
    } \
  while (0)

static void
ref_obj_free (const tr_ref * ref)
{
  assert (ref != NULL);
  assert (ref->vftbl != NULL);

  if (ref->vftbl->free != NULL)
    (*ref->vftbl->free) (ref);
}

static const char *
ref_obj_to_string (const tr_ref * ref)
{
  assert (ref != NULL);
  assert (ref->vftbl != NULL);

  return ref->vftbl->to_string != NULL ? (*ref->vftbl->to_string) (ref) : NULL;
}

void
tr_ref_init_impl (tr_ref             * ref,
                  const tr_ref_vftbl * vftbl)
{
  assert (ref != NULL);
  assert (vftbl != NULL);

  ref->vftbl = vftbl;
  ref->count = 1;
}

void
tr_ref_inc_impl (const tr_ref * ref,
                 const char   * file,
                 int            line)
{
  if (ref == NULL)
    return;

  assert (ref->vftbl != NULL);

  const int new_count = TR_ATOMIC_INC (((tr_ref *) ref)->count);

  assert (new_count > 1);

  op_dbgmsg ("incrementing");
}

void
tr_ref_dec_impl (const tr_ref * ref,
                 const char   * file,
                 int            line)
{
  if (ref == NULL)
    return;

  assert (ref->vftbl != NULL);

  const int new_count = TR_ATOMIC_DEC (((tr_ref *) ref)->count);

  assert (new_count >= 0);

  op_dbgmsg ("decrementing");

  if (new_count == 0)
    ref_obj_free (ref);
}
