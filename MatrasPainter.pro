TEMPLATE = subdirs

SUBDIRS = \
    MatrasPainterLib \
    MatrasPainterApp

MatrasPainterApp.depends = MatrasPainterLib
