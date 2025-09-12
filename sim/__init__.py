from . import operators, ui
# from .utils import ESPcom

bl_info = {
    "name": "UWB-KITty",
    "blender": (4, 0, 0),
    "category": "Development",
}

modules = [
    operators,
    ui,
]


def register():
    for mod in modules:
        mod.register()


def unregister():
    for mod in modules:
        mod.unregister()


if __name__ == "__main__":
    register()
