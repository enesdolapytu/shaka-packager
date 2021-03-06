// Copyright 2014 Google Inc. All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd
//
// Defines the muxer interface.

#ifndef PACKAGER_MEDIA_BASE_MUXER_H_
#define PACKAGER_MEDIA_BASE_MUXER_H_

#include <memory>
#include <vector>

#include "packager/base/time/clock.h"
#include "packager/media/base/media_handler.h"
#include "packager/media/base/muxer_options.h"
#include "packager/media/event/muxer_listener.h"
#include "packager/media/event/progress_listener.h"
#include "packager/status.h"

namespace shaka {
namespace media {

class MediaSample;

/// Muxer is responsible for taking elementary stream samples and producing
/// media containers. An optional KeySource can be provided to Muxer
/// to generate encrypted outputs.
class Muxer : public MediaHandler {
 public:
  explicit Muxer(const MuxerOptions& options);
  virtual ~Muxer();

  /// Cancel a muxing job in progress. Will cause @a Run to exit with an error
  /// status of type CANCELLED.
  void Cancel();

  /// Set a MuxerListener event handler for this object.
  /// @param muxer_listener should not be NULL.
  void SetMuxerListener(std::unique_ptr<MuxerListener> muxer_listener);

  /// Set a ProgressListener event handler for this object.
  /// @param progress_listener should not be NULL.
  void SetProgressListener(std::unique_ptr<ProgressListener> progress_listener);

  const std::vector<std::shared_ptr<const StreamInfo>>& streams() const {
    return streams_;
  }

  /// Inject clock, mainly used for testing.
  /// The injected clock will be used to generate the creation time-stamp and
  /// modification time-stamp of the muxer output.
  /// If no clock is injected, the code uses base::Time::Now() to generate the
  /// time-stamps.
  /// @param clock is the Clock to be injected.
  void set_clock(base::Clock* clock) {
    clock_ = clock;
  }

 protected:
  /// @name MediaHandler implementation overrides.
  /// @{
  Status InitializeInternal() override { return Status::OK; }
  Status Process(std::unique_ptr<StreamData> stream_data) override;
  Status OnFlushRequest(size_t input_stream_index) override { return Finalize(); }
  /// @}

  const MuxerOptions& options() const { return options_; }
  MuxerListener* muxer_listener() { return muxer_listener_.get(); }
  ProgressListener* progress_listener() { return progress_listener_.get(); }
  base::Clock* clock() { return clock_; }

 private:
  // Initialize the muxer.
  virtual Status InitializeMuxer() = 0;

  // Final clean up.
  virtual Status Finalize() = 0;

  // Add a new sample.
  virtual Status AddSample(
      size_t stream_id,
      const MediaSample& sample) = 0;

  // Finalize the segment or subsegment.
  virtual Status FinalizeSegment(
      size_t stream_id,
      const SegmentInfo& segment_info) = 0;

  MuxerOptions options_;
  std::vector<std::shared_ptr<const StreamInfo>> streams_;
  std::vector<uint8_t> current_key_id_;
  bool encryption_started_ = false;
  bool cancelled_;

  std::unique_ptr<MuxerListener> muxer_listener_;
  std::unique_ptr<ProgressListener> progress_listener_;
  // An external injected clock, can be NULL.
  base::Clock* clock_;

  DISALLOW_COPY_AND_ASSIGN(Muxer);
};

}  // namespace media
}  // namespace shaka

#endif  // PACKAGER_MEDIA_BASE_MUXER_H_
