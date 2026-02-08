from . import draw_toggle, draw_distances_toggle, serial_modal, manager_modal, tick_modal

modules = [
    draw_toggle,
    draw_distances_toggle,
    serial_modal,
    manager_modal,
    tick_modal,
]


def register():
    for mod in modules:
        mod.register()


def unregister():
    for mod in modules:
        mod.unregister()
