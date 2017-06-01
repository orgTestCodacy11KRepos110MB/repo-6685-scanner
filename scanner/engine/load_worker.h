/* Copyright 2016 Carnegie Mellon University
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include "scanner/engine/runtime.h"
#include "scanner/util/common.h"
#include "scanner/util/queue.h"

namespace scanner {
namespace internal {

struct LoadWorkerArgs {
  // Uniform arguments
  i32 node_id;
  // Per worker arguments
  int worker_id;
  storehouse::StorageConfig* storage_config;
  Profiler& profiler;
  i32 load_sparsity_threshold;
};

class LoadWorker {
 public:
  LoadWorker(const LoadWorkerArgs& args);

  std::tuple<IOItem, EvalWorkEntry> execute(
      std::tuple<IOItem, LoadWorkEntry>& entry);

 private:
  struct VideoIndexEntry {
    i32 width;
    i32 height;
    i32 channels;
    FrameType frame_type;
    proto::VideoDescriptor::VideoCodecType codec_type;
    std::unique_ptr<storehouse::RandomReadFile> file;
    u64 file_size;
    std::vector<i64> keyframe_positions;
    std::vector<i64> keyframe_byte_offsets;
  };

  VideoIndexEntry read_video_index(i32 table_id, i32 column_id, i32 item_id);

  void read_other_column(i32 table_id, i32 column_id, i32 item_id,
                         i32 item_start, i32 item_end,
                         const std::vector<i64>& rows,
                         ElementList& element_list);

  void read_video_column(const LoadWorker::VideoIndexEntry& index_entry,
                         const std::vector<i64>& rows,
                         i64 start_offset,
                         ElementList& element_list);
  const i32 node_id_;
  const i32 worker_id_;
  Profiler& profiler_;
  // Setup a distinct storage backend for each IO thread
  std::unique_ptr<storehouse::StorageBackend> storage_;
  // Caching table metadata
  std::map<i32, TableMetadata> table_metadata_;
  // To ammortize opening files
  i32 last_table_id_ = -1;
  std::map<std::tuple<i32, i32, i32>, VideoIndexEntry> index_;
  i32 load_sparsity_threshold_;

};

}
}
