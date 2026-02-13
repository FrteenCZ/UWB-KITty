import bpy
from ..globals import device_manager
from . import tick_modal


class WM_OT_add_device(bpy.types.Operator):
    """Add a new device to the persistent list"""
    bl_idname = "wm.add_device"
    bl_label = "Add Device"

    def draw(self, context):
        layout = self.layout
        props = context.scene.uwb_kitty_props.add_device_props
        layout.prop(props, "id")
        layout.prop(props, "blender_object")
        layout.prop(props, "role")

    def invoke(self, context, event):
        add_props = context.scene.uwb_kitty_props.add_device_props
        
        # Find the highest existing ID to suggest a new one
        max_id = -1
        for device in context.scene.uwb_kitty_props.devices:
            if device.id > max_id:
                max_id = device.id
        add_props.id = max_id + 1

        # Pre-fill with active object if available
        active_obj = context.active_object
        if active_obj:
            add_props.blender_object = active_obj
        
        return context.window_manager.invoke_props_dialog(self, width=400)

    def execute(self, context):
        scene_props = context.scene.uwb_kitty_props
        add_props = scene_props.add_device_props

        # Check for duplicate IDs
        for device in scene_props.devices:
            if device.id == add_props.id:
                self.report({'ERROR'}, f"Device ID {add_props.id} already exists.")
                return {'CANCELLED'}

        if not add_props.blender_object:
            self.report({'ERROR'}, "Blender Object must be selected.")
            return {'CANCELLED'}

        # Add the new device to the persistent collection
        new_device_prop = scene_props.devices.add()
        new_device_prop.id = add_props.id
        new_device_prop.blender_object_name = add_props.blender_object.name
        new_device_prop.role = add_props.role

        # Clear the dialog properties for the next use
        add_props.blender_object = None
        
        # Reload devices in the manager if it's running
        if tick_modal._timer_handle is not None:
             device_manager.load_devices_from_properties(context)

        self.report({'INFO'}, f"Device '{new_device_prop.id}' added to list.")
        return {'FINISHED'}


class WM_OT_remove_device(bpy.types.Operator):
    """Remove a device from the persistent list"""
    bl_idname = "wm.remove_device"
    bl_label = "Remove Device"

    index: bpy.props.IntProperty() # type: ignore

    def execute(self, context):
        scene_props = context.scene.uwb_kitty_props
        scene_props.devices.remove(self.index)
        
        # Reload devices in the manager if it's running
        if tick_modal._timer_handle is not None:
             device_manager.load_devices_from_properties(context)
             
        self.report({'INFO'}, "Device removed.")
        return {'FINISHED'}


def register():
    bpy.utils.register_class(WM_OT_add_device)
    bpy.utils.register_class(WM_OT_remove_device)


def unregister():
    bpy.utils.unregister_class(WM_OT_add_device)
    bpy.utils.unregister_class(WM_OT_remove_device)
