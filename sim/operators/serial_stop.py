import bpy  # type: ignore
from .serial_modal import SERIAL_OT_StartESP


class SERIAL_OT_StopESP(bpy.types.Operator):
    """Stop reading from ESP"""
    bl_idname = "wm.serial_stop_esp"
    bl_label = "Stop ESP Serial"

    def execute(self, context):
        if SERIAL_OT_StartESP._thread:
            SERIAL_OT_StartESP._thread.stop()
            SERIAL_OT_StartESP._thread.join()
            SERIAL_OT_StartESP.running = False
            self.report({'INFO'}, "Serial thread stopped")
        return {'FINISHED'}


def register():
    bpy.utils.register_class(SERIAL_OT_StopESP)


def unregister():
    bpy.utils.unregister_class(SERIAL_OT_StopESP)
