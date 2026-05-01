QT += core gui widgets network
CONFIG += c++17
TARGET = icp_petri
TEMPLATE = app

SOURCES += \
    main.cpp \
    model/pn_place.cpp \
    model/pn_transition.cpp \
    model/pn_arc.cpp \
    model/pn_net.cpp \
    model/pn_file_parser.cpp \
    model/pn_file_writer.cpp \
    codegen/code_generator.cpp \
    network/udp_client.cpp \
    gui/main_window.cpp \
    gui/app_controller.cpp \
    gui/graphics_editor.cpp \
    gui/graphics_place_item.cpp \
    gui/graphics_transition_item.cpp \
    gui/graphics_arc_item.cpp \
    gui/properties_panel.cpp \
    gui/monitor_panel.cpp \
    gui/inject_panel.cpp \
    gui/event_log_view.cpp \
    gui/monitor_adapter.cpp \
    gui/dialogs/new_net_dialog.cpp \
    gui/dialogs/place_dialog.cpp \
    gui/dialogs/transition_dialog.cpp \
    gui/dialogs/arc_dialog.cpp \
    gui/dialogs/variables_dialog.cpp

HEADERS += \
    inc/udp_protocol.h \
    inc/pn_model.h \
    model/pn_place.h \
    model/pn_transition.h \
    model/pn_arc.h \
    model/pn_net.h \
    model/pn_file_parser.h \
    model/pn_file_writer.h \
    codegen/code_generator.h \
    network/udp_client.h \
    gui/main_window.h \
    gui/app_controller.h \
    gui/graphics_editor.h \
    gui/graphics_place_item.h \
    gui/graphics_transition_item.h \
    gui/graphics_arc_item.h \
    gui/properties_panel.h \
    gui/monitor_panel.h \
    gui/inject_panel.h \
    gui/event_log_view.h \
    gui/monitor_adapter.h \
    gui/dialogs/new_net_dialog.h \
    gui/dialogs/place_dialog.h \
    gui/dialogs/transition_dialog.h \
    gui/dialogs/arc_dialog.h \
    gui/dialogs/variables_dialog.h

INCLUDEPATH += . inc model engine codegen network gui
