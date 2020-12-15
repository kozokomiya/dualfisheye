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
 * Free Software Foundation, Inc., 51 Franklin Street, Suite 500,
 * Boston, MA 02110-1335, USA.
 */
/**
 * SECTION:element-gstdualfisheye
 *
 * The dualfisheye element does dual-fisheye to equirectangular conversion stuff.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch-1.0 -v videotestsrc ! video/x-raw,format=RGBx,width=1280,height=720 ! dualfisheye ! autovideosink
 * ]|
 * Convert input video soruce from dual-fisheye to equirectangular.
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gst/gst.h>
#include <gst/video/video.h>
#include <gst/video/gstvideofilter.h>
#include <string>
#include "numpy.hpp"
#include "gstdualfisheye.hpp"

GST_DEBUG_CATEGORY_STATIC (gst_dualfisheye_debug_category);
#define GST_CAT_DEFAULT gst_dualfisheye_debug_category

/* prototypes */

static void gst_dualfisheye_set_property (GObject * object,
    guint property_id, const GValue * value, GParamSpec * pspec);
static void gst_dualfisheye_get_property (GObject * object,
    guint property_id, GValue * value, GParamSpec * pspec);
static void gst_dualfisheye_dispose (GObject * object);
static void gst_dualfisheye_finalize (GObject * object);

static gboolean gst_dualfisheye_start (GstBaseTransform * trans);
static gboolean gst_dualfisheye_stop (GstBaseTransform * trans);
static gboolean gst_dualfisheye_set_info (GstVideoFilter * filter,
    GstCaps * incaps, GstVideoInfo * in_info, GstCaps * outcaps,
    GstVideoInfo * out_info);
// static GstFlowReturn gst_dualfisheye_transform_frame (GstVideoFilter * filter,
//     GstVideoFrame * inframe, GstVideoFrame * outframe);
static GstFlowReturn gst_dualfisheye_transform_frame_ip (GstVideoFilter *
    filter, GstVideoFrame * frame);

static int c16toi(char c);
static void set_border_scalar(GstDualfisheye * dualfisheye);
static void dispsize(GstDualfisheye *dualfisheye, cv::Mat &mat);
static void border(GstDualfisheye *dualfisheye, cv::Mat &mat);
static void antirotate(GstDualfisheye *dualfisheye, cv::Mat &mat);
static void equirectangular(GstDualfisheye *dualfisheye, cv::Mat &mat);

enum
{
  PROP_0,
  PROP_ANTIROTATE,
  PROP_EQUIRECTANGULAR,
  PROP_DISPSIZE,
  PROP_BORDERWIDTH,
  PROP_BORDERCOLOR
};

#define DEFAULT_PROP_ANTIROTATE FALSE
#define DEFAULT_PROP_EQUIRECTANGULAR TRUE
#define DEFAULT_PROP_DISPSIZE FALSE
#define DEFAULT_PROP_BORDERWIDTH 0
#define DEFAULT_PROP_BORDERCOLOR "#000000"

/* pad templates */

/* FIXME: add/remove formats you can handle */
// #define VIDEO_SRC_CAPS GST_VIDEO_CAPS_MAKE("{ I420, Y444, Y42B, UYVY, RGBA }")
#define VIDEO_SRC_CAPS GST_VIDEO_CAPS_MAKE("{ RGBx, RGBA }")

/* FIXME: add/remove formats you can handle */
// #define VIDEO_SINK_CAPS GST_VIDEO_CAPS_MAKE("{ I420, Y444, Y42B, UYVY, RGBA }")
#define VIDEO_SINK_CAPS GST_VIDEO_CAPS_MAKE("{ RGBx, RGBA }")


/* class initialization */

G_DEFINE_TYPE_WITH_CODE (GstDualfisheye, gst_dualfisheye, GST_TYPE_VIDEO_FILTER,
    GST_DEBUG_CATEGORY_INIT (gst_dualfisheye_debug_category, "dualfisheye", 0,
        "debug category for dualfisheye element"));

