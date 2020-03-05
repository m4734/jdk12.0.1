/*
 * Copyright (c) 2017, 2018, Oracle and/or its affiliates. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 *
 */

#ifndef SHARE_VM_GC_G1_G1FULLGCOOPCLOSURES_INLINE_HPP
#define SHARE_VM_GC_G1_G1FULLGCOOPCLOSURES_INLINE_HPP

#include "gc/g1/g1Allocator.inline.hpp"
#include "gc/g1/g1ConcurrentMarkBitMap.inline.hpp"
#include "gc/g1/g1FullGCMarker.inline.hpp"
#include "gc/g1/g1FullGCOopClosures.hpp"
#include "gc/g1/heapRegionRemSet.hpp"
#include "memory/iterator.inline.hpp"
#include "oops/access.inline.hpp"
#include "oops/compressedOops.inline.hpp"
#include "oops/oop.inline.hpp"

template <typename T>
inline void G1MarkAndPushClosure::do_oop_work(T* p) {
  _marker->mark_and_push(p);
}

inline void G1MarkAndPushClosure::do_oop(oop* p) {
  do_oop_work(p);
}

inline void G1MarkAndPushClosure::do_oop(narrowOop* p) {
  do_oop_work(p);
}

inline bool G1MarkAndPushClosure::do_metadata() {
  return true;
}

inline void G1MarkAndPushClosure::do_klass(Klass* k) {
  _marker->follow_klass(k);
}

inline void G1MarkAndPushClosure::do_cld(ClassLoaderData* cld) {
  _marker->follow_cld(cld);
}

template <class T> inline void G1AdjustClosure::adjust_pointer(T* p) {
  T heap_oop = RawAccess<>::oop_load(p);
  if (CompressedOops::is_null(heap_oop)) {
    return;
  }

  oop obj = CompressedOops::decode_not_null(heap_oop);
  assert(Universe::heap()->is_in(obj), "should be in heap");
  if (G1ArchiveAllocator::is_archived_object(obj)) {
    // We never forward archive objects.
    return;
  }
  if (true)
  {
/*
  oop group_start=0;
  oop group_end=0;
  unsigned long group_pd;
  unsigned long group_nd;
*/
  //cgmin adjust pointer modify
//  G1CollectedHeap* g1h = G1CollectedHeap::heap();
  HeapRegion* hr = G1CollectedHeap::heap()->hrm()->addr_to_region((HeapWord*)(obj));
  unsigned long *pnMap = hr->_pnMap;
//  unsigned long *ndMap = hr->_ndMap;
  int mapIndex = ((unsigned long)(HeapWord*)obj - hr->_b4)/(4096);
  if (pnMap[mapIndex] != 0)
  {
    unsigned long *dvMap = hr->_dvMap;
    if (pnMap[mapIndex] == 1)
      RawAccess<IS_NOT_NULL>::oop_store(p, oop((HeapWord*)obj + dvMap[mapIndex]));
    else if (pnMap[mapIndex] == 2)
      RawAccess<IS_NOT_NULL>::oop_store(p, oop((HeapWord*)obj - dvMap[mapIndex]));
    return;
  }
  /*
  if (pdMap[mapIndex] != 0)
  {
      RawAccess<IS_NOT_NULL>::oop_store(p, oop((HeapWord*)obj + pdMap[mapIndex]));
      return;
      }

  if (ndMap[mapIndex] != 0) // OR
  {
    
      oop forwardee = oop((HeapWord*)obj + pdMap[mapIndex] - ndMap[mapIndex]);
      if (forwardee != obj->forwardee())
      {
        printf("%p != %p | %p %d %lu %lu\n", obj->forwardee(),forwardee,obj,mapIndex,pdMap[mapIndex],ndMap[mapIndex]);
        forwardee = obj->forwardee();
        }
      RawAccess<IS_NOT_NULL>::oop_store(p, forwardee);

      RawAccess<IS_NOT_NULL>::oop_store(p, oop((HeapWord*)obj - ndMap[mapIndex]));
//  RawAccess<IS_NOT_NULL>::oop_store(p, obj->forwardee());

    return;
    
    
    if (obj->forwardee() != oop((HeapWord*)obj + group_pd - group_nd))
    {
      printf("%p != %p | %p %p %p %lu %lu\n", obj->forwardee(),oop((HeapWord*)obj + group_pd - group_nd),obj,group_start,group_end,group_pd,group_nd);
    }
    
    
    printf("%p %p %lu %lu\n",group_start,group_end,group_pd,group_nd);
    if (oop(((HeapWord*)obj + group_pd) - group_nd) != obj->forwardee())
      printf("error %p + %ld = %p != %p\n",obj,(long int)group_pd-(long int)group_nd,(void*)(((HeapWord*)obj+group_pd)-group_nd),obj->forwardee());
      else
      printf("T %p okoko %p + %ld = %p != %p\n",p,obj,(long int)group_pd-(long int)group_nd,(void*)(((HeapWord*)obj+group_pd)-group_nd),obj->forwardee());

  }
  */
} // false
  oop forwardee = obj->forwardee();
  if (forwardee == NULL) {
    // Not forwarded, return current reference.
    assert(obj->mark_raw() == markOopDesc::prototype_for_object(obj) || // Correct mark
           obj->mark_raw()->must_be_preserved(obj) || // Will be restored by PreservedMarksSet
           (UseBiasedLocking && obj->has_bias_pattern_raw()), // Will be restored by BiasedLocking
           "Must have correct prototype or be preserved, obj: " PTR_FORMAT ", mark: " PTR_FORMAT ", prototype: " PTR_FORMAT,
           p2i(obj), p2i(obj->mark_raw()), p2i(markOopDesc::prototype_for_object(obj)));
    return;
  }

  // Forwarded, just update.
//  if (Universe::heap()->is_in_reserved(forwardee) == false)
//    printf("error %p %p\n",(HeapWord*)obj,(HeapWord*)forwardee);
  assert(Universe::heap()->is_in_reserved(forwardee), "should be in object space");
  RawAccess<IS_NOT_NULL>::oop_store(p, forwardee);
}

inline void G1AdjustClosure::do_oop(oop* p)       { do_oop_work(p); }
inline void G1AdjustClosure::do_oop(narrowOop* p) { do_oop_work(p); }

inline bool G1IsAliveClosure::do_object_b(oop p) {
  return _bitmap->is_marked(p) || G1ArchiveAllocator::is_closed_archive_object(p);
}

template<typename T>
inline void G1FullKeepAliveClosure::do_oop_work(T* p) {
  _marker->mark_and_push(p);
}

#endif
