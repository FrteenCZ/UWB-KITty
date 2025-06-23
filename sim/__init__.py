from . import operators, ui_panel

bl_info = {
    "name": "UWB-KITty",
    "blender": (4, 0, 0),
    "category": "3D View",
}

modules = [
    operators,
    ui_panel,
]


def register():
    for mod in modules:
        mod.register()


def unregister():
    for mod in modules:
        mod.unregister()


if __name__ == "__main__":
    register()
