from . import draw_toggle

modules = [
    draw_toggle,
]


def register():
    for mod in modules:
        mod.register()


def unregister():
    for mod in modules:
        mod.unregister()
