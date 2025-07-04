import bpy
from ..utils.ESPcom import SerialThread

class SERIAL_OT_ModalESP(bpy.types.Operator):
    """Continuously read from ESP"""
    bl_idname = "wm.serial_modal_esp"
    bl_label = "ESP Serial Modal Operator"

    _timer = None
    _thread = None

    def modal(self, context, event):
        if event.type == 'TIMER':
            self._thread.send_trilateration(context)            

        if not self._thread.keep_running:
            self.cancel(context)
            return {'CANCELLED'}

        return {'PASS_THROUGH'}

    def execute(self, context):
        # Start serial thread
        self._thread = SerialThread(port='/dev/ttyUSB0', baudrate=115200)
        self._thread.start()

        wm = context.window_manager
        self._timer = wm.event_timer_add(0.5, window=context.window)
        wm.modal_handler_add(self)
        return {'RUNNING_MODAL'}

    def cancel(self, context):
        if self._thread:
            self._thread.stop()
            self._thread.join()
        wm = context.window_manager
        wm.event_timer_remove(self._timer)
        print("Modal operator stopped.")

def menu_func(self, context):
    self.layout.operator(SERIAL_OT_ModalESP.bl_idname)

def register():
    bpy.utils.register_class(SERIAL_OT_ModalESP)
    bpy.types.TOPBAR_MT_file.append(menu_func)

def unregister():
    bpy.utils.unregister_class(SERIAL_OT_ModalESP)
    bpy.types.TOPBAR_MT_file.remove(menu_func)