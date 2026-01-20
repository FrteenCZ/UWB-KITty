import bpy  # type: ignore
from ..utils.ESPcom import SerialThread
from ..comunication_protocol.protocol import parse_packet


class SERIAL_OT_StartESP(bpy.types.Operator):
    """Start reading from ESP serial port"""
    bl_idname = "wm.serial_start_esp"
    bl_label = "Start ESP Serial"

    _thread = None
    running = False

    def execute(self, context):
        port = context.scene.serial_props.port
        if port == "NONE":
            self.report({'ERROR'}, "No serial devices available")
            return {'CANCELLED'}

        SERIAL_OT_StartESP._thread = SerialThread(
            process_latest_line=parse_packet,
            port=port,
            baudrate=115200)
        SERIAL_OT_StartESP._thread.start()
        SERIAL_OT_StartESP.running = True

        self.report({'INFO'}, f"Serial thread started on {port}")
        return {'FINISHED'}


def register():
    bpy.utils.register_class(SERIAL_OT_StartESP)


def unregister():
    bpy.utils.unregister_class(SERIAL_OT_StartESP)
