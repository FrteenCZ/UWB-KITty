from . import operators, ui
from .utils import objectProperties

bl_info = {
    "name": "UWB-KITty",
    "blender": (4, 0, 0),
    "category": "Development",
}

modules = [
    operators,
    ui,
    objectProperties
]


def register():
    for mod in modules:
        mod.register()


def unregister():
    for mod in modules:
        mod.unregister()


if __name__ == "__main__":
    register()
