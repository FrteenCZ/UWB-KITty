import bpy  # type: ignore
from ..devices.manager import manager as DeviceManager
from .serial_modal import SERIAL_OT_StartESP
from ..devices.device import Device


class VIEW3D_PT_manager_panel(bpy.types.Operator):
    """Run DeviceManager update"""
    bl_idname = "wm.update_manager"
    bl_label = "Update DeviceManager"

    def execute(self, context):
        DeviceManager.update()
        self.report({'INFO'}, "DeviceManager updated")
        return {'FINISHED'}

class VIEW3D_PT_distance_sender_panel(bpy.types.Operator):
    """Send distances to ESP devices"""
    bl_idname = "wm.send_distances"
    bl_label = "Send Distances"

    def execute(self, context):
        targets = bpy.data.collections.get("DistanceTargets").objects
        DeviceManager.devices.get("default").send_distances(targets)
        self.report({'INFO'}, "Distances sent")
        return {'FINISHED'}

class VIEW3D_PT_add_device_panel(bpy.types.Operator):
    """Add a new device to DeviceManager"""
    bl_idname = "wm.add_device"
    bl_label = "Add Device"

    def execute(self, context):
        obj = bpy.data.objects.get("Cube")
        device = Device(device_id="default", blender_object=obj, role="TAG")
        DeviceManager.add_device(device=device, port=SERIAL_OT_StartESP._thread)
        self.report({'INFO'}, "Device added")
        return {'FINISHED'}

def register():
    bpy.utils.register_class(VIEW3D_PT_manager_panel)
    bpy.utils.register_class(VIEW3D_PT_distance_sender_panel)
    bpy.utils.register_class(VIEW3D_PT_add_device_panel)


def unregister():
    bpy.utils.unregister_class(VIEW3D_PT_manager_panel)
    bpy.utils.unregister_class(VIEW3D_PT_distance_sender_panel)
    bpy.utils.unregister_class(VIEW3D_PT_add_device_panel)
