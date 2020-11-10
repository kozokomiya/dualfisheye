/* GStreamer
 * Copyright (C) 2020 FIXME <fixme@example.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef _GST_DUALFISHEYE_H_
#define _GST_DUALFISHEYE_H_

#include <gst/video/video.h>
#include <gst/video/gstvideofilter.h>
#include <opencv2/opencv.hpp>

G_BEGIN_DECLS

#define GST_TYPE_DUALFISHEYE   (gst_dualfisheye_get_type())
#define GST_DUALFISHEYE(obj)   (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_DUALFISHEYE,GstDualfisheye))
#define GST_DUALFISHEYE_CLASS(klass)   (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_DUALFISHEYE,GstDualfisheyeClass))
#define GST_IS_DUALFISHEYE(obj)   (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_DUALFISHEYE))
#define GST_IS_DUALFISHEYE_CLASS(obj)   (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_DUALFISHEYE))

typedef struct _GstDualfisheye GstDualfisheye;
typedef struct _GstDualfisheyeClass GstDualfisheyeClass;

struct _GstDualfisheye
{
  GstVideoFilter base_dualfisheye;

  gboolean equirectangular;
  gboolean antirotate;
  gboolean dispsize;
  guint border_width;
  const gchar* border_color;
  cv::Scalar border_scalar;
  cv::Mat xmap;
  cv::Mat ymap;
};

struct _GstDualfisheyeClass
{
  GstVideoFilterClass base_dualfisheye_class;
};

GType gst_dualfisheye_get_type (void);

G_END_DECLS

#endif
