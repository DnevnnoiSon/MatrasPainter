TEMPLATE = subdirs

SUBDIRS = \
    MatrasPainterLib \
    MatrasPainterApp

# Сборка библиотеки первой:
MatrasPainterApp.depends = MatrasPainterLib
