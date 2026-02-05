import bpy  # type: ignore
from ..globals import device_manager
from .serial_modal import SERIAL_OT_StartESP
from ..devices.device import Device
from . import tick_modal


class AddDeviceProperties(bpy.types.PropertyGroup):
    device_id: bpy.props.StringProperty(name="Device ID")  # type: ignore
    blender_object: bpy.props.PointerProperty(
        name="Blender Object",
        type=bpy.types.Object,
    )  # type: ignore
    role: bpy.props.EnumProperty(
        name="Role",
        items=[
            ("TAG", "Tag", "A device that is being tracked"),
            ("ANCHOR", "Anchor", "A fixed device used for reference"),
            ("NONE", "None", "No specific role"),
        ],
    )  # type: ignore


class WM_OT_add_device(bpy.types.Operator):
    """Add a new device to DeviceManager"""
    bl_idname = "wm.add_device"
    bl_label = "Add Device"

    def draw(self, context):
        layout = self.layout
        props = context.window_manager.add_device_props
        layout.prop(props, "device_id")
        layout.prop(props, "blender_object")
        layout.prop(props, "role")

    def invoke(self, context, event):
        return context.window_manager.invoke_props_dialog(self, width=400)

    def execute(self, context):
        props = context.window_manager.add_device_props
        if not props.device_id:
            self.report({'ERROR'}, "Device ID cannot be empty.")
            return {'CANCELLED'}

        if props.blender_object is None:
            self.report({'ERROR'}, "Blender Object must be selected.")
            return {'CANCELLED'}

        device = Device(
            device_id=props.device_id,
            blender_object=props.blender_object,
            role=props.role,
        )

        if not SERIAL_OT_StartESP._thread or not SERIAL_OT_StartESP._thread.is_alive():
            self.report(
                {'ERROR'}, "Serial port not running. Please start it first.")
            return {'CANCELLED'}

        device_manager.add_device(
            device=device, port=SERIAL_OT_StartESP._thread)

        if tick_modal._timer_handle is None:
            bpy.ops.wm.tick_start()

        self.report({'INFO'}, f"Device '{props.device_id}' added.")
        return {'FINISHED'}


def register():
    bpy.utils.register_class(AddDeviceProperties)
    bpy.types.WindowManager.add_device_props = bpy.props.PointerProperty(
        type=AddDeviceProperties)
    bpy.utils.register_class(WM_OT_add_device)


def unregister():
    bpy.utils.unregister_class(WM_OT_add_device)
    del bpy.types.WindowManager.add_device_props
    bpy.utils.unregister_class(AddDeviceProperties)
