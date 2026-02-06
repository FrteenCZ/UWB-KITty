import bpy # type: ignore
import serial.tools.list_ports
from ..operators.serial_modal import SERIAL_OT_StartESP
from ..operators.tick_modal import _timer_handle

class UWB_UL_device_list(bpy.types.UIList):
    def draw_item(self, context, layout, data, item, icon, active_data, active_propname, index):
        if self.layout_type in {'DEFAULT', 'COMPACT'}:
            layout.label(text=item.id, icon='DECORATE_LINKED')
            layout.label(text=item.blender_object_name, icon='OBJECT_DATA')
            layout.label(text=item.role, icon='USER')
        elif self.layout_type in {'GRID'}:
            layout.alignment = 'CENTER'
            layout.label(text="", icon_value=icon)

class VIEW3D_PT_device_manager_panel(bpy.types.Panel):
    bl_label = "Device Manager"
    bl_space_type = 'VIEW_3D'
    bl_region_type = 'UI'
    bl_category = 'UWB-KITty'

    def draw(self, context):
        layout = self.layout
        scene = context.scene
        
        row = layout.row()
        row.template_list("UWB_UL_device_list", "", scene.uwb_kitty_props, "devices", scene.uwb_kitty_props, "active_device_index")
        
        col = row.column(align=True)
        col.operator("wm.add_device", icon='ADD', text="")
        col.operator("wm.remove_device", icon='REMOVE', text="").index = scene.uwb_kitty_props.active_device_index


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
    )


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
            if _timer_handle is None:
                layout.operator("wm.tick_start", text="Start Simulation")
            else:
                layout.operator("wm.tick_stop", text="Stop Simulation")
        else:
            layout.operator("wm.serial_start_esp", text="Connect", icon="PLAY")

def register():
    bpy.utils.register_class(UWB_UL_device_list)
    bpy.utils.register_class(VIEW3D_PT_device_manager_panel)
    bpy.utils.register_class(SerialProperties) # Register SerialProperties
    bpy.types.Scene.serial_props = bpy.props.PointerProperty(type=SerialProperties) # Attach to Scene
    bpy.utils.register_class(VIEW3D_PT_comunication_panel)

def unregister():
    bpy.utils.unregister_class(VIEW3D_PT_comunication_panel)
    bpy.utils.unregister_class(VIEW3D_PT_device_manager_panel)
    bpy.utils.unregister_class(UWB_UL_device_list)
    del bpy.types.Scene.serial_props # Unregister from Scene
    bpy.utils.unregister_class(SerialProperties) # Unregister SerialProperties
