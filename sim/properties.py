import bpy  # type: ignore

_is_updating_from_list = False


def update_active_object(self, context):
    """When the active device index changes, update the active object in the scene."""
    global _is_updating_from_list
    if _is_updating_from_list:
        return

    if self.active_device_index < len(self.devices):
        device_prop = self.devices[self.active_device_index]
        obj = bpy.data.objects.get(device_prop.blender_object_name)
        if obj:
            try:
                _is_updating_from_list = True
                context.view_layer.objects.active = obj
            finally:
                _is_updating_from_list = False


class DeviceProperties(bpy.types.PropertyGroup):
    """Properties for a single UWB device (for persistence)"""
    id: bpy.props.StringProperty(name="Device ID")  # type: ignore
    blender_object_name: bpy.props.StringProperty(
        name="Blender Object Name")  # type: ignore
    role: bpy.props.EnumProperty(
        name="Role",
        items=[
            ("TAG", "Tag", "A device that is being tracked"),
            ("ANCHOR", "Anchor", "A fixed device used for reference"),
            ("NONE", "None", "No specific role"),
        ],
    )  # type: ignore


class AddDeviceDialogProperties(bpy.types.PropertyGroup):
    """Properties for the Add Device dialog"""
    device_id: bpy.props.StringProperty(name="Device ID")  # type: ignore
    blender_object: bpy.props.PointerProperty(
        name="Blender Object",
        type=bpy.types.Object
    )  # type: ignore
    role: bpy.props.EnumProperty(
        name="Role",
        items=[
            ("TAG", "Tag", "A device that is being tracked"),
            ("ANCHOR", "Anchor", "A fixed device used for reference"),
            ("NONE", "None", "No specific role"),
        ],
    )  # type: ignore


class UWBKittyProperties(bpy.types.PropertyGroup):
    """Addon properties"""
    devices: bpy.props.CollectionProperty(
        type=DeviceProperties)  # type: ignore
    active_device_index: bpy.props.IntProperty(
        update=update_active_object)  # type: ignore
    add_device_props: bpy.props.PointerProperty(
        type=AddDeviceDialogProperties)  # type: ignore


def register():
    bpy.utils.register_class(DeviceProperties)
    bpy.utils.register_class(AddDeviceDialogProperties)
    bpy.utils.register_class(UWBKittyProperties)
    bpy.types.Scene.uwb_kitty_props = bpy.props.PointerProperty(
        type=UWBKittyProperties)


def unregister():
    del bpy.types.Scene.uwb_kitty_props
    bpy.utils.unregister_class(UWBKittyProperties)
    bpy.utils.unregister_class(AddDeviceDialogProperties)
    bpy.utils.unregister_class(DeviceProperties)
