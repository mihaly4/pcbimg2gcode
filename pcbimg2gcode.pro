TEMPLATE = subdirs

CONFIG += ordered

QMAKE_CXXFLAGS += -Wno-unused-parameter -Wno-unused-variable -Wno-deprecated-declarations -Wno-unused-parameter

SUBDIRS += \
img2gcode \
gcode2img

