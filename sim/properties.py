import bpy # type: ignore

class DeviceProperties(bpy.types.PropertyGroup):
    """Properties for a single UWB device (for persistence)"""
    id: bpy.props.StringProperty(name="Device ID")
    blender_object_name: bpy.props.StringProperty(name="Blender Object Name")
    role: bpy.props.EnumProperty(
        name="Role",
        items=[
            ("TAG", "Tag", "A device that is being tracked"),
            ("ANCHOR", "Anchor", "A fixed device used for reference"),
            ("NONE", "None", "No specific role"),
        ],
    )

class AddDeviceDialogProperties(bpy.types.PropertyGroup):
    """Properties for the Add Device dialog"""
    device_id: bpy.props.StringProperty(name="Device ID")
    blender_object: bpy.props.PointerProperty(
        name="Blender Object",
        type=bpy.types.Object
    )
    role: bpy.props.EnumProperty(
        name="Role",
        items=[
            ("TAG", "Tag", "A device that is being tracked"),
            ("ANCHOR", "Anchor", "A fixed device used for reference"),
            ("NONE", "None", "No specific role"),
        ],
    )

class UWBKittyProperties(bpy.types.PropertyGroup):
    """Addon properties"""
    devices: bpy.props.CollectionProperty(type=DeviceProperties)
    active_device_index: bpy.props.IntProperty()
    add_device_props: bpy.props.PointerProperty(type=AddDeviceDialogProperties)

def register():
    bpy.utils.register_class(DeviceProperties)
    bpy.utils.register_class(AddDeviceDialogProperties)
    bpy.utils.register_class(UWBKittyProperties)
    bpy.types.Scene.uwb_kitty_props = bpy.props.PointerProperty(type=UWBKittyProperties)

def unregister():
    del bpy.types.Scene.uwb_kitty_props
    bpy.utils.unregister_class(UWBKittyProperties)
    bpy.utils.unregister_class(AddDeviceDialogProperties)
    bpy.utils.unregister_class(DeviceProperties)
