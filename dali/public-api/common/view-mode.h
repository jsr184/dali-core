#ifndef __DALI_VIEW_MODE_H__
#define __DALI_VIEW_MODE_H__

//
// Copyright (c) 2014 Samsung Electronics Co., Ltd.
//
// Licensed under the Flora License, Version 1.0 (the License);
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://floralicense.org/license/
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an AS IS BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

namespace Dali
{

/**
 * Stereoscopic view modes
 */
enum ViewMode
{
  MONO,              ///< Monoscopic (single camera). This is the default
  STEREO_HORIZONTAL, ///< Stereoscopic. Frame buffer is split horizontally with the left and right camera views in their respective sides.
  STEREO_VERTICAL,   ///< Stereoscopic. Frame buffer is split vertically with the left camera view at the top and the right camera view at the bottom.
  STEREO_INTERLACED  ///< Stereoscopic. Left/Right camera views are rendered into the framebuffer on alternate frames.
};

} // namespace Dali

#endif // __DALI_VIEW_MODE_H__
