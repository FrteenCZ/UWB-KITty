import bpy
from ..utils.ESPcom import SerialThread


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
            port=port,
            baudrate=115200)
        SERIAL_OT_StartESP._thread.start()
        SERIAL_OT_StartESP.running = True

        self.report({'INFO'}, f"Serial thread started on {port}")
        return {'FINISHED'}


class SERIAL_OT_StopESP(bpy.types.Operator):
    """Stop reading from ESP"""
    bl_idname = "wm.serial_stop_esp"
    bl_label = "Stop ESP Serial"

    def execute(self, context):
        # Stop the simulation. The tick_stop operator handles the check.
        bpy.ops.wm.tick_stop()

        if SERIAL_OT_StartESP._thread:
            SERIAL_OT_StartESP._thread.stop()
            SERIAL_OT_StartESP._thread.join()
            SERIAL_OT_StartESP.running = False
            self.report({'INFO'}, "Serial thread stopped")
        return {'FINISHED'}


def register():
    bpy.utils.register_class(SERIAL_OT_StartESP)
    bpy.utils.register_class(SERIAL_OT_StopESP)


def unregister():
    bpy.utils.unregister_class(SERIAL_OT_StartESP)
    bpy.utils.unregister_class(SERIAL_OT_StopESP)