static void
gst_dualfisheye_class_init (GstDualfisheyeClass * klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GstBaseTransformClass *base_transform_class =
      GST_BASE_TRANSFORM_CLASS (klass);
  GstVideoFilterClass *video_filter_class = GST_VIDEO_FILTER_CLASS (klass);

  /* Setting up pads and setting metadata should be moved to
     base_class_init if you intend to subclass this class. */
  gst_element_class_add_pad_template (GST_ELEMENT_CLASS (klass),
      gst_pad_template_new ("src", GST_PAD_SRC, GST_PAD_ALWAYS,
          gst_caps_from_string (VIDEO_SRC_CAPS)));
  gst_element_class_add_pad_template (GST_ELEMENT_CLASS (klass),
      gst_pad_template_new ("sink", GST_PAD_SINK, GST_PAD_ALWAYS,
          gst_caps_from_string (VIDEO_SINK_CAPS)));

  gst_element_class_set_static_metadata (GST_ELEMENT_CLASS (klass),
      "Dualfisheye", "Filter", "Dual Fisheye Converter (RGBx)",
      "Kozo Komiya <kozo.komiya@gmail.com");

  gobject_class->set_property = gst_dualfisheye_set_property;
  gobject_class->get_property = gst_dualfisheye_get_property;

  g_object_class_install_property (gobject_class, PROP_ANTIROTATE,
      g_param_spec_boolean (
          "antirotate", 
          "Antirotate", 
          "Do Antirotate Processing",
          DEFAULT_PROP_ANTIROTATE,
          GParamFlags(GST_PARAM_CONTROLLABLE | G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE)
      )
  );
  g_object_class_install_property (gobject_class, PROP_DISPSIZE,
      g_param_spec_boolean (
          "dispsize", 
          "Dispsize", 
          "Display Size Value",
          DEFAULT_PROP_DISPSIZE,
          GParamFlags(GST_PARAM_CONTROLLABLE | G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE)
      )
  );
  g_object_class_install_property (gobject_class, PROP_EQUIRECTANGULAR,
      g_param_spec_boolean (
          "equirectangular", 
          "Equirectangular", 
          "Do Equirectangular Conversion",
          DEFAULT_PROP_EQUIRECTANGULAR,
          GParamFlags(GST_PARAM_CONTROLLABLE | G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE)
      )
  );
  g_object_class_install_property (gobject_class, PROP_BORDERWIDTH,
      g_param_spec_uint (
          "borderwidth", 
          "BorderWidth", 
          "Center Border Width",
          0,
          G_MAXUINT,
          DEFAULT_PROP_BORDERWIDTH,
          GParamFlags(GST_PARAM_CONTROLLABLE | G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE)
      )
  );
  g_object_class_install_property (gobject_class, PROP_BORDERCOLOR,
      g_param_spec_string (
          "bordercolor", 
          "BorderColor", 
          "Center Border Color",
          DEFAULT_PROP_BORDERCOLOR,
          GParamFlags(GST_PARAM_CONTROLLABLE | G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE)
      )
  );
  
  gobject_class->dispose = gst_dualfisheye_dispose;
  gobject_class->finalize = gst_dualfisheye_finalize;
  base_transform_class->start = GST_DEBUG_FUNCPTR (gst_dualfisheye_start);
  base_transform_class->stop = GST_DEBUG_FUNCPTR (gst_dualfisheye_stop);
  video_filter_class->set_info = GST_DEBUG_FUNCPTR (gst_dualfisheye_set_info);
  // video_filter_class->transform_frame =
  //     GST_DEBUG_FUNCPTR (gst_dualfisheye_transform_frame);
  video_filter_class->transform_frame_ip =
      GST_DEBUG_FUNCPTR (gst_dualfisheye_transform_frame_ip);
}

static int c16toi(char c) {
  if ('0' <= c && c <= '9') return c - '0';
  if ('A' <= c && c <= 'F') return c - 'A' + 10;
  if ('a' <= c && c <= 'f') return c - 'a' + 10;
  return 0;
}

static void 
set_border_scalar (GstDualfisheye * self)
{
  const gchar *str = self->border_color;

  if (strlen(str) < 7) {
    GST_DEBUG_OBJECT(self, "border_color format error (%s)", str);
    return;
  }
  guint8 b = c16toi(str[1]) * 16 + c16toi(str[2]);
  guint8 g = c16toi(str[3]) * 16 + c16toi(str[4]);
  guint8 r = c16toi(str[5]) * 16 + c16toi(str[6]);
  self->border_scalar = cv::Scalar(b, g, r);
}

