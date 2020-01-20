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

#include "precompiled.hpp"
#include "classfile/classLoaderDataGraph.hpp"
#include "gc/g1/g1CollectedHeap.hpp"
#include "gc/g1/g1FullCollector.hpp"
#include "gc/g1/g1FullGCMarker.hpp"
#include "gc/g1/g1FullGCMarkTask.hpp"
#include "gc/g1/g1FullGCOopClosures.inline.hpp"
#include "gc/g1/g1FullGCReferenceProcessorExecutor.hpp"
#include "gc/shared/gcTraceTime.inline.hpp"
#include "gc/shared/referenceProcessor.hpp"
#include "memory/iterator.inline.hpp"

G1FullGCMarkTask::G1FullGCMarkTask(G1FullCollector* collector) :
    G1FullGCTask("G1 Parallel Marking Task", collector),
    _root_processor(G1CollectedHeap::heap(), collector->workers()),
    _terminator(collector->workers(), collector->array_queue_set()) {
  // Need cleared claim bits for the roots processing
  ClassLoaderDataGraph::clear_claimed_marks();
}

void G1FullGCMarkTask::work(uint worker_id) {
  Ticks start = Ticks::now();
  ResourceMark rm;
  G1FullGCMarker* marker = collector()->marker(worker_id);
  MarkingCodeBlobClosure code_closure(marker->mark_closure(), !CodeBlobToOopClosure::FixRelocations);

  if (ClassUnloading) {
    _root_processor.process_strong_roots(
        marker->mark_closure(),
        marker->cld_closure(),
        &code_closure);
  } else {
    _root_processor.process_all_roots_no_string_table(
        marker->mark_closure(),
        marker->cld_closure(),
        &code_closure);
  }

  // Mark stack is populated, now process and drain it.
  marker->complete_marking(collector()->oop_queue_set(), collector()->array_queue_set(), _terminator.terminator());

  // This is the point where the entire marking should have completed.
  assert(marker->oop_stack()->is_empty(), "Marking should have completed");
  assert(marker->objarray_stack()->is_empty(), "Array marking should have completed");
  log_task("Marking task", worker_id, start);
}


//cgmin
G1FullGCMarkGroupTask::G1FullGCMarkGroupTask(G1FullCollector* collector) :
    G1FullGCTask("cgmin G1 Group Marking Task", collector),
    _hrclaimer(collector->workers()) {
}


void G1FullGCMarkGroupTask::work(uint worker_id) {

  G1FindGroupClosure closure(collector()->mark_bitmap());
  G1CollectedHeap::heap()->heap_region_par_iterate_from_start(&closure,&_hrclaimer);

}

G1FullGCMarkGroupTask::G1FindGroupClosure::G1FindGroupClosure(G1CMBitMap* bitmap) :
//    _g1h(G1CollectedHeap::heap()),
    _bitmap(bitmap) {}

bool G1FullGCMarkGroupTask::G1FindGroupClosure::do_heap_region(HeapRegion *hr) {
  if (hr->is_humongous()) {
//???
  }
  else if (hr->is_pinned())
  {
//???
  }
  else
  {
    hr->find_group(_bitmap);
    /*
    HeapWord* limit = hr->scan_limit();
    HeapWord* next_addr = hr->bottom();
    HeapWord* zero;
    if (!_bitmap->is_marked(next_addr))
      next_addr = _bitmap->get_next_marked_addr(next_addr, limit);
    while (next_addr < limit) {
      zero = _bitmap->get_next_unmarked_addr(next_addr, limit);
      printf("%lu\n",pointer_delta(zero,next_addr));//next_addr-zero);
      next_addr = _bitmap->get_next_marked_addr(zero,limit);
    }
    */
  }
  return false;
}

