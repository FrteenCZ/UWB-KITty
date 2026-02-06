import bpy
from bpy.app.handlers import persistent
from . import operators, ui, properties
from .utils import objectProperties

bl_info = {
    "name": "UWB-KITty",
    "blender": (4, 0, 0),
    "category": "Development",
}

modules = [
    operators,
    ui,
    objectProperties,
    properties,
]

@persistent
def depsgraph_update_handler(scene):
    """Handler to sync active object with the device list."""
    if properties._is_updating_from_list:
        return

    active_obj = bpy.context.active_object
    if not active_obj:
        return

    props = scene.uwb_kitty_props
        
    for i, device in enumerate(props.devices):
        if device.blender_object_name == active_obj.name:
            if props.active_device_index != i:
                props.active_device_index = i
            break

def register():
    for mod in modules:
        mod.register()
    
    bpy.app.handlers.depsgraph_update_post.append(depsgraph_update_handler)


def unregister():
    for mod in modules:
        mod.unregister()

    bpy.app.handlers.depsgraph_update_post.remove(depsgraph_update_handler)


if __name__ == "__main__":
    register()
