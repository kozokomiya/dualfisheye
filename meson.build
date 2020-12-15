# Project Name
project('gstdualfisheye', 'cpp', default_options : ['cpp_std=c++11'])

plugin_c_args = []

gst_dep = dependency('gstreamer-1.0',
    fallback: ['gstreamer', 'gst_dep']
)
gst_video_dep = dependency('gstreamer-video-1.0',
    fallback: ['gstreamer-video', 'gst_video_dep']
)
opencv_dep = dependency('opencv4',
    fallback: ['opencv', 'opencv_dep']
)

dualfisheye_sources = [
    'src/gstdualfisheye.cpp',
]

gstdualfisheye = library('gstdualfisheye',
    dualfisheye_sources,
    c_args: plugin_c_args,
    dependencies: [gst_dep, gst_video_dep, opencv_dep]
)