static void
gst_dualfisheye_init (GstDualfisheye * self)
{
  GST_DEBUG_OBJECT (self, "gst_dualfisheye_init()");

  int rows, cols;
  std::vector<gfloat> data;
  try {
    aoba::LoadArrayFromNumpy("./xmap.npy", rows, cols, data);
    cv::Mat mat(rows, cols, CV_32F, (void *)data.data());
    GST_DEBUG_OBJECT (self, "load xmap size=(%dx%d)", rows, cols);
    self->xmap = mat.clone();
  } 
  catch (const std::runtime_error &e) {
    GST_DEBUG_OBJECT (self, "Can't read 'xmap.npy'");
    exit(1);
  }
  try {
    aoba::LoadArrayFromNumpy("./ymap.npy", rows, cols, data);
    cv::Mat mat(rows, cols, CV_32F, (void *)data.data());
    GST_DEBUG_OBJECT (self, "load ymap size=(%dx%d)", rows, cols);
    self->ymap = mat.clone();
  } 
  catch (const std::runtime_error &e) {
    GST_DEBUG_OBJECT (self, "Can't read 'ymap.npy'");
    exit(1);
  }

  self->antirotate = DEFAULT_PROP_ANTIROTATE;
  self->dispsize = DEFAULT_PROP_DISPSIZE;
  self->equirectangular = DEFAULT_PROP_EQUIRECTANGULAR;
  self->border_width = DEFAULT_PROP_BORDERWIDTH;
  self->border_color = DEFAULT_PROP_BORDERCOLOR;
  set_border_scalar(self);
}

static void dispsize(GstDualfisheye *self, cv::Mat &mat)
{
  char buf[64];

  g_snprintf(buf, 64, "size: %dx%d", mat.size().width, mat.size().height);
  cv::putText(mat, buf, cv::Point(20, 80), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(20,230,20), 1, cv::LINE_AA);
}

static void border(GstDualfisheye *self, cv::Mat &mat)
{
  guint w1 = self->border_width / 2;
  guint w = mat.size().width;
  guint h = mat.size().height;

  cv::rectangle(mat, cv::Point(w/4 - w1, 0), cv::Point(w/4+w1, h), self->border_scalar, -1);
  cv::rectangle(mat, cv::Point(w/4*3 - w1, 0), cv::Point(w/4*3+w1, h), self->border_scalar, -1);
}

static void antirotate(GstDualfisheye *self, cv::Mat &mat)
{

}

static void equirectangular(GstDualfisheye *self, cv::Mat &mat)
{
  cv::remap(mat, mat, self->xmap, self->ymap, cv::INTER_NEAREST, cv::BORDER_CONSTANT);
}

