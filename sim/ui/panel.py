import bpy  # type: ignore
import serial.tools.list_ports
from ..operators.serial_modal import SERIAL_OT_StartESP

# Panel for toggling object tracking in the 3D View


class VIEW3D_PT_tracking_panel(bpy.types.Panel):
    bl_label = "Object Tracker"
    bl_space_type = 'VIEW_3D'
    bl_region_type = 'UI'
    bl_category = 'UWB-KITty'

    def draw(self, context):
        obj = context.active_object
        if obj is not None:
            self.layout.label(text=f"Tracking: '{obj.name}'")
        else:
            self.layout.label(text="No object selected")

        self.layout.operator(
            "view3d.toggle_object_tracking", text="Toggle Tracking")


# This panel is for displaying distance measurements in the 3D view.
class VIEW3D_PT_distance_panel(bpy.types.Panel):
    bl_label = "Distance Display"
    bl_space_type = 'VIEW_3D'
    bl_region_type = 'UI'
    bl_category = 'UWB-KITty'

    def draw(self, context):
        self.layout.operator("view3d.toggle_distance_draw")


def get_serial_devices(self, context):
    ports = serial.tools.list_ports.comports()
    if not ports:
        return [("NONE", "No devices found", "No /dev/ttyUSB* devices")]
    return [(d.device, d.device, f"Serial device at {d.device}") for d in ports]


class SerialProperties(bpy.types.PropertyGroup):
    port: bpy.props.EnumProperty(
        name="Serial Port",
        description="Select ESP device to connect",
        items=get_serial_devices
    )  # type: ignore


class VIEW3D_PT_comunication_panel(bpy.types.Panel):
    bl_label = "Serial comunication"
    bl_idname = 'VIEW_PT_comunication_panel'
    bl_space_type = 'VIEW_3D'
    bl_region_type = 'UI'
    bl_category = 'UWB-KITty'

    def draw(self, context):
        layout = self.layout
        props = context.scene.serial_props

        layout.prop(props, "port")
        if SERIAL_OT_StartESP.running:
            layout.operator("wm.serial_stop_esp", text="Stop", icon="CANCEL")
        else:
            layout.operator("wm.serial_start_esp", text="Connect", icon="PLAY")


class SERIAL_PT_ObjectPanel(bpy.types.Panel):
    bl_label = "ESP Object Settings"
    bl_idname = "SERIAL_PT_object_settings"
    bl_space_type = 'PROPERTIES'
    bl_region_type = 'WINDOW'
    bl_context = "object"

    def draw(self, context):
        layout = self.layout
        obj = context.object

        if obj:
            layout.prop(obj.serial_props, "role", expand=True)


def register():
    bpy.utils.register_class(VIEW3D_PT_tracking_panel)
    bpy.utils.register_class(VIEW3D_PT_distance_panel)
    bpy.utils.register_class(VIEW3D_PT_comunication_panel)
    bpy.utils.register_class(SerialProperties)
    bpy.utils.register_class(SERIAL_PT_ObjectPanel)
    bpy.types.Scene.serial_props = bpy.props.PointerProperty(
        type=SerialProperties)


def unregister():
    bpy.utils.unregister_class(VIEW3D_PT_tracking_panel)
    bpy.utils.unregister_class(VIEW3D_PT_distance_panel)
    bpy.utils.unregister_class(VIEW3D_PT_comunication_panel)
    bpy.utils.unregister_class(SerialProperties)
    bpy.utils.unregister_class(SERIAL_PT_ObjectPanel)
    del bpy.types.Scene.serial_props
