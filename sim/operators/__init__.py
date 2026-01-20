from . import draw_toggle, draw_distances_toggle, serial_modal, serial_stop

modules = [
    draw_toggle,
    draw_distances_toggle,
    serial_modal,
    serial_stop,
]


def register():
    for mod in modules:
        mod.register()


def unregister():
    for mod in modules:
        mod.unregister()
