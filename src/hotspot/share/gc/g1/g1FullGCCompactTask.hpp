/*
 * Copyright (c) 2017, Oracle and/or its affiliates. All rights reserved.
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

#ifndef SHARE_GC_G1_G1FULLGCCOMPACTTASK_HPP
#define SHARE_GC_G1_G1FULLGCCOMPACTTASK_HPP

#include "gc/g1/g1FullGCCompactionPoint.hpp"
#include "gc/g1/g1FullGCScope.hpp"
#include "gc/g1/g1FullGCTask.hpp"
#include "gc/g1/g1StringDedup.hpp"
#include "gc/g1/heapRegionManager.hpp"
#include "gc/shared/referenceProcessor.hpp"

class G1CollectedHeap;
class G1CMBitMap;

class G1FullGCCompactTask : public G1FullGCTask {
protected:
  HeapRegionClaimer _claimer;

private:
  void compact_region(HeapRegion* hr);

public:
  G1FullGCCompactTask(G1FullCollector* collector) :
    G1FullGCTask("G1 Compact Task", collector),
    _claimer(collector->workers()) { } 
  void work(uint worker_id);
  void serial_compaction();

  class G1CompactRegionClosure : public StackObj {
    G1CMBitMap* _bitmap;

  public:
    G1CompactRegionClosure(G1CMBitMap* bitmap) : _bitmap(bitmap) {
t2_sum = 0;
cgmin_s2 = cgmin_b2 = s2_sum = b2_sum = 0;
 }
~G1CompactRegionClosure() {
// printf("T %d\n",t2_sum);
//printf("S %d %d %d %d\n",cgmin_s2,s2_sum,cgmin_b2,b2_sum);
 }

    size_t apply(oop object);
int t2_sum; //cgmin
int cgmin_s2,cgmin_b2,s2_sum,b2_sum;
  };
};

#endif // SHARE_GC_G1_G1FULLGCCOMPACTTASK_HPP