void
gst_dualfisheye_set_property (GObject * object, guint property_id,
    const GValue * value, GParamSpec * pspec)
{
  GstDualfisheye *self = GST_DUALFISHEYE (object);

  GST_DEBUG_OBJECT (self, "set_property (id=%d)", property_id);

  switch (property_id) {
    case PROP_ANTIROTATE:
      self->antirotate = g_value_get_boolean(value);
      GST_DEBUG_OBJECT (self, "set antirotate (%d)", self->antirotate);
      break;
    case PROP_EQUIRECTANGULAR:
      self->equirectangular = g_value_get_boolean(value);
      GST_DEBUG_OBJECT (self, "set equirectangular (%d)", self->equirectangular);
      break;
    case PROP_DISPSIZE:
      self->dispsize = g_value_get_boolean(value);
      GST_DEBUG_OBJECT (self, "set dispsize (%d)", self->dispsize);
      break;
    case PROP_BORDERWIDTH:
      self->border_width = g_value_get_uint(value);
      GST_DEBUG_OBJECT (self, "set border_width (%d)", self->border_width);
      break;
    case PROP_BORDERCOLOR:
      self->border_color = g_value_get_string(value);
      set_border_scalar(self);
      GST_DEBUG_OBJECT (self, "set border_color (%s)", self->border_color);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

void
gst_dualfisheye_get_property (GObject * object, guint property_id,
    GValue * value, GParamSpec * pspec)
{
  GstDualfisheye *self = GST_DUALFISHEYE (object);

  GST_DEBUG_OBJECT (self, "get_property");

  switch (property_id) {
    case PROP_ANTIROTATE:
      g_value_set_boolean(value, self->antirotate);
      break;
    case PROP_EQUIRECTANGULAR:
      g_value_set_boolean(value, self->equirectangular);
      break;
    case PROP_DISPSIZE:
      g_value_set_boolean(value, self->dispsize);
      break;
    case PROP_BORDERWIDTH:
      g_value_set_uint(value, self->border_width);
      break;
    case PROP_BORDERCOLOR:
      g_value_set_string(value, self->border_color);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

void
gst_dualfisheye_dispose (GObject * object)
{
  GstDualfisheye *dualfisheye = GST_DUALFISHEYE (object);

  GST_DEBUG_OBJECT (dualfisheye, "dispose");

  /* clean up as possible.  may be called multiple times */

  G_OBJECT_CLASS (gst_dualfisheye_parent_class)->dispose (object);
}

void
gst_dualfisheye_finalize (GObject * object)
{
  GstDualfisheye *dualfisheye = GST_DUALFISHEYE (object);

  GST_DEBUG_OBJECT (dualfisheye, "finalize");

  /* clean up object here */

  G_OBJECT_CLASS (gst_dualfisheye_parent_class)->finalize (object);
}

static gboolean
gst_dualfisheye_start (GstBaseTransform * trans)
{
  GstDualfisheye *dualfisheye = GST_DUALFISHEYE (trans);

  GST_DEBUG_OBJECT (dualfisheye, "start");

  return TRUE;
}

static gboolean
gst_dualfisheye_stop (GstBaseTransform * trans)
{
  GstDualfisheye *dualfisheye = GST_DUALFISHEYE (trans);

  GST_DEBUG_OBJECT (dualfisheye, "stop");

  return TRUE;
}

static gboolean
gst_dualfisheye_set_info (GstVideoFilter * filter, GstCaps * incaps,
    GstVideoInfo * in_info, GstCaps * outcaps, GstVideoInfo * out_info)
{
  GstDualfisheye *dualfisheye = GST_DUALFISHEYE (filter);

  GST_DEBUG_OBJECT (dualfisheye, "set_info");

  return TRUE;
}

// /* transform */
// static GstFlowReturn
// gst_dualfisheye_transform_frame (GstVideoFilter * filter,
//     GstVideoFrame * inframe, GstVideoFrame * outframe)
// {
//   GstDualfisheye *self = GST_DUALFISHEYE (filter);
//
//   GST_DEBUG_OBJECT (self, "transform_frame");
//
//   return GST_FLOW_OK;
// }

static GstFlowReturn
gst_dualfisheye_transform_frame_ip (GstVideoFilter * filter,
    GstVideoFrame * frame)
{
  GstDualfisheye *self = GST_DUALFISHEYE (filter);
  gint width = GST_VIDEO_FRAME_COMP_WIDTH(frame, 0);
  gint height = GST_VIDEO_FRAME_COMP_HEIGHT(frame, 0);
  guint8 *frame_data = (guint8 *) GST_VIDEO_FRAME_PLANE_DATA(frame, 0);
  cv::Mat mat(height, width, CV_8UC4, frame_data);

  GST_LOG_OBJECT (self, "transform_frame_ip");
  if (self->equirectangular) {
    equirectangular(self, mat);
  }
  if (self->border_width > 0) {
    border(self, mat);
  }
  if (self->antirotate) {
    antirotate(self, mat);
  }
  if (self->dispsize) {
    dispsize(self, mat);
  }

  return GST_FLOW_OK;
}

static gboolean
plugin_init (GstPlugin * plugin)
{

  /* FIXME Remember to set the rank if it's an element that is meant
     to be autoplugged by decodebin. */
  return gst_element_register (plugin, "dualfisheye", GST_RANK_NONE,
      GST_TYPE_DUALFISHEYE);
}

/* FIXME: these are normally defined by the GStreamer build system.
   If you are creating an element to be included in gst-plugins-*,
   remove these, as they're always defined.  Otherwise, edit as
   appropriate for your external plugin package. */
#ifndef VERSION
#define VERSION "1.0.0"
#endif
#ifndef PACKAGE
#define PACKAGE "My Package"
#endif
#ifndef PACKAGE_NAME
#define PACKAGE_NAME "My First Package"
#endif
#ifndef GST_PACKAGE_ORIGIN
#define GST_PACKAGE_ORIGIN "http://FIXME.org/"
#endif

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    dualfisheye,
    "dual-fisheye converter",
    plugin_init, VERSION, "LGPL", PACKAGE_NAME, GST_PACKAGE_ORIGIN)
